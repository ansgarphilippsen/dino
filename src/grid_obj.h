#ifndef GRID_OBJ_H
#define GRID_OBJ_H

#include "set.h"
#include "render.h"

enum {GRID_DOTS, GRID_LINES, GRID_FACES};

//#ifdef LINUX
//extern struct DBM_GRID_NODE _DBM_GRID_NODE;
//extern struct DBM_SET _DBM_SET;
//#endif
//#ifdef SGI
//extern struct DBM_GRID_NODE;
//extern struct DBM_SET;
//#endif
//#ifdef DEC
//extern struct DBM_GRID_NODE;
//extern struct DBM_SET;
//#endif

extern struct GRID_POINT _GRID_POINT;

typedef struct GRID_VERT
{
  int p,sel;
  struct GRID_POINT *gp;
  int p1,p2,p3,p4;
  float v[3],n[3],c[4],t[2];
  float z;
}gridVert;

typedef struct GRID_FACE
{
  float *v1,*n1,*c1; 
  float *v2,*n2,*c2; 
  float *v3,*n3,*c3; 
  float *t1,*t2,*t3;
}gridFace;

typedef struct GRID_OBJ {
  int type;
  char name[256];
  struct DBM_GRID_NODE *node;
  Select select;
  struct RENDER render;
  double r,g,b;

  gridVert *vert;
  int vertc;
  gridFace *face;
  int facec;

  int step;

  int map;
  unsigned int texname;

  float level_start,level_end, level_step;
}gridObj;


int gridObjCommand(struct DBM_GRID_NODE *node, gridObj *obj, int wc, char **wl);
int gridObjComSet(gridObj *obj, int wc, char **wl);
int gridObjComGet(gridObj *obj, int wc, char **wl);
int gridObjComRenew(gridObj *obj, int wc, char **wl);
int gridObjComMap(gridObj *obj, int wc, char **wl);

int gridObjSet(gridObj *obj, Set *set, int flag);
int gridObjGet(gridObj *obj, char *prop);
int gridObjRenew(gridObj *obj, Set *set, Select *sel, int vflag);

int gridObjRender(struct DBM_GRID_NODE *,gridObj *obj, int wc, char **wl);

int gridGenerate(gridObj *obj, Select *sel);
int gridContour(gridObj *obj, Select *sel);

int gridObjIsWithin(gridObj *obj, float *p, float d2);

#endif
