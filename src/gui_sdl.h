#include <sys/time.h>
#include <SDL/SDL.h>

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

struct GUI_KEY_EVENT {
  int key;
  char command[256];
};

struct GUI
{
  int win_width,win_height;
  int bpp;
  int sdl_flags;

  int inside;
  int redraw;

  int fixz;
  double trans_fact;

  int stereo_available;

  int stereo_mode;
  int stereo_y_offset;
  double eye_dist, eye_offset;

  char message_string[256],message_string2[256];

  int om_flag;

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

  struct GUI_KEY_EVENT key_event[GUI_MAX_KEY_EVENTS];

};

int guiInit(void (*)(int, char **), int*, char ***);
int guiMInit(void (*)(int, char **), int*, char ***);

int guiMainLoop(void);

int guiResolveColor(const char *name, float *r, float *g, float *b);

int guiMessage(char *m);

void guiSwapBuffers(void);

#endif
