#ifdef USE_MESA
#include <MesaGL/gl.h>
#include <MesaGL/glu.h>
#include <MesaGL/glx.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#ifndef GFX_H
#define GFX_H

#include "dbm.h"
#include "xtal.h"
#include "transform.h"
#include "render.h"

enum                     {GFX_CENTER,
			  GFX_LEFT,
			  GFX_RIGHT};

enum          {GFX_PERSP,
	       GFX_ORTHO};

struct GFX_LIGHT {
  int on,local;
  GLfloat pos[4];
  GLfloat amb[4],diff[4],spec[4];
  GLfloat kc,kl,kq;
  GLfloat spotc,spotd[3],spote;
};

struct GFX_CLIP {
  int on;
  transMat transform;
  GLfloat dir[3],pos[3];
  GLdouble eq[4];
};

struct GFX
{
  int mode;
  transMat transform;
  GLdouble scale;
  GLdouble fovy,aspect;
  GLdouble left,right,bottom,top;
  GLfloat r,g,b;
  GLfloat fog_color[4];
  int fog;
  float fog_near,fog_far,fog_dist;

  int axisflag;
  int cpflag;
  int stereo_view;
  int slab_flag;
  int fixz;
  int spin,sx,sy,sdx,sdy;
  int dither;

  int offline;
  int smooth;

  int show;

  GLuint sphere_list,bond_list;

  struct GFX_LIGHT light[8];
  struct RENDER_MATERIAL defmat;

  struct GFX_CLIP clip[6];

  GLvoid *save_buf;
  int save_buf_size;
  GLdouble pmat[16];

  int sort;
  GLint fbuf_size,fbuf_entries;
  GLfloat *fbuf;

#ifdef EXPO
  float limx1,limx2,limy1,limy2,limz1,limz2;
#endif

};

int gfxInit(void);
int gfxGLInit(void);

int gfxRedraw(void);
int gfxRedraw2(void);
int gfxSceneRedraw(int clear);

int gfxDrawStructObj(struct STRUCT_OBJ *obj);
int gfxDrawScalObj(struct SCAL_OBJ *obj);
int gfxDrawSurfObj(struct SURF_OBJ *obj);
int gfxDrawGeomObj(struct GEOM_OBJ *obj);
int gfxDrawCell(struct XTAL *xtal);

int gfxSetup(void);

int gfxSetFog(void);
int gfxSetSlab(float near, float far);
int gfxSetProjection(int view);
int gfxSetViewport(void);

int gfxResizeEvent(void);
int gfxSaveDepthBuffer(void);
int gfxRestoreDepthBuffer(void);

int gfxGenFeedback(void);
int gfxDrawFeedback(void);

int gfxSetAccProjection(int view, float, float);
int gfxAccFrustum(GLdouble left, GLdouble right, 
		  GLdouble bottom, GLdouble top,
		  GLdouble zNear, GLdouble zFar,
		  GLdouble pixdx, GLdouble pixdy);

int gfxAccPerspective(GLdouble fovy, GLdouble aspect,
		      GLdouble zNear, GLdouble zFar,
		      GLdouble pixdx, GLdouble pixdy);

#endif




