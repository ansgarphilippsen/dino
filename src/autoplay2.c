#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

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

  apIdleReset();
  
  return 0;
}


int apParse(void)
{
  char *buf,*p;
  struct stat sb;
  char *sep=" \t\n";
  int entryc,itemc,flag;

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
	    strcpy(ap.e[entryc].name,strtok(NULL,sep));
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
	    strcpy(ap.e[entryc].inf,strtok(NULL,sep));
	    flag|=0x8;
	  } else if(!strcmp(p,"dir")) {
	    /* dir */
	    strcpy(ap.e[entryc].dir,strtok(NULL,sep));
	    flag|=0x10;
	  } else if(!strcmp(p,"item")) {
	    /* item */
	    strcpy(ap.e[entryc].b[itemc].name,strtok(NULL,sep));
	    strcpy(ap.e[entryc].b[itemc].scr,strtok(NULL,sep));
	    strcpy(ap.e[entryc].b[itemc].inf,strtok(NULL,sep));
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
  int i,j;
  char sdir[256];
  FILE *f;
  struct stat sb;
  char com[1024];

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
    
    for(j=0;j<ap.e[i].bc;j++) {
      if(stat(ap.e[i].b[j].scr,&sb)!=0) {
	fprintf(stderr,"prep error: %s: %s\n",ap.e[i].b[j].scr, strerror(errno));
	return -1;
      }
      if(stat(ap.e[i].b[j].inf,&sb)!=0) {
	fprintf(stderr,"prep error: %s: %s\n",ap.e[i].b[j].inf, strerror(errno));
	return -1;
      }
      
    }
    
    chdir(sdir);
    
  }

  apEntryActivate(ap.e+0);
  apItemActivate(ap.e+0,0);

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

#ifdef SGI
  char *font_name1="-adobe-itc bookman-*-r-*-*-24-*-*-*-*-*-*-*";
  char *font_name2="-adobe-itc bookman-*-i-*-*-20-*-*-*-*-*-*-*";
#else
  char *font_name1="-*-times-medium-r-normal-*-24-*-75-75-*-*-*-*";
  char *font_name2="-*-times-medium-r-normal-*-18-*-75-75-*-*-*-*";
