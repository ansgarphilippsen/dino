/************************************************

dino: gui_glut.c

Responsible for the graphical user interface
Uses GLUT

*************************************************/

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

#include "dino.h"
#include "com.h"
#include "gui.h"
#include "gfx.h"
#include "extension.h"
#include "glw.h"
#include "om_glut.h"
#include "input.h"
#include "transform.h"
#include "cl.h"
#include "AppPlus.h"

#ifdef SPACETEC
#include "spacetec.h"
#endif

struct GUI gui;
extern struct GFX gfx;

extern int debug_mode,gfx_mode,stereo_mode;

#ifdef CORE_DEBUG
extern FILE *core_debug;
#endif

static void guiReshape(int width, int height)
{
  gui.win_width=width;
  gui.win_height=height;
  glutSetWindow(gui.glut_status);
  glutReshapeWindow(width,20);
  glutPositionWindow(0,height-20);
  glutSetWindow(gui.glut_main);
  
  gfxSetViewport();
  comRedraw();
}

static void guiTimer(int value)
{
  if(gui.redraw) {
    gui.redraw=0;
    gfxRedraw();
  }
  comTimeProc();
  glutTimerFunc(5,guiTimer,0);
}

static void guiMouseFunc(int button, int state, int x, int y)
{
  struct timeval tp;
  struct timezone tzp;
  long tc,dt;
  int dx,dy;
  int mod=glutGetModifiers();

  gui.modifiers=mod;

  gettimeofday(&tp,&tzp);
  tc=tp.tv_sec*1000000L+tp.tv_usec;

  dx=gui.last_x-x;
  dy=gui.last_y-y;

  if(button==GLUT_LEFT_BUTTON) {
    gui.mbs[0]=state;

    if(state==GLUT_UP && gui.mbs[1]==GLUT_UP && gui.mbs[2]==GLUT_UP) {
      dt=tc-gui.timecode;
      if(dt<200000 || 
	 (dx==0 && dy==0 && dt<300000)) {
	glutSetWindow(gui.glut_main);
	comPick(x,y,0);
      }
    }
  }

  gui.timecode=tc;
  
  if(button==GLUT_MIDDLE_BUTTON) {
    gui.mbs[1]=state;
  }

  if(button==GLUT_RIGHT_BUTTON) {
    gui.mbs[2]=state;
  }

  gui.last_x=x;
  gui.last_y=y;
  gfx.sx=x;
  gfx.sy=y;
  gfx.sdx=0;
  gfx.sdy=0;
}


static void guiMotionFunc(int x, int y)
{
  int dx,dy;
  int mod=gui.modifiers;
  int mask=0;

  dx=gui.last_x-x;
  dy=gui.last_y-y;
  gfx.sx=x;
  gfx.sy=y;
  gui.last_x=x;
  gui.last_y=y;

  if(gui.mbs[0]==GLUT_DOWN)
    mask+=Button1Mask;
  if(gui.mbs[1]==GLUT_DOWN)
    mask+=Button2Mask;
  if(gui.mbs[2]==GLUT_DOWN)
    mask+=Button3Mask;

  if(mod & GLUT_ACTIVE_SHIFT)
    mask+=ShiftMask;
  if(mod & GLUT_ACTIVE_CTRL)
    mask+=ControlMask;
  //  if(mod & GLUT_ALT_SHIFT)
  //  mask+=ShiftMask;

  comTransform(TRANS_MOUSE, mask, 0, dx);
  comTransform(TRANS_MOUSE, mask, 1, dy);
}

static void guiKeyboardFunc(unsigned char key, int x, int y)
{
  comWriteCharBuf(key);
}

static void guiSpecialFunc(int key, int x, int y)
{
  switch(key) {
  case GLUT_KEY_UP: 
    comWriteCharBuf(27);
    comWriteCharBuf('[');
    comWriteCharBuf('A');
    break;
  case GLUT_KEY_DOWN: 
    comWriteCharBuf(27);
    comWriteCharBuf('[');
    comWriteCharBuf('B');
    break;
  case GLUT_KEY_LEFT: 
    comWriteCharBuf(27);
    comWriteCharBuf('[');
    comWriteCharBuf('D');
    break;
  case GLUT_KEY_RIGHT:
    comWriteCharBuf(27);
    comWriteCharBuf('[');
    comWriteCharBuf('C');
    break;
  }
}


