#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>

#include "autoplay.h"
#include "com.h"
#include "gui.h"
#include "shell.h"
#include "gfx.h"

extern struct SHELL shell;

extern struct GUI gui;
extern struct GFX gfx;

#define AUTOPLAY_IDLE0 500
#define AUTOPLAY_IDLE1 50
#define AUTOPLAY_IDLE2 3
#define AUTOPLAY_IDLE3 3


struct AUTOPLAY ap;

int apInit()
{
  if(apParse()!=0) {
    return -1;
  }

  strcpy(ap.help,"\nINFO\n");

  ap.rotx2=(float)rnd2();
  ap.roty2=(float)rnd2();
  ap.rotz2=(float)rnd2();

  ap.language=AP_GERMAN;

  ap.help_on=0;

  apIdleReset();
  
  return 0;
}

static void apCheckName(char *n)
{
  int i,l;

  if(n==NULL) return;

  l=strlen(n);

  for(i=0;i<l;i++)
    if(n[i]=='_') n[i]=' ';
}

int apParse(void)
{
  char *buf,*p;
  struct stat sb;
  char *sep=" \t\n";
  int entryc,itemc,flag;
  int i;
  char iname[4][8]={"OFF","ATM","TRC","SRF"};

  fstat(fileno(ap.f),&sb);

  buf=malloc(sb.st_size+128);

  fread(buf,sizeof(char),sb.st_size,ap.f);

  strcat(buf," END");

  p=strtok(buf,sep);

  entryc=0;
  while(strcmp(p,"END")) {
    if(p[0]=='#') {
      /*
	comment
      */
      p=strtok(NULL,"\n");
    } else {
      if(!strcmp(p,"entry")) {
	/*
	  entry
	*/
	flag=0;
	itemc=0;
	p=strtok(NULL,sep);
	if(strcmp(p,"{")) {
	  fprintf(stderr,"parse error: expected '{' after 'entry'\n");
	  return -1;
	}
	while(strcmp(p,"}")) {
	  p=strtok(NULL,sep);
	  if(p==NULL) {
	    fprintf(stderr,"parse error: unexpected EOF encountered\n");
	  } else if(!strcmp(p,"name")) {
	    /* name */
	    strcpy(ap.e[entryc].name1,strtok(NULL,sep));
	    strcpy(ap.e[entryc].name2,strtok(NULL,sep));
	    apCheckName(ap.e[entryc].name1);
	    apCheckName(ap.e[entryc].name2);
	    flag|=0x1;
	  } else if(!strcmp(p,"on_load")) {
	    /* on_load */
	    strcpy(ap.e[entryc].on_load,strtok(NULL,sep));
	    flag|=0x2;
	  } else if(!strcmp(p,"on_activate")) {
	    /* on_activate */
	    strcpy(ap.e[entryc].on_activate,strtok(NULL,sep));
	    flag|=0x4;
	  } else if(!strcmp(p,"info")) {
	    /* info */
	    strcpy(ap.e[entryc].inf1,strtok(NULL,sep));
	    strcpy(ap.e[entryc].inf2,strtok(NULL,sep));
	    flag|=0x8;
	  } else if(!strcmp(p,"dir")) {
	    /* dir */
	    strcpy(ap.e[entryc].dir,strtok(NULL,sep));
	    flag|=0x10;
	  } else if(!strcmp(p,"item")) {
	    /* item */
	    strcpy(ap.e[entryc].b[itemc].name1,strtok(NULL,sep));
	    strcpy(ap.e[entryc].b[itemc].name2,strtok(NULL,sep));
	    apCheckName(ap.e[entryc].b[itemc].name1);
	    apCheckName(ap.e[entryc].b[itemc].name2);
	    ap.e[entryc].b[itemc].subc=atoi(strtok(NULL,sep));
	    for(i=0;i<4;i++) {
	      if(ap.e[entryc].b[itemc].subc & (1<<i)) {
		strcpy(ap.e[entryc].b[itemc].sub[i].scr,strtok(NULL,sep));
		strcpy(ap.e[entryc].b[itemc].sub[i].name,iname[i]);
		ap.e[entryc].b[itemc].sub[i].flag=1;
	      } else {
		ap.e[entryc].b[itemc].sub[i].flag=0;
	      }
	    }
	    ap.e[entryc].b[itemc].subi=0;
	    strcpy(ap.e[entryc].b[itemc].inf1,strtok(NULL,sep));
	    strcpy(ap.e[entryc].b[itemc].inf2,strtok(NULL,sep));
	    itemc++;
	  } else if(!strcmp(p,"}")) {
	    break;
	  } else {
	    fprintf(stderr,"parse error: unknown entry command %s\n",p);
	    return -1;
	  }
	}
	/*
	  end of entry 
	*/
	if(! (flag&0x10)) {
	  strcpy(ap.e[entryc].dir,".");
	}
	if((flag&0xf)!=0xf) {
	  fprintf(stderr,"parse error: incomplete entry\n");
	  return -1;
	}
	if(itemc==0) {
	  fprintf(stderr,"parse error: no item for entry\n");
	  return -1;
	}
	ap.e[entryc].bc=itemc;
	entryc++;
      }

    }
    p=strtok(NULL,sep);
  }

  ap.ec=entryc;

  free(buf);

  return 0;
}

