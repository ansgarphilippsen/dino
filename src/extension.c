#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

#include "dino.h"
#include "extension.h"
#include "gui.h"

extern struct GUI gui;
extern int debug_mode;

XDevice *extDialBoxInit(Display *dpy)
{
  Bool exists;
  XExtensionVersion *version;
  XDeviceInfoPtr deviceInfo, device;
  XAnyClassPtr any;
  XButtonInfoPtr b;
  XValuatorInfoPtr v;
  XAxisInfoPtr a;
  int opcode,eventBase,errorBase;
  int numDev,numButtons,numDials;
  int *dialsResolution;
  int i,j,k;
  XDevice *dialsDevice;
  XDevice *retdev=NULL;
  
  exists=XQueryExtension(dpy,"XInputExtension", &opcode, &eventBase, &errorBase);
  if(!exists){
    goto nodevice;
  }
  version=XGetExtensionVersion(dpy,"XInputExtension");
  if(version==NULL || ((int)version)==NoSuchExtension){

    goto nodevice;
  }
  XFree(version);
  
  deviceInfo=XListInputDevices(dpy,&numDev);
  debmsg("extensions: ");
  if(deviceInfo){
    for(i=0;i<numDev;i++){
      device=&deviceInfo[i];
      any=(XAnyClassPtr)device->inputclassinfo;
      debmsg(device->name);
      if(!strncmp(device->name,"dial+buttons",strlen("dial+buttons"))){
	v=NULL;
	b=NULL;
	for(j=0;j<device->num_classes;j++){
	  switch(any->class){
	  case ButtonClass:
	    b=(XButtonInfoPtr)any;
	    numButtons=b->num_buttons;
	    break;
	  case ValuatorClass:
	    v=(XValuatorInfoPtr) any;
	    numDials=v->num_axes;
	    dialsResolution=(int*)malloc(sizeof(int)*numDials);
	    a=(XAxisInfoPtr)((char*)v+sizeof(XValuatorInfo));
	    for(k=0;k<numDials;k++,a++){
	      dialsResolution[k]=a->resolution;
	    }
	  }
	  any=(XAnyClassPtr) ((char*)any+any->length);
	}

	dialsDevice=XOpenDevice(dpy,device->id);
	
	if(dialsDevice!=NULL){
	  XFreeDeviceList(deviceInfo);
	  gui.xiEventBase=eventBase;
	  retdev= dialsDevice; 
	  break;
	}	      
      }
    }
  }
  XFreeDeviceList(deviceInfo);
  
 nodevice:
  return retdev;;
}


XDevice *extSpaceballInit(Display *dpy)
{
  Bool exists;
  XExtensionVersion *version;
  XDeviceInfoPtr deviceInfo, device;
  XAnyClassPtr any;
  XButtonInfoPtr b;
  XValuatorInfoPtr v;
  XAxisInfoPtr a;
  int opcode,eventBase,errorBase;
  int numDev,numButtons,numDials;
  int *dialsResolution;
  int i,j,k;
  XDevice *spaceballDevice;
  
  exists=XQueryExtension(dpy,"XInputExtension", &opcode, &eventBase, &errorBase);
  if(!exists){
    goto nodevice;
  }
  version=XGetExtensionVersion(dpy,"XInputExtension");
  if(version==NULL || ((int)version)==NoSuchExtension){

    goto nodevice;
  }
  XFree(version);
  
  deviceInfo=XListInputDevices(dpy,&numDev);
  if(deviceInfo){
    for(i=0;i<numDev;i++){
      device=&deviceInfo[i];
      any=(XAnyClassPtr)device->inputclassinfo;
      if(!strncmp(device->name,"spaceball",strlen("spaceball"))){
	v=NULL;
	b=NULL;
	for(j=0;j<device->num_classes;j++){
	  switch(any->class){
	  case ButtonClass:
	    b=(XButtonInfoPtr)any;
	    numButtons=b->num_buttons;
	    break;
	  case ValuatorClass:
	    v=(XValuatorInfoPtr) any;
	    numDials=v->num_axes;
	    dialsResolution=(int*)malloc(sizeof(int)*numDials);
	    a=(XAxisInfoPtr)((char*)v+sizeof(XValuatorInfo));
	    for(k=0;k<numDials;k++,a++){
	      dialsResolution[k]=a->resolution;
	    }
	  }
	  any=(XAnyClassPtr) ((char*)any+any->length);
	}

//	spaceballDevice=XOpenDevice(dpy,device->id);
	spaceballDevice=NULL;

	if(spaceballDevice!=NULL){
	  XFreeDeviceList(deviceInfo);
	  gui.xiEventBase=eventBase;
	  return spaceballDevice; 
	}		      
      }
    }
  }
  XFreeDeviceList(deviceInfo);
  
 nodevice:
  return NULL;
}
