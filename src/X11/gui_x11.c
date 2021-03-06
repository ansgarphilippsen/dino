/************************************************

dino: gui.c

Responsible for the graphical user interface. This
includes the GLX window, event processing and the
user menu

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifndef INTERNAL_COLOR
#ifdef LINUX
#include <gdbm.h>
#endif
#ifdef SGI
#include <ndbm.h>
#endif
#ifdef DEC
#include <ndbm.h>
#endif
#endif

#include <X11/Xlib.h>
#include <Xm/XmAll.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Intrinsic.h>
#include <X11/keysym.h>
#include <X11/extensions/XInput.h>
#include <Xm/DragDrop.h>

#include "dino.h"
#include "gui_x11.h"
#include "om_x11.h"
#include "extension.h"
#include "AppPlus.h"
#include "cl.h"

#ifdef SGI_STEREO
#include "sgi_stereo.h"
#endif

#ifdef SPACETEC
#include "spacetec.h"
#endif

#include "gui_terminal.h"

static void gl_info();
static int init_main();
static XVisualInfo *init_visual(int df, int sf,int st, int af, int cf);
static Colormap get_colormap(XVisualInfo *vinfo);
static void glx_init(Widget ww, XtPointer clientData, XtPointer call);
static void glx_expose(Widget ww, XtPointer clientData, XtPointer call);
static void glx_resize(Widget ww, XtPointer clientData, XtPointer call);
static void glx_input(Widget ww, XtPointer clientData, XtPointer call);
static void check_redraw(void);
static int xinput_ext_init(void);
static int dialbox_init(void);
static int spaceball_init(void);
static void extension_event(Widget w, XtPointer client_data, XEvent *event);
static void handle_drop(Widget w, XtPointer client_data, XtPointer call_data);
static void register_dnd(Widget site);
static int error_handler(Display *d, XErrorEvent *e);
static int error_io_handler(Display *d);
static int pad_init(void);
static int set_stereo(int m);
static XVisualInfo *get_offscreen_visual(int af);
static void hide_cursor();
static void show_cursor();

#ifndef INTERNAL_COLOR
static int init_colordb(void);
#endif

struct GUI gui;

#define MAX_OFFSCREEN_CONTEXT 8

static struct _OFFSCREEN_CONTEXT {
  int used;
  Pixmap pm;
  GLXPixmap glx_pm;
  GLXContext glx_context;
}offscreen_context_list[MAX_OFFSCREEN_CONTEXT];
static int offscreen_context_count=0;

extern struct OBJECT_MENU om;
extern struct USER_MENU um;

extern int debug_mode,gfx_flags;

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


/***********************************************
   guiInit
   -------

*************************************************/

// cmiMessage can't be used here! revert to good'ol fprintf

static void outMessage(const char *m) {fprintf(stdout,m);}

static int gui_demo_flag=0;

//int guiInit(void (*func)(int, char **), int argc, char **argv)
int guiInit(int argc, char **argv)
{
  cmiToken t;
  int i,j,ret;
  EventMask em;
  Arg arg[10];
  int glxmajor,glxminor;
  char message[256];
  int nostereo=0;
  int use_stereo=0;
  int dblbuff = 1;
  int consflag = 0;
#ifdef SGI
  int stereo_available=0;
#endif
  XVisualInfo *vi;
  int icf=0;
#ifdef SGI
  int ev,er;
#endif
#ifdef DEC
  int ev,er;
#endif
#ifdef SUN
  int ev,er;
#endif

  for(i=0;i<MAX_OFFSCREEN_CONTEXT;i++)
    offscreen_context_list[i].used=0;

  for(i=0;i<argc;i++) {
    if(clStrcmp(argv[i],"-iconic")) {
      icf=1;
    } else if(clStrcmp(argv[i],"-demo")) {
      gui_demo_flag=1;
    }
  }
  
#ifdef SGI
  // TODO remove duplicated stereo flags
  if(gfx_flags & DINO_FLAG_NOSTEREO) {
    nostereo=1;
    use_stereo=0;
  }
#else
  if(gfx_flags & DINO_FLAG_STEREO) {
    use_stereo=1;
    nostereo=0;
  }
#endif

  // register cmi callbacks for GUI
  cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback);

#ifndef INTERNAL_COLOR
  // Initialize the RGB color database
  init_colordb();
