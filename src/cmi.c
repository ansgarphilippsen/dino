/*********************************
 
  cmi.c  common message interface

  allows communication between
  the dino backend and frontend
  in particular for flexible
  GUI handling

*********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmi.h"

/*
  global struct for common message interface
*/
struct CMI cmi;


/*
  initialize cmi stuff
*/

void cmiInit()
{
  cmi.cb_count=0;
}

/*
  a submitted token gets send to its
  receiving callbacks
*/

void cmiSubmit(const cmiToken *t)
{
  int i;

  for(i=0;i<cmi.cb_count;i++) {
    if(t->target & cmi.cb_list[i].m) {
      if(cmi.cb_list[i].f!=NULL) {
	(*cmi.cb_list[i].f)(t);
      }
    }
  }
}

/*
  send a query for information
*/

void cmiQuery(const cmiToken *t2)
{
  cmiToken t;
  memcpy(&t,t2,sizeof(cmiToken));
  t.target |= CMI_QUERY;
  cmiSubmit(&t);
}

/*
  register a callback 
*/

void cmiRegisterCallback(int m, cmiCallbackFunc f)
{
  if(cmi.cb_count<CMI_MAX_CALLBACKS) {
    cmi.cb_list[cmi.cb_count].f=f;
    cmi.cb_list[cmi.cb_count].m=m;
    cmi.cb_count++;
  }
}

/*
  call the timer callback
*/

void cmiTimer(void)
{
  (*cmi.tf)();
}

/*
  register the timer callback
*/

void cmiRegisterTimer(cmiTimerFunc f)
{
  cmi.tf=f;
}

// shortcuts

static cmiToken t_init_gl={CMI_TARGET_GFX, CMI_INITGL, NULL};

void cmiInitGL(void)
{
  cmiSubmit(&t_init_gl);
}

static cmiToken t_refresh={CMI_TARGET_GUI, CMI_REFRESH, NULL};

void cmiRefresh(void)
{
  cmiSubmit(&t_refresh);
}

static cmiToken t_resize={CMI_TARGET_GFX, CMI_RESIZE, NULL};

void cmiResize(int w, int h)
{
  int v[2];
  t_resize.value=v;
  v[0]=w; v[1]=h;
  cmiSubmit(&t_resize);
}

static cmiToken t_redraw={CMI_TARGET_GFX, CMI_REDRAW, NULL};

void cmiRedraw(void)
{
  cmiSubmit(&t_redraw);
}

static cmiToken t_message={CMI_TARGET_COM, CMI_MESSAGE, NULL};

void cmiMessage(const char *s)
{
  t_message.value=(void *)s;
  cmiSubmit(&t_message);
}

static cmiToken t_command={CMI_TARGET_COM, CMI_RAW, NULL};

void cmiCommand(const char *s)
{
  t_command.value=s;
  cmiSubmit(&t_command);
}

static cmiToken t_stereo={CMI_TARGET_GUI, CMI_STEREO, NULL};
void cmiStereo(int m)
{
  t_stereo.value=&m;
  cmiSubmit(&t_stereo);
}

