/************************************************

dino: gui.c

Responsible for the graphical user interface. This
includes the GLX window, event processing and the
user menu

*************************************************/

const char version_string[]="0.8.3";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef LINUX
#include <gdbm.h>
#endif
#ifdef SGI
#include <ndbm.h>
#endif
#ifdef DEC
#include <ndbm.h>
#endif

#include <X11/Xlib.h>
#include <Xm/XmAll.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Intrinsic.h>

#include <X11/keysym.h>
#include <X11/extensions/XInput.h>

#ifdef EXPO
#include <Xm/MwmUtil.h>
#endif

#ifdef SGI
#include <X11/extensions/SGIStereo.h>
#endif

#include "dino.h"
#include "com.h"
#include "gui.h"
#include "gfx.h"
#include "extension.h"
#include "GLwStereo.h"
#include "om.h"
#include "input.h"
#include "transform.h"

#ifdef EXPO
#include "autoplay.h"
#endif
#ifdef SPACETEC
#include "spacetec.h"
#endif

struct GUI gui;
extern struct GFX gfx;
extern struct OBJECT_MENU om;
extern struct USER_MENU um;

extern int debug_mode,gfx_mode,stereo_mode;

#ifdef CORE_DEBUG
extern FILE *core_debug;
#endif
#ifdef EXPO
static String fallback_resources[]={
  "*Background: #00003f",
  "*Foreground: #ffffaf",
  "*geometry: +0+0",
  "*frame*width: 1024",
  //  "*frame*height: 1024",
  "*frame*height: 1008",
  "*frame*x: 0",
  "*frame*y: 0",
  "*frame*topOffset: 0",
  "*frame*bottomOffset: 0",
  "*frame*rightOffset: 0",
  "*frame*leftOffset: 0",
  "*frame*marginWidth: 0",
  "*frame*marginHeight: 0",
  "*frame2*width: 256",
  "*frame2*height: 1008",
  "*clientDecoration: 0",
  "*transientDecoration: 0",
  NULL
};
#else
static String fallback_resources[]={
  "*sgiMode: True",
  "*useSchemes: all",
  "*Background: #00003f",
  "*Foreground: #ffffaf",
  "*geometry: -0+0",
  "*frame*width: 958",
  "*frame*height: 958",
  "*frame*x: 3",
  "*frame*y: 3",
  "*frame*topOffset: 2",
  "*frame*bottomOffset: 2",
  "*frame*rightOffset: 2",
  "*frame*leftOffset: 2",
  "*frame*shadowType: SHADOW_IN",
  NULL
};
#endif

static struct GUI_KEY_EVENT gui_default_key_event[]={
    {XK_F1,"scene stereo"},
    {0,""}
};

#ifdef EXPO
static char *expo_message="DINO by Ansgar Philippsen  -  Special EXPO2000 version  -  http://www.bioz.unibas.ch/~xray/dino";
#endif

/***********************************************
   guiInit
   -------

   sets up the main window with menu bar, status
   bar and GLX window, creates the user menu,
   checks existence of dialbox, assigns default
   event mappings and load rgb color database

   arguments: main callback function, command
              line parameters

*************************************************/

