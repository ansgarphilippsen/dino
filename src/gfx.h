#include "gl_include.h"

#ifndef GFX_H
#define GFX_H

#include "dbm.h"
#include "xtal.h"
#include "transform.h"
#include "render.h"
#include "cmi.h"

enum                     {GFX_CENTER,
			  GFX_LEFT,
			  GFX_RIGHT};

enum          {GFX_PERSP,
	       GFX_ORTHO};

enum {GFX_STEREO_OFF=0,
      GFX_STEREO_SPLIT,
      GFX_STEREO_HW,
      GFX_STEREO_INTERLACED};

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
  
  // this is a copy from struct GUI to allow
  // frontend - backend separation
  int win_width,win_height;
  int stereo_active,stereo_mode,split_mode,stencil_dirty;
  float eye_dist, eye_offset;

  int current_view;

  GLfloat fog_color[4];
  int fog,fog_mode;
  float fog_near,fog_far,fog_density;
  float fog_near_offset,fog_far_offset;

  int axisflag;
  int cpflag;
  int slab_flag;
  int fixz;
  int anim;
  int sx,sy,sdx,sdy;
  int dither;

  int dlist_flag;

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

  int use_dlist_flag;

};

#ifdef __cplusplus
extern "C" {
#endif

int gfxInit(void);

int gfxCMIInit(void);
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

void gfxCMICallback(const cmiToken *t);

#ifdef __cplusplus
}
#endif

#endif




