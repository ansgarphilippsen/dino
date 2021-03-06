#ifndef _GUI_GLUT_H
#define _GUI_GLUT_H

#include <sys/time.h>

#include "cmi.h"

#ifdef USE_FREEGLUT
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
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

struct GUI
{
  int win_width,win_height;
  int glx_vis;

  int inside;
  int redraw;

  int fixz;
  double trans_fact;

  int stereo_available;

  int stereo_mode;
  int stereo_y_offset;
  double eye_dist, eye_offset;

  int pad1v, pad2u,pad2v;

  char message_string[256],message_string2[256];

  int om_flag;

  int last_dial[32];

  int spacetecDevice;

  int last_spaceball[32];

  int xiEventBase;

  void (*callback)(int, char **);

  int glut_main;
  int glut_status;
  int glut_om;
  int glut_um;
  int mbs[3];
  int modifiers;
  long timecode;

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

};

int guiMInit(void (*)(int, char **), int*, char ***);

int guiMessage2(char *m);



#endif
