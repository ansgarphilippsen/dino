/***********************************************************

                GLwStereo calls       1997 AP

     This library was modified from the example provided
     by the OpenGL library:
                            Simple OpenGL Stereo Demo 

************************************************************/

#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>

#ifdef LINUX
#ifdef USE_MESA
#include <MesaGL/GLwMDrawA.h>
#else
#include <GL/GLwMDrawA.h>
#endif
#endif
#ifdef SGI
#include <X11/GLw/GLwMDrawA.h>
#endif
#ifdef DEC
#include <X11/GLw/GLwMDrawA.h>
#endif
#ifdef SUN
#include <X11/GLw/GLwMDrawA.h>
#endif

#ifdef SGI
#include <X11/extensions/SGIStereo.h>
#include <X11/extensions/XSGIvc.h>
#endif

#include "dino.h"
#include "GLwStereo.h"
#include "gfx.h"
#include "rex.h"
#include "gui.h"

#ifdef SGI
#include "sgi_video.h"
#endif

extern struct GUI gui;

static struct GLW_STEREO {
  Display *display;
  GLXDrawable drawable;
  int mode;
#ifdef SGI
  XSGIvcVideoFormatInfo vc_stereo,vc_mono;
#endif
  long mask;
  char stereo_high[256];
} GLwStereo;

char *glw_monitor_cmd[]=
{
  "/usr/gfx/setmon -n 72HZ",   /* GLW_STEREO_NONE */
  "/usr/gfx/setmon -n STR_BOT",   /* GLW_STEREO_LOW */
  "/usr/gfx/setmon -n 1024x768_96"    /* GLW_STEREO_HIGH */
};

extern struct GFX gfx;

int GLwStereoInit(Display *display, GLXDrawable drawable)
{
#ifdef SGI
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
#endif


  GLwStereo.display = display;
  GLwStereo.drawable = drawable;
  GLwStereo.mode=GLW_STEREO_NONE;

#ifdef SGI
  sgi_video_get_list(&vc,&vl);

  strcpy(stereo_command,"");

  for(i=0;i<vc;i++) {
    if(rex("1024x768_96s*",vl[i])) {
      sprintf(stereo_command,"/usr/gfx/setmon -n 1024x768_96s");
      gui.stereo_y_offset=0;
      break;
    }

    if(rex("1025x768_96s*",vl[i])) {
      sprintf(stereo_command,"/usr/gfx/setmon -n 1025x768_96s");
      gui.stereo_y_offset=256;
      break;
    }

  }
 
  strcpy(GLwStereo.stereo_high,stereo_command);

  if(strlen(stereo_command)==0)
    return -1;

#endif

  return 0;
}

int GLwStereoCommand(int mode)
{
#ifdef SGI
  // attach to different context
  if(GLwStereo.mode!=mode) {
    XSGISetStereoMode(GLwStereo.display,GLwStereo.drawable,
		      492,532,
		      STEREO_OFF);
    XSync( GLwStereo.display, False);

    switch(mode) {
    case GLW_STEREO_NONE:
      system(glw_monitor_cmd[mode]);
      break;
    case GLW_STEREO_LOW:
      XSGISetStereoMode(GLwStereo.display,GLwStereo.drawable,
			492,532,
			STEREO_BOTTOM);
      system(glw_monitor_cmd[mode]);
      break;
    case GLW_STEREO_HIGH:
      system(GLwStereo.stereo_high);
      break;
    }
    XSync(GLwStereo.display, False);
    GLwStereo.mode=mode;
  }
#endif

  return 0;
}


void GLwStereoDrawBuffer(GLenum view)
{
#ifdef SGI
  switch(GLwStereo.mode) {
  case GLW_STEREO_NONE:
    glDrawBuffer(GL_BACK);
    break;
  case GLW_STEREO_LOW:
    glDrawBuffer(GL_BACK);
    if(view==GLW_STEREO_LEFT)
      XSGISetStereoBuffer(GLwStereo.display,GLwStereo.drawable,
			  STEREO_BUFFER_LEFT);
    else 
      XSGISetStereoBuffer(GLwStereo.display,GLwStereo.drawable,
			  STEREO_BUFFER_RIGHT);
    XSync(GLwStereo.display,False);
    break;
  case GLW_STEREO_HIGH:
    if(view==GLW_STEREO_LEFT)
      glDrawBuffer(GL_BACK_LEFT);
    else
      glDrawBuffer(GL_BACK_RIGHT);
    break;
  }

#else

  glDrawBuffer(GL_BACK);

#endif
}

void GLwStereoClear(GLbitfield mask)
{
  glClear(mask);
}

int GLwStereoPerspective(GLdouble fovy, GLdouble aspect,
                         GLdouble znear, GLdouble zfar,
                         GLdouble iod, GLdouble fd)
{
  GLdouble angle,fr;

  gluPerspective(fovy,aspect,znear,zfar);

  if(iod!=0.0) {
    fr=(-fd-znear)/(zfar-znear);
    //    angle=180.0/M_PI*atan(0.01*iod/fd);
    angle=180.0/M_PI*atan(fd/(2.0*iod));
    glTranslated(0.0,0.0,gfx.transform.tra[2]);
    glRotated(-angle,0.0,1.0,0.0);
    glTranslated(0.0,0.0,-gfx.transform.tra[2]);
  }

  return 0;
}


int GLwStereoPerspective2(GLdouble fovy, GLdouble aspect,
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

int GLwStereoOrtho(GLdouble left, GLdouble right, 
		   GLdouble bottom, GLdouble top,
		   GLdouble near, GLdouble far,
		   GLdouble eye_dist, GLdouble eye_offset)
{
  double angle;
  glOrtho(left,right,bottom,top,near,far);
  if(eye_offset!=0.0) {
    angle=180.0/M_PI*atan(eye_offset/(2*eye_dist));
    glTranslated(0.0,0.0,-near);
    glRotated(-angle,0.0,1.0,0.0);
    glTranslated(0.0,0.0,near);
  }
  return 0;
}