#endif

  xfs1=XLoadQueryFont(gui.dpy,font_name1);
  xfs2=XLoadQueryFont(gui.dpy,font_name2);

  if(xfs1!=0) {
    xfe1=XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG,XmFONT_IS_FONT,xfs1);
    xfl1=XmFontListAppendEntry(NULL,xfe1);
  } else {
    fprintf(stderr,"\nfont '%s' not found",font_name1);
  }

  if(xfs2!=0) {
    xfe2=XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG,XmFONT_IS_FONT,xfs2);
    xfl2=XmFontListAppendEntry(NULL,xfe2);
  } else {
    fprintf(stderr,"\nfont '%s' not found",font_name2);
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

  XtSetArg(arg[0],XmNinitialFocus,gui.glxwindow);
  XtSetArg(arg[1],XmNtraversalOn,False);
  XtSetArg(arg[2],XmNresizable,True);

  /* top level form inside frame */
  ap.form=XtCreateManagedWidget("ap_form",xmFormWidgetClass,
				 ap.frame,arg,3);

  XtSetArg(arg[0],XmNinitialFocus,gui.glxwindow);
  XtSetArg(arg[1],XmNtraversalOn,False);
  XtSetArg(arg[2],XmNpacking,XmPACK_TIGHT);
  XtSetArg(arg[3],XmNorientation,XmVERTICAL);
  XtSetArg(arg[4],XmNrightAttachment, XmATTACH_FORM);
  XtSetArg(arg[5],XmNleftAttachment, XmATTACH_FORM);
  XtSetArg(arg[6],XmNtopAttachment, XmATTACH_FORM);
  XtSetArg(arg[7],XmNradioBehavior, True);


  ap.area1=XtCreateManagedWidget("ap_area1",xmRowColumnWidgetClass,
				 ap.form,arg,8);


  /*
    fill the first area with the entries
  */
  for(i=0;i<ap.ec;i++) {
    
    xms=XmStringCreateLtoR(ap.e[i].name, XmSTRING_DEFAULT_CHARSET);
    argc=3;
    XtSetArg(arg[0],XmNlabelString,xms);
    XtSetArg(arg[1],XmNindicatorOn,False);
    XtSetArg(arg[2],XmNfontList,xfl1);
    if(i==0) {
      XtSetArg(arg[3],XmNset,True);
      argc=4;
      ap.ce=ap.e+0;
    }
    
    ap.e[i].w=XtCreateManagedWidget(NULL,xmToggleButtonWidgetClass,
				    ap.area1,arg,argc);

    XtAddCallback(ap.e[i].w, XmNarmCallback, apEntryCallback, ap.e+i);

    
    for(j=0;j<ap.e[i].bc;j++) {
      ap.e[i].slist[j]=XmStringCreate(ap.e[i].b[j].name,
				      XmSTRING_DEFAULT_CHARSET);
    }
  }

  
  argc=0;
  XtSetArg(arg[0],XmNvisibleItemCount,10);
  XtSetArg(arg[1],XmNinitialFocus,gui.glxwindow);
  XtSetArg(arg[2],XmNtraversalOn,False);
  XtSetArg(arg[3],XmNrightAttachment, XmATTACH_FORM);
  XtSetArg(arg[4],XmNleftAttachment, XmATTACH_FORM);
  XtSetArg(arg[5],XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg(arg[6],XmNtopWidget, ap.area1);
  XtSetArg(arg[7],XmNheight, 200);
  XtSetArg(arg[8],XmNfontList,xfl2);
  XtSetArg(arg[9],XmNitems,ap.e[0].slist);
  XtSetArg(arg[10],XmNitemCount,ap.e[0].bc);

  ap.area2=XtCreateManagedWidget("ap_list",xmListWidgetClass,
				ap.form,arg,11);


  XmListSelectPos(ap.area2,1,False);

  XtAddCallback(ap.area2,XmNbrowseSelectionCallback,apItemCallback,NULL);

  XtSetArg(arg[0],XmNinitialFocus,gui.glxwindow);
  XtSetArg(arg[1],XmNtraversalOn,False);
  XtSetArg(arg[2],XmNrightAttachment, XmATTACH_FORM);
  XtSetArg(arg[3],XmNleftAttachment, XmATTACH_FORM);
  XtSetArg(arg[4],XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg(arg[5],XmNtopWidget, ap.area2);
  XtSetArg(arg[6],XmNeditable, False);
  XtSetArg(arg[7],XmNheight, 200);
  XtSetArg(arg[8],XmNeditMode, XmMULTI_LINE_EDIT);
  XtSetArg(arg[9],XmNvalue,ap.help);


  ap.area3=XtCreateManagedWidget("ap_info",xmTextWidgetClass,
				ap.form,arg,10);


  XtSetArg(arg[0],XmNinitialFocus,gui.glxwindow);
  XtSetArg(arg[1],XmNtraversalOn,False);
  XtSetArg(arg[2],XmNpacking,XmPACK_TIGHT);
  XtSetArg(arg[3],XmNorientation,XmVERTICAL);
  XtSetArg(arg[4],XmNrightAttachment, XmATTACH_FORM);
  XtSetArg(arg[5],XmNleftAttachment, XmATTACH_FORM);
  XtSetArg(arg[6],XmNtopAttachment, XmATTACH_WIDGET);
  XtSetArg(arg[7],XmNtopWidget, ap.area3);
  XtSetArg(arg[8],XmNbottomAttachment, XmATTACH_FORM);
  XtSetArg(arg[9],XmNradioBehavior, False);


  ap.area4=XtCreateManagedWidget("ap_area4",xmRowColumnWidgetClass,
				 ap.form,arg,10);


  xms=XmStringCreateLtoR("Hilfe", XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  XtSetArg(arg[1],XmNindicatorOn,False);
  XtSetArg(arg[2],XmNfontList,xfl1);
  
  ap.helpb=XtCreateManagedWidget(NULL,xmPushButtonWidgetClass,
				 ap.area4,arg,3);
  
  XtAddCallback(ap.helpb, XmNactivateCallback, apHelpCallback, 0);
  
  /* generate the help dialog */

  xms=XmStringCreateLtoR("MESSAGEBOX", XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);

  /*
  ap.helpd1=XtCreateWidget("helpd", xmMessageBoxWidgetClass,
			   gui.top,arg,1);
  */


  XtSetArg(arg[0],XmNallowOverlap,True);
  XtSetArg(arg[1],XmNbuttonFontList,NULL);
  XtSetArg(arg[2],XmNdefaultPosition,True);
  XtSetArg(arg[3],XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL);
  XtSetArg(arg[4],XmNnoResize,True);
  XtSetArg(arg[4],XmNresizePolicy,XmRESIZE_NONE);
  XtSetArg(arg[5],XmNwidth,300);
  XtSetArg(arg[6],XmNheight,300);
  XtSetArg(arg[7],XmNmwmDecorations,0);
  XtSetArg(arg[8],XmNmwmFunctions,0);

  ap.helpd1=XmCreateBulletinBoardDialog(gui.top,"helpd",
					arg,9);

  //  XtSetArg(arg[0],XmNinputCallback,apHelp2Callback);
  XtSetArg(arg[0],XmNx,0);
  XtSetArg(arg[1],XmNy,0);
  XtSetArg(arg[2],XmNwidth,300);
  XtSetArg(arg[3],XmNheight,300);
  XtSetArg(arg[4],XmNborderWidth,0);

  ap.helpd2=XtCreateManagedWidget("helpd2",xmDrawingAreaWidgetClass,
				  ap.helpd1,arg,5);


  XtAddCallback(ap.helpd2, XmNinputCallback, apHelpInputCallback, 0);
  XtAddCallback(ap.helpd2, XmNexposeCallback, apHelpExposeCallback, 0);


  return 0;
}

void apEntryCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  XmToggleButtonCallbackStruct *cb;
  struct AP_ENTRY *e;

  cb=(XmToggleButtonCallbackStruct *)call;
  e=(struct AP_ENTRY *)clientData;

  if(cb->reason==XmCR_ARM && cb->set==0) {
    ap.ce=e;
    apEntryActivate(e);
    XtVaSetValues(ap.area2,
		  XmNitems,e->slist,
		  XmNitemCount,e->bc,
		  NULL);
    XmListSelectPos(ap.area2,1,True);

  }
}

void apItemCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  XmListCallbackStruct *cb;

  cb=(XmListCallbackStruct *)call;

//  fprintf(stderr,"\n%s %s",ap.ce->name,ap.ce->b[cb->item_position-1].name);
  apItemActivate(ap.ce,cb->item_position-1);
}


void apEntryActivate(struct AP_ENTRY *e)
{
  char buf[1024];
  char cwd[256];

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
}

void apItemActivate(struct AP_ENTRY *e,int n)
{
  char buf[1024];
  char cwd[256];

  getcwd(cwd,255);

  chdir(e->dir);

  strcpy(buf,"@");
  strcat(buf,e->b[n].scr);
  shell.greedy=1;
  comRawCommand(".*.* hide");
  comRawCommand(buf);
  shellWork();
  shell.greedy=0;

  chdir(cwd);
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
  int newi;
  ap.idle0++;
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
      /* change item */
      ap.idle2=1;
      ap.idle3++;
      newi=(int)(rnd()*(double)ap.ce->bc);
      newi%=ap.ce->bc;
      XmListSelectPos(ap.area2,newi,True);
    }
    if(ap.idle3>AUTOPLAY_IDLE3) {
      /* change entry */
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

void apHelpCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  XtManageChild(ap.helpd1);
}

void apHelpInputCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  XtUnmanageChild(ap.helpd1);
}

void apHelpExposeCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  XTextItem ti;
  static XFontStruct *f=NULL;
  static int init = 1;
  XGCValues xgcv;

  fprintf(stderr,"\nhelp callback");

  
  if(init) {
    xgcv.foreground=WhitePixel(gui.dpy,DefaultScreen(gui.dpy));
    xgcv.background=BlackPixel(gui.dpy,DefaultScreen(gui.dpy));
    ap.help_gc=XCreateGC(gui.dpy,XtWindow(ap.helpd2),
			 GCForeground | GCBackground,
			 &xgcv);
    init=0;

    f=XLoadQueryFont(gui.dpy,"-*-times-medium-r-normal-*-14-*-75-75-*-*-*-*");
  }

  ti.chars="test\ntest";
  ti.nchars=strlen("test\ntest");
  ti.delta=f->max_bounds.width/2;
  ti.font=f->fid;

  XClearWindow(gui.dpy,XtWindow(ww));
  XDrawText(gui.dpy,XtWindow(ww),ap.help_gc,30,30,&ti,1);

}
