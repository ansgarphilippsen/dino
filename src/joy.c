#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>                                                       

#include "joy.h"
#include "com.h"

static int joy_fd;

int jInit()
{
#ifdef LINUX
  // old kernels
  joy_fd=open("/dev/js0",O_NONBLOCK);
  if(joy_fd==-1) {
    // new kernels
    joy_fd=open("/dev/input/js0",O_NONBLOCK);
    if(joy_fd==-1)
      return -1;
    else
      return 0;
  } else {
    return 0;
  }
#else
  return -1;
#endif
}

int jCheck()
{
#ifdef LINUX
  struct js_event *e;
  struct js_event buf[256];
  static int threshold=200;
  static int factor=10;
  int num,i;
  
  num=read(joy_fd,buf,sizeof(buf));

  if(num>0)
    for(i=0;i<num/sizeof(struct js_event);i++) {
      e=&buf[i];

    switch(e->type) {
    case JS_EVENT_INIT:
      break;
    case JS_EVENT_BUTTON:
      break;
    case JS_EVENT_AXIS:
      switch(e->number) {
      case 0: /* x-translation */
	/* hack for werid joystick behaviour */
	/* removed again
	if(abs(e->value)>threshold)
	  comTransform(TRANS_SPACEBALL, 0, 4, e->value/factor);
	break;
	*/
      case 1: /* y-translation */
	break;
      case 2: /* z-translation */
	break;
      case 3: /* x-rotation */
	if(abs(e->value)>threshold)
	  comTransform(TRANS_SPACEBALL, 0, 3, e->value/factor);
	break;
      case 4: /* y-rotation */
	if(abs(e->value)>threshold)
	  comTransform(TRANS_SPACEBALL, 0, 4, e->value/factor);
	break;
      case 5: /* z-rotation */
	if(abs(e->value)>threshold)
	  comTransform(TRANS_SPACEBALL, 0, 5, e->value/factor);
	break;
      }
      break;
    }
  }

  return 0;
#else
  return -1
#endif
}
