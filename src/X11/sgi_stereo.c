/***********************************************************

                SGIStereo calls       1997/2002 AP

     This library was modified from the example provided
     by the OpenGL library:
                            Simple OpenGL Stereo Demo 

************************************************************/

#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include "sgi_stereo.h"
#include "sgi_video.h"
#include "glw.h"
#include "dino.h"
#include "com.h"
#include "rex.h"
#include "gui_x11.h"
#include "om_x11.h"

extern struct GUI gui;
extern struct OBJECT_MENU om;

extern int debug_mode,video_mode, stereo_available;

struct SGI_STEREO_INFO SGIStereo;

char *glw_monitor_cmd[]=
{
  "/usr/gfx/setmon -n 72HZ",   /* SGI_STEREO_NONE */
  "/usr/gfx/setmon -n STR_BOT",   /* SGI_STEREO_LOW */
  "/usr/gfx/setmon -n 1024x768_96"    /* SGI_STEREO_HIGH */
};

static int SGIStereoExtractResolution(const char *s, int *x, int *y)
{
  char buf[512],*p;

  strncpy(buf,s,512);

  (*x)=0;
  (*y)=0;

  p=strrchr(buf,'_');
  if(p==NULL)
    return -1;

  p[0]='\0';

  p=strrchr(buf,'x');
  if(p==NULL)
    return -1;

  p[0]='\0';

  (*x)=atoi(buf);
  (*y)=atoi(p+1);
  
  return 0;
}


int SGIStereoInit(Display *display, GLXDrawable drawable, int am)
{
  XSGIvcVideoFormatInfo *vc_info;
  XSGIvcVideoFormatInfo vc_pattern;
  XSGIvcChannelInfo *c_info;
  long vc_mask;
  int i,vc_count;
  char t[256],*p;
  char **fc;
  int major,minor;
  int vc_ext=1;
  int vc;
  char **vl;
  char stereo_command[256];
  char message[256];

  SGIStereo.display = display;
  SGIStereo.drawable = drawable;
  SGIStereo.available_mode=am;
  SGIStereo.active=0;

  strcpy(stereo_command,"");

  if(am==SGI_STEREO_LOW)
    return 0;
  
  if(sgi_video_get_list(&vc,&vl)!=0) {
    SGIStereo.available_mode=SGI_STEREO_LOW;
    return -1;
  }

  debmsg("Detected Stereo Video Modes:\n");
  for(i=0;i<vc;i++) {
    sprintf(message,"%d: %s",i+1,vl[i]);
    debmsg(message);
  }

  if(video_mode!=0) {
    /*
      take the user supplied video mode
    */
    if(video_mode<1 || video_mode>i) {
      comMessage("warning: supplied stereo video mode not found, using default\n");
      video_mode=0;
    } else {
      sprintf(stereo_command,"/usr/gfx/setmon -n %s",vl[video_mode-1]);
      SGIStereoExtractResolution(vl[video_mode-1],\
				 &SGIStereo.resx,&SGIStereo.resy);
    }
  }

  if(video_mode==0) {

    // first look for 1280x1024 stereo mode
    for(i=0;i<vc;i++) {
      if(rex("128*x*",vl[i])) {
	sprintf(stereo_command,"/usr/gfx/setmon -n %s",vl[i]);
	SGIStereoExtractResolution(vl[i],\
				   &SGIStereo.resx,&SGIStereo.resy);
	break;
      }
    }

    if(i==vc) {
      // then for 1024x768
      for(i=0;i<vc;i++) {
	if(rex("102*x768*",vl[i])) {
	  sprintf(stereo_command,"/usr/gfx/setmon -n %s",vl[i]);
	  SGIStereoExtractResolution(vl[i],\
				     &SGIStereo.resx,&SGIStereo.resy);
	  break;
	}
      }

      if(i==vc) {
	/*
	  no def vidmode found, using first one
	*/
	sprintf(stereo_command,"/usr/gfx/setmon -n %s",vl[0]);
	SGIStereoExtractResolution(vl[0],\
				   &SGIStereo.resx,&SGIStereo.resy);
      }
    }
  }
  sprintf(message,"using stereo command: %s",stereo_command);
  strcpy(SGIStereo.stereo_high,stereo_command);
  debmsg(message);

  // quick hack!!
  if(SGIStereo.resx==1025)
    SGIStereo.y_offset=256;
  else
    SGIStereo.y_offset=0;

  return 0;
}

