#include <stdio.h>
#include <math.h>

#include <X11/Intrinsic.h>

#ifndef AUTOPLAY_H
#define AUTOPLAY_H

/*
  IDLE1: approximately in 1/10 seconds
*/


#define AP_ENGLISH 0
#define AP_GERMAN 1

#define rnd() ((double)random()/(pow(2,31)-1))
#define rnd2() (rnd()*4.0-2.0)

struct MENU_BUTTON {
  int x1,y1,x2,y2,w,h;
};

struct AP_BUTTON {
  int x1,y1,x2,y2,w,h;

  int active;
  char name1[64],name2[64];
  char help1[1024],help2[1024];
  char inf1[256],inf2[256];

  struct AP_BUTTON_SUB {
    int x1,y1,x2,y2,w,h;
    char name[16],scr[256];
    int flag;
  } sub[4];
  int subc,subi;

};

struct AP_ENTRY {
  int x1,y1,x2,y2,w,h;
  int active;
  char name1[64],name2[64];
  char help1[1024],help2[1024];
  struct AP_BUTTON b[32];
  XmString slist[32];
  int bc;
  char on_load[256],on_activate[256],inf1[256],inf2[256],dir[256];
};


struct AUTOPLAY {
  int flag;
  FILE *f;

  Widget frame,area;
  GC gc1,gc2,gc3,gc4;

  //  Widget helpb, helpd1,helpd2;
  GC help_gc;

  XFontStruct *fs1,*fs2,*fs3,*fs4,*fs5;

  struct AP_ENTRY e[32];
  int ec;
  struct AP_ENTRY *ce;
  struct AP_BUTTON *cb;

  char help[1024];
  struct MENU_BUTTON helpb,langb;

  int idle0,idle1,idle2,idle3;

  float rotx,roty,rotz;
  float rotx2,roty2,rotz2;
  float rotxd,rotyd,rotzd;

  int language;
  
  Window helpw;
  unsigned int helpcx,helpcy,helpcw,helpch;
  int help_on;

};

int apInit(void);

int apParse(void);
int apPrep(void);

int apMenuInit(void);
int apMenuRecreate(void);

void apEntryActivate(struct AP_ENTRY *e);
void apItemActivate(struct AP_ENTRY *e,int n, int m);

void apInputCallback(Widget ww, XtPointer clientData, XtPointer call);
void apExposeCallback(Widget ww, XtPointer clientData, XtPointer call);

int apPutInfo(unsigned int x, unsigned int y, unsigned int w, unsigned int h, char *buf, XFontStruct *fs, Display *dpy, Window win, GC gc, int delta);

void apIdle(void);
void apIdleReset(void);

void apCreateHelp(void);
void apShowHelp(Window w);
void apHelpEvent(Window w, XEvent *event, void *ptr);


#endif
