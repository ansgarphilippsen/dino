#ifndef _GUI_X11_H
#define _GUI_X11_H

#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/Intrinsic.h>

#include <GL/glx.h>

#ifdef LINUX
#include "GLwMDrawA.h"
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
#ifdef SUN
#include <ndbm.h>
#endif
#endif


#include "gui_ext.h"


#define GUI_NONE 0
#define GUI_MOUSE 1
#define GUI_DIALS 2
#define GUI_SPACEBALL 3

#define GUI_MAX_EVENT_MAPS 30
#define GUI_MAX_KEY_EVENTS 64

// some generic stuff
#define GUI_SHIFT_MASK   (1<<0)
#define GUI_LOCK_MASK    (1<<1)
#define GUI_CNTRL_MASK   (1<<2)
#define GUI_MOD1_MASK    (1<<3)
#define GUI_MOD2_MASK    (1<<4)
#define GUI_MOD3_MASK    (1<<5)
#define GUI_MOD4_MASK    (1<<6)
#define GUI_MOD5_MASK    (1<<7)
#define GUI_BUTTON1_MASK (1<<8)
#define GUI_BUTTON2_MASK (1<<9)
#define GUI_BUTTON3_MASK (1<<10)
#define GUI_BUTTON4_MASK (1<<11)
#define GUI_BUTTON5_MASK (1<<12)


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

  int glxvisinfo[64],glxvisinfo_count;

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

  int last_x, last_y;
  struct timeval tp_button;

#ifndef INTERNAL_COLOR
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
#endif
  struct GUI_CUSTOM_EVENT ce;

  struct GUI_KEY_EVENT key_event[GUI_MAX_KEY_EVENTS];

#ifdef EXPO
  int idle;
#endif
};

// other declarations already in gui_ext.h

int guiMessage2(char *m);

void guiRegisterCustomEvent(Window w, guiCustomFunc f, void *ptr);
void guiRegisterUserMenu(Window w);
int guiCheckCustomEvent(XEvent *event);

void guiTimeProc(XtPointer client_data);

#endif