static char *um_list[]={
  "  autoslab  ","scene autoslab",
  "  center CP  ","scene center $CP"
};

static int um_list_count=2;

static void um_cb(int value)
{
  char com[256];
  sprintf(com,"%s",um_list[value*2+1]);
  glutSetWindow(gui.glut_main);
  comRawCommand(com);
}

static void draw_string(const char *s)
{
  int i;
  for(i=0;i<clStrlen(s);i++) 
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15,s[i]);
}

static void guiStatusExpose()
{
  int i,w,h;

  glutSetWindow(gui.glut_status);
  w=glutGet(GLUT_WINDOW_WIDTH);
  h=glutGet(GLUT_WINDOW_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,w,h,0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor3f(1.0,1.0,1.0);
  glRectd(0,0,w,h);
  glColor3f(0.0,0.0,0.0);
  glRasterPos2d(1,16);
  draw_string(gui.message_string);
  glutSwapBuffers();
  glutSetWindow(gui.glut_main);
}

static int guiInitRGB()
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


int guiInit(void (*func)(int, char **), int *argc, char ***argv)
{
  int sw,sh,i;

  clStrcpy(gui.message_string,"Ready");

  guiInitRGB();

  debmsg("glutInit()");
  glutInit(argc,(*argv));

  sw=glutGet(GLUT_SCREEN_WIDTH);
  sh=glutGet(GLUT_SCREEN_HEIGHT);

  debmsg("glut: setting up main window");
  /* main gfx window */
  glutInitWindowSize(sh,sh);
  glutInitWindowPosition(sw-sh,0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

  if(glutDeviceGet(GLUT_HAS_SPACEBALL)) {
    fprintf(stderr,"glut spaceball detected\n");
  }

  gui.glut_main=glutCreateWindow("dino gfx");

  glutDisplayFunc(comRedraw);
  glutReshapeFunc(guiReshape);
  glutMouseFunc(guiMouseFunc);
  glutMotionFunc(guiMotionFunc);
  glutKeyboardFunc(guiKeyboardFunc);
  glutSpecialFunc(guiSpecialFunc);
 
  glutTimerFunc(5,guiTimer,0);

  debmsg("glut: setting up user menu");
  // user menu

  gui.glut_um=glutCreateMenu(um_cb);
  for(i=0;i<um_list_count;i++)
    glutAddMenuEntry(um_list[i*2],i);

  glutAttachMenu(GLUT_RIGHT_BUTTON);

  debmsg("glut: setting up status bar");
  /* status window */
  gui.glut_status=glutCreateSubWindow(gui.glut_main,0,500-20,500,20);
  glutDisplayFunc(guiStatusExpose);

  debmsg("glut: setting up object menu");
  /* object menu window */
  omInit();
  gui.om_flag=1;

  debmsg("glut: setting main focus");
  /* set back to main gfx */
  glutSetWindow(gui.glut_main);


  gui.callback=func;
  gui.redraw=0;
  gui.stereo_available=0;
  gui.stereo_mode=GUI_STEREO_OFF;
  gui.eye_dist=150.0;
  gui.eye_offset=10.0;
  gui.mbs[0]=GLUT_UP;
  gui.mbs[1]=GLUT_UP;
  gui.mbs[2]=GLUT_UP;

  gfxGLInit();

  return 0;
}

int guiMainLoop()
{
  glutMainLoop();
  return 0;
}


// END OF GLUT STUFF

// CODE BELOW HERE NEEDS REWORK !


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
  clStrncpy(gui.message_string,m,256);
  glutSetWindow(gui.glut_status);
  glutPostRedisplay();
  glutSetWindow(gui.glut_main);
  
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
#ifndef GLUT_GUI
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
      
#endif
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

void guiSwapBuffers()
{
  glutSwapBuffers();
}
