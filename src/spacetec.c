#include <stdio.h>

#include "spacetec.h"
#include "transform.h"
#include "com.h"

SPW_InputDispatchStruct spacetec_dispatch;

int spacetecInit(Display *dpy, Window w, char *n) 
{
  if (SPW_InputCheckForSpaceball(dpy, w, n) == SPW_FALSE) { 
    return 0;
  }

  SPW_InputInitDispatchStruct(&spacetec_dispatch); 
//  spacetec_dispatch.handle_motion = spacetecMotion; 
//  spacetec_dispatch.handle_bpress = spacetecButtonPress; 
//  spacetec_dispatch.handle_brelease = spacetecButtonRelease; 
  
  return 1;
}

void spacetecEventHandler(Display *dpy, XEvent *e) 
{ 
  SPW_InputEvent spwEvent;
  if (SPW_InputIsSpaceballEvent(dpy, e, &spwEvent) == SPW_TRUE) { 
    spacetec_dispatch.xevent = e; 
    spacetec_dispatch.spwevent = &spwEvent; 
//    SPW_InputDispatchEx(&spacetec_dispatch); 
    if(spwEvent.type==SPW_InputMotionEvent) {
      spacetecMotion(dpy,spwEvent.fData,NULL);
    }
  } 
}


void spacetecMotion(Display *dpy, float data[7], void *appdata) 
{
  int i;
//  fprintf(stderr,"motion: %f %f %f %f %f %f %f %f\n",
//	  data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]);
//  fprintf(stderr,"motion: %5d %5d %5d %5d %5d %5d %5d %5d\n",
//	  data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7]); 

  for(i=0;i<6;i++)
    if(data[i]!=0.0)
      comTransform(TRANS_SPACEBALL,0, i, data[i]); 
   
}

void spacetecButtonPress(Display *dpy, int button, void *appdata) 
{

} 

void spacetecButtonRelease(Display *dpy, int button, void *appdata) 
{

} 