int apPrep()
{
  int i,j,k;
  char sdir[256];
  FILE *f;
  struct stat sb;
  char com[1024],*buf;

  for(i=0;i<ap.ec;i++) {
    getcwd(sdir,255);
    if(chdir(ap.e[i].dir)!=0) {
      fprintf(stderr,"prep error: could not chdir to %s\n",ap.e[i].dir);
      return -1;
    }
    if(stat(ap.e[i].on_load,&sb)!=0) {
      fprintf(stderr,"prep error: %s: %s\n",ap.e[i].on_load, strerror(errno));
      return -1;
    }
    strcpy(com,"@");
    strcat(com,ap.e[i].on_load);
    shell.greedy=1;
    comRawCommand(com);
    shellWork();
    shell.greedy=0;
    if(stat(ap.e[i].on_activate,&sb)!=0) {
      fprintf(stderr,"prep error: %s: %s\n",ap.e[i].on_activate, strerror(errno));
      return -1;
    }
    
    // check info1
    if(stat(ap.e[i].inf1,&sb)!=0) {
      fprintf(stderr,"prep error: %s: %s\n",ap.e[i].inf1, strerror(errno));
      return -1;
    } else {
      // load info into string 
      buf=ap.e[i].help1;
      memset(buf,0,1024);
      f=fopen(ap.e[i].inf1,"r");
      if(sb.st_size>1023)
	fread(buf,sizeof(char),1023,f);
      else
	fread(buf,sizeof(char),sb.st_size,f);
      for(k=0;k<1024;k++)
	if(isspace(buf[k])) buf[k]=' ';
    }

    // check info2
    if(stat(ap.e[i].inf2,&sb)!=0) {
      fprintf(stderr,"prep error: %s: %s\n",ap.e[i].inf2, strerror(errno));
      return -1;
    } else {
      // load info into string 
      buf=ap.e[i].help2;
      memset(buf,0,1024);
      f=fopen(ap.e[i].inf2,"r");
      if(sb.st_size>1023)
	fread(buf,sizeof(char),1023,f);
      else
	fread(buf,sizeof(char),sb.st_size,f);
      for(k=0;k<1024;k++)
	if(isspace(buf[k])) buf[k]=' ';
    }

    
    for(j=0;j<ap.e[i].bc;j++) {
      for(k=0;k<4;k++) {
	if(ap.e[i].b[j].sub[k].flag)
	  if(stat(ap.e[i].b[j].sub[k].scr,&sb)!=0) {
	    fprintf(stderr,"prep error: %s: %s\n",ap.e[i].b[j].sub[k].scr, strerror(errno));
	    return -1;
	  }
      }
      // check info 1
      if(stat(ap.e[i].b[j].inf1,&sb)!=0) {
	fprintf(stderr,"prep error: %s: %s\n",ap.e[i].b[j].inf1, strerror(errno));
	return -1;
      } else {
	// load info into string 
	memset(ap.e[i].b[j].help1,0,1024);
	f=fopen(ap.e[i].b[j].inf1,"r");
	buf=ap.e[i].b[j].help1;
	if(sb.st_size>1023)
	  fread(buf,sizeof(char),1023,f);
	else
	  fread(buf,sizeof(char),sb.st_size,f);
	for(k=0;k<1024;k++)
	  if(isspace(buf[k])) buf[k]=' ';
      }
      // check info 2
      if(stat(ap.e[i].b[j].inf2,&sb)!=0) {
	fprintf(stderr,"prep error: %s: %s\n",ap.e[i].b[j].inf2, strerror(errno));
	return -1;
      } else {
	// load info into string 
	memset(ap.e[i].b[j].help2,0,1024);
	f=fopen(ap.e[i].b[j].inf2,"r");
	buf=ap.e[i].b[j].help2;
	if(sb.st_size>1023)
	  fread(buf,sizeof(char),1023,f);
	else
	  fread(buf,sizeof(char),sb.st_size,f);
	for(k=0;k<1024;k++)
	  if(isspace(buf[k])) buf[k]=' ';
      }
      
    }
    
    chdir(sdir);
    
  }

  ap.ce=NULL;

  apEntryActivate(ap.e+0);

  /*
  apItemActivate(ap.e+0,0,0);
  */

  return 0;
}

int apMenuInit()
{
  int i,j;
  Arg arg[20];
  int argc;
  XmString xms;
  XFontStruct *xfs1,*xfs2,*xfs3;
  XmFontListEntry xfe1,xfe2,xfe3;
  XmFontList xfl1,xfl2,xfl3;
  Dimension f_w,f_h;

  char *font_name1="-*-times-medium-r-normal-*-24-*-75-75-*-*-*-*";
  char *font_name2="-*-times-medium-r-normal-*-18-*-75-75-*-*-*-*";

  xfs1=XLoadQueryFont(gui.dpy,font_name1);
  xfs2=XLoadQueryFont(gui.dpy,font_name2);

  if(xfs1!=0) {
    xfe1=XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG,XmFONT_IS_FONT,xfs1);
    xfl1=XmFontListAppendEntry(NULL,xfe1);
  } else {
    fprintf(stderr,"\nfont '%s' not found",font_name1);
    exit(1);
  }

  if(xfs2!=0) {
    xfe2=XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG,XmFONT_IS_FONT,xfs2);
    xfl2=XmFontListAppendEntry(NULL,xfe2);
  } else {
    fprintf(stderr,"\nfont '%s' not found",font_name2);
    exit(1);
  }


  /* top level frame */
  ap.frame=XmCreateFrame(gui.form, "frame2", NULL, 0);
  XtVaSetValues(ap.frame,
                XmNrightAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_WIDGET,
		XmNleftWidget, gui.frame,
                XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget,gui.mform,
                NULL);
  XtManageChild(ap.frame);

  XtSetArg(arg[0],XmNwidth,&f_w);
  XtSetArg(arg[1],XmNheight,&f_h);
  XtGetValues(ap.frame,arg,2);
  
  XtSetArg(arg[0],XmNinitialFocus,gui.glxwindow);
  XtSetArg(arg[1],XmNtraversalOn,False);
  XtSetArg(arg[2],XmNwidth,f_w);
  XtSetArg(arg[3],XmNheight,f_h);
  XtSetArg(arg[4],XmNx,0);
  XtSetArg(arg[5],XmNy,0);
  XtSetArg(arg[6],XmNborderWidth,0);

  ap.area=XtCreateManagedWidget("area",xmDrawingAreaWidgetClass,
				ap.frame,arg,7);


  XtAddCallback(ap.area, XmNinputCallback, apInputCallback, 0);
  XtAddCallback(ap.area, XmNexposeCallback, apExposeCallback, 0);

  apCreateHelp();

  return 0;
}