#endif


  if(gfx_flags & DINO_FLAG_NOGFX) {
    debmsg("opening display for nogfx modus");
    gui.dpy=XOpenDisplay(getenv("DISPLAY"));
    gui.glxwindow=DefaultRootWindow(gui.dpy);
    gui.nogfx_flag=1;
  } else {
    // open top level app
    debmsg("guiInit: opening top level app widget");
    gui.top=XtOpenApplication(&gui.app,"dino",NULL,0,
			      &argc,argv,
			      fallback_resources,
			      appPlusShellWidgetClass,
			      NULL,0
			      );
    debmsg("guiInit: setting display");
    gui.dpy=XtDisplay(gui.top); 
    gui.nogfx_flag=0;
  }

  debmsg("guiInit: checking GLX availability");
  if(!glXQueryExtension(gui.dpy,NULL,NULL)) {
    fprintf(stderr,"GLX extension is not available\n");
    gui.om_flag=0;
    return -1;
  } else {
    glXQueryVersion(gui.dpy,&glxmajor,&glxminor);
    if(glxmajor==1 && glxminor>=2) {
      sprintf(message,"guiInit: found GLX Version %d.%d\n",glxmajor,glxminor);
      debmsg(message);
    } else {
      fprintf(stderr,"GLX Version must be 1.2 or above, but found %d.%d\n",
	      glxmajor,glxminor);
      return -1;
    }
  }

  if(gui.nogfx_flag) {
    debmsg("creating offscreen rendering context");
    if(guiCreateOffscreenContext(500,500,0)<0) {
      return -1;
    }
    gui.glxcontext=glXGetCurrentContext();
    gui.om_flag=0;
  } else {
    // initialize main X11 widgets
    if(init_main()<0) {
      return -1;
    }    
    if((gfx_flags & DINO_FLAG_NOOBJMENU) ||
       gui_demo_flag) {
      gui.om_flag=0;
    } else {
      debmsg("guiInit: initializing om\n");
      omInit(icf);
      gui.om_flag=1;
    }
    //  pad_init();

    if(gfx_flags & DINO_FLAG_CONS) {
      consflag=1;
    } else {
      consflag=0;
    }

    if(gfx_flags & DINO_FLAG_NODBLBUFF) {
      dblbuff=0;
    }


    
    debmsg("guiInit: searching for visual");
    /* 
       Find the best visual
    */

    // set to 0 per default
    gui.stereo_available=0;
    
#ifdef SGI_STEREO
    if(nostereo==0 && XSGIStereoQueryExtension(gui.dpy,&ev,&er) ) {
      debmsg("using stereo");
      use_stereo=1;
      stereo_available=SGI_STEREO_HIGH;
    } else {
      stereo_available=SGI_STEREO_NONE;
    }
#endif

#ifdef LINUX_STEREO
    if(use_stereo) {
      debmsg("trying stereo visual");
    }
#endif
    
    if(gfx_flags & DINO_FLAG_NOSTENCIL) { 
	vi=init_visual(dblbuff,use_stereo,0,0,consflag);
    } else {
      // try with stencil buffer
      vi=init_visual(dblbuff,use_stereo,1,0,consflag);
      if(!vi) {
	gfx_flags=gfx_flags | DINO_FLAG_NOSTENCIL;
	vi=init_visual(dblbuff,use_stereo,0,0,consflag);
      }
    }

#ifdef LINUX_STEREO    
    // try again without stereo support
    if(use_stereo) {
      if(!vi) {
	fprintf(stderr,"No stereo visual found, using mono\n");
	use_stereo=0;
	if(gfx_flags & DINO_FLAG_NOSTENCIL) { 
	  vi=init_visual(dblbuff,use_stereo,0,0,consflag);
	} else {
	  vi=init_visual(dblbuff,use_stereo,1,0,consflag);
	  if(!vi) {
	    gfx_flags=gfx_flags | DINO_FLAG_NOSTENCIL;
	    vi=init_visual(dblbuff,use_stereo,0,0,consflag);
	  }
	}
	gui.stereo_available=0;
      } else {
	fprintf(stderr,"Stereo visual found!\n");
	gui.stereo_available=1;
      }
    }
#endif

#ifdef SGI_STEREO
    if(!vi && use_stereo) {
      vi=init_visual(dblbuff,0,1,0,consflag);
      if(!vi) {
	gfx_flags=gfx_flags | DINO_FLAG_NOSTENCIL;
	vi=init_visual(dblbuff,0,0,0,consflag);
      }
      
      stereo_available=SGI_STEREO_LOW;
    }
#endif

    gui.visinfo=vi;
    
    if(!gui.visinfo) {
      outMessage("fatal error: no suitable visual found\n");
      return -1;
    }
    
#ifdef SGI_STEREO
    if(stereo_available==SGI_STEREO_HIGH) {
      outMessage("HighEnd stereo detected\n");
      gui.stereo_available=1;
    } else if(stereo_available==SGI_STEREO_LOW) {
      outMessage("LowEnd stereo detected\n");
      gui.stereo_available=1;
    } else {
      gui.stereo_available=0;
    }
#endif
    
    
    debmsg("guiInit: setting colormap");
    gui.cmap=get_colormap(gui.visinfo);
    
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
    XtAddCallback(gui.glxwin, GLwNginitCallback, glx_init, NULL);
    XtAddCallback(gui.glxwin, GLwNexposeCallback, glx_expose, NULL);
    XtAddCallback(gui.glxwin, GLwNresizeCallback, glx_resize, NULL);
    XtAddCallback(gui.glxwin, GLwNinputCallback, glx_input, NULL);
    
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
    
#ifdef SGI_STEREO
    if(stereo_available!=SGI_STEREO_NONE) {
      debmsg("guiInit: initializing stereo");
      if(SGIStereoInit(gui.dpy,XtWindow(gui.glxwin),stereo_available)<0) {
	outMessage("error during stereo initialization, LowEnd stereo forced\n");
      }
    }
#endif
    
    /*
      assign callback function
    */
    //gui.callback=func;
    
    /*
      check for a dialbox
    */
    
    debmsg("guiInit: checking for extra input devices");
    if(dialbox_init()) {
      outMessage("Dialbox detected\n");
    }
    if(spaceball_init()) {
      if(gui.spaceballDevice!=NULL)
	outMessage("Spaceball detected\n");
    }
    
  } // if nogfx

  /*
    reset redraw flag
  */
  gui.redraw=0;

  // display info about OpenGL
  gl_info();

  if(gui_demo_flag) {
    guiGrab(1);
    XWarpPointer(gui.dpy,None,XtWindow(gui.glxwin),0,0,0,0,1,1);
    hide_cursor();
  }
  
  /*
    as a last step, grab the error handler
  */
  XSetErrorHandler(error_handler);
  XSetIOErrorHandler(error_io_handler);

  /*
    initialization completed
  */
  return 0;
}

