#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Intrinsic.h>

#ifdef LINUX
#ifdef USE_MESA
#include <MesaGL/GLwMDrawA.h>
#else
#include "GLwMDrawA.h"
#endif
#endif

#ifdef SGI
#include <X11/GLw/GLwMDrawA.h>
#endif
#ifdef DEC
#include <X11/GLw/GLwMDrawA.h>
#endif

#ifdef SUN
#include <GL/GLwMDrawA.h>
#endif

#ifdef USE_MESA
#include <MesaGL/glx.h>
#else
#include <GL/glx.h>
#endif

#ifdef LINUX
#include <gdbm.h>
#endif
#ifdef SGI
#include <ndbm.h>
#endif
#ifdef DEC
#include <ndbm.h>
#endif

#ifdef SUN
#include <ndbm.h>
#endif

#ifndef GUI_H
#define GUI_H

#define GUI_NONE 0
#define GUI_MOUSE 1
#define GUI_DIALS 2
#define GUI_SPACEBALL 3

#define GUI_MAX_EVENT_MAPS 30
#define GUI_MAX_KEY_EVENTS 64

enum {GUI_STEREO_OFF=0, GUI_STEREO_NORMAL=1, GUI_STEREO_SPLIT=2};

enum {GLX_DBL_1, GLX_DBL_2, GLX_DBL_3, GLX_STE_1, GLX_STE_2, GLX_STE_3};

struct GUI_EVENT_MAP {
  int device;
  int axis;
  int device_state;
  int command;
  double factor;
};

typedef void (*guiCustomFunc)(Window w,XEvent *event,void *ptr);


struct GUI_CUSTOM_EVENT_ENTRY {
  Window w;
  guiCustomFunc f;
  void *ptr;
};

struct GUI_CUSTOM_EVENT {
  struct GUI_CUSTOM_EVENT_ENTRY entry[1024];
  int entry_count;
};

struct GUI_KEY_EVENT {
  int key;
  char command[256];
};

struct GUI
{
  XtAppContext app;
  Display *dpy;
  
  XVisualInfo *visinfo;
  Position win_x,win_y;
  Position win_xs,win_ys;
  Dimension win_width,win_height;
  Dimension win_widths,win_heights;
  Colormap cmap;
  GLXContext glxcontext;
  Window glxwindow;
  int glx_vis;

  int inside;
  int redraw;

  int fixz;
  double trans_fact;

  int stereo_available;
  XVisualInfo *stereo_visinfo;
  GLXContext stereo_glxcontext;
  int stereo_mode;
  int stereo_y_offset;
  double eye_dist, eye_offset;

  Widget top,form,frame,glxwin,menu,mform,message,message2;
  Widget menu_file, menu_filec, menu_fileb[2];
  Widget menu_help;

  Window user_menu;

  Window pad1, pad2;
  int pad1v, pad2u,pad2v;

  char message_string[256],message_string2[256];

  int om_flag;

  XDevice *dialsDevice;
  int last_dial[32];

  int spacetecDevice;

  XDevice *spaceballDevice;
  int last_spaceball[32];

  int xiEventBase;

  void (*callback)(int, char **);
#ifdef GLUT_GUI
  int glut_main;
  int glut_status;
  int glut_om;
  int glut_um;
  int mbs[3];
  int modifiers;
  long timecode;
#endif
  int last_x, last_y;
  struct timeval tp_button;

#ifdef LINUX
  GDBM_FILE cdbm;
  char gdbm_tmpfile[256];
#endif
#ifdef SGI
  DBM *cdbm;
#endif
#ifdef DEC
  DBM *cdbm;
#endif
#ifdef SUN
  DBM *cdbm;
#endif

  struct GUI_CUSTOM_EVENT ce;

  struct GUI_KEY_EVENT key_event[GUI_MAX_KEY_EVENTS];

#ifdef EXPO
  int idle;
#endif
};

int guiInit(void (*)(int, char **), int*, char ***);
int guiMInit(void (*)(int, char **), int*, char ***);

int guiMainLoop(void);

int guiResolveColor(const char *name, float *r, float *g, float *b);

int guiMessage(char *m);
int guiMessage2(char *m);

void guiRegisterCustomEvent(Window w, guiCustomFunc f, void *ptr);
void guiRegisterUserMenu(Window w);
int guiCheckCustomEvent(XEvent *event);

void guiTimeProc(XtPointer client_data);

void guiSwapBuffers(void);

#endif