int guiInit(void (*func)(int, char **), int *argc, char ***argv)
{
  int i,j;
  EventMask em;
  Arg arg[10];
  XmString xms;
  char major[8],minor[8];
#ifdef SGI
  int ev,er;
#endif
#ifdef DEC
  int ev,er;
#endif
#ifdef SUN
  int ev,er;
#endif

  /*
    Initialze the RGB color database
  */
  guiInitRGB();

#ifdef EXPO
  XtSetArg(arg[0],XmNmwmDecorations,0);
#endif

  /*
    open top level app
  */
  debmsg("guiInit: opening top level widget");
  gui.top=XtOpenApplication(&gui.app,"dino",NULL,0,
			    argc,(*argv),
			    fallback_resources,
			    topLevelShellWidgetClass,
#ifdef EXPO
			    arg,3
#else
			    NULL,0
#endif
			    );
  debmsg("guiInit: setting display");
  gui.dpy=XtDisplay(gui.top);

  
  /* 
     Create form, which will hold
     the menu bar, the glx window
     and the status bar
  */
  debmsg("guiInit: creating main layout");
  gui.form=XmCreateForm(gui.top, "form", NULL, 0);
  XtManageChild(gui.form);

  /*
    Create form that will hold message areas
  */
  debmsg("guiInit: creating message form");
  XtSetArg(arg[0],XmNleftAttachment,XmATTACH_FORM);
  XtSetArg(arg[1],XmNbottomAttachment,XmATTACH_FORM);
  XtSetArg(arg[2],XmNrightAttachment,XmATTACH_FORM);
  gui.mform=XtCreateManagedWidget("mform",xmFormWidgetClass,
				  gui.form,
				  arg,3);


  
  /*
    Create the status bars at the bottom
  */
  debmsg("guiInit: creating message label 1");
#ifdef EXPO
  strcpy(gui.message_string,expo_message);
#else
  strcpy(gui.message_string,"Ready");
#endif
  xms= XmStringCreateLtoR(gui.message_string, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
#ifdef EXPO
  XtSetArg(arg[1],XmNalignment, XmALIGNMENT_CENTER);
#else
  XtSetArg(arg[1],XmNalignment, XmALIGNMENT_BEGINNING);
#endif
  XtSetArg(arg[2],XmNtopAttachment,XmATTACH_FORM);
  XtSetArg(arg[3],XmNleftAttachment,XmATTACH_FORM);
  XtSetArg(arg[4],XmNbottomAttachment,XmATTACH_FORM);
  gui.message=XtCreateManagedWidget("message",xmLabelWidgetClass,
				    gui.mform,arg,5);


  debmsg("guiInit: creating message label 2");
#ifdef EXPO
  strcpy(gui.message_string2,"Powered by Linux");
#else
  strcpy(gui.message_string2,version_string);
#endif
  xms= XmStringCreateLtoR(gui.message_string2, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  XtSetArg(arg[1],XmNalignment, XmALIGNMENT_END);
  XtSetArg(arg[2],XmNtopAttachment,XmATTACH_FORM);
  XtSetArg(arg[3],XmNrightAttachment,XmATTACH_FORM);
  XtSetArg(arg[4],XmNbottomAttachment,XmATTACH_FORM);
  XtSetArg(arg[5],XmNleftAttachment,XmATTACH_WIDGET);
  XtSetArg(arg[6],XmNleftWidget,gui.message);

  gui.message2=XtCreateManagedWidget("message2",xmLabelWidgetClass,
				    gui.mform,arg,7);

  
  /*
    Create the frame that will enclose
    the GLX window
  */
  debmsg("guiInit: creating glx frame");
  gui.frame=XmCreateFrame(gui.form, "frame", NULL, 0);
  XtVaSetValues(gui.frame,
#ifndef EXPO
                XmNrightAttachment, XmATTACH_FORM,
#endif
		XmNtopAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget,gui.mform,
		XmNwidth,HeightOfScreen(XtScreen(gui.frame))-56,
		XmNheight,HeightOfScreen(XtScreen(gui.frame))-56,
                NULL);
  XtManageChild(gui.frame);

#ifdef EXPO
  apMenuInit();
#endif

  /*
    initialize GLX 
  */
  debmsg("guiInit: checking GLX availability");
  if(!glXQueryExtension(gui.dpy,NULL,NULL)) {
    fprintf(stderr,"GLX extension not available\n");
    gui.om_flag=0;
    return -1;
  }
 
#ifndef EXPO
  omInit();
  gui.om_flag=1;
#else
  omInit();
  gui.om_flag=1;
#endif

  debmsg("guiInit: searching for visual");
  /* 
     Find the best visual
  */

  guiInitVisual();
  if(gui.visinfo==NULL) {
    fprintf(stderr,"No suitable visual found\n");
    return -1;
  }

#ifdef SGI
  if(stereo_mode) {
    if(XSGIStereoQueryExtension(gui.dpy,&ev,&er)) {
      /*
	try to find an additional stereo visual
      */
      debmsg("guiInit: checking stereo capabilities");
      guiInitStereo();
      if(gui.stereo_visinfo!=NULL) {
	gui.stereo_available=GLW_STEREO_HIGH;
	fprintf(stderr,"HighEnd stereo detected\n");
      } else {
	gui.stereo_available=GLW_STEREO_LOW;
	fprintf(stderr,"LowEnd stereo detected\n");
      }
    } else {
      gui.stereo_available=GLW_STEREO_NONE;
    }
  }
#else
  gui.stereo_available=GLW_STEREO_NONE;
#endif

  gui.stereo_mode=GUI_STEREO_OFF;
  gui.eye_dist=150.0;
  gui.eye_offset=10.0;

  if(gui.stereo_available==GLW_STEREO_HIGH) {
    gui.visinfo=gui.stereo_visinfo;
  }
  
  /*
    get the colormap matching the visual
  */
  debmsg("guiInit: setting colormap");
  gui.cmap=guiGetColorMap(gui.visinfo);

  /*
    finally create GLX window
  */
  debmsg("guiInit: creating and initializing GLX area");
  gui.glxwin=XtVaCreateManagedWidget("glxwin",
				     glwMDrawingAreaWidgetClass,
				     gui.frame,
				     GLwNvisualInfo, gui.visinfo,
				     XtNcolormap, gui.cmap,
				     NULL);

  /* 
     Add callbacks for the glx widget 
  */
  XtAddCallback(gui.glxwin, GLwNginitCallback, guiGlxInit, NULL);
  XtAddCallback(gui.glxwin, GLwNexposeCallback, guiGlxExpose, NULL);
  XtAddCallback(gui.glxwin, GLwNresizeCallback, guiGlxResize, NULL);
  XtAddCallback(gui.glxwin, GLwNinputCallback, guiGlxInput, NULL);

#ifdef EXPO
  XtSetSensitive(gui.glxwin,True);
#endif


  /*
    The main window is now ready
  */
  debmsg("guiInit: calling XtRealizeWidget");
  XtRealizeWidget(gui.top);
  
  /*
    make the created GLX window the current
    GLX context
  */
  debmsg("guiInit: making GLX window current context");
  GLwDrawingAreaMakeCurrent(gui.glxwin, gui.glxcontext);
  if(gui.stereo_available!=GLW_STEREO_NONE) {
    debmsg("guiInit: initializing stereo");
    if(GLwStereoInit(gui.dpy,XtWindow(gui.glxwin))==-1) {
      fprintf(stderr,"No HighEnd Stereo Video Mode found, LowEnd stereo forced\n");
      gui.stereo_available=GLW_STEREO_LOW;
    }
  }


  
  /*
    assign callback function
  */
  gui.callback=func;
  
  /***
  debmsg("guiInit: assigning default key mappings");
  for(i=0,j=0;i<GUI_MAX_KEY_EVENTS;i++) {
    if(gui_default_key_event[j].key>0) {
      gui.key_event[i].key=gui_default_key_event[j].key;
      strcpy(gui.key_event[i].command,gui_default_key_event[j].command);
      j++;
    } else {
      gui.key_event[i].key=0;
      strcpy(gui.key_event[i].command,"");
    }
  }
  ***/

  debmsg("guiInit: calling inputInit");
  if(inputInit()==-1) {
    fprintf(stderr,"\nfatal memory error in inputInit");
    dinoExit(-1);
  }

  /*
    check for a dialbox
  */
  debmsg("guiInit: checking for extra input devices");
  if(guiDialboxInit()) {
    fprintf(stderr,"Dialbox detected\n");
  }
  if(guiSpaceballInit()) {
    if(gui.spaceballDevice!=NULL)
      fprintf(stderr,"Spaceball detected\n");
    else
      fprintf(stderr,"\n");
  }


  /*
    reset redraw flag
  */
  gui.redraw=0;
  
  /*
    print info about graphics subsystem
  */
  debmsg("guiInit: info");
  fprintf(stderr,"Graphics Subsystem ID: %s %s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER));
  sprintf(major,"%c",glGetString(GL_VERSION)[0]);
  major[1]='\0';
  sprintf(minor,"%c",glGetString(GL_VERSION)[2]);
  minor[1]='\0';

  if(atoi(major)<1 || (atoi(major)==1 && atoi(minor)<1)) {
    fprintf(stderr,"OpenGL version %d.%d or above required, found %s.%s instead\n",
	    1,1,major,minor);
    gui.om_flag=0;
    return -1;
  } else {
    fprintf(stderr,"OpenGL Version %s.%s\n",major,minor);
  }

  /*
    check OpenGL extensions 
  */
  debmsg("Extensions:");
  debmsg(glGetString(GL_EXTENSIONS));
  
  /*
    as a last step, grab the error handler
  */
  XSetErrorHandler(guiErrorHandler);
  XSetIOErrorHandler(guiIOErrorHandler);

  /*
    initialization completed
  */
  return 0;
}

int guiInitRGB()
{
#ifdef LINUX
  char *rgbfile1;
  FILE *rgbfile2;
  char rgb_line[256],*rgb_sep,*rgb_nam;
  unsigned short usp[6];
  datum key,content;
  GDBM_FILE gdbm_f;
#endif

  debmsg("guiInit: loading rgb database");
#ifdef LINUX
  strcpy(gui.gdbm_tmpfile,"");
  rgbfile1=tmpnam(NULL);
  rgbfile2=fopen("/usr/lib/X11/rgb.txt","r");
  if(rgbfile2==NULL) {
    fprintf(stderr,"cannot find rgb.txt\n");
    return -1;
  } else {
    strcpy(gui.gdbm_tmpfile,rgbfile1);
    gdbm_f=gdbm_open(rgbfile1,0,GDBM_NEWDB,0600,0);
    if(gdbm_f==NULL) {
      fprintf(stderr,"cannot open rgb tmp file %s\n",rgbfile1);
      return -1;
    }
    fgets(rgb_line,256,rgbfile2); /* first line is comment */
    do {
      fgets(rgb_line,256,rgbfile2);
      if (rgb_line[0]!='!' && rgb_line[0]!='\n') {
	rgb_sep=strchr(rgb_line,'\t');
	if(rgb_sep!=NULL) {
	  rgb_sep[0]='\0';
	  rgb_nam=strrchr(rgb_sep+1,'\t')+1;
	  if(rgb_nam!=NULL) {
	    rgb_nam[strlen(rgb_nam)-1]='\0';
	    rgb_line[3]='\0';
	    rgb_line[7]='\0';
	    rgb_line[11]='\0';
	    usp[0]=(unsigned short)atoi(rgb_line+0)*256;
	    usp[1]=(unsigned short)atoi(rgb_line+4)*256;
	    usp[2]=(unsigned short)atoi(rgb_line+8)*256;
	    key.dptr=rgb_nam;
	    key.dsize=strlen(rgb_nam);
	    content.dptr=(char *)usp;
	    content.dsize=sizeof(usp);
	    gdbm_store(gdbm_f,key,content,GDBM_REPLACE);
	  }
	}
      }
    } while(!feof(rgbfile2));
    gdbm_close(gdbm_f);
  }
  gui.cdbm=gdbm_open(rgbfile1,0,GDBM_READER,0,0);
  
#endif
#ifdef SGI
  gui.cdbm=dbm_open("/usr/lib/X11/rgb",0,0);
#endif
#ifdef DEC
  gui.cdbm=dbm_open("/usr/lib/X11/rgb",0,0);
#endif
#ifdef SUN
  gui.cdbm=dbm_open("/usr/openwin/lib/rgb",0,0);
#endif

  if(gui.cdbm==NULL) {
    fprintf(stderr,"rgb color database not found\n");
    return -1;
  }
  return 0;
}

int guiInitVisual()
{
  int buf[64];
  int bufc=0,i,j;
  int depthi,redi,bluei,greeni,alphai;
  int d[]={16,12,8,6,4,1};
  int c[]={12,8,6,5,4,2,1};
  char message[256];

  buf[bufc++]=GLX_RGBA;
  buf[bufc++]=GLX_DOUBLEBUFFER;
  buf[bufc++]=GLX_DEPTH_SIZE;
  depthi=bufc++;
  buf[bufc++]=GLX_RED_SIZE;
  redi=bufc++;
  buf[bufc++]=GLX_BLUE_SIZE;
  bluei=bufc++;
  buf[bufc++]=GLX_GREEN_SIZE;
  greeni=bufc++;
  buf[bufc++]=None;
  /*
    alpha not required ?
  buf[bufc++]=GLX_ALPHA_SIZE;
  alphai=bufc++;
  buf[alphai]=0;
  */
  for(j=0;j<sizeof(c)/sizeof(int);j++) {
    for(i=0;i<sizeof(d)/sizeof(int);i++) {
      buf[depthi]=d[i];
      buf[redi]=c[j];
      buf[bluei]=c[j];
      buf[greeni]=c[j];
      gui.visinfo=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),buf);
      if(gui.visinfo!=NULL) {
	sprintf(message,"found visual with depth %d and rgba %d",d[i],c[j]);
	debmsg(message);
	return 0;
      }
    }
  }
  return -1;
}

int guiInitStereo()
{
  int buf[64];
  int bufc=0,i,j;
  int depthi,redi,bluei,greeni,alphai;
  int d[]={12,8,6,4,1};
  int c[]={8,6,5,4,2,1};
  char message[256];

  buf[bufc++]=GLX_RGBA;
  buf[bufc++]=GLX_STEREO;
  buf[bufc++]=GLX_DOUBLEBUFFER;
  buf[bufc++]=GLX_DEPTH_SIZE;
  depthi=bufc++;
  buf[bufc++]=GLX_RED_SIZE;
  redi=bufc++;
  buf[bufc++]=GLX_BLUE_SIZE;
  bluei=bufc++;
  buf[bufc++]=GLX_GREEN_SIZE;
  greeni=bufc++;
  /*
  buf[bufc++]=GLX_ALPHA_SIZE;
  alphai=bufc++;
  */
  buf[bufc++]=None;

  for(j=0;j<sizeof(c);j++) {
    for(i=0;i<sizeof(d);i++) {
      buf[depthi]=d[i];
      buf[redi]=c[j];
      buf[bluei]=c[j];
      buf[greeni]=c[j];
//      buf[alphai]=c[j];
      gui.stereo_visinfo=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),buf);
      if(gui.stereo_visinfo!=NULL) {
	sprintf(message,"found stereo visual with depth %d and rgba %d",d[i],c[j]);
	debmsg(message);
	return 0;
      }
    }
  }
  return -1;
}