/*
  print info about graphics subsystem
*/
static void gl_info()
{
  char major[8],minor[8];

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
    exit(-1);
  } else {
    fprintf(stderr,"OpenGL Version %s.%s\n",major,minor);
  }
  
  /*
    check OpenGL extensions 
  */
  debmsg("Extensions:");
  debmsg(glGetString(GL_EXTENSIONS));
}

/*
  set up the main gfx window
*/

static int init_main()
{
  XmString xms;
  Arg arg[10];
  int width,height;

  /* 
     Create form, which will hold
     the menu bar, the glx window
     and the status bar
  */
  debmsg("guiInit: creating main layout");
//  XtSetArg(arg[0],XmNdropSiteActivity,XmDROP_SITE_INACTIVE);
  gui.form=XmCreateForm(gui.top, "form", arg, 0);
  XtManageChild(gui.form);

  // Create form that will hold message areas
  debmsg("guiInit: creating message form");
  XtSetArg(arg[0],XmNleftAttachment,XmATTACH_FORM);
  XtSetArg(arg[1],XmNbottomAttachment,XmATTACH_FORM);
  XtSetArg(arg[2],XmNrightAttachment,XmATTACH_FORM);
//  XtSetArg(arg[3],XmNdropSiteActivity,XmDROP_SITE_INACTIVE);
  gui.mform=XtCreateManagedWidget("mform",xmFormWidgetClass,
				  gui.form,
				  arg,3);

//  register_dnd(gui.mform);
  
  // Create the status bars at the bottom
  debmsg("guiInit: creating message label 1");
  strcpy(gui.message_string,"Ready");

  xms= XmStringCreateLtoR(gui.message_string, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  XtSetArg(arg[1],XmNalignment, XmALIGNMENT_BEGINNING);
  XtSetArg(arg[2],XmNtopAttachment,XmATTACH_FORM);
  XtSetArg(arg[3],XmNleftAttachment,XmATTACH_FORM);
  XtSetArg(arg[4],XmNbottomAttachment,XmATTACH_FORM);
//  XtSetArg(arg[5],XmNdropSiteActivity,XmDROP_SITE_INACTIVE);

  gui.message=XtCreateManagedWidget("message",xmLabelWidgetClass,
				    gui.mform,arg,5);

//  register_dnd(gui.message);

  debmsg("guiInit: creating message label 2");
  if(gui_demo_flag) {
    strcpy(gui.message_string2,"DINO (c) Ansgar Philippsen - http://www.dino3d.org");
  } else {
    strcpy(gui.message_string2,VERSION);
  }
  xms= XmStringCreateLtoR(gui.message_string2, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  XtSetArg(arg[1],XmNalignment, XmALIGNMENT_END);
  XtSetArg(arg[2],XmNtopAttachment,XmATTACH_FORM);
  XtSetArg(arg[3],XmNrightAttachment,XmATTACH_FORM);
  XtSetArg(arg[4],XmNbottomAttachment,XmATTACH_FORM);
  XtSetArg(arg[5],XmNleftAttachment,XmATTACH_WIDGET);
  XtSetArg(arg[6],XmNleftWidget,gui.message);
//  XtSetArg(arg[7],XmNdropSiteActivity,XmDROP_SITE_INACTIVE);

  gui.message2=XtCreateManagedWidget("message2",xmLabelWidgetClass,
				    gui.mform,arg,7);
//  register_dnd(gui.message2);

  
  // Create the frame that will contain GLX win
  debmsg("guiInit: creating glx frame");
  gui.frame=XmCreateFrame(gui.form, "frame", NULL, 0);
  if(gui_demo_flag) {
    width=WidthOfScreen(XtScreen(gui.frame));
    height=HeightOfScreen(XtScreen(gui.frame));
  } else {
    width=HeightOfScreen(XtScreen(gui.frame))-56;
    height=HeightOfScreen(XtScreen(gui.frame))-56;
  }
  XtVaSetValues(gui.frame,
                XmNrightAttachment, XmATTACH_FORM,
		XmNtopAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget,gui.mform,
#ifndef DARWIN // workaround for unexplained problem under darwin
		XmNwidth,width,
		XmNheight,height,
#endif
                NULL);
  debmsg("guiInit: managing frame");
  XtManageChild(gui.frame);

  return 0;
}



/***************************************

  guiTimeProc
  -----------

  the periodic scheduler

****************************************/

void guiTimeProc(XtPointer client_data)
{
  check_redraw();

  cmiTimer();

  guitTimeProc();

  XtAppAddTimeOut(gui.app,2,(XtTimerCallbackProc)guiTimeProc,NULL);
}

static void check_redraw(void)
{
  if(gui.redraw) {
    gui.redraw=0;
    glXWaitX();
    cmiRedraw();
    //glXWaitGL();
  }
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

  // register timer first
  if(!gui.nogfx_flag) {
    debmsg("setting Xtimer");
    XtAppAddTimeOut(gui.app,100,(XtTimerCallbackProc)guiTimeProc,NULL);

    /* endless loop */
    while(1){
      /* get next event */
      
      XtAppNextEvent(gui.app,&event);
      
      switch(event.type) {
      case Expose:
	if(event.xexpose.count>0)
	  continue;
	break;
	//case MapNotify: fprintf(stderr,"map\n"); break;
	//case UnmapNotify: fprintf(stderr,"unmap\n"); break;
      }
      
      /*
	if dials are connected and
	a dials device was detected,
	branch to dials event handler
      */
      if(gui.dialsDevice!=NULL || gui.spaceballDevice!=NULL)
	if(event.xany.type>gui.xiEventBase)
	  extension_event(XtWindowToWidget(event.xany.display, event.xany.window),NULL,&event);
      
      
      /***
	  #ifdef SPACETEC
	  if(gui.spacetecDevice)
	  spacetecEventHandler(gui.dpy, &event);
	  #endif
      */
      
      // events for the user and object menu
      guiCheckCustomEvent(&event);
      
      
      XtDispatchEvent(&event);
    }
  } else {
    // nogfx modus
    debmsg("preparing offscreen main loop");
    debmsg("initializing viewport");
    cmiInitGL();
    debmsg("initializing GL");
    cmiResize(500,500);
    debmsg("entering offscreen main loop");
    while(1) {
      guitTimeProc();
      cmiTimer();
      if(gui.redraw) {
	gui.redraw=0;
	cmiRedraw();
      }
    }
  }
  
  /* to make the compiler happy */
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

/*
  second, small message area
 */

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


/*
  swap buffers
*/

void guiSwapBuffers()
{
  if(!(gfx_flags & DINO_FLAG_NODBLBUFF)) {
    glXSwapBuffers(gui.dpy, gui.glxwindow);
  }
}


/*
  custom events from object or user menu
  are handled here
*/

void guiRegisterCustomEvent(Window w, guiCustomFunc f, void *ptr)
{
  gui.ce.entry[gui.ce.entry_count].w=w;
  gui.ce.entry[gui.ce.entry_count].f=f;
  gui.ce.entry[gui.ce.entry_count].ptr=ptr;
  gui.ce.entry_count++;
}

int guiCheckCustomEvent(XEvent *event)
{
#ifndef GLUT_GUI
  int i;

  /*
  fprintf(stderr,"\n%d",event->type);
  */

  /* this is an ugly patch */
  if(gui.om_flag) {
    if(om.pflag)
      if(event->type==ButtonRelease)
	if(event->xbutton.button==3) {
	  om.pflag=0;
	  XUnmapWindow(om.dpy,om.pwin);
	  omExposeEvent();
	}
  }
  


  for(i=0;i<gui.ce.entry_count;i++)
    if(event->xany.window==gui.ce.entry[i].w) {
      (gui.ce.entry[i].f)(gui.ce.entry[i].w,event,gui.ce.entry[i].ptr);
      return 1;
    }
      
#endif
  return 0;
}

void guiRegisterUserMenu(Window w)
{
  gui.user_menu=w;
}


/*
  set stereo mode
  returns the actual stereo mode, 
  0 for off
  1 for on
*/

static int set_stereo(int m) 
{
#ifdef SGI_STEREO
  if(m) {
    SGISwitchStereo(SGI_STEREO_ON);
  } else {
    SGISwitchStereo(SGI_STEREO_OFF);
  }
  return SGIStereoIsActive();
#else
  // HACK - TODO proper stereo mode setting
  /*
    set gl context to mono or stereo one
  */
  return m;
#endif
}

int guiQueryStereo(void)
{
#ifdef SGI_STEREO
  if(gui.stereo_available==SGI_STEREO_NONE)
    return 0;
  else
    return 1;
#else
  return gui.stereo_available;
#endif
}

void guiCMICallback(const cmiToken *t)
{
  const char **cp;
  int m;
  cp=(const char **)t->value; // for DS and OBJ commands
  if(t->target==CMI_TARGET_GUI) {
    switch(t->command) {
    case CMI_REFRESH: gui.redraw++; break;
    case CMI_MESSAGE: guiMessage((char *)t->value); break;
    case CMI_CHECKR: check_redraw(); break;
    case CMI_DS_NEW: if(gui.om_flag) {omAddDB(cp[0]);} break;
    case CMI_DS_DEL: if(gui.om_flag) {omDelDB(cp[0]);} break;
    case CMI_DS_REN: /*TODO*/ break;
    case CMI_OBJ_NEW: if(gui.om_flag) {omAddObj(cp[0],cp[1]);} break;
    case CMI_OBJ_DEL: if(gui.om_flag) {omDelObj(cp[0],cp[1]);} break;
    case CMI_OBJ_REN: /*TODO*/ break;
    case CMI_OBJ_SHOW: if(gui.om_flag) {omShowObj(cp[0],cp[1]);} break;
    case CMI_OBJ_HIDE: if(gui.om_flag) {omHideObj(cp[0],cp[1]);} break;
    }
  }
}

int guiSetStereo(int m)
{
  return set_stereo(m);
}


#ifndef INTERNAL_COLOR

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

#endif

/*
  inline or static functions
*/
#ifndef INTERNAL_COLOR
static int init_colordb()
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
#endif


/*
  find the best X11 visual available
*/

static XVisualInfo *init_visual(int dbl_flag, int use_stereo, int use_stencil, int use_accum, int cons_flag)
{
  int buf[64];
  int bufc=0,i,j,k;
  int depthi,redi,bluei,greeni,alphai,accumi;
  int d[]={16,12,8,6,4,1};
  int c[]={16,12,8,6,5,4,2,1};
  int a[]={8,4};
  int is,js,ks;
  char message[256];
  XVisualInfo *vis;

  buf[bufc++]=GLX_RGBA;

  // only if GLX theoretically supports stereo
#ifdef GLX_STEREO
  if(use_stereo) {
    buf[bufc++]=GLX_STEREO;
  } else {
    debmsg("stereo deactivated");
  }
#endif

  if(dbl_flag)
    buf[bufc++]=GLX_DOUBLEBUFFER;

  if(use_stencil) {
    buf[bufc++]=GLX_STENCIL_SIZE;
    buf[bufc++]=1;
  } else {
    debmsg("stencil buffer deactivated\n");
  }

  buf[bufc++]=GLX_DEPTH_SIZE;
  depthi=bufc++;
  buf[bufc++]=GLX_RED_SIZE;
  redi=bufc++;
  buf[bufc++]=GLX_BLUE_SIZE;
  bluei=bufc++;
  buf[bufc++]=GLX_GREEN_SIZE;
  greeni=bufc++;
  if(use_accum) {
    buf[bufc++]=GLX_ACCUM_RED_SIZE;
    accumi=bufc++;
    buf[bufc++]=GLX_ACCUM_BLUE_SIZE;
    bufc++;
    buf[bufc++]=GLX_ACCUM_GREEN_SIZE;
    bufc++;
  }
  buf[bufc++]=None;

  if(cons_flag) {
    is=2; // start at 8-bit z-depth
    js=3; // start at 5-bit color depth
    ks=1; // start at 4-bit accum
  } else {
    js=0;
    is=0;
    ks=0;
  }

  for(j=js;j<sizeof(c)/sizeof(int);j++) {
    for(i=is;i<sizeof(d)/sizeof(int);i++) {
      buf[depthi]=d[i];
      buf[redi]=c[j];
      buf[bluei]=c[j];
      buf[greeni]=c[j];
      if(use_accum) {
        for(k=ks;k<sizeof(a)/sizeof(int);k++) {
          buf[accumi+0]=a[k];
          buf[accumi+2]=a[k];
          buf[accumi+4]=a[k];
          sprintf(message,"trying visual with depth %d, rgba %d and accum %d",d[i],c[j],a[k]);
          debmsg(message);
          vis=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),buf);
          if(vis) {
            sprintf(message,"using this visual");
            debmsg(message);
            return vis;
          }
        }
      } else {
        sprintf(message,"trying visual with depth %d and rgba %d",d[i],c[j]);
        debmsg(message);
        vis=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),buf);
        if(vis) {
          sprintf(message,"using this visual");
          debmsg(message);
          return vis;
        }
      }
    }
  }
  debmsg("no suitable visual found");
  return NULL;
}

