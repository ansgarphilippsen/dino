#ifdef USE_MESA
#include <MesaGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef RENDER_H
#define RENDER_H

#ifdef SGI
//extern union DBM_NODE ;
#endif
#ifdef DEC
#endif
#ifdef LINUX
#endif


enum             {RENDER_SIMPLE,
		  RENDER_CPK,
		  RENDER_CUSTOM,
		  RENDER_POINT,
		  RENDER_LINE,
		  RENDER_SURFACE,
		  RENDER_ON,
		  RENDER_OFF,
		  RENDER_HELIX,
		  RENDER_STRAND,
		  RENDER_STRAND2,
		  RENDER_TUBE,
		  RENDER_SLINE,
		  RENDER_CYLINDER,
		  RENDER_HSC,
		  RENDER_EXT_NA};

typedef struct RENDER_MATERIAL {
  float amb[4],diff[4],spec[4], emm[4],shin;
}RenderMaterial;

typedef struct RENDER {
  int show;
  int mode;
  int detail,detail2;
  int nice;
  int dbl_light;
  int face_reverse;
  /* this could all be float */
  double line_width;
  double point_size;
  double bond_width;
  double sphere_radius;
  double helix_width,helix_thickness;
  double strand_width,strand_thickness;
  double arrow_thickness;
  double tube_ratio;
  double sugar_thickness,base_thickness;
  double transparency;
  GLuint list;
  int cgfx_flag;

  int strand_method;
  int helix_method;
  int na_method;

  int cull;

  int stipple_flag;
  GLint stipple_factor;
  GLushort stipple_pattern;
  float stipplei,stippleo;

  float polyf,polyu;

  struct RENDER_MATERIAL mat;
}Render;


int renderSet(struct RENDER *render, int owc, char **owl);
int renderMaterialSet(struct RENDER_MATERIAL *mat, int owc, char **owl);

#endif








