/************************************************

dino: gui_glut.c

Responsible for the graphical user interface
Uses GLUT

*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifndef INTERNAL_COLOR
#ifdef LINUX
#include <gdbm.h>
#endif
#ifdef OSX
#include <ndbm.h>
#endif
#ifdef SGI
#include <ndbm.h>
#endif
#ifdef DEC
#include <ndbm.h>
#endif
#endif

#include "dino.h"
#include "gui.h"
#include "om_glut.h"
#include "cl.h"

#include "cmi.h"

struct GUI gui;

extern int debug_mode,gfx_mode,stereo_mode;

#ifdef CORE_DEBUG
extern FILE *core_debug;
#endif

static void gui_redraw()
{
  gui.redraw++;
}

static void gui_reshape(int width, int height)
{
  cmiToken t;
  int val[2];

  
  gui.win_width=width;
  gui.win_height=height;

  t.target=CMI_TARGET_GFX;
  t.command=CMI_RESIZE;
  t.value=val;
  val[0]=width;
  val[1]=height;
  cmiSubmit(&t);

  glutSetWindow(gui.glut_status);
  glutReshapeWindow(width,20);
  glutPositionWindow(0,height-20);
  glutSetWindow(gui.glut_main);
  
  gui_redraw();
}


static void gui_timer(int value)
{
  cmiToken t;

  if(gui.redraw) {
    gui.redraw=0;
    glutSetWindow(gui.glut_main);
    t.target=CMI_TARGET_GFX;
    t.command=CMI_REDRAW;
    t.value=NULL;
    cmiSubmit(&t);
  }

  cmiTimer();

  glutTimerFunc(5,gui_timer,0);
}

static void gui_mouse_func(int button, int state, int x, int y)
{
  cmiToken t;
  int val[5];
  int mod=glutGetModifiers();

  gui.modifiers=mod;
  
  t.target=CMI_TARGET_COM;
  t.command=CMI_INPUT;
  t.value=val;

  val[0]=CMI_INPUT_MOUSE;

  if(state==GLUT_UP) {
    val[1]=CMI_BUTTON_PRESS;
  } else {
    val[1]=CMI_BUTTON_RELEASE;
  }

  val[2]=0;
  if(mod & GLUT_ACTIVE_SHIFT)
    val[2]+=CMI_SHIFT_MASK;
  if(mod & GLUT_ACTIVE_CTRL)
    val[2]+=CMI_CNTRL_MASK;

  switch(button) {
  case GLUT_LEFT_BUTTON: val[2]+=CMI_BUTTON1_MASK ; break;
  case GLUT_MIDDLE_BUTTON: val[2]+=CMI_BUTTON3_MASK ; break;
  case GLUT_RIGHT_BUTTON: val[2]+=CMI_BUTTON2_MASK ; break;
  }

  val[3]=x;
  val[4]=y;

  cmiSubmit(&t);

  if(button==GLUT_LEFT_BUTTON) {
    gui.mbs[0]=state;
  }
  if(button==GLUT_MIDDLE_BUTTON) {
    gui.mbs[1]=state;
  }
  if(button==GLUT_RIGHT_BUTTON) {
    gui.mbs[2]=state;
  }

}

static void gui_motion_func(int x, int y)
{
  cmiToken t;
  int val[5];
  int mod=gui.modifiers;

  t.target=CMI_TARGET_COM;
  t.command=CMI_INPUT;
  t.value=val;

  val[0]=CMI_INPUT_MOUSE;
  val[1]=CMI_MOTION;
  val[2]=0;
  if(gui.mbs[0]==GLUT_DOWN)
    val[2]+=CMI_BUTTON1_MASK;
  if(gui.mbs[1]==GLUT_DOWN)
    val[2]+=CMI_BUTTON2_MASK;
  if(gui.mbs[2]==GLUT_DOWN)
    val[2]+=CMI_BUTTON3_MASK;

  if(mod & GLUT_ACTIVE_SHIFT)
    val[2]+=CMI_SHIFT_MASK;
  if(mod & GLUT_ACTIVE_CTRL)
    val[2]+=CMI_CNTRL_MASK;

  val[3]=x;
  val[4]=y;

  cmiSubmit(&t);
}

static void gui_keyboard_func(unsigned char key, int x, int y)
{
  cmiToken t;
  int val[3];

  t.target=CMI_TARGET_COM;
  t.command=CMI_INPUT;
  t.value=val;

  val[0]=CMI_INPUT_KEYBOARD;
  val[1]=CMI_BUTTON_RELEASE;
  val[2]=(int)key;

  cmiSubmit(&t);
}

static void gui_special_func(int key, int x, int y)
{
  cmiToken t;
  int val[3];

  t.target=CMI_TARGET_COM;
  t.command=CMI_INPUT;
  t.value=val;

  val[0]=CMI_INPUT_KEYBOARD;
  val[1]=CMI_BUTTON_RELEASE;

  gui.modifiers=glutGetModifiers();

  switch(key) {
  case GLUT_KEY_UP:   
    val[2]=(int)27; cmiSubmit(&t);
    val[2]=(int)'['; cmiSubmit(&t);
    val[2]=(int)'A'; cmiSubmit(&t);
    break;
  case GLUT_KEY_DOWN: 
    val[2]=(int)27; cmiSubmit(&t);
    val[2]=(int)'['; cmiSubmit(&t);
    val[2]=(int)'B'; cmiSubmit(&t);
    break;
  case GLUT_KEY_LEFT: 
    val[2]=(int)27; cmiSubmit(&t);
    val[2]=(int)'['; cmiSubmit(&t);
    val[2]=(int)'D'; cmiSubmit(&t);
    break;
  case GLUT_KEY_RIGHT:
    val[2]=(int)27; cmiSubmit(&t);
    val[2]=(int)'['; cmiSubmit(&t);
    val[2]=(int)'C'; cmiSubmit(&t);
    break;
  }
}

static void gui_spaceball_motion_func(int x, int y, int z)
{
  cmiToken t;
}

static void gui_spaceball_rotation_func(int x, int y, int z)
{
    cmiToken t;
}

static void gui_dials_func(int d, int v)
{
  cmiToken t;
}


static char *um_list[]={
  "  autoslab  ","scene autoslab",
  "  center CP  ","scene center $CP"
};

static int um_list_count=2;

static void um_cb(int value)
{

  cmiToken t;

  char com[256];
  sprintf(com,"%s",um_list[value*2+1]);
  glutSetWindow(gui.glut_main);

  t.target=CMI_TARGET_COM;
  t.command=CMI_RAW;
  t.value=com;
  cmiSubmit(&t);
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


#ifndef INTERNAL_COLOR
static int guiInitRGB()
{
  return -1;
}
#endif

int guiInit(int *argc, char ***argv)
{
  cmiToken t;
  int sw,sh,i;

  cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback);
  clStrcpy(gui.message_string,"Ready");

#ifndef INTERNAL_COLOR
  guiInitRGB();
#endif

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

  glutDisplayFunc(gui_redraw);
  glutReshapeFunc(gui_reshape);
  glutMouseFunc(gui_mouse_func);
  glutMotionFunc(gui_motion_func);
  glutKeyboardFunc(gui_keyboard_func);
  glutSpecialFunc(gui_special_func);
  glutSpaceballMotionFunc(gui_spaceball_motion_func);
  glutSpaceballRotateFunc(gui_spaceball_rotation_func);
  glutDialsFunc(gui_dials_func);


  debmsg("glut: setting up user menu");
  // user menu

#ifndef OSX 
  gui.glut_um=glutCreateMenu(um_cb);
  for(i=0;i<um_list_count;i++)
    glutAddMenuEntry(um_list[i*2],i);

  glutAttachMenu(GLUT_RIGHT_BUTTON);
#endif
  
  debmsg("glut: setting up status bar");
  /* status window */
  gui.glut_status=glutCreateSubWindow(gui.glut_main,0,sh-20,sh,20);
  glutDisplayFunc(guiStatusExpose);

