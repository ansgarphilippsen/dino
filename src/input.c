/**************************************

  input.c

  handle input events received from
  GUI (mouse, keyboard, spaceball etc)
  and handle them according to lookup
  table

  currently, this is only an interface
  that receives cmi messages and passes
  them to comTransform()

***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cmi.h"
#include "com.h"

void inputInit(void)
{
}

static void mouse_func(int ev, int state, int x, int y)
{
  static int last_x,last_y;
  static struct timeval tp0,tp1;
  static long dt;
  int dx=last_x-x;
  int dy=last_y-y;
  int but,mod;

  but=state & CMI_BUT_MASK;
  mod=state & CMI_MOD_MASK;

  switch(ev) {
  case CMI_BUTTON_PRESS:
    gettimeofday(&tp0,NULL);
    break;
  case CMI_BUTTON_RELEASE: 
    gettimeofday(&tp1,NULL);
    dt=(long)(tp1.tv_sec-tp0.tv_sec)*1000000+(tp1.tv_usec-tp0.tv_usec);
    if(but==CMI_BUTTON1_MASK) {
      if(dt<200000 || (dx==0 && dy==0 && dt<300000)) {
	if(mod==CMI_SHIFT_MASK) {
	  comPick(x,y,1);
	} else {
	  comPick(x,y,0);
	}
      }
    } else if(but==CMI_BUTTON4_MASK) {
      comTransform(TRANS_MOUSE, mod,2,-1.0);
    } else if(but==CMI_BUTTON5_MASK) {
      comTransform(TRANS_MOUSE, mod,2,1.0);
    }

    

    break;
  case CMI_MOTION: 
    comTransform(TRANS_MOUSE, state, 0, dx);
    comTransform(TRANS_MOUSE, state, 1, dy);
    break;
  }

  last_x=x;
  last_y=y;
}

static void spcb_func(int state, int axis, int value)
{
  comTransform(TRANS_SPACEBALL, state, axis, value);
}

static void keyb_func(int state, int key)
{
  unsigned char k;
  if(state==CMI_BUTTON_RELEASE) {
    switch(key) {
    case CMI_KEY_RETURN: 
      comWriteCharBuf(0x0D); 
      break;
    case CMI_KEY_BKSPC: 
      comWriteCharBuf(0x08); 
      break;
    case CMI_KEY_UP: 
      comWriteCharBuf(0x1B);
      comWriteCharBuf('[');
      comWriteCharBuf('A');
      break;
    case CMI_KEY_DOWN: 
      comWriteCharBuf(0x1B);
      comWriteCharBuf('[');
      comWriteCharBuf('B');
      break;
    case CMI_KEY_LEFT: 
      comWriteCharBuf(0x1B);
      comWriteCharBuf('[');
      comWriteCharBuf('D');
      break;
    case CMI_KEY_RIGHT: 
      comWriteCharBuf(0x1B);
      comWriteCharBuf('[');
      comWriteCharBuf('C');
      break;
    default:
      {
	k=(unsigned char)(key & 0xff);
	comWriteCharBuf(k);
      }
    }

    
  }
}

void inputProcess(cmiToken *t)
{
  int *ip;

  ip=(int *)t->value;

  switch(ip[0]) {
  case CMI_INPUT_MOUSE:
    mouse_func(ip[1],ip[2],ip[3],ip[4]);
    break;
  case CMI_INPUT_KEYBOARD:
    keyb_func(ip[1],ip[2]);
    break;
  case CMI_INPUT_SPACEBALL:
    spcb_func(ip[2],ip[3],ip[4]);
  }
}
