#ifdef USE_MESA
#include <MesaGL/gl.h>
#include <MesaGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef SGI
#include <X11/extensions/SGIStereo.h>
#include <X11/extensions/XSGIvc.h>
#endif

#ifndef GLW_STEREO_H
#define GLW_STEREO_H

enum {GLW_STEREO_NONE=0,
      GLW_STEREO_LOW=1,
      GLW_STEREO_HIGH=2};

enum {GLW_STEREO_LEFT=0,
      GLW_STEREO_RIGHT};

struct GLW_STEREO {
  Display *display;
  GLXDrawable drawable;
  int mode;
#ifdef SGI
  XSGIvcVideoFormatInfo vc_stereo,vc_mono;
#endif
  long mask;
  char stereo_high[256];
  int resx,resy,y_offset;
};


int GLwStereoInit(Display *display, GLXDrawable drawable);
int GLwStereoCommand(int mode);
void GLwStereoDrawBuffer(GLenum mode);
void GLwStereoClear(GLbitfield mask);
int GLwStereoPerspective(GLdouble fovy, GLdouble aspect,
			 GLdouble znear, GLdouble zfar, 
			 GLdouble eye_dist, GLdouble eye_offset);
int GLwStereoOrtho(GLdouble left, GLdouble right, 
		   GLdouble bottom, GLdouble top,
		   GLdouble near, GLdouble far,
		   GLdouble eye_dist, GLdouble eye_offset);

#endif






