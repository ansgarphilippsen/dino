/**************************************

  input.c

  handle input events received from
  GUI (mouse, keyboard, spaceball etc)
  and handle them according to lookup
  table

  currently, this is only an interface
  that receives cmi messages and passes
  them to comTransform()

  NOT USED YET !

***************************************/

#include <stdio.h>
#include <stdlib.h>

#include "cmi.h"
#include "com.h"

void inputInit(void)
{
}

static void mouse_func(int ev, int state, int x, int y)
{
  static int last_x,last_y;

  switch(ev) {
  case CMI_BUTTON_PRESS: 
    break;
  case CMI_BUTTON_RELEASE: 
    break;
  case CMI_MOTION: 
    comTransform(TRANS_MOUSE, state, 0, last_x-x);
    comTransform(TRANS_MOUSE, state, 1, last_y-y);
    break;
  }

  last_x=x;
  last_y=y;
}

static void keyb_func(int state, int key)
{
  unsigned char k=(unsigned char)key;
  if(state==CMI_BUTTON_RELEASE)
    comWriteCharBuf(k);
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
  }
}
