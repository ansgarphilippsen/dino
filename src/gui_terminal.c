#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h> /* for struct winsize */
#include <termios.h>   /* for struct termios */
#include <string.h>    /* for memset */

#include "gui_terminal.h"
#include "shell_raw.h"
#include "dino.h"
#include "cl.h"
#include "Cmalloc.h"

extern int debug_mode;

static struct winsize wsize;
static struct termios tmode,tsavemode;

static void finish(int n);
static void inter(int n);
static void sig_winch(int n);
static void insert_char(char c);
static void parse_esc(char *b);
static void parse_tmpbuf(int n, char *tmpbuf);
static void line_submit(void);
static void line_bs(void);
static void erase_line(void);
static void draw_line(void);
static void curs_move(int n);
static void line_reset(void);
static void get_history(int dir);

static struct INBUF {
  char *buf;
  int pos,count,max;
  int offset,maxlen;
} inbuf;

static struct CHARBUF {
  char *buf;
  int count,max;
}charbuf;

static char *prompt="dino> ";
static int suspend;

int guitInit()
{
  // set up the terminal
  debmsg("guit: setting signal handlers");

  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGHUP, finish);
  signal(SIGINT, inter);
  signal(SIGTERM, finish);

  signal(SIGWINCH, sig_winch);
  ioctl(fileno(stdin),TIOCGWINSZ,&wsize);
  inbuf.maxlen=wsize.ws_col-strlen(prompt)-1;

  debmsg("guit: setting terminal properties");

  if(tcgetattr(0,&tmode)<0){
    perror("tcgetattr");
    dinoExit(1);
  }

  tsavemode=tmode;
  tmode.c_cc[VMIN]=0;
  tmode.c_cc[VTIME]=0;
  tmode.c_lflag &= ~ICANON;
  tmode.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL);
  tmode.c_cflag &= ~PARENB |  CS8;
  tmode.c_iflag &= ~ICRNL | ~INPCK |~ISTRIP | ~IXON;

  if(tcsetattr(0,TCSAFLUSH,&tmode)<0){
    perror("tcsetattr");
    dinoExit(1);
  }

  debmsg("guit: settig up default values");
  charbuf.max=256;
  charbuf.buf=Cmalloc(charbuf.max);
  charbuf.count=0;

  inbuf.max=2048;
  inbuf.buf=Cmalloc(inbuf.max);

  suspend=0;

  line_reset();

  guitWrite(prompt);


  return 0;
}

void guitSuspend(int f)
{
  suspend=f;
  if(!suspend)
    guitWrite(prompt);
}

void guitWrite(const char *s)
{
  write(fileno(stdout),s,clStrlen(s));
}

void guitAddChar(unsigned char c)
{
  if(charbuf.count<charbuf.max)
    charbuf.buf[charbuf.count++]=c;
}


/*
  periodically called through the gui
*/


void guitTimeProc()
{
  int rc;
  unsigned char tmpbuf[64],c;

  if(!suspend) {
    // check for stdin input
    if((rc=read(fileno(stdin),tmpbuf,63))>0) {
      parse_tmpbuf(rc,tmpbuf);
    }
    
    // check charbuf
    if(charbuf.count>0) {
      parse_tmpbuf(charbuf.count,charbuf.buf);
      charbuf.count=0;
    }
  }
}

static void parse_tmpbuf(int rc, char *tmpbuf) 
{
  int i;
  unsigned char c;

  tmpbuf[rc]='\0';
  i=0;
  while(i<rc) {
    c=tmpbuf[i];
    if(!isprint(c)) {
      switch(c) {
      case 0x1B: /* ESC */
	i++;
	if(i<c) {
	  if(tmpbuf[i]=='[' || tmpbuf[i]=='O') {
	    i++;
	    parse_esc(tmpbuf+i);
	    i=rc;
	  } else {
	  }
	}
	break;
      case 0x9B: /* CSI */
	i++;
	parse_esc(tmpbuf+i);
	i=rc;
	break;
      case 0x0D: /* CR */
      case 0x0A: /* LF/NL */
	i++;
	line_submit();
	break;
      case 0x7f:
      case 0x08: /* BS */
	i++;
	line_bs();
	break;
      case 0x09: /* HT */
	// TODO command completion
	i++;
	break;
      case 0x0B: /* VT */
	i++;
	erase_line();
	line_reset();
	break;
      default:
	//	fprintf(stderr,"KEY %d\n",c);
	i++;
	break;
      }
    } else {
      insert_char(c);
      i++;
    }
  }
}