void apEntryActivate(struct AP_ENTRY *e)
{
  char buf[1024];
  char cwd[256];
  int i;

  if(e==NULL)
    return;

  if(e==ap.ce)
    return;

  if(ap.ce!=NULL)
    ap.ce->active=0;
  e->active=1;
  ap.ce=e;

  getcwd(cwd,255);
  
  chdir(e->dir);

  strcpy(buf,"@");
  strcat(buf,e->on_activate);
  shell.greedy=1;
  comRawCommand(".*.* hide");
  comRawCommand(buf);
  shellWork();
  shell.greedy=0;

  chdir(cwd);

  for(i=0;i<e->bc;i++) {
    if(e->b[i].sub[2].flag) {
      e->b[i].subi=2;
      apItemActivate(e,i,2);
    } else if(e->b[i].sub[1].flag) {
      e->b[i].subi=1;
      apItemActivate(e,i,1);
    } else {
      e->b[i].subi=0;
      apItemActivate(e,i,0);
    }
  }

  XClearArea(gui.dpy,XtWindow(ap.area),0,0,1,1,True);

}

void apItemActivate(struct AP_ENTRY *e,int n,int m)
{
  char buf[1024];
  char cwd[256];
  int i;

  /* implement radio button behaviour here */

  e->b[n].subi=m;

  getcwd(cwd,255);

  chdir(e->dir);

  strcpy(buf,"@");
  strcat(buf,e->b[n].sub[m].scr);

  shell.greedy=1;
  comRawCommand(buf);
  shellWork();
  shell.greedy=0;

  chdir(cwd);

  /* force expose event */
  XClearArea(gui.dpy,XtWindow(ap.area),0,0,1,1,True);


}

void apIdleReset(void)
{
  ap.idle0=0;
  ap.idle1=0;
  ap.idle2=0;
  ap.idle3=0;
}

void apIdle(void)
{
  int newi,j;
  ap.idle0++;

  if(AUTOPLAY_IDLE0<0)
    return;

  if(ap.idle0>AUTOPLAY_IDLE0)
    ap.idle1++;
  
  if(ap.idle1>0) {
    if(ap.idle1>AUTOPLAY_IDLE1+1) {
      ap.idle1=1;
      ap.idle2++;
      ap.rotx=ap.rotx2;
      ap.roty=ap.roty2;
      ap.rotz=ap.rotz2;
      ap.rotx2=(float)rnd2();
      ap.roty2=(float)rnd2();
      ap.rotz2=(float)rnd2();
      ap.rotxd=(ap.rotx2-ap.rotx)/(float)AUTOPLAY_IDLE1;
      ap.rotyd=(ap.roty2-ap.roty)/(float)AUTOPLAY_IDLE1;
      ap.rotzd=(ap.rotz2-ap.rotz)/(float)AUTOPLAY_IDLE1;
    }
    if(ap.idle2>AUTOPLAY_IDLE2) {
      /* change items */
      for(j=0;j<ap.ce->bc;j++) {
	newi=1+(int)(rnd()*3.0);
	if(ap.ce->b[j].sub[newi].flag) {
	  ap.ce->b[j].subi=newi;
	  apItemActivate(ap.ce,j,newi);
	}
	  
	  
      }

      ap.idle2=1;
      ap.idle3++;
    }
    if(ap.idle3>AUTOPLAY_IDLE3) {
      /* change entry */
      if(ap.ec>1) {
	do {
	  newi=(int)(rnd()*(float)(ap.ec));
	} while(&ap.e[newi]==ap.ce);
	apEntryActivate(&ap.e[newi]);
      }      
      ap.idle3=1;
    }
    if(ap.idle2>0) {
      ap.rotx+=ap.rotxd;
      ap.roty+=ap.rotyd;
      ap.rotz+=ap.rotzd;
      transCommand(&gfx.transform,TRANS_ROTX,-1,ap.rotx);
      transCommand(&gfx.transform,TRANS_ROTY,-1,ap.roty);
      transCommand(&gfx.transform,TRANS_ROTZ,-1,ap.rotz);
      comRedraw();
    }
  }
}

void apInputCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  int i,k;
  int xp,yp;
  int hit;
  struct AP_BUTTON *bp;
  XWindowChanges xwc;
  XmDrawingAreaCallbackStruct *cb=( XmDrawingAreaCallbackStruct *)call;

  if(cb->event!=NULL) {
    switch(cb->event->type) {
    case KeyPress: break;
    case KeyRelease: break;
    case ButtonPress:
      break;
    case ButtonRelease:
      /* check the entries first */
      xp=cb->event->xbutton.x;
      yp=cb->event->xbutton.y;
      hit=-1;
      for(i=0;i<ap.ec;i++) {
	if(xp>ap.e[i].x1 && xp<ap.e[i].x2 &&
	   yp>ap.e[i].y1 && yp<ap.e[i].y2) {
	  hit=i; 
	  break;
	}
      }
      if(hit>-1) {
	apEntryActivate(ap.e+hit);
	/*
	apItemActivate(ap.e+hit,0);
	*/
	break;
      }

      /* check for a list hit */
      for(i=0;i<ap.ce->bc;i++) {
	if(xp>ap.ce->b[i].x1 && xp<ap.ce->b[i].x2 &&
	   yp>ap.ce->b[i].y1 && yp<ap.ce->b[i].y2) {
	  hit=i;
	  break;
	}
      }
      if(hit>-1) {
	bp=&ap.ce->b[hit];
	/* search the subbuttons */
	for(k=0;k<4;k++) {
	  if(bp->sub[k].flag)
	    if(xp>bp->sub[k].x1 && xp<bp->sub[k].x2 &&
	       yp>bp->sub[k].y1 && yp<bp->sub[k].y2) {
	      apItemActivate(ap.ce,hit,k);
	      break;
	    }
	}
	break;
      }

      // check help button 
      if(xp>ap.helpb.x1 && xp<ap.helpb.x2 &&
	 yp>ap.helpb.y1 && yp<ap.helpb.y2) {
	xwc.x=ap.helpcx;
	xwc.y=ap.helpcy;
	xwc.width=ap.helpcw;
	xwc.height=ap.helpch;
	XMapRaised(gui.dpy,ap.helpw);
	ap.help_on=1;
	/*
	XReconfigureWMWindow(gui.dpy,ap.helpw,DefaultScreen(gui.dpy),
			     CWX | CWY  | CWWidth | CWHeight, &xwc);
	*/
      }

      if(xp>ap.langb.x1 && xp<ap.langb.x2 &&
	 yp>ap.langb.y1 && yp<ap.langb.y2) {
	if(ap.language==AP_GERMAN) {
	  ap.language=AP_ENGLISH;
	} else {
	  ap.language=AP_GERMAN;
	}
	XClearArea(gui.dpy,XtWindow(ap.area),0,0,1,1,True);
	if(ap.help_on) {
	  XClearArea(gui.dpy,ap.helpw,0,0,1,1,True);
	}
      }
	 

      break;
    case MotionNotify:
      break;

    }
    ap.idle2=1;
  }
}

void apExposeCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  XTextItem ti;
  static XFontStruct *fs1=NULL, *fs2=NULL, *fs3=NULL, *fs4=NULL, *fs5=NULL;
  static int init = 1;
  XGCValues xgcv;
  XColor color;
  Colormap cm=DefaultColormap(gui.dpy,DefaultScreen(gui.dpy));
  Window win=XtWindow(ww);
  Display *dpy=gui.dpy;

  static Window root;
  static unsigned int x,y,width,height,bw,depth;
  static unsigned int offset_x,offset_y;
  static unsigned int entry_w,entry_h;
  static unsigned int spacing1,spacing2;
  static unsigned int list_w;
  unsigned int list_h;
  static unsigned int button_w,button_h;
  static unsigned int info_w,info_h;
  static unsigned int sub_w,sub_h;
  unsigned int total_y;
  unsigned int fx,fy,fx2,fy2;

  struct AP_BUTTON *bp;

  int entry_c,button_c,k;

  ti.delta=0;
  
  if(init) {
    xgcv.foreground=WhitePixel(dpy,DefaultScreen(dpy));
    xgcv.background=BlackPixel(dpy,DefaultScreen(dpy));

    if(XParseColor(dpy,cm,"#efef4f",&color)!=0) {
      if(XAllocColor(dpy,cm,&color)!=0)
	xgcv.foreground=color.pixel;
    }
    if(XParseColor(dpy,cm,"#1f001f",&color)!=0) {
      if(XAllocColor(dpy,cm,&color)!=0)
	xgcv.background=color.pixel;
    }


    ap.gc1=XCreateGC(dpy,win,
		     GCForeground | GCBackground,
		     &xgcv);

    xgcv.background=WhitePixel(dpy,DefaultScreen(dpy));
    xgcv.foreground=BlackPixel(dpy,DefaultScreen(dpy));
    ap.gc2=XCreateGC(dpy,win,
		     GCForeground | GCBackground,
		     &xgcv);

    xgcv.foreground=WhitePixel(dpy,DefaultScreen(dpy));
    xgcv.background=BlackPixel(dpy,DefaultScreen(dpy));

    if(XParseColor(dpy,cm,"#001f4f",&color)!=0) {
      if(XAllocColor(dpy,cm,&color)!=0)
	xgcv.foreground=color.pixel;
    }
    if(XParseColor(dpy,cm,"white",&color)!=0) {
      if(XAllocColor(dpy,cm,&color)!=0)
	xgcv.background=color.pixel;
    }
      
    ap.gc3=XCreateGC(dpy,win,
		     GCForeground | GCBackground,
		     &xgcv);

    xgcv.foreground=WhitePixel(dpy,DefaultScreen(dpy));
    xgcv.background=BlackPixel(dpy,DefaultScreen(dpy));
    ap.gc4=XCreateGC(dpy,win,
		     GCForeground | GCBackground,
		     &xgcv);

    fs1=XLoadQueryFont(dpy,
		       "-*-times-medium-r-normal-*-12-*-75-75-*-*-*-*");
    if(fs1==NULL) {
      fprintf(stderr,"\nerror loading font");
      exit(1);
    }

    ap.fs1=fs1;

    fs2=XLoadQueryFont(dpy,
		       "-*-times-medium-r-normal-*-18-*-75-75-*-*-*-*");
    if(fs2==NULL) {
      fprintf(stderr,"\nerror loading font");
      exit(1);
    }

    ap.fs2=fs2;

    fs3=XLoadQueryFont(dpy,
		       "-*-times-medium-r-normal-*-24-*-75-75-*-*-*-*");
    if(fs3==NULL) {
      fprintf(stderr,"\nerror loading font");
      exit(1);
    }

    ap.fs3=fs3;

    fs4=XLoadQueryFont(dpy,
		       "-*-times-bold-r-normal-*-12-*-75-75-*-*-*-*");
    if(fs4==NULL) {
      fprintf(stderr,"\nerror loading font");
      exit(1);
    }

    ap.fs4=fs4;

    fs5=XLoadQueryFont(dpy,
		       "-*-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*");
    if(fs5==NULL) {
      fprintf(stderr,"\nerror loading font");
      exit(1);
    }

    ap.fs5=fs5;
    
    /* get the geometry of the drawing area */
    XGetGeometry(dpy,win,&root,&x,&y,&width,&height,&bw,&depth);

    /* set some values before beginning to draw */
    offset_x=10; 
    offset_y=20;
    entry_w=width-(2*offset_x);
    entry_h=28; 
    spacing1=10;
    spacing2=20;
    button_w=entry_w;
    button_h=48;
    sub_w=entry_w/4-6;
    sub_h=button_h/2-4;
    list_w=button_w;
    info_w=entry_w;
    info_h=20*20;
    ap.helpb.x1=offset_x;
    ap.helpb.w=info_w/2-offset_x;
    ap.helpb.h=32;
    ap.helpb.x2=ap.helpb.x1+ap.helpb.w;
    
    ap.langb.x1=2*offset_x+ap.helpb.w;
    ap.langb.w=ap.helpb.w;
    ap.langb.h=ap.helpb.h;
    ap.langb.x2=ap.langb.x1+ap.langb.w;
    
    init=0;
    
  } /* end static */



  /* prepare to draw by clearing the window */
  //  XClearWindow(dpy,win);
  XFillRectangle(dpy,win,ap.gc2,0,0,width,height);

  /* start the drawing */
  total_y=offset_y;


  /* help button */

  ap.helpb.y1=total_y;
  ap.helpb.y2=ap.helpb.y1+ap.helpb.h;

  XFillRectangle(dpy,win,ap.gc3,
		 ap.helpb.x1,ap.helpb.y1,
		 ap.helpb.w,ap.helpb.h);
  XDrawRectangle(dpy,win,ap.gc1,
		 ap.helpb.x1,ap.helpb.y1,
		 ap.helpb.w,ap.helpb.h);
  

  if(ap.language==AP_GERMAN) {
    ti.chars="Hilfe";
    ti.nchars=strlen("Hilfe");
    ti.font=fs2->fid;
    fx=(ap.helpb.w-XTextWidth(fs2,"Hilfe",strlen("Hilfe")))/2;
  } else {
    ti.chars="Help";
    ti.nchars=strlen("Help");
    ti.font=fs2->fid;
    fx=(ap.helpb.w-XTextWidth(fs2,"Help",strlen("Help")))/2;
  }  

  XDrawText(dpy,win,ap.gc4,ap.helpb.x1+fx,ap.helpb.y1+22,&ti,1);

  // language button
  ap.langb.y1=total_y;
  ap.langb.y2=ap.langb.y1+ap.langb.h;

  XFillRectangle(dpy,win,ap.gc3,
		 ap.langb.x1,ap.langb.y1,
		 ap.langb.w,ap.langb.h);
  XDrawRectangle(dpy,win,ap.gc1,
		 ap.langb.x1,ap.langb.y1,
		 ap.langb.w,ap.langb.h);
  

  if(ap.language==AP_GERMAN) {
    ti.chars="English";
    ti.nchars=strlen("English");
    ti.font=fs2->fid;
    fx=(ap.langb.w-XTextWidth(fs2,"English",strlen("English")))/2;
  } else {
    ti.chars="Deutsch";
    ti.nchars=strlen("Deutsch");
    ti.font=fs2->fid;
    fx=(ap.langb.w-XTextWidth(fs2,"Deutsch",strlen("Deutsch")))/2;
  }  

  XDrawText(dpy,win,ap.gc4,ap.langb.x1+fx,ap.langb.y1+22,&ti,1);

  total_y+=ap.helpb.h+spacing2;

  XDrawRectangle(dpy,win,ap.gc4,0,total_y,width,0);

  total_y+=spacing1;

  if(ap.language==AP_GERMAN) {
    ti.chars="Struktur:";
    ti.nchars=strlen("Struktur:");
    ti.font=fs2->fid;
  } else {
    ti.chars="Structure:";
    ti.nchars=strlen("Structure:");
    ti.font=fs2->fid;
  }

  XDrawText(dpy,win,ap.gc4,offset_x,total_y+20,&ti,1);

  total_y+=spacing1+20;

  for(entry_c=0;entry_c<ap.ec;entry_c++) {
    ap.e[entry_c].x1=offset_x;
    ap.e[entry_c].y1=total_y;
    ap.e[entry_c].w=entry_w;
    ap.e[entry_c].h=entry_h;
    ap.e[entry_c].x2=entry_w+offset_x;
    ap.e[entry_c].y2=entry_h+total_y;

    if(ap.e[entry_c].active) {
      XFillRectangle(dpy,win,ap.gc1,
		     ap.e[entry_c].x1,ap.e[entry_c].y1,
		     ap.e[entry_c].w,ap.e[entry_c].h);
      XDrawRectangle(dpy,win,ap.gc3,
		     ap.e[entry_c].x1,ap.e[entry_c].y1,
		     ap.e[entry_c].w,ap.e[entry_c].h);
    } else {
      XFillRectangle(dpy,win,ap.gc3,
		     ap.e[entry_c].x1,ap.e[entry_c].y1,
		     ap.e[entry_c].w,ap.e[entry_c].h);
      XDrawRectangle(dpy,win,ap.gc1,
		     ap.e[entry_c].x1,ap.e[entry_c].y1,
		     ap.e[entry_c].w,ap.e[entry_c].h);
    }

    if(ap.language==AP_GERMAN) {
      ti.chars=ap.e[entry_c].name1;
      ti.nchars=strlen(ap.e[entry_c].name1);
      ti.font=fs2->fid;
    } else {
      ti.chars=ap.e[entry_c].name2;
      ti.nchars=strlen(ap.e[entry_c].name2);
      ti.font=fs2->fid;
    }

    //    fx=offset_x+(entry_w-XTextWidth(fs2,ti.chars,ti.nchars))/2;
    fx=offset_x+5;
    fy=total_y+20;
    
    if(ap.e[entry_c].active)
      XDrawText(dpy,win,ap.gc2,fx,fy,&ti,1);
    else
      XDrawText(dpy,win,ap.gc1,fx,fy,&ti,1);

    total_y+=entry_h+spacing1;

  }

  total_y+=spacing2;

  /*
    now draw the button list of the active entry
    the list has a fixed height and a different background
  */

  /* title */
  if(ap.language==AP_GERMAN) {
    ti.chars="Objekte von ";
    ti.nchars=strlen("Objekte von ");
    ti.font=fs2->fid;
  } else {
    ti.chars="Objects of";
    ti.nchars=strlen("Objects of");
    ti.font=fs2->fid;
  }
  XDrawText(dpy,win,ap.gc4,offset_x,total_y+20,&ti,1);

  total_y+=24;

  if(ap.language==AP_GERMAN) {
    ti.chars=ap.ce->name1;
    ti.nchars=strlen(ap.ce->name1);
    ti.font=fs2->fid;
  } else {
    ti.chars=ap.ce->name2;
    ti.nchars=strlen(ap.ce->name2);
    ti.font=fs2->fid;
  }

  XDrawText(dpy,win,ap.gc1, offset_x,total_y+20,&ti,1);

  total_y+=spacing1+24;

  list_h=button_h*ap.ce->bc;

  XFillRectangle(dpy,win,ap.gc3,offset_x,total_y,list_w,list_h+4);
  XDrawRectangle(dpy,win,ap.gc1,offset_x,total_y,list_w,list_h+4);


  for(button_c=0;button_c<ap.ce->bc;button_c++) {
    bp=&ap.ce->b[button_c];
    bp->x1=offset_x+2;
    bp->y1=(button_c*button_h)+total_y+2;
    bp->w=button_w-4;
    bp->h=button_h-4;
    bp->x2=bp->x1+bp->w;
    bp->y2=bp->y1+bp->h;

    if(ap.language==AP_GERMAN) {
      ti.chars=bp->name1;
      ti.nchars=strlen(bp->name1);
      ti.font=fs2->fid;
    } else {
      ti.chars=bp->name2;
      ti.nchars=strlen(bp->name2);
      ti.font=fs2->fid;
    }

    fx=bp->x1+5;
    fy=bp->y2-3;
    
    XDrawText(dpy,win,ap.gc1,fx,fy-sub_h,&ti,1);
  

    fx=bp->x1+3;
    fy=bp->y2-sub_h;
    for(k=0;k<4;k++) {
      if(bp->sub[k].flag) {
	bp->sub[k].x1=fx;
	bp->sub[k].y1=fy;
	bp->sub[k].w=sub_w;
	bp->sub[k].h=sub_h;
	bp->sub[k].x2= bp->sub[k].x1+bp->sub[k].w;
	bp->sub[k].y2= bp->sub[k].y1+bp->sub[k].h;
	
	
	ti.chars=bp->sub[k].name;
	ti.nchars=strlen(bp->sub[k].name);
	ti.font=fs4->fid;
	
	fx2=(sub_w-XTextWidth(fs4,ti.chars,ti.nchars))/2;
	
	if(bp->subi==k) {
	  XFillRectangle(dpy,win,ap.gc1,
			 bp->sub[k].x1,bp->sub[k].y1,
			 bp->sub[k].w,bp->sub[k].h);
	  XDrawText(dpy,win,ap.gc2,fx+fx2,fy+sub_h-5,&ti,1);
	} else {
	  XDrawRectangle(dpy,win,ap.gc1,
			 bp->sub[k].x1,bp->sub[k].y1,
			 bp->sub[k].w,bp->sub[k].h);
	  XDrawText(dpy,win,ap.gc1,fx+fx2,fy+sub_h-5,&ti,1);
	}
      }
      fx+=sub_w+4;
    }

    
  }

  total_y+=list_h+spacing2;
  XDrawRectangle(dpy,win,ap.gc4,0,total_y,width,0);

  /* info window */
  //  XFillRectangle(dpy,win,ap.gc2,offset_x,total_y,info_w,info_h);
  //  XDrawRectangle(dpy,win,ap.gc4,offset_x,total_y,info_w,info_h);

  if(ap.language==AP_GERMAN) {
    fy=apPutInfo(offset_x-2,total_y+5,info_w+2,info_h,ap.ce->help1,
		 fs5,dpy,win,ap.gc4,18);
  } else {
    fy=apPutInfo(offset_x,total_y+5,info_w,info_h,ap.ce->help2,
		 fs5,dpy,win,ap.gc4,18);
  }

  total_y+=info_h+spacing2;
}

