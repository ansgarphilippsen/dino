#ifndef _GRID_H
#define _GRID_H

#include <stdio.h>

#include "render.h"

struct GRID_VERT
{
  float v[3],n[3],c[3];
};

struct GRID_TRI
{
  float v1[3],n1[3],c1[3];
  float v2[3],n2[3],c2[3];
  float v3[3],n3[3],c3[3];
};

typedef struct GRID_FIELD
{
  int width,height;
  float scale_x,scale_y,scale_z;
  float offset_x,offset_y,offset_z;
  unsigned char *data;
}gridField;

typedef struct GRID_OBJ
{
  int type;
  char name[256];
  struct RENDER render;
  gridField *field;
  struct DBM_GRID_NODE *node;
  struct GRID_TRI *tri;
  int tri_count;
  unsigned char r,g,b;
}gridObj;

typedef struct DBM_GRID_NODE 
{
  int type;
  char name[256];
  char path[256];
  gridObj **obj;
  int obj_count,obj_max;
  gridField field;
  float scalx,scaly,scalz;
}dbmGridNode;

int gridNewNode(dbmGridNode *node);
int gridReadNode(int fd, char *fn,dbmGridNode *node);
int gridCommand(dbmGridNode *node, int wc, char **wl);
int gridNew(dbmGridNode *node, int wc, char **wl);

int gridSet(dbmGridNode *node, int wc, char **wl);

gridObj *gridNewObj(dbmGridNode *node, char *name);
int gridDelObj(dbmGridNode *node, char *name);

int gridObjCommand(gridObj *obj, int wc, char **wl);
int gridSetDefault(gridObj *obj);

int gridObjRenew(gridObj *obj, char *set, char *sel);

int gridObjDraw(gridObj *obj);

#endif
