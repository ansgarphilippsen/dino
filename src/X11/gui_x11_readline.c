#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
 
#include <unistd.h>

#include <termios.h>
#include <readline/readline.h>

#include "gui_terminal.h"
#include "dino.h"
#include "shell_raw.h"
#include "cl.h"

extern int debug_mode;

static struct termios tmode,tsavemode;

void guit_cbhandler(char* line)
{
  shellParseRaw(line,1);
}

// set up terminal
int guitInit()
{
  if(tcgetattr(STDIN_FILENO, &tmode) < 0) {
    perror("tcgetattr");
    exit(1);
  }

  memcpy(&tsavemode,&tmode,sizeof(struct termios));

  tmode.c_cc[VMIN]=0;
  tmode.c_cc[VTIME]=0;
  tmode.c_lflag &= ~ICANON;
  
  //tmode.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE | ECHOCTL);
  //tmode.c_cflag &= ~PARENB |  CS8;
  //tmode.c_iflag &= ~ICRNL | ~INPCK |~ISTRIP | ~IXON;

  if(tcsetattr(0,TCSANOW,&tmode)<0){
    perror("tcsetattr");
    exit(1);
  }

  rl_callback_handler_install("dino> ", guit_cbhandler);
  return 0;
}

void guitOutit()
{
  rl_callback_handler_remove();

  tcsetattr(0,TCSAFLUSH,&tsavemode);
}

void guitSuspend(int f)
{
  /*
  suspend=f;
  if(!suspend)
    guitWrite(prompt);
  */
}

void guitWrite(const char *s)
{
  write(fileno(stdout),s,clStrlen(s));
}

void guitAddChar(unsigned char c)
{
  rl_stuff_char(c);
  rl_callback_read_char();
}

void guitTimeProc()
{
  fd_set fds;
  struct timeval tv={0,1};
  
  FD_ZERO(&fds);
  FD_SET(fileno(stdin),&fds);

  if( select(FD_SETSIZE, &fds, NULL, NULL, &tv) >0) {
    rl_callback_read_char();
  }
}