int apPutInfo(unsigned int x, unsigned int y, unsigned int w, unsigned int h, char *buf2, XFontStruct *fs, Display *dpy, Window win, GC gc, int delta)
{
  XTextItem ti;

  char buf[1024],line[256],line2[256],*p,*delim=" ";
  unsigned int current_y;

  //  ti.delta=fs->max_bounds.width/2;
  ti.delta=0;
  ti.font=fs->fid;

  if(buf2==NULL) return 0;
  if(strlen(buf2)<=0) return 0;

  strncpy(buf,buf2,1024);

  p=strtok(buf,delim);

  strcpy(line,"");
  strcpy(line2,"");

  current_y=y+delta;

  while(p!=NULL) {

    strncat(line,p,256);
    strcat(line," ");

    if(XTextWidth(fs,line,strlen(line))>w) {
      ti.chars=line2;
      ti.nchars=strlen(line2);

      //      fprintf(stderr,"%s\n",line2);
      XDrawText(dpy,win,gc,x,current_y,&ti,1);

      current_y+=delta;

      strcpy(line,p);
      strcat(line," ");
      strcpy(line2,"");
      
    } else {
      strcpy(line2,line);
    }

    p=strtok(NULL,delim);

  }

  ti.chars=line;
  ti.nchars=strlen(line);
  
  XDrawText(dpy,win,gc,x,current_y,&ti,1);


  return current_y;
}

