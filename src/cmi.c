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

void cmiQuery(const cmiToken *t)
{
  t->target |= CMI_QUERY;
  cmiSubmit(t);
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
