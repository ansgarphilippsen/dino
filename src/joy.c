#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>                                                       

#include "joy.h"
#include "com.h"

static int joy_fd;

int jInit(char *name)
{
#ifdef LINUX
  // old kernels
  strcpy(name,"Joystick");
  joy_fd=open("/dev/js0",O_NONBLOCK);
  if(joy_fd==-1) {
    // new kernels
    joy_fd=open("/dev/input/js0",O_NONBLOCK);
    if(joy_fd==-1) {
      return -1;
    } else { 
      ioctl(joy_fd,JSIOCGNAME(128),name);
      return 0;
    }
  } else {
    ioctl(joy_fd,JSIOCGNAME(128),name);
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
  static int threshold=1000;
  static int factor=10;
  static int value[]={0,0,0,0,0,0};
  int num,i,val;
  
  while((num=read(joy_fd,buf,sizeof(buf)))>0) {
    for(i=0;i<num/sizeof(struct js_event);i++) {
      e=&buf[i];
      
      switch(e->type) {
      case JS_EVENT_INIT:
	break;
      case JS_EVENT_BUTTON:
	break;
      case JS_EVENT_AXIS:
	if(abs(e->value)>threshold) {
	  if(e->value<0)
	    val=-((abs(e->value)-threshold)/factor);
	  else
	    val=+((abs(e->value)-threshold)/factor);
	} else {
	  val=0;
	}
	value[e->number]=val;
	break;
      }
    }
  }  

  if(value[0]!=0)
    comTransform(TRANS_SPACEBALL,0,GC_ROTX,value[0]);
  if(value[1]!=0)
    comTransform(TRANS_SPACEBALL,0,GC_ROTZ,value[1]);
  if(value[2]!=0)
    comTransform(TRANS_SPACEBALL,0,GC_ROTY,value[2]);
  if(value[3]!=0)
    comTransform(TRANS_SPACEBALL,0,GC_TRANSX,value[3]);
  if(value[4]!=0)
    comTransform(TRANS_SPACEBALL,0,GC_TRANSZ,value[4]);
  if(value[5]!=0)
    comTransform(TRANS_SPACEBALL,0,GC_TRANSY,value[5]);


  //  fprintf(stderr,"\n");
  for(i=0;i<6;i++) {
    //    fprintf(stderr,"%d: %+6d  ",i,value[i]);
    if(value[i]>0)
      comTransform(TRANS_SPACEBALL,0,i,value[i]);
  }
  return 0;
#else
  return -1
#endif
}
