#ifndef _CMI_H
#define _CMI_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void * cmiTokenPtr;

// non-graphical backend
#define CMI_TARGET_COM   1<<0

// opengl backend
#define CMI_TARGET_GFX   1<<1

// graphical user interface
#define CMI_TARGET_GUI   1<<2

#define CMI_QUERY        1<<7

// token commands
enum {
  CMI_NONE=0,

  // TARGET_COM
  CMI_EXIT,     // exit request  (NULL)
  CMI_RAW,      // send raw command  (char *)
  CMI_INPUT,    // input event (int [N])

  // TARGET_GFX
  CMI_INITGL,   // opengl can be initialized now (NULL)
  CMI_REDRAW,   // do actual redraw (NULL)
  CMI_RESIZE,   // window resize (int[2])

  // TARGET_GUI
  CMI_MESSAGE,  // send message (char *)
  CMI_REFRESH,  // request refresh (NULL)
  CMI_NEW,
  CMI_DEL,
  CMI_REN,
  CMI_SHOW,
  CMI_HIDE
};

// input types
#define CMI_INPUT_NONE      0
#define CMI_INPUT_KEYBOARD  1
#define CMI_INPUT_MOUSE     2
#define CMI_INPUT_SPACEBALL 3
#define CMI_INPUT_DIALBOX   4
#define CMI_INPUT_TABLET    5
#define CMI_INPUT_PAD       6

#define CMI_BUTTON_PRESS    1
#define CMI_BUTTON_RELEASE  2
#define CMI_MOTION          3

// input modifiers
#define CMI_SHIFT_MASK   (1<<0)
#define CMI_LOCK_MASK    (1<<1)
#define CMI_CNTRL_MASK   (1<<2)
#define CMI_MOD1_MASK    (1<<3)
#define CMI_MOD2_MASK    (1<<4)
#define CMI_MOD3_MASK    (1<<5)
#define CMI_MOD4_MASK    (1<<6)
#define CMI_MOD5_MASK    (1<<7)
#define CMI_BUTTON1_MASK (1<<8)
#define CMI_BUTTON2_MASK (1<<9)
#define CMI_BUTTON3_MASK (1<<10)
#define CMI_BUTTON4_MASK (1<<11)
#define CMI_BUTTON5_MASK (1<<12)

// special keys starting above 0x100
#define CMI_KEY_SPECIAL 0x100
#define CMI_KEY_RETURN  0x101
#define CMI_KEY_BKSPC   0x102
#define CMI_KEY_DELETE  0x103
#define CMI_KEY_UP      0x104
#define CMI_KEY_DOWN    0x105
#define CMI_KEY_LEFT    0x106
#define CMI_KEY_RIGHT   0x107

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

// some often used functions follow

void cmiInitGL(void);
void cmiRefresh(void);
void cmiResize(int w, int h);
void cmiMessage(const char *s);
void cmiCommand(const char *s);
void cmiRedraw(void);

#ifdef __cplusplus
}
#endif

#endif