void apCreateHelp()
{
  Display *dpy=gui.dpy;
  int scrn=DefaultScreen(dpy);
  Window root=DefaultRootWindow(dpy);
  Colormap cm=DefaultColormap(dpy,scrn);

  ap.helpcw=600;
  ap.helpch=800;
  ap.helpcx=(1280-ap.helpcw)/2;
  ap.helpcy=(1024-ap.helpch)/2;


  ap.helpw=XCreateSimpleWindow(dpy,root,
			       ap.helpcx,ap.helpcy,
			       ap.helpcw,ap.helpch,
			       1,
			       WhitePixel(dpy,scrn),
			       BlackPixel(dpy,scrn));

  XSelectInput(dpy,ap.helpw,
	       ExposureMask | 
	       ButtonPressMask | 
	       ButtonReleaseMask |
	       KeyPressMask |
	       KeyReleaseMask |
	       OwnerGrabButtonMask |
	       StructureNotifyMask |
	       EnterWindowMask |
	       LeaveWindowMask);
  
  guiRegisterCustomEvent(ap.helpw,apHelpEvent,NULL);


}


void apHelpEvent(Window w, XEvent *event, void *ptr)
{
  switch(event->type) {
  case Expose:
    apShowHelp(w);
    break;
  case ConfigureNotify:
    break;
  case ButtonPress:
    break;
  case ButtonRelease:
    XUnmapWindow(gui.dpy,ap.helpw);
    ap.help_on=0;
    break;
  case KeyPress:
  case KeyRelease:
    XUnmapWindow(gui.dpy,ap.helpw);
    ap.help_on=0;
    break;
  case EnterNotify:
  case LeaveNotify:
    break;
  }

}

static char *ap_help_e[]={
"Realtime 3D Visualization of biological structures",
"--",
"On this terminal you have the chance to look at biological molecules
(proteins and DNA) displayed in three dimensions.",
"The device in front of you is a spaceball. Twist and turn its black knob and
the objects on the screen will follow. The spaceball is sensitive to the force
you apply: try rotating at different speeds",
"Use the mouse to select one of the structures listed on the right side.
Each structure consists of several objects. Then use the mouse to select one
of the four display modes (not all modes are available all the time):",
"OFF: The object is not visible",
"ATM: The atomic structure is drawn, lines representing chemical bonds between
atoms. This mode should convey how complex a biological macromolecule is",
"TRC: This shows a backbone trace, a schematic representation. Although not
as detailed as the atomic information, the overall fold and shape can be seen",
"SRF: Rendering of the molecular surface. All inside features are hidden.",
"---",
"This demonstration software is derived from a real visualization programm
called DINO, written by Ansgar Philippsen at the Biozentrum in Basel, 
Switzerland. If you are interested in using this demonstration or would like
to know more about the real DINO version, please visit the DINO homepage at
http://www.bioz.unibas.ch/~xray/dino or contact the author at 
ansgar.philippsen@unibas.ch",
"This computer is running Linux 2.2.15 with a VooDoo3 3000 AGP graphics card
on XFree86 v4.0, using the new Direct Rendering Infrastructure.",
"--",
"PRESS MOUSE-BUTTON OR KEY TO EXIT HELP"
};

