#include <math.h>

#include "glw.h"

#ifdef SGI_STEREO
//#include "sgi_stereo.h"
#endif

#include "gfx.h"

extern struct GFX gfx;

/*
int glwStereoInit()
{
#ifdef SGI_STEREO
  return SGIStereoInit(gui.dpy,XtWindow(gui.glxwin));
#else
  return -1;
#endif
}

void glwStereoMode(GLenum mode)
{
#ifdef SGI_STEREO
  //  SGIStereoMode(mode);
#endif
}

void glwStereoSwitch(int mode)
{
#ifdef SGI_STEREO
  SGISwitchStereo(mode);
#endif
}
*/

void glwDrawBuffer(GLenum mode)
{
#ifdef SGI_STEREO
  SGIStereoDrawBuffer(mode);
#else
  glDrawBuffer(GL_BACK);
#endif
}

void glwClear(GLbitfield mask)
{
  glClear(mask);
}

void glwPerspective(GLdouble fovy, GLdouble aspect,
		    GLdouble znear, GLdouble zfar, 
		    GLdouble iod, GLdouble fd)
{
  GLdouble angle,fr;

  gluPerspective(fovy,aspect,znear,zfar);

  if(iod!=0.0) {
    fr=(-fd-znear)/(zfar-znear);
    angle=180.0/M_PI*atan(fd/(2.0*iod));
    glTranslated(0.0,0.0,gfx.transform.tra[2]);
    glRotated(-angle,0.0,1.0,0.0);
    glTranslated(0.0,0.0,-gfx.transform.tra[2]);
  }
}

void glwOrtho(GLdouble left, GLdouble right, 
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
}
