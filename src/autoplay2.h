#include <stdio.h>
#include <math.h>

#include <X11/Intrinsic.h>

#ifndef AUTOPLAY_H
#define AUTOPLAY_H

/*
  IDLE1: approximately in 1/10 seconds
*/

#define AUTOPLAY_IDLE0 1000
#define AUTOPLAY_IDLE1 50
#define AUTOPLAY_IDLE2 2
#define AUTOPLAY_IDLE3 2

#define rnd() ((double)random()/(pow(2,31)-1))
#define rnd2() (rnd()*4.0-2.0)

struct AP_BUTTON {
  Widget w;
  char name[64];
  char help[256];
  char scr[256],inf[256];
};

struct AP_ENTRY {
  Widget w;
  char name[64];
  char help[256];
  struct AP_BUTTON b[32];
  XmString slist[32];
  int bc;
  char on_load[256],on_activate[256],inf[256],dir[256];
};

struct AUTOPLAY {
  int flag;
  FILE *f;

  Widget frame,form;
  Widget area1,area2,area3,area4,area5;

  Widget helpb, helpd1,helpd2;
  GC help_gc;

  struct AP_ENTRY e[32];
  int ec;
  struct AP_ENTRY *ce;

  char help[1024];

  int idle0,idle1,idle2,idle3;

  float rotx,roty,rotz;
  float rotx2,roty2,rotz2;
  float rotxd,rotyd,rotzd;
};

int apInit(void);

int apParse(void);
int apPrep(void);

int apMenuInit(void);
int apMenuRecreate(void);

void apEntryCallback(Widget ww, XtPointer clientData, XtPointer call);
void apItemCallback(Widget ww, XtPointer clientData, XtPointer call);

void apEntryActivate(struct AP_ENTRY *e);
void apItemActivate(struct AP_ENTRY *e,int n);

void apHelpCallback(Widget ww, XtPointer clientData, XtPointer call);
void apHelpInputCallback(Widget ww, XtPointer clientData, XtPointer call);
void apHelpExposeCallback(Widget ww, XtPointer clientData, XtPointer call);

void apIdle(void);
void apIdleReset(void);

#endif