/*
  get the colormap for a specific visual
*/

static Colormap get_colormap(XVisualInfo *vinfo)
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

  glx_init
  ----------

  This event handler is called after the
  GLX window has been correctly set up
  by the system. Only now OpenGL calls
  are valid, and OpenGL is initialized
  through gl_init

*****************************************/

static void glx_init(Widget ww, XtPointer clientData, XtPointer call)
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

  debmsg("glx_init: creating context");
  gui.glxcontext = glXCreateContext(gui.dpy, gui.visinfo, 0, True);

  if (gui.glxcontext==NULL)
    XtAppError(gui.app,"Could not create GLX context");

  gui.glxwindow=XtWindow(ww);

  /*
    this patches the event mask
     to include OwnerGrab for
     the user popup to work!
  */

  XGetWindowAttributes(gui.dpy,gui.glxwindow,&xwa);
  xws.event_mask=xwa.your_event_mask | OwnerGrabButtonMask;
  XChangeWindowAttributes(gui.dpy,gui.glxwindow,CWEventMask,&xws);

  glXMakeCurrent(gui.dpy, gui.glxwindow, gui.glxcontext);

  gui.inside=0;

  /* init the GL params */
  //debmsg("glx_init: calling gfxGLInit");

#ifdef LINUX
  //gfxSetViewport();