static void parse_esc(char *b)
{
  if(clStrcmp(b,"A")) {
    // up
    get_history(-1);
  } else if(clStrcmp(b,"B")) {
    // down
    get_history(1);
  } else if(clStrcmp(b,"D")) {
    // left
    curs_move(-1);
  } else if(clStrcmp(b,"C")) {
    // right
    curs_move(+1);
  } else if(b[0]==0xb2) {
    // DEL
  }
}

/*
  submit the finished line to the 
  raw prompt processing
*/

static void line_submit()
{
  write(fileno(stdout),"\n",1);
  inbuf.buf[inbuf.count]='\0';
  shellParseRaw(inbuf.buf,1);
  line_reset();
  if(!suspend)
    guitWrite(prompt);
}

/*
  command line manipulation
*/

static char cmd_buf[256];

static void line_reset(void)
{
  memset(inbuf.buf,0,inbuf.max);
  inbuf.pos=0;
  inbuf.offset=0;
  inbuf.count=0;
}

/*
  move cursor left or right
*/

static void curs_move(int n)
{
  if(inbuf.pos+n>=0 && inbuf.pos+n<=inbuf.count) {
    erase_line();
    inbuf.pos+=n;
    draw_line();
  }
}

/*
  insert char at current cursor position
*/

static void insert_char(char c)
{
  int i;

  erase_line();
  for(i=inbuf.count;i>inbuf.pos;i--) 
    inbuf.buf[i]=inbuf.buf[i-1];

  inbuf.buf[inbuf.pos]=c;
  inbuf.pos++;
  inbuf.count++;
  draw_line();
}

/*
  backspace (erase character left of cursor)
*/

static void line_bs()
{
  int i;
  if(inbuf.pos>0) {
    erase_line();
    for(i=inbuf.pos;i<inbuf.count;i++)
      inbuf.buf[i-1]=inbuf.buf[i];
    inbuf.buf[--inbuf.count]='\0';
    inbuf.pos--;
    draw_line();
  }
}

/*
  erase entire line
*/

static void erase_line(void)
{
  int j,i=0;

  for(j=0;j<inbuf.pos-inbuf.offset;j++)
    cmd_buf[i++]=0x08;
  for(j=0;j<inbuf.maxlen;j++)
    cmd_buf[i++]=0x20;
  for(j=0;j<inbuf.maxlen;j++)
    cmd_buf[i++]=0x08;

  write(fileno(stdout),cmd_buf,i);
}

/*
  draw entire line (assumes cursor is at start)
*/

static void draw_line(void)
{
  int ccount;

  ccount=inbuf.count;
  if(ccount>=inbuf.maxlen) {
    // complete line doesn't fit
    inbuf.offset=inbuf.pos-inbuf.maxlen;
    if(inbuf.offset<0) {
      inbuf.offset=0;
    }
    ccount=inbuf.maxlen;
  } else {
    inbuf.offset=0;
  }

  write(fileno(stdout),inbuf.buf+inbuf.offset,ccount);
  memset(cmd_buf,0x08,ccount-inbuf.pos+inbuf.offset);
  write(fileno(stdout),cmd_buf,ccount-inbuf.pos+inbuf.offset);
}

void guitOutit()
{
  tcsetattr(0,TCSAFLUSH,&tsavemode);
}

static void finish(int n)
{
  shellOut("\n");
  guitOutit();
  dinoExit(-1);
}

static void inter(int n)
{
#ifdef USE_CMI
  //cmiInterrupt();
  finish(0);
#else
  finish();
#endif
}

static void sig_winch(int n)
{
  ioctl(fileno(stdin),TIOCGWINSZ,&wsize);
  inbuf.maxlen=wsize.ws_col-strlen(prompt)-1;
  erase_line();
  draw_line();
}

static void get_history(int dir)
{
  const char *l;

  if((l=shellGetHistory(dir))!=NULL) {
    erase_line();
    clStrcpy(inbuf.buf,l);
    inbuf.count=clStrlen(l);
    inbuf.pos=inbuf.count;
    inbuf.offset=0;
    draw_line();
  }


}