/**************************************
   guiMainLoop
   -----------

   main event loop, processes incoming
   events, checks for duplicate expose,
   handles extension events
****************************************/

int guiMainLoop()
{
  XEvent event;
  Boolean dummy;

  /* endless loop */
  while(1){
    /* get next event */
#ifdef CORE_DEBUG
    fprintf(core_debug,"[NextEvent");
    fflush(core_debug);
#endif
    /*
      TEST
    glXWaitGL();
    */
    XtAppNextEvent(gui.app,&event);
#ifdef CORE_DEBUG
    fprintf(core_debug,"] ");
#endif

    /* check for multiple expose events */
    if(event.type==Expose) {
      if(event.xexpose.count>0)
	continue;
    }


    /*
      if dials are connected and
      a dials device was detected,
      branch to dials event handler
    */

    if(gui.dialsDevice!=NULL || gui.spaceballDevice!=NULL)
      if(event.xany.type>gui.xiEventBase)
	    guiExtensionHandler(XtWindowToWidget(event.xany.display, event.xany.window),NULL,&event);
#ifdef SPACETEC
    if(gui.spacetecDevice)
      spacetecEventHandler(gui.dpy, &event);
#endif
    guiCheckCustomEvent(&event);
#ifdef CORE_DEBUG
    fprintf(core_debug,"[DispatchEvent %d",event.type);
    fflush(core_debug);
#endif
//    glXWaitX();
    XtDispatchEvent(&event);
#ifdef CORE_DEBUG
    fprintf(core_debug,"]\n");
#endif
  }
  
  /* to make the compiler happy */
  return 0;
}