#endif

  //  gfxGLInit();
  cmiInitGL();
  cmiResize(gui.win_width, gui.win_height);
}


/*****************************

  glx_expose
  ------------

  called after the GLX window
  has received an exposure
  request.

*****************************/

static void glx_expose(Widget ww, XtPointer clientData, XtPointer call)
{
  //  debmsg("redraw request");
  //debmsg("calling Redraw");
  //comRedraw();
  cmiRefresh();
}

/*****************************

  glx_resize
  ------------

  called after the GLX window
  has received a resize
  request.

*****************************/

static void glx_resize(Widget ww, XtPointer clientData, XtPointer call)
{
  GLwDrawingAreaCallbackStruct *callData;
  callData = (GLwDrawingAreaCallbackStruct*) call;
  gui.win_width=callData->width;
  gui.win_height=callData->height;

  cmiResize(gui.win_width,gui.win_height);
  cmiRefresh();
  
  /*
  gfxSetViewport();
  debmsg("resize request");
  debmsg("calling Redraw");
  gfxResizeEvent();
  comRedraw();
  */
}



/*****************************

  glx_input
  -----------

  called after the GLX window
  has received an input
  event, like  mouse or keyb
  event.

*****************************/

static void glx_input(Widget ww, XtPointer clientData, XtPointer call)
{
  cmiToken t;
  int val[5];
  int i;
  int dx,dy;
  //  double val;
  //struct timeval tp;
  //struct timezone tzp;
  //long dt;
  char kbuf[32];
  int kcount;
  KeySym key;


  /*
    reading pointer
  */
  XmDrawingAreaCallbackStruct *cd =
    (XmDrawingAreaCallbackStruct *) call;

  //gettimeofday(&tp,&tzp);

  /* 
     switch depending on event type
  */
  switch(cd->event->type){

    /*
  case KeyRelease:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;
    
    val[0]=CMI_INPUT_KEYBOARD;
    val[1]=CMI_BUTTON_PRESS;

    kcount=XLookupString(&(cd->event->xkey),kbuf,sizeof(kbuf),&key,NULL);
    kbuf[kcount]='\0';
    for(i=0;i<kcount;i++) {
      val[2]=(int)kbuf[i];
      cmiSubmit(&t);
    }
    break;
    */
  case KeyRelease:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;
    
    val[0]=CMI_INPUT_KEYBOARD;
    val[1]=CMI_BUTTON_RELEASE;

    kcount=XLookupString(&(cd->event->xkey),kbuf,sizeof(kbuf),&key,NULL);
    kbuf[kcount]='\0';
    switch(key) {
    case XK_Return:
    case XK_Linefeed:
    case XK_KP_Enter:
      val[2]=CMI_KEY_RETURN;
      cmiSubmit(&t);
      break;
    case XK_Delete:
      val[2]=CMI_KEY_DELETE;
      cmiSubmit(&t);
      break;
    case XK_Up:
      val[2]=CMI_KEY_UP;
      cmiSubmit(&t);
      break;
    case XK_Down:
      val[2]=CMI_KEY_DOWN;
      cmiSubmit(&t);
      break;
    case XK_Left:
      val[2]=CMI_KEY_LEFT;
      cmiSubmit(&t);
      break;
    case XK_Right:
      val[2]=CMI_KEY_RIGHT;
      cmiSubmit(&t);
      break;
    default:
      {
	for(i=0;i<kcount;i++) {
	  val[2]=(int)kbuf[i];
	  cmiSubmit(&t);
	}
      }
    }
    break;

  case ButtonPress:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[0]=CMI_INPUT_MOUSE;
    val[1]=CMI_BUTTON_PRESS;
    val[2]=0;
    if(cd->event->xmotion.state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(cd->event->xmotion.state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(cd->event->xmotion.state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;
    val[3]=cd->event->xbutton.x;
    val[4]=cd->event->xbutton.y;

    cmiSubmit(&t);

    gui.last_x=cd->event->xbutton.x;
    gui.last_y=cd->event->xbutton.y;
    //    memcpy(&gui.tp_button,&tp,sizeof(struct timezone));
    if(cd->event->xbutton.button==3) {
      if(gui.om_flag) {
	om.pwin=gui.user_menu;
	om.pflag=1;
	XMoveWindow(gui.dpy,gui.user_menu,
		    cd->event->xbutton.x_root,cd->event->xbutton.y_root);
	XMapRaised(gui.dpy,gui.user_menu);
      }
    }
    break;
  case ButtonRelease:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[0]=CMI_INPUT_MOUSE;
    val[1]=CMI_BUTTON_RELEASE;
    val[2]=0;
    if(cd->event->xmotion.state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(cd->event->xmotion.state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(cd->event->xmotion.state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;
    val[3]=cd->event->xbutton.x;
    val[4]=cd->event->xbutton.y;

    cmiSubmit(&t);

    dx=gui.last_x-cd->event->xbutton.x;
    dy=gui.last_y-cd->event->xbutton.y;
    /*********
	      dt=(long)(tp.tv_sec-gui.tp_button.tv_sec)*1000000+(tp.tv_usec-gui.tp_button.tv_usec);
	      if(dt<200000 ||
	      (dx==0 && dy==0 && dt<300000)) {
	      if(cd->event->xbutton.state & Button1Mask) {
	      if(cd->event->xbutton.state & ShiftMask)
	      comPick(cd->event->xmotion.x,cd->event->xmotion.y,1);
	      else
	      comPick(cd->event->xmotion.x,cd->event->xmotion.y,0);
	      }
	      }
    ************/

    if(cd->event->xbutton.button==3) {
      if(gui.om_flag) {
	om.pflag=0;
	XUnmapWindow(gui.dpy,gui.user_menu);
      }
    }

    break;
  case MotionNotify:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;
    
    val[0]=CMI_INPUT_MOUSE;
    val[1]=CMI_MOTION;
    val[2]=0;

    if(cd->event->xmotion.state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(cd->event->xmotion.state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(cd->event->xmotion.state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;

    val[3]=cd->event->xbutton.x;
    val[4]=cd->event->xbutton.y;

    cmiSubmit(&t);

    dx=gui.last_x-cd->event->xmotion.x;
    dy=gui.last_y-cd->event->xmotion.y;
    gui.last_x=cd->event->xmotion.x;
    gui.last_y=cd->event->xmotion.y;

    //comTransform(TRANS_MOUSE,cd->event->xmotion.state,0,dx);
    //comTransform(TRANS_MOUSE,cd->event->xmotion.state,1,dy);
    
    break;
  case EnterNotify:
    gui.inside=1;
    break;
  case LeaveNotify: 
    gui.inside=0;
    break;
  }
}

/*************************************

  extension_event
  --------------

  called from the main loop whenever
  an extension event is encountered

*************************************/

static int deviceMotionNotify=0,deviceButtonPress=0,
    deviceButtonPressGrab=0,deviceButtonRelease=0;

static void extension_event(Widget w, XtPointer client_data, XEvent *event)
{
  int i,current_val;
  static int val_save[] = {0,0,0,0,0,0};
  static int val_count = 0;
  static int val_max = 1;
  static int val_thres=3;
  XDeviceMotionEvent *device_motion;
  static int dial_state[] = {0,0,0,0,0,0,0,0};

  // use CMI to transmit event!
  cmiToken t;
  int val[5],num,diff;

  if(event->type == deviceMotionNotify){
    device_motion=(XDeviceMotionEvent *)event;

    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[1]=CMI_MOTION;

    val[2]=0;
   
    if(device_motion->state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(device_motion->state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(device_motion->state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(device_motion->state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(device_motion->state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(device_motion->state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(device_motion->state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;

    
    if(gui.spaceballDevice!=NULL) {
      if(device_motion->deviceid==gui.spaceballDevice->device_id) {
	if(device_motion->first_axis==0) {
	  if(device_motion->axis_data[0]!=0) {

	    if(val_count>=val_max) {
	      val[0]=CMI_INPUT_SPACEBALL;
	      for(i=0;i<6;i++) {
		val[3]=i;
		val[4]=val_max*(int)((float)val_save[i]/(float)val_max);
		cmiSubmit(&t);
	      } 
	      val_count=0;
	    } else {
	      for(i=0;i<6;i++) {
		current_val=device_motion->axis_data[i]; 
		val_save[i]=current_val*3;
	      }
	      val_count++;
	    }
	  }
	}
      }
    }

    if(gui.dialsDevice!=NULL) {
      if(device_motion->deviceid==gui.dialsDevice->device_id) {
	num = device_motion->first_axis;
	diff=(int)device_motion->axis_data[0]-dial_state[num];
	dial_state[num]=(int)device_motion->axis_data[0];
	val[0]=CMI_INPUT_DIALBOX;
	val[3]=num;
	val[4]=diff;
	cmiSubmit(&t);
      }
    }
  }

    /*
    fprintf(stderr,"\nfa: %d  ac: %d",num, device_motion->axes_count);
    for(i=0;i<6;i++)
      fprintf(stderr,"[#%d: %d]",i,device_motion->axis_data[i]);
    */
    // TODO DIALS
    /********
    if(gui.dialsDevice!=NULL)
      if(device_motion->deviceid==gui.dialsDevice->device_id) {
	diff=(int)device_motion->axis_data[0]-gui.last_dial[num];
	gui.last_dial[num]=(int)device_motion->axis_data[0];
      }
    *******/
    // TODO SPACEBALL
    /**********
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
    *********/
	  
}


static int dialbox_init()
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

static int spaceball_init()
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

    if((gui.spaceballDevice=extSpaceballInit(gui.dpy))==NULL)
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
    
    //XSelectExtensionEvent(gui.dpy,gui.glxwindow,eventList,4);
    if(gui_demo_flag) {
      XSelectExtensionEvent(gui.dpy,RootWindow(gui.dpy,0),eventList,4);
    } else {
      XSelectExtensionEvent(gui.dpy,XtWindow(gui.frame),eventList,4);
    }    
    
    return 1;
#ifdef SPACETEC
  }
#endif
}


static int error_handler(Display *d, XErrorEvent *e)
{
  char buffer[256];
  XGetErrorText(d,e->error_code,buffer,255);
  fprintf(stderr,"WARNING: Caught X error: %s\n",buffer);
  return 0;
}

static int error_io_handler(Display *d)
{
  fprintf(stderr,"Caught fatal X error - exiting\n");
  dinoExit(-1);
  return 0;
}


static void handle_drop(Widget w, XtPointer client_data, XtPointer call_data)
{
}

static void register_dnd(Widget site)
{
  Atom    importList [1];
  Arg     args [4];
  int      nargs;
  Atom    COMPOUND_TEXT;

  COMPOUND_TEXT = XmInternAtom(XtDisplay(site), 
			       "COMPOUND_TEXT", False);
  importList[0] = COMPOUND_TEXT;
  nargs = 0;

  XtSetArg(args [nargs], XmNimportTargets, importList); nargs++;
  XtSetArg(args [nargs], XmNnumImportTargets, 1); nargs++;
  XtSetArg(args [nargs], XmNdropSiteOperations, XmDROP_COPY); nargs++;
  XtSetArg(args [nargs], XmNdropProc, handle_drop); nargs++;
  XmDropSiteRegister(site, args, nargs);
}




// NOT USED

/************************************

  xinput_ext_init
  --------------

  look for a dialbox and set it up

**************************************/


static int xinput_ext_init()
{
  XEventClass  dMotionNotifyClass,dButtonPressClass,
    dButtonPressGrabClass,dButtonReleaseClass;
  XEventClass eventList[4];

  // check for a dialbox
  
  if((gui.dialsDevice=extFindDevice(gui.dpy,"dialbox"))==NULL) {
    if((gui.dialsDevice=extFindDevice(gui.dpy,"dial+buttons"))==NULL) {
      return 0;
    }
  }

  if(gui.dialsDevice) {
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
  }

  // look for a spaceball
  
  return 1;
}

/*
  setup the pads

  NOT WORKING YET
  TODO
*/

static int pad_init()
{
  gui.pad1v=8;
  gui.pad2u=2;
  gui.pad2v=3;
  /*  
  gui.pad1 = XCreateSimpleWindow(gui.dpy,DefaultRootWindow(gui.dpy),
				 0,0,100,50*gui.pad1v,1,
				 BlackPixel(gui.dpy,DefaultScreen(gui.dpy)),
				 WhitePixel(gui.dpy,DefaultScreen(gui.dpy)));

  XMapWindow(gui.dpy,gui.pad1);
  */
  return 0;
}


int guiCreateOffscreenContext(int w, int h, int af)
{
  int i;
  XVisualInfo *visinfo;
  struct _OFFSCREEN_CONTEXT *oc;

  for(i=0;i<MAX_OFFSCREEN_CONTEXT;i++)
    if(!offscreen_context_list[i].used) {
      break;
    }
  
  if(i==MAX_OFFSCREEN_CONTEXT) {
    outMessage("error: maximal offscreen context count reached");
    return -1;
  }
  
  oc=&offscreen_context_list[i];

  // get visual
  debmsg("retrieving offscreen visual");
  if((visinfo=get_offscreen_visual(af))==NULL) {
    outMessage("error: failed to find visual for offscreen rendering\n");
    return -1;
  }
  
  // create new context
  debmsg("creating new context");
  if((oc->glx_context=glXCreateContext(gui.dpy, visinfo, 0, False))==NULL) {
    outMessage("error: offscreen rendering context could not be created");
    return -1;
  }

  // create X pixmap
  debmsg("creating X pixmap");
  oc->pm=XCreatePixmap(gui.dpy,gui.glxwindow,w,h,visinfo->depth);

  // convert to glx pixmap
  debmsg("converting to glx pixmap");
  oc->glx_pm=glXCreateGLXPixmap(gui.dpy, visinfo, oc->pm);

  if(glXMakeCurrent(gui.dpy,oc->glx_pm,oc->glx_context)==False) {
    outMessage("glXMakeCurrent failed for offscreen rendering context");
  }

  oc->used=1;
  return i;
}

int guiDestroyOffscreenContext(int c)
{
  struct _OFFSCREEN_CONTEXT *oc;
  if(c<MAX_OFFSCREEN_CONTEXT) {
    if(offscreen_context_list[c].used) {
      oc=&offscreen_context_list[c];
      glXMakeCurrent(gui.dpy,gui.glxwindow,gui.glxcontext);
      glXDestroyGLXPixmap(gui.dpy,oc->glx_pm);
      XFreePixmap(gui.dpy,oc->pm);
      oc->used=0;
    }
  }
  return 0;
}

static XVisualInfo *get_offscreen_visual(int af)
{
  XVisualInfo *vi=NULL;
  int use_stencil;
  int consflag;

  if(gfx_flags | DINO_FLAG_NOSTENCIL) {
    use_stencil=0;
  } else {
    use_stencil=1;
  }

  if(gfx_flags | DINO_FLAG_CONS) {
    consflag=1;
  } else {
    consflag=0;
  }

  if(af) {
    vi=init_visual(0,0,use_stencil,1,consflag);
    if(!vi) {
      outMessage("could not find visual with accumulation buffer\n");
    }
  } else {
    vi=init_visual(0,0,use_stencil,0,consflag);
  }
  return vi;
}

void guiExit()
{
  glXWaitX();
  glFinish();
  glXWaitGL();
  XSync(gui.dpy,True);
  XCloseDisplay(gui.dpy);
}

int guiGrab(int s)
{
  if(s) {
    XtGrabPointer(gui.glxwin, 
		  True, 0, GrabModeAsync,GrabModeAsync,None,None,CurrentTime);
  } else {
    XtUngrabPointer(gui.glxwin,CurrentTime);
  }
  return 0;
}

static void hide_cursor()
{
#if 0
  char bm[]={0,0,0,0,0,0,0,0};
  Pixmap pix = XCreateBitmapFromData(gui.dpy,XtWindow(gui.glxwin),bm,8,8);
  XColor black;
  memset(&black,0,sizeof(XColor));
  black.flags = DoRed | DoGreen | DoBlue;
  Cursor pointer = XCreatePixmapCursor(gui.dpy,pix,pix,&black,&black,0,0);
  XFreePixmap(gui.dpy,pix);
  XDefineCursor(gui.dpy,XtWindow(gui.glxwin),pointer);
#endif
}

static void show_cursor()
{

}