int SGISwitchStereo(int m)
{
  Arg arg[4];
  Position ax,ay;
  Dimension aw,ah;
  int rh,wh;
  Position sx,sy;
  Dimension sw,sh;
  XWindowChanges xwc;

  
  if(SGIStereo.active && m==SGI_STEREO_OFF) {
    // turn active stereo off;
    SGIStereo.active=0;

    SGIStereoCommand(SGI_STEREO_NONE);
    
    XtSetArg(arg[1],XmNx,gui.win_xs);
    XtSetArg(arg[0],XmNy,gui.win_ys);
    XtSetArg(arg[2],XmNwidth,gui.win_widths);
    XtSetArg(arg[3],XmNheight,gui.win_heights);
    XtSetValues(gui.top,arg,4);
    
    if(gui.om_flag) {
      xwc.y=gui.win_ys;
      XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			   CWY, &xwc);
    }      
    
    // notify about resize;
    cmiResize(gui.win_width,gui.win_height);
  }

  if(!SGIStereo.active && m==SGI_STEREO_ON) {
    // turn stereo on;

    XtSetArg(arg[0],XmNx,&gui.win_xs);
    XtSetArg(arg[1],XmNy,&gui.win_ys);
    XtSetArg(arg[2],XmNwidth,&gui.win_widths);
    XtSetArg(arg[3],XmNheight,&gui.win_heights);
    XtGetValues(gui.top,arg,4);
    wh=gui.win_height;
    rh=HeightOfScreen(DefaultScreenOfDisplay(gui.dpy));

    if(SGIStereo.available_mode==SGI_STEREO_HIGH) {
      
      sx=SGIStereo.resx-SGIStereo.resy+17;
      sy=SGIStereo.y_offset;
      sw=SGIStereo.resy-20;
      sh=SGIStereo.resy-20;
      
      SGIStereoCommand(SGI_STEREO_HIGH);
      SGIStereo.active=1;
      
      /*
	maybe it would be adviseable to 
	get the new values from the display
      */
      XtSetArg(arg[1],XmNx,sx);
      XtSetArg(arg[0],XmNy,sy);
      XtSetArg(arg[2],XmNwidth,sw);
      XtSetArg(arg[3],XmNheight,sh);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=sy+30;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      

      // notify about size change;
      cmiResize(sw,sh);

    } else if(SGIStereo.available_mode==SGI_STEREO_LOW) {

      // turn on stereo mode;
      SGIStereoCommand(SGI_STEREO_LOW);
      SGIStereo.active=1;

      // modify window for stereo display;
      sx=gui.win_xs;
      sy=(Position)rh/2;
      sw=gui.win_widths;
      sh=gui.win_heights/2;
      XtSetArg(arg[1],XmNx,sx);
      XtSetArg(arg[0],XmNy,sy);
      XtSetArg(arg[2],XmNwidth,sw);
      XtSetArg(arg[3],XmNheight,sh);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=sy+30;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }
      
      // notify about resize;
      cmiResize(gui.win_width,gui.win_height/2);

    }

  }

  /*************************** OLD

  switch(SGIStereo.available_mode) {

  case SGI_STEREO_LOW:
    wh=gui.win_height;
    rh=HeightOfScreen(DefaultScreenOfDisplay(gui.dpy));
    
    if(!SGIStereo.active) {
      XtSetArg(arg[0],XmNx,&gui.win_xs);
      XtSetArg(arg[1],XmNy,&gui.win_ys);
      XtSetArg(arg[2],XmNwidth,&gui.win_widths);
      XtSetArg(arg[3],XmNheight,&gui.win_heights);
      XtGetValues(gui.top,arg,4);
    }
    
    sx=gui.win_xs;
    sy=(Position)rh/2;
    sw=gui.win_widths;
    sh=gui.win_heights/2;
    
    switch(m) {
    case GUI_STEREO_OFF:
      SGIStereoCommand(SGI_STEREO_NONE);
      gui.stereo_mode=GUI_STEREO_OFF;

      XtSetArg(arg[1],XmNx,gui.win_xs);
      XtSetArg(arg[0],XmNy,gui.win_ys);
      XtSetArg(arg[2],XmNwidth,gui.win_widths);
      XtSetArg(arg[3],XmNheight,gui.win_heights);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=gui.win_ys;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      

      // reset projection matrix
      gfx.aspect=(double)gui.win_width/(double)gui.win_height;
      gfxSetProjection(gfx.stereo_view);
      gfxSetFog();

      break;
    case GUI_STEREO_NORMAL:
      SGIStereoCommand(SGI_STEREO_LOW);
      gui.stereo_mode=GUI_STEREO_NORMAL;
      
      XtSetArg(arg[1],XmNx,sx);
      XtSetArg(arg[0],XmNy,sy);
      XtSetArg(arg[2],XmNwidth,sw);
      XtSetArg(arg[3],XmNheight,sh);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=sy+30;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;

      break;
    }
    break;
  case SGI_STEREO_HIGH:
    wh=gui.win_height;
    rh=HeightOfScreen(DefaultScreenOfDisplay(gui.dpy));
    
    if(gui.stereo_mode==GUI_STEREO_OFF) {
      XtSetArg(arg[0],XmNx,&gui.win_xs);
      XtSetArg(arg[1],XmNy,&gui.win_ys);
      XtSetArg(arg[2],XmNwidth,&gui.win_widths);
      XtSetArg(arg[3],XmNheight,&gui.win_heights);
      XtGetValues(gui.top,arg,4);
    }
    
    sx=SGIStereo.resx-SGIStereo.resy+17;
    sy=SGIStereo.y_offset;
    sw=SGIStereo.resy-20;
    sh=SGIStereo.resy-20;
    
    switch(m) {
    case GUI_STEREO_OFF:
      SGIStereoCommand(SGI_STEREO_NONE);
      gui.stereo_mode=GUI_STEREO_OFF;

      XtSetArg(arg[1],XmNx,gui.win_xs);
      XtSetArg(arg[0],XmNy,gui.win_ys);
      XtSetArg(arg[2],XmNwidth,gui.win_widths);
      XtSetArg(arg[3],XmNheight,gui.win_heights);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=gui.win_ys;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      
      // reset projection matrix
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;
      gfxSetProjection(gfx.stereo_view);
      gfxSetFog();
      break;
    case GUI_STEREO_NORMAL:
      SGIStereoCommand(SGI_STEREO_HIGH);
      gui.stereo_mode=GUI_STEREO_NORMAL;
      
      XtSetArg(arg[1],XmNx,sx);
      XtSetArg(arg[0],XmNy,sy);
      XtSetArg(arg[2],XmNwidth,sw);
      XtSetArg(arg[3],XmNheight,sh);
      XtSetValues(gui.top,arg,4);

      if(gui.om_flag) {
	xwc.y=sy+30;
	XReconfigureWMWindow(om.dpy,om.top,om.scrn,
			     CWY, &xwc);
      }      
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;
      break;
    }
    break;
  }

*****************/
  return 0;
}

