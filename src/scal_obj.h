#ifndef SCAL_OBJ_H
#define SCAL_OBJ_H

#include "rex.h"
#include "render.h"
#include "set.h"

enum {SCAL_MODE_POINT,SCAL_MODE_LINE,SCAL_MODE_SURFACE};

#ifdef LINUX
extern struct SCAL_FIELD _SCAL_FIELD;
extern struct DBM_SCAL_NODE _DBM_SCAL_NODE;
extern struct DBM_SET _DBM_SET;
extern struct SCAL_OCTREE_ENTRY _SCAL_OCTREE_ENTRY;
#endif
#ifdef SGI
//extern struct SCAL_FIELD;
//extern struct DBM_SCAL_NODE;
//extern struct DBM_SET;
//extern struct SCAL_OCTREE_ENTRY;
#endif
#ifdef DEC
extern struct SCAL_FIELD;
extern struct DBM_SCAL_NODE;
extern struct DBM_SET;
extern struct SCAL_OCTREE_ENTRY;
#endif

struct SCAL_POINT {
  float rad;
  float c[4];
  float v[4];
  float n[4];
  float val;
  int uvw[4];
  int nc;
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
  //  struct SCAL_POINT *p0,*p1;
  //  float w;
  int pi0,pi1;
  //  float v1[3],v2[3];
};

struct SCAL_FACE {
  float v1[3],v2[3],v3[3];
  float n1[3],n2[3],n3[3];
  //  float n[3];
  //  float c[4];
  //  struct SCAL_POINT *p0,*p1,*p2;
  //  int pi0,pi1,pi2;
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
  int method;
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


#endif