/********************

 guiGetColorMap
 --------------

 returns colormap for
 requested visual
********************/

Colormap guiGetColorMap(XVisualInfo *vinfo)
{
  Status status;
  XStandardColormap *standardCmaps;
  Colormap cmap;
  int i, numCmaps;

  status=XmuLookupStandardColormap(gui.dpy,
				   vinfo->screen, vinfo->visualid,
                                   vinfo->depth,XA_RGB_DEFAULT_MAP,
                                   False,True);
  if(status==1){
    status=XGetRGBColormaps(gui.dpy,
			    RootWindow(gui.dpy,vinfo->screen),
                            &standardCmaps,&numCmaps,
			    XA_RGB_DEFAULT_MAP);
    if(status==1)
      for(i=0;i<numCmaps;i++)
        if(standardCmaps[i].visualid == vinfo->visualid){
          cmap=standardCmaps[i].colormap;
          XFree(standardCmaps);
          return cmap;
        }
  }

  cmap=XCreateColormap(gui.dpy,
		       RootWindow(gui.dpy, vinfo->screen),
                       vinfo->visual, AllocNone);
  return cmap;
}

/*****************************************

  guiGlxInit
  ----------

  This event handler is called after the
  GLX window has been correctly set up
  by the system. Only now OpenGL calls
  are valid, and OpenGL is initialized
  through gfxGLInit();

*****************************************/

void guiGlxInit(Widget ww, XtPointer clientData, XtPointer call)
{
  Arg arg[5];
  XWindowAttributes xwa;
  XSetWindowAttributes xws;

  XtSetArg(arg[0],GLwNvisualInfo,&gui.visinfo);
  XtSetArg(arg[1],XmNwidth,&gui.win_width);
  XtSetArg(arg[2],XmNheight,&gui.win_height);
  XtSetArg(arg[3],XmNx,&gui.win_x);
  XtSetArg(arg[4],XmNy,&gui.win_y);

  XtGetValues(ww,arg,5);

  debmsg("guiGlxInit: creating context");
  gui.glxcontext = glXCreateContext(gui.dpy, gui.visinfo, 0, True);

  if (gui.glxcontext==NULL)
    XtAppError(gui.app,"Could not create GLX context");

  gui.glxwindow=XtWindow(ww);

  /*
    this patches the event mask
     to include OwnerGrab for
     the user popup to work!
  */
#ifndef EXPO
  XGetWindowAttributes(gui.dpy,gui.glxwindow,&xwa);
  xws.event_mask=xwa.your_event_mask | OwnerGrabButtonMask;
  XChangeWindowAttributes(gui.dpy,gui.glxwindow,CWEventMask,&xws);
#endif  
  glXMakeCurrent(gui.dpy, gui.glxwindow, gui.glxcontext);

  gui.inside=0;

  /* init the GL params */
  debmsg("guiGlxInit: calling gfxGLInit");

#ifdef LINUX
  gfxSetViewport();
#endif

  gfxGLInit();
}