int SGIStereoCommand(int mode)
{
  switch(mode) {
  case SGI_STEREO_NONE:
    XSGISetStereoMode(SGIStereo.display,SGIStereo.drawable,
		      492,532,
		      STEREO_OFF);
    system(glw_monitor_cmd[mode]);
    break;
  case SGI_STEREO_LOW:
    XSGISetStereoMode(SGIStereo.display,SGIStereo.drawable,
		      492,532,
		      STEREO_BOTTOM);
    system(glw_monitor_cmd[mode]);
    break;
  case SGI_STEREO_HIGH:
    system(SGIStereo.stereo_high);
    break;
  }
  XSync(SGIStereo.display, False);

  return 0;
}

void SGIStereoDrawBuffer(GLenum view)
{
  if(SGIStereo.active) {
    if(SGIStereo.available_mode==SGI_STEREO_HIGH) {
      if(view==GLW_STEREO_LEFT) {
	glDrawBuffer(GL_BACK_LEFT);
      } else {
	glDrawBuffer(GL_BACK_RIGHT);
      }
    } else {
      if(view==GLW_STEREO_LEFT) {
	XSGISetStereoBuffer(SGIStereo.display,SGIStereo.drawable,
			    STEREO_BUFFER_LEFT);
      } else { 
	XSGISetStereoBuffer(SGIStereo.display,SGIStereo.drawable,
			    STEREO_BUFFER_RIGHT);
      }
      XSync(SGIStereo.display,False);
    }
  } else {
    glDrawBuffer(GL_BACK);
  }
}

// older version

int SGIStereoPerspective2(GLdouble fovy, GLdouble aspect,
			 GLdouble znear, GLdouble zfar, 
			 GLdouble iod, GLdouble fd)
{
  GLdouble mat[16];
  GLdouble fov2, left, right, bottom, top;

  fov2=((fovy*M_PI)/180.0)/2.0;

  top=znear / (cos(fov2) / sin(fov2));
  bottom=-top;
  right=top*aspect;
  left=-right;

  mat[0]=2.0*znear/(right-left);
  mat[1]=0.0;
  mat[2]=0.0;
  mat[3]=0.0;

  mat[4]=0.0;
  mat[5]=2.0*znear/(top-bottom);
  mat[6]=0.0;
  mat[7]=0.0;

  mat[8]=(right+left)/(right-left);
  mat[9]=(top+bottom)/(top-bottom);
  mat[10]=-(zfar+znear)/(zfar-znear);
  mat[11]=-1.0;
  
  mat[12]=mat[0]*iod*0.01;
  mat[13]=0.0;
  mat[14]=-2.0*zfar*znear/(zfar-znear);
  mat[15]=0.0;

  /*
    adjust mat[8] and mat[12] depending on the skew
    imposed by the stereo
  */

  /*
  mat[8]+=iod*0;
  mat[12]+=-iod*1;
  */

  glLoadMatrixd(mat);

  //  glFrustum(left,right,bottom,top,znear,zfar);

  //  gluPerspective(fovy,aspect,znear,zfar);

  return 0;
}

int SGIStereoIsActive(void)
{
  return SGIStereo.active;
}
