#ifndef GEOM_OBJ_H
#define GEOM_OBJ_H

#include "render.h"
#include "cgfx.h"

//#ifdef LINUX
//extern struct DBM_GEOM_NODE _DBM_GEOM_NODE;
//struct DBM_SET;
//#else

struct DBM_GEOM_NODE;
struct DBM_SET;

//#endif

/* geometric primitives */

enum          {GEOM_ELE_UNKNOWN=0,
	       GEOM_ELE_POINT, 
	       GEOM_ELE_LINE, 
	       GEOM_ELE_TRI,
	       GEOM_ELE_RECT,
	       GEOM_ELE_LABEL};

typedef struct GEOM_POINT {
  float ps;  /* pointsize for simple mode */
  float r;   /* radius of sphere in custom mode */
  float c[4],v[3];
}geomPoint;

typedef struct GEOM_LINE {
  float lw;   /* linewidth in simple mode */
  float rmat[16],length,r;  /* cylinder props */
  float c[4],v1[3],v2[3];
}geomLine;

typedef struct GEOM_TRI {
  float lw;
  int fill;
  float c[4],v1[3],v2[3],v3[3];
}geomTri;

typedef struct GEOM_RECT {
  float lw;
  int fill;
  float c[4],v1[3],v2[3],v3[3],v4[4];
}geomRect;

typedef struct GEOM_LABEL {
  char s[128];
  float p[3],c[3];
}geomLabel;

typedef struct GEOM_SPHERE {
  float c[4],v[3],r;
}geomSphere;

typedef struct GEOM_CYL {
  float rmat[16],length;
  float c[4],v1[3],v2[3],r;
}geomCyl;

typedef struct GEOM_OBJ {
  int type;
  char name[64];

  struct DBM_GEOM_NODE *node;
  struct RENDER render;

  geomPoint *point;
  int point_count,point_max;

  geomLine *line;
  int line_count,line_max;

  geomTri *tri;
  int tri_count,tri_max;

  geomRect *rect;
  int rect_count,rect_max;

  geomLabel *label;
  int label_count, label_max;

  geomSphere *sphere;
  int sphere_count,sphere_max;

  geomCyl *cyl;
  int cyl_count,cyl_max;

  cgfxVA va;

  double pos[3],dir[3],eq[4];

  float r,g,b;

}geomObj;

geomObj *geomNewObj(struct DBM_GEOM_NODE *node, char *name, int type);
int geomDelObj(struct DBM_GEOM_NODE *node,char *name);

int geomObjCommand(struct DBM_GEOM_NODE *, struct GEOM_OBJ *, int wc, char **wl);

int geomObjGet(geomObj *obj, int wc, char **wl);
int geomObjAdd(geomObj *obj,int wc, char **wl);
int geomObjDel(geomObj *obj,int wc, char **wl);
int geomObjList(geomObj *obj, int wc, char **wl);
int geomObjSet(geomObj *obj, struct DBM_SET *, int flag);
int geomEleMatch(char *expr,int type, int n);
int geomObjRegen(geomObj *);
int geomSmooth(geomObj *obj);

#endif