/*****************************

  guiGLXExpose
  ------------

  called after the GLX window
  has received an exposure
  request.

*****************************/

void guiGlxExpose(Widget ww, XtPointer clientData, XtPointer call)
{
  debmsg("redraw request");
  debmsg("calling Redraw");
  comRedraw();
}


/*****************************

  guiGLXResize
  ------------

  called after the GLX window
  has received a resize
  request.

*****************************/

void guiGlxResize(Widget ww, XtPointer clientData, XtPointer call)
{
  GLwDrawingAreaCallbackStruct *callData;
  callData = (GLwDrawingAreaCallbackStruct*) call;
  gui.win_width=callData->width;
  gui.win_height=callData->height;
  //  gfxSetup();
  gfxSetViewport();
  debmsg("resize request");
  debmsg("calling Redraw");
  gfxResizeEvent();
  /*
  gfxRedraw();
  */
  comRedraw();
}



/*****************************

  guiGLXInput
  -----------

  called after the GLX window
  has received an input
  event, like  mouse or keyb
  event.

*****************************/

void guiGlxInput(Widget ww, XtPointer clientData, XtPointer call)
{
  int i;
  int dx,dy;
  double val;
  struct timeval tp;
  struct timezone tzp;
  long dt;
  char kbuf[32];
  int kcount;
  KeySym key;


  /*
    reading pointer
  */
  XmDrawingAreaCallbackStruct *cd =
    (XmDrawingAreaCallbackStruct *) call;

  gettimeofday(&tp,&tzp);

#ifdef EXPO
  apIdleReset();
#endif
 
  /* 
     switch depending on event type
  */
  switch(cd->event->type){

  case KeyPress:
    kcount=XLookupString(&(cd->event->xkey),kbuf,sizeof(kbuf),&key,NULL);
    kbuf[kcount]='\0';
    for(i=0;i<kcount;i++)
      comWriteCharBuf(kbuf[i]);
    break;
  case KeyRelease:
    kcount=XLookupString(&(cd->event->xkey),kbuf,sizeof(kbuf),&key,NULL);
    kbuf[kcount]='\0';
    switch(key) {
    case XK_Return:
    case XK_Linefeed:
    case XK_KP_Enter:
      comWriteCharBuf(13);
      break;
    case XK_Delete:
      comWriteCharBuf(8);
      break;
    case XK_Up:
      comWriteCharBuf(27);
      comWriteCharBuf('[');
      comWriteCharBuf('A');
      break;
    case XK_Down:
      comWriteCharBuf(27);
      comWriteCharBuf('[');
      comWriteCharBuf('B');
      break;
    case XK_Left:
      comWriteCharBuf(27);
      comWriteCharBuf('[');
      comWriteCharBuf('D');
      break;
    case XK_Right:
      comWriteCharBuf(27);
      comWriteCharBuf('[');
      comWriteCharBuf('C');
      break;
    }
    break;

  case ButtonPress:
    gui.last_x=cd->event->xbutton.x;
    gui.last_y=cd->event->xbutton.y;
    gfx.sx=gui.last_x;
    gfx.sy=gui.last_y;
    gfx.sdx=0;
    gfx.sdy=0;
    memcpy(&gui.tp_button,&tp,sizeof(struct timezone));
    if(cd->event->xbutton.button==3) {
#ifndef EXPO
      om.pwin=gui.user_menu;
      om.pflag=1;
      XMoveWindow(gui.dpy,gui.user_menu,
		  cd->event->xbutton.x_root,cd->event->xbutton.y_root);
      XMapRaised(gui.dpy,gui.user_menu);
#endif
    }
    break;
  case ButtonRelease:
    dx=gui.last_x-cd->event->xbutton.x;
    dy=gui.last_y-cd->event->xbutton.y;
    dt=(long)(tp.tv_sec-gui.tp_button.tv_sec)*1000000+(tp.tv_usec-gui.tp_button.tv_usec);
    if(dt<200000 ||  /* 200 milliseconds */
       (dx==0 && dy==0 && dt<300000)) {
      if(cd->event->xbutton.state & Button1Mask) {
	if(cd->event->xbutton.state & ShiftMask)
	  comPick(cd->event->xmotion.x,cd->event->xmotion.y,1);
	else
	  comPick(cd->event->xmotion.x,cd->event->xmotion.y,0);
      }
    }
//    fprintf(stderr,"\n%d %d",dx,dy);
    if(cd->event->xbutton.state == Button1Mask) {
      gfx.sdx=gfx.sx-cd->event->xbutton.x;
      gfx.sdy=gfx.sy-cd->event->xbutton.y;
    }

    if(cd->event->xbutton.button==3) {
      om.pflag=0;
      XUnmapWindow(gui.dpy,gui.user_menu);
    }

    break;
  case MotionNotify:
    dx=gui.last_x-cd->event->xmotion.x;
    dy=gui.last_y-cd->event->xmotion.y;
    gfx.sx=gui.last_x;
    gfx.sy=gui.last_y;
    gui.last_x=cd->event->xmotion.x;
    gui.last_y=cd->event->xmotion.y;


    comTransform(TRANS_MOUSE,cd->event->xmotion.state,0,dx);
    comTransform(TRANS_MOUSE,cd->event->xmotion.state,1,dy);
    
    break;
  case EnterNotify:
    gui.inside=1;
    break;
  case LeaveNotify: 
    gui.inside=0;
    break;
  }
}


/***************************************

  guiTimeProc
  -----------

  is called periodically through itself,
  is needed e.g. for the terminal to
  work

****************************************/

