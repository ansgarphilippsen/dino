#ifndef _CMI_H
#define _CMI_H

typedef void * cmiTokenPtr;

// non-graphical backend
#define CMI_TARGET_COM   1<<0

// opengl backend
#define CMI_TARGET_GFX   1<<1

// graphical user interface
#define CMI_TARGET_GUI   1<<2

#define CMI_QUERY        1<<7

// token command
enum {
  CMI_NONE=0,

  CMI_RAW,
  CMI_CHAR,
  
  CMI_REFRESH,    // request refresh
  CMI_REDRAW,     // do actual redraw
  CMI_VIEWPORT,   // viewport change
  CMI_RESIZE,     // window resize
  CMI_INPUT,      // input event     
  CMI_PICK,
  CMI_TRANSFORM,

  CMI_NEW,
  CMI_DEL,
  CMI_REN,
  CMI_SHOW,
  CMI_HIDE,
};

typedef struct CMI_TOKEN {
  int target;         // target
  int command;        // command
  cmiTokenPtr value;  // value of message token depending on command
}cmiToken;

typedef void (*cmiCallbackFunc)(const cmiToken *);
typedef void (*cmiTimerFunc)(void);

#define CMI_MAX_CALLBACKS 32

struct CMI {
  struct CMI_CALLBACK_LIST {
    cmiCallbackFunc f;
    int m;
  }cb_list[CMI_MAX_CALLBACKS];
  int cb_count;

  cmiTimerFunc tf;
};

void cmiInit(void);

void cmiSubmit(const cmiToken *t);
void cmiTimer(void);

void cmiQuery(const cmiToken *t);

void cmiRegisterCallback(int mask, cmiCallbackFunc f);
void cmiRegisterTimer(cmiTimerFunc f);

#endif
