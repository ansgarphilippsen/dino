#ifndef _GUI_OSX_H
#define _GUI_OSX_H

#import <Cocoa/Cocoa.h>
#include <sys/time.h>

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

  void (*callback)(int, char **);

  int last_x, last_y;
  struct timeval tp_button;

  struct GUI_KEY_EVENT key_event[GUI_MAX_KEY_EVENTS];
};

void guiTimeProc();

#endif
