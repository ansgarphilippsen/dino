/*

  Various Open GL Wrapper calls

*/

#ifndef _GLW_H
#define _GLW_H

#include <GL/gl.h>
#include <GL/glu.h>

enum {GLW_STEREO_CENTER=0,
      GLW_STEREO_LEFT,
      GLW_STEREO_RIGHT};

/*
int glwStereoInit(void);
void glwStereoMode(GLenum mode);
void glwStereoSwitch(int mode);
*/

void glwDrawBuffer(GLenum mode);
void glwClear(GLbitfield mask);
void glwPerspective(GLdouble fovy, GLdouble aspect,
		    GLdouble znear, GLdouble zfar, 
		    GLdouble eye_dist, GLdouble eye_offset);
void glwOrtho(GLdouble left, GLdouble right, 
	      GLdouble bottom, GLdouble top,
	      GLdouble near, GLdouble far,
	      GLdouble eye_dist, GLdouble eye_offset);


#endif