void guiTimeProc(XtPointer client_data)
{
  XExposeEvent expose;


  if(gui.redraw) {
    gui.redraw=0;
    gfxRedraw();
    /****
    expose.type=Expose;
    expose.serial=-1;
    expose.send_event=0;
    expose.display=gui.dpy;
    expose.window=gui.glxwindow;
    expose.x=0;
    expose.y=0;
    expose.width=gui.win_width;
    expose.height=gui.win_height;
    expose.count=0;
    XtDispatchEvent((XEvent *)&expose);
    ****/
  }
  comTimeProc();
#ifndef NEW_GUI_MAIN
  XtAppAddTimeOut(gui.app,10,(XtTimerCallbackProc)guiTimeProc,NULL);
#endif
}


/******************************************

  guiCreateMenu
  -------------

  Creates the main menu, is not needed now

*******************************************/

void guiCreateMenu()
{
  gui.menu_help=XmCreateCascadeButton(gui.menu,"help", NULL, 0);
  XtVaSetValues(gui.menu_help,
		XmNlabelString, XmStringCreate("Help",XmSTRING_DEFAULT_CHARSET),
		NULL);
  XtAddCallback(gui.menu_help,XmNactivateCallback, (XtCallbackProc)guiMenuHelp, NULL);
  XtVaSetValues(gui.menu,
		XmNmenuHelpWidget, (XtArgVal)gui.menu_help);
  XtManageChild(gui.menu_help);


  gui.menu_file=XmCreatePulldownMenu(gui.menu,"file", NULL, 0);
  XtVaSetValues(gui.menu_file,
		XmNlabelString, XmStringCreate("File",XmSTRING_DEFAULT_CHARSET),
		NULL);

  gui.menu_filec=XmCreateCascadeButton(gui.menu, "filec", NULL, 0);
  XtVaSetValues(gui.menu_filec,
		XmNsubMenuId, gui.menu_file,
		XmNlabelString, XmStringCreate("File", XmSTRING_DEFAULT_CHARSET),
		NULL);
  XtManageChild(gui.menu_filec);

  gui.menu_fileb[0]=XmCreatePushButtonGadget(gui.menu_file, "button1", NULL, 0);
  XtVaSetValues(gui.menu_fileb[0],
		XmNlabelString, XmStringCreate("button 1", XmSTRING_DEFAULT_CHARSET),
		NULL);
  gui.menu_fileb[1]=XmCreatePushButtonGadget(gui.menu_file, "button2", NULL, 0);
  XtVaSetValues(gui.menu_fileb[1],
		XmNlabelString, XmStringCreate("button 2", XmSTRING_DEFAULT_CHARSET),
		NULL);
  XtManageChildren(gui.menu_fileb,2);
}

void guiMenuHelp(Widget w, caddr_t d1, caddr_t d2)
{
  fprintf(stdout,"Help selected\n");
}


/************************************

  guiDialboxInit
  --------------

  look for a dialbox and set it up

**************************************/

/*
  defined globaly, these values will be changed by the macros below
*/
int deviceMotionNotify=0,deviceButtonPress=0,
    deviceButtonPressGrab=0,deviceButtonRelease=0;

int guiDialboxInit()
{
  XEventClass  dMotionNotifyClass,dButtonPressClass,
    dButtonPressGrabClass,dButtonReleaseClass;
  XEventClass eventList[4];

  gui.dialsDevice=extDialBoxInit(gui.dpy);

  if(gui.dialsDevice==NULL)
    return 0;
  
  /* 
     these macros find the correct event type value for the requested
     event class and store them in the global vars
  */
  DeviceMotionNotify(gui.dialsDevice, deviceMotionNotify,dMotionNotifyClass);
  DeviceButtonPress(gui.dialsDevice, deviceButtonPress,dButtonPressClass);
  DeviceButtonPressGrab(gui.dialsDevice,deviceButtonPressGrab,dButtonPressGrabClass);
  DeviceButtonRelease(gui.dialsDevice,deviceButtonRelease,dButtonReleaseClass);
  
  eventList[0]=dMotionNotifyClass;
  eventList[1]=dButtonPressClass;
  eventList[2]=dButtonPressGrabClass;
  eventList[3]=dButtonReleaseClass;
  
  XSelectExtensionEvent(gui.dpy,gui.glxwindow,eventList,4);
  
  return 1;
}

int guiSpaceballInit()
{
  XEventClass  dMotionNotifyClass,dButtonPressClass,
    dButtonPressGrabClass,dButtonReleaseClass;
  XEventClass eventList[4];


  /*
    check first for the spacetec spaceball, then
    for the generic spaceball
  */
#ifdef SPACETEC
  if(spacetecInit(gui.dpy,gui.glxwindow,"DINO")) {
    gui.spacetecDevice=1;
    return 1;
  } else {
#endif
    gui.spacetecDevice=0;
    gui.spaceballDevice=extSpaceballInit(gui.dpy);
    
    if(gui.spaceballDevice==NULL)
      return 0;
    
    /* 
       these macros find the correct event type value for the requested
       event class and store them in the global vars
    */
    DeviceMotionNotify(gui.spaceballDevice, deviceMotionNotify,dMotionNotifyClass);
    DeviceButtonPress(gui.spaceballDevice, deviceButtonPress,dButtonPressClass);
    DeviceButtonPressGrab(gui.spaceballDevice,deviceButtonPressGrab,dButtonPressGrabClass);
    DeviceButtonRelease(gui.spaceballDevice,deviceButtonRelease,dButtonReleaseClass);
    
    eventList[0]=dMotionNotifyClass;
    eventList[1]=dButtonPressClass;
    eventList[2]=dButtonPressGrabClass;
    eventList[3]=dButtonReleaseClass;
    
    XSelectExtensionEvent(gui.dpy,gui.glxwindow,eventList,4);
    
    return 1;
#ifdef SPACETEC
  }
#endif
}


/*************************************

  guiExtensionHandler
  --------------

  called from the main loop whenever
  a dial event was encountered

*************************************/