#ifndef OSX
  debmsg("glut: setting up object menu");
  /* object menu window */
  omInit();
  gui.om_flag=1;
#endif

  debmsg("glut: setting main focus");
  /* set back to main gfx */
  glutSetWindow(gui.glut_main);

  gui.redraw=0;
  gui.stereo_available=0;
  gui.stereo_mode=GUI_STEREO_OFF;
  gui.eye_dist=150.0;
  gui.eye_offset=10.0;
  gui.mbs[0]=GLUT_UP;
  gui.mbs[1]=GLUT_UP;
  gui.mbs[2]=GLUT_UP;

#ifdef USE_FREEGLUT
  fprintf(stdout,"Using freeglut (http://freeglut.sf.net)\n");
#else
  fprintf(stdout,"Using GLUT (c) Mark J. Kilgard\n");
#endif

  t.target=CMI_TARGET_GFX;
  t.command=CMI_INITGL;
  t.value=NULL;
  cmiSubmit(&t);

  glutTimerFunc(100,gui_timer,0);

  return 0;
}

void guiCMICallback(const cmiToken *t)
{
  if(t->target==CMI_TARGET_GUI) {
    switch(t->command) {
    case CMI_REFRESH: gui.redraw++; break;
    case CMI_MESSAGE: guiMessage((char *)t->value); break;
    }
  }
}

int guiMainLoop()
{
  glutMainLoop();
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
  return 0;
}

void guiSwapBuffers()
{
  glutSwapBuffers();
}


#ifndef INTERNAL_COLOR
/************************

  guiResolveColor
  ---------------

  translates a string
  into rgb

************************/

// ALSO needs to be rewritten to work with GLUT

int guiResolveColor(const char *oname, float *r, float *g, float *b)
{
    (*r)=0.0;
    (*g)=0.0;
    (*b)=0.0;
    return -1;
}
#endif