#if 1
static char *ap_help_g[]={
"Echtzeit Visualisierung von biologischen Strukturen",
"--",
"An diesem Bildschirm hast Du die Moeglichkeit, biologische Molekuele -
Proteine und DNA - in drei Dimensionen dargestellt zu betrachten.",
"Das Geraet vor Dir ist ein Spaceball. Drehe an seinem schwarzen Knauf
und das Objekt auf dem Bildschirm wird folgen. Der Spaceball reagiert auf
Kraft empfindlich, probiere das Objekt mit verschiedener
Geschwindigkeit zu drehen",
"Verwende die Maus, um eine der Strukturen anzuwaehlen, die auf der
rechten Seite aufgelistet sind. Jede Struktur besteht aus mehreren Objekten.
Waehle anschliessend mit der Maus eine der vier moeglichen
Darstellungen:",
"OFF: Das Objekt wird nicht dargestellt",
"ATM: Die Atomstruktur wird gezeigt, Linien entsprechen chemischen Bindungen.
Diese Darstellung soll Dir einen Eindruck von der Komplexitaet biologischer
Makromolekuele geben.",
"TRC: Zeigt das Rueckgrat ('backbone'), eine schematische Wiedergabe der
Struktur. Obwohl nicht so detailliert wie die atomare Darstellung, macht
sie dafuer die Faltung und Topologie sichtbar.",
"SRF: Die Oberflaeche ist dargestellt, die inneren Details sind versteckt",
"--",
"Diese Demo-Software ist von einem Programm namens DINO abgeleitet,
entwickelt von Ansgar Philippsen am Biozentrum in Basel (Schweiz). Falls
Du interessiert bist diese Demonstration zu nutzen, oder mehr ueber die echte
DINO Version erfahren willst, besuche bitte die DINO Homepage auf
www.bioz.unibas.ch/~xray/dino oder kontaktiere den Autor per E-mail:
ansgar.philippsen@unibas.ch",
"Auf diesem Computer laueft Linux 2.2.15 mit einer VooDoo3 3000 AGP Graphik
Karte unter XFree86 v4.0, mittels der neuen 'Direct Rendering Infrastructure'",
"--",
"MAUSCLICK ODER TASTE SCHLIESSEN DIESES FENSTER"
};
#else
static char *ap_help_g[]={
"Echtzeit Visualisierung von biologischen Strukturen",
"Sie haben an diesem Bildschirm die Moeglichkeit, biologische Molekuele -
Proteine und DNA - in drei Dimensionen dargestellt zu betrachten.",
"Das Geraet vor Ihnen ist ein Spaceball. Drehen Sie an seinem schwarzen Knauf
und das Objekt auf dem Bildschirm wird folgen. Der Spaceball reagiert auf
Kraft empfindlich, probieren Sie das Objekt mit verschiedener
Geschwindigkeit zu drehen",
"Verwenden Sie die Maus, um eine der Strukturen anzuwaehlen, die auf der
rechten Seite aufgelistet sind. Jede Struktur besteht aus mehreren Objekten.
Waehlen Sie anschliessend mit der Maus eine der vier moeglichen
Darstellungen:",
"OFF: Das Objekt wird nicht dargestellt",
"ATM: Die Atomstruktur wird gezeigt, Linien entsprechen chemischen Bindungen.
Diese Darstellung soll Ihnen einen Eindruck von der Komplexitaet biologischer
Makromolekuele geben.",
"TRC: Zeigt das Rueckgrat ('backbone'), eine schematische Wiedergabe der
Struktur. Obwohl nicht so detailliert wie die atomare Darstellung, macht
sie dafuer die Faltung und Topologie sichtbar.",
"SRF: Die Oberflaeche ist dargestellt, die inneren Details sind versteckt",
"  ",
"Diese Demo-Software ist von einem Programm namens DINO abgeleitet,
entwickelt von Ansgar Philippsen am Biozentrum in Basel (Schweiz). Falls
Sie interessiert sind diese Demonstration zu nutzen, oder mehr ueber die echte
DINO Version erfahren wollen, besuchen Sie bitte die DINO Homepage auf
www.bioz.unibas.ch/~xray/dino oder kontaktieren Sie den Autor per E-mail:
ansgar.philippsen@unibas.ch",
"Auf diesem Computer laueft Linux 2.2.15 mit einer VooDoo3 3000 AGP Graphik
Karte unter XFree86 v4.0, mittels der neuen 'Direct Rendering Infrastructure'"
};
#endif

static int ap_help_npe=sizeof(ap_help_e)/sizeof(char *);
static int ap_help_npg=sizeof(ap_help_g)/sizeof(char *);


void apShowHelp(Window win)
{
  Display *dpy=gui.dpy;
  int i;

  unsigned int total_y,offset_x,width;

  XFillRectangle(dpy,win,ap.gc3,0,0,ap.helpcw,ap.helpch);

  total_y=20;
  offset_x=20;
  width=ap.helpcw-offset_x*2;

  if(ap.language==AP_ENGLISH) {
    for(i=0;i<ap_help_npe;i++) {
      if(ap_help_e[i][0]=='-') {
	XDrawRectangle(dpy,win,ap.gc1,5,total_y+10,ap.helpcw-10,0);
	total_y+=10;
      } else {
	total_y=apPutInfo(offset_x,total_y,width,ap.helpch,
			  ap_help_e[i],ap.fs5,gui.dpy,win,ap.gc1,22);
      }

      total_y+=10;
    }
    
  } else {
    total_y-=5;
    for(i=0;i<ap_help_npg;i++) {
      if(ap_help_g[i][0]=='-') {
	XDrawRectangle(dpy,win,ap.gc1,5,total_y+10,ap.helpcw-10,0);
	total_y+=10;
      } else {
	total_y=apPutInfo(offset_x,total_y,width,ap.helpch,
			  ap_help_g[i],ap.fs5,gui.dpy,win,ap.gc1,22);
      }
      total_y+=10;
    }
  }
}