void guiExtensionHandler(Widget w, XtPointer client_data, XEvent *event)
{
  int i,num,diff;
  double val,va[6];
  XDeviceMotionEvent *device_motion;

  if(event->type == deviceMotionNotify){
    device_motion=(XDeviceMotionEvent *)event;
    num=(int)device_motion->first_axis;

    /*
    fprintf(stderr,"\nfa: %d  ac: %d",num, device_motion->axes_count);
    for(i=0;i<6;i++)
      fprintf(stderr,"[#%d: %d]",i,device_motion->axis_data[i]);
    */
    if(gui.dialsDevice!=NULL)
      if(device_motion->deviceid==gui.dialsDevice->device_id) {
	diff=(int)device_motion->axis_data[0]-gui.last_dial[num];
	gui.last_dial[num]=(int)device_motion->axis_data[0];

	comTransform(TRANS_DIALS,device_motion->state,num,diff);
      }

    if(gui.spaceballDevice!=NULL)
      if(device_motion->deviceid==gui.spaceballDevice->device_id)
	if(device_motion->first_axis==0) {
	  if(device_motion->axis_data[0]!=0)
	    comTransform(TRANS_SPACEBALL, device_motion->state,
			 0,device_motion->axis_data[0]);
	  if(device_motion->axis_data[1]!=0)
	    comTransform(TRANS_SPACEBALL, device_motion->state,
			 1,device_motion->axis_data[1]);
	  if(device_motion->axis_data[2]!=0)
	    comTransform(TRANS_SPACEBALL, device_motion->state,
			 2,device_motion->axis_data[2]);
	  if(device_motion->axis_data[3]!=0)
	    comTransform(TRANS_SPACEBALL, device_motion->state,
			 3,device_motion->axis_data[3]);
	  if(device_motion->axis_data[4]!=0)
	    comTransform(TRANS_SPACEBALL, device_motion->state,
			 4,device_motion->axis_data[4]);
	  if(device_motion->axis_data[5]!=0)
	    comTransform(TRANS_SPACEBALL, device_motion->state,
			 5,device_motion->axis_data[5]);
	}
  } 
	  
}


/************************************

  guiStereo
  ---------

  handles stereo mode switches

**************************************/

int guiStereo(int m)
{
#ifdef SGI
  Arg arg[4];
  Position ax,ay;
  Dimension aw,ah;
  int rh,wh;
  Position sx,sy;
  Dimension sw,sh;
  XWindowChanges xwc;


  switch(gui.stereo_available) {

  case GLW_STEREO_LOW:
    wh=gui.win_height;
    rh=HeightOfScreen(DefaultScreenOfDisplay(gui.dpy));
    
    if(gui.stereo_mode==GUI_STEREO_OFF) {
      XtSetArg(arg[0],XmNx,&gui.win_xs);
      XtSetArg(arg[1],XmNy,&gui.win_ys);
      XtSetArg(arg[2],XmNwidth,&gui.win_widths);
      XtSetArg(arg[3],XmNheight,&gui.win_heights);
      XtGetValues(gui.top,arg,4);
    }
    
    sx=gui.win_xs;
    sy=(Position)rh/2;
    sw=gui.win_widths;
    sh=gui.win_heights/2;
    
    switch(m) {
    case GUI_STEREO_OFF: /* off */
      GLwStereoCommand(GLW_STEREO_NONE);
      gui.stereo_mode=GUI_STEREO_OFF;

      XtSetArg(arg[1],XmNx,gui.win_xs);
      XtSetArg(arg[0],XmNy,gui.win_ys);
      XtSetArg(arg[2],XmNwidth,gui.win_widths);
      XtSetArg(arg[3],XmNheight,gui.win_heights);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=gui.win_ys;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      

      // reset projection matrix
      gfx.aspect=(double)gui.win_width/(double)gui.win_height;
      gfxSetProjection(gfx.stereo_display);
      gfxSetFog();

      break;
    case GUI_STEREO_NORMAL: /* on */
      GLwStereoCommand(GLW_STEREO_LOW);
      gui.stereo_mode=GUI_STEREO_NORMAL;
      
      XtSetArg(arg[1],XmNx,sx);
      XtSetArg(arg[0],XmNy,sy);
      XtSetArg(arg[2],XmNwidth,sw);
      XtSetArg(arg[3],XmNheight,sh);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=sy+30;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;

      break;
    }
    break;
  case GLW_STEREO_HIGH:
    wh=gui.win_height;
    rh=HeightOfScreen(DefaultScreenOfDisplay(gui.dpy));
    
    if(gui.stereo_mode==GUI_STEREO_OFF) {
      XtSetArg(arg[0],XmNx,&gui.win_xs);
      XtSetArg(arg[1],XmNy,&gui.win_ys);
      XtSetArg(arg[2],XmNwidth,&gui.win_widths);
      XtSetArg(arg[3],XmNheight,&gui.win_heights);
      XtGetValues(gui.top,arg,4);
    }
    
    sx=256;
    sy=gui.stereo_y_offset;
    sw=768;
    sh=768;
    
    switch(m) {
    case GUI_STEREO_OFF: /* off */
      GLwStereoCommand(GLW_STEREO_NONE);
      gui.stereo_mode=GUI_STEREO_OFF;

      XtSetArg(arg[1],XmNx,gui.win_xs);
      XtSetArg(arg[0],XmNy,gui.win_ys);
      XtSetArg(arg[2],XmNwidth,gui.win_widths);
      XtSetArg(arg[3],XmNheight,gui.win_heights);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=gui.win_ys;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      
      // reset projection matrix
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;
      gfxSetProjection(gfx.stereo_display);
      gfxSetFog();
      break;
    case GUI_STEREO_NORMAL: /* on */
      GLwStereoCommand(GLW_STEREO_HIGH);
      gui.stereo_mode=GUI_STEREO_NORMAL;
      
      XtSetArg(arg[1],XmNx,sx);
      XtSetArg(arg[0],XmNy,sy);
      XtSetArg(arg[2],XmNwidth,sw);
      XtSetArg(arg[3],XmNheight,sh);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=sy+30;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;
      break;
    }
    break;
  }
#endif
  return 0;
}

