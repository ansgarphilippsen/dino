#ifndef SCAL_OBJ_H
#define SCAL_OBJ_H

#include "rex.h"
#include "render.h"
#include "set.h"
#include "surf_obj.h"

enum {SCAL_MODE_POINT,SCAL_MODE_LINE,SCAL_MODE_SURFACE};

struct SCAL_POINT {
  float rad; // radius for grid type
  float c[4]; // color
  float v[4]; // vertex
  float n[4]; // normal
  float val; // value for grid type
  int uvw[4]; // index of nearest uvw grid point (TODO: why 4 elements?)
  int nc;  // neighbour point count
  int fi[4];
};

struct SCAL_VECT {
  float v1[3],v2[3]; /* line in space */
  float grad[3]; /* gradient */
  float n[3]; /* direction */
  float length; /* actual length */
  float c[3]; /* color */
  int uvw[4]; /* grid point */
};

struct SCAL_LINE {
  int pi0,pi1;
};

struct SCAL_FACE {
  float v1[3],v2[3],v3[3];
  float n1[3],n2[3],n3[3];
#ifdef CONTOUR_COLOR
  float c1[4],c2[4],c3[4];
#endif
  int pi0,pi1,pi2; // index pointer to vertices
  int sflag; // selection flag
};

#ifdef VR
struct SCAL_VR {
  unsigned int tex;
  float xyz1[3],xyz2[3];
  float tex1[3],tex2[3];
  unsigned int usize,vsize,wsize;
  float *data;
  float start, end;
};
#endif

struct SCAL_SLAB {
  int usize,vsize,size;
  float *data;
  unsigned char *tex;
  double dir[3],center[3];
  double corner[2][3];
  float point[13][3];
  int pointc;
  int line[144][2];
  int linec;
  double bound[4][3];
  unsigned int texname;
};

// 2 structs for contour to surface conversion

#define SCAL2SURF_MAX_FV 16

struct SCAL2SURF_VERT {
  float p[3],n[3]; // position and normal
  int fi[SCAL2SURF_MAX_FV],fc;    // face indeces and count
  int flag;
};

struct SCAL2SURF_FACE {
  int i1,i2,i3;     // vertex indices
  float area, n[3]; // face area and normal
  int flag;
  int fc,fi[SCAL2SURF_MAX_FV];
  float ref[3*SCAL2SURF_MAX_FV];
};

// surf object

typedef struct SCAL_OBJ {
  int type;
  char name[256];
  Select select;
  struct RENDER render;
  float r,g,b,a;
  int u_start,u_end,v_start,v_end,w_start,w_end;
  int u_center,v_center,w_center;
  int u_size,v_size, w_size;
  int ou_size,ov_size, ow_size;
  float level;
  int step;
  struct SCAL_POINT *point;
  int point_count;
  int point_max;
  struct SCAL_LINE *line;
  int line_count;
  int line_max;
  struct SCAL_FACE *face;
  int face_count;
  int face_max;
  struct SCAL_VECT *vect;
  int vect_count;
  int vect_max;
#ifdef VR
  struct SCAL_VR vr;
#endif
  struct SCAL_FIELD *field;
  struct DBM_SCAL_NODE *node;
#ifdef BONO
  struct SCAL_OCTREE octree;
#endif
  struct SCAL_SLAB slab;
  int contour_method;
}scalObj;

int scalObjCommand(struct DBM_SCAL_NODE *node,scalObj *obj,int wc,char **wl);
int scalObjComRenew(scalObj *obj,int wc,char **wl);
int scalObjComSet(scalObj *obj,int wc,char **wl);
int scalObjComGet(scalObj *obj,int wc,char **wl);

int scalObjRenew(scalObj *obj, Set *set, Select *sel);
int scalObjSet(scalObj *obj, Set *s, int flag);
int scalObjGet(scalObj *obj, char *prop);

int scalObjRender(struct DBM_SCAL_NODE *node, scalObj *obj, int wc, char **wl);
int scalObjCp(scalObj *o1, scalObj *o2);
int scalAddPoint(struct SCAL_POINT **, int *c, int *m, double *v);
int scalAddLine(struct SCAL_LINE **, int *c, int *m, int i1, int i2);
int scalAddFace(struct SCAL_FACE **, int *c, int *m, int i1, int i2, int i3);
int scalAddVect(struct SCAL_VECT **f, int *c, int *m, double *v, double *n);

int scalGrid(scalObj *obj, Select *sel);
int scalGrad(scalObj *obj, Select *sel);

int scalSlab(scalObj *obj, Select *sel);
int scalSlabIntersect(scalObj *obj);
int scalSlabIsSelected(scalObj *obj, int ec, Select *sel);
int scalSlabEvalPOV(scalObj *obj, int c, POV *poe);


#ifdef VR
int scalVR(scalObj *obj, Select *sel);
int scalDrawVR(scalObj *obj);
#endif

int scalSetDefault(scalObj *obj);

int scalObjIsWithin(scalObj *obj, float *p, float d2);

int scalObj2Surf(scalObj *obj,surfObj *surf);

#endif