/************************

  guiResolveColor
  ---------------

  translates a string
  into rgb

************************/

int guiResolveColor(const char *oname, float *r, float *g, float *b)
{
  int i;
  datum key, content;
  unsigned short rr,gg,bb;
  unsigned short usp[6];
  char name[256];

  strncpy(name,oname,255);

  for(i=0;i<strlen(name);i++)
    if(name[i]=='_') name[i]=' ';

  key.dptr=name;
  key.dsize=strlen(name);
 
  if(gui.cdbm==NULL)
    return -1;

#ifdef LINUX
  content=gdbm_fetch(gui.cdbm,key);
#endif
#ifdef SGI
  content=dbm_fetch(gui.cdbm,key);
#endif
#ifdef DEC
  content=dbm_fetch(gui.cdbm,key);
#endif
#ifdef SUN
  content=dbm_fetch(gui.cdbm,key);
#endif

  if(content.dptr==NULL)
    return -1;

  memcpy(usp,content.dptr,6);

  rr=usp[0];
  gg=usp[1];
  bb=usp[2];

  (*r)=(float)rr/65535.0;
  (*g)=(float)gg/65535.0;
  (*b)=(float)bb/65535.0;

  return 0;
}

/***********************************

  guiMessage
  ----------

  displays a message in the status
  bar

*************************************/

int guiMessage(char *m)
{
  Arg arg[10];
  XmString xms;

#ifndef EXPO
  strcpy(gui.message_string,m);

  xms= XmStringCreateLtoR(gui.message_string, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);

  XtSetValues(gui.message,arg,1);
#endif
  return 0;
}

int guiMessage2(char *m)
{
  Arg arg[10];
  XmString xms;

  strcpy(gui.message_string2,m);

  xms= XmStringCreateLtoR(gui.message_string2, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);

  XtSetValues(gui.message2,arg,1);

  return 0;
}

void guiRegisterCustomEvent(Window w, guiCustomFunc f, void *ptr)
{
  gui.ce.entry[gui.ce.entry_count].w=w;
  gui.ce.entry[gui.ce.entry_count].f=f;
  gui.ce.entry[gui.ce.entry_count].ptr=ptr;
  gui.ce.entry_count++;
}

int guiCheckCustomEvent(XEvent *event)
{
  int i;

  /*
  fprintf(stderr,"\n%d",event->type);
  */

  /* this is an ugly patch */
  if(om.pflag)
    if(event->type==ButtonRelease)
      if(event->xbutton.button==3) {
	om.pflag=0;
	XUnmapWindow(om.dpy,om.pwin);
	omExposeEvent();
      }
  


  for(i=0;i<gui.ce.entry_count;i++)
    if(event->xany.window==gui.ce.entry[i].w) {
      (gui.ce.entry[i].f)(gui.ce.entry[i].w,event,gui.ce.entry[i].ptr);
      return 1;
    }
      

  return 0;
}

void guiRegisterUserMenu(Window w)
{
  gui.user_menu=w;
}


int guiErrorHandler(Display *d, XErrorEvent *e)
{
  char buffer[256];
  XGetErrorText(d,e->error_code,buffer,255);
  fprintf(stderr,"\nWARNING: Caught X error: %s",buffer);
  return 0;
}

int guiIOErrorHandler(Display *d)
{
  fprintf(stderr,"\nCaught fatal X error - exiting\n");
  dinoExit(-1);
  return 0;
}

int guiMInit(void (*func)(int, char **), int *argc, char ***argv)
{
  int i,j;
  char major[8],minor[8];
#ifdef SGI
  int ev,er;
#endif
#ifdef DEC
  int ev,er;
#endif
#ifdef SUN
  int ev,er;
#endif
  int visbuf[]={GLX_RGBA,None};
  /*
    Initialze the RGB color database
  */
  guiInitRGB();

  /*
    open top level app
  */
  debmsg("guiMInit: opening display");
  XtToolkitInitialize();
  gui.app=XtCreateApplicationContext();
  if((gui.dpy=XtOpenDisplay(gui.app,NULL,NULL,NULL,NULL,0,argc,(*argv)))==NULL){
    fprintf(stderr,"guiMInit: error opening display\n");
    return -1;
  }



  /*
    initialize GLX 
  */
  debmsg("guiMInit: checking GLX availability");
  if(!glXQueryExtension(gui.dpy,NULL,NULL)) {
    fprintf(stderr,"GLX extension not available\n");
    gui.om_flag=0;
    return -1;
  }

  /*
    assign simple visual
  */  
  gui.visinfo=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),visbuf);
 
  gui.om_flag=0;


  gui.callback=func;
  
  gui.redraw=0;
  
  /*
    print info about graphics subsystem
  */
  debmsg("guiInit: info");
  fprintf(stderr,"Graphics Subsystem ID: %s %s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER));
  sprintf(major,"%c",glGetString(GL_VERSION)[0]);
  major[1]='\0';
  sprintf(minor,"%c",glGetString(GL_VERSION)[2]);
  minor[1]='\0';

  if(atoi(major)<1 || (atoi(major)==1 && atoi(minor)<1)) {
    fprintf(stderr,"OpenGL version %d.%d or above required, found %s.%s instead\n",
	    1,1,major,minor);
    gui.om_flag=0;
    return -1;
  } else {
    fprintf(stderr,"OpenGL Version %s.%s\n",major,minor);
  }

  /*
    check OpenGL extensions 
  */
  debmsg("Extensions:");
  debmsg(glGetString(GL_EXTENSIONS));
  
  /*
    as a last step, grab the error handler
  */
  XSetErrorHandler(guiErrorHandler);
  XSetIOErrorHandler(guiIOErrorHandler);

  /*
    initialization completed
  */
  return 1;
}
