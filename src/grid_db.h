#include <stdio.h>

#ifndef GRID_DB_H
#define GRID_DB_H

#include "dbm_common.h" 
#include "render.h"
#include "rex.h"
#include "grid_obj.h"
#include "cubearray.h"
#include "set.h"
#include "transform.h"

//#ifdef LINUX
//extern union DBM_NODE _DBM_NODE;
//#endif
//#ifdef SGI
//extern union DBM_NODE;
//#endif
//#ifdef DEC
//extern union DBM_NODE;
//#endif

enum {GRID_SMODE_ALL, GRID_SMODE_ANY};

enum {GRID_SEL_HEIGHT,
      GRID_SEL_U,
      GRID_SEL_V,
      GRID_SEL_ANUM,
      GRID_SEL_ANAME,
      GRID_SEL_RNUM,
      GRID_SEL_RNAME,
      GRID_SEL_CHAIN,
      GRID_SEL_MODEL,
      GRID_SEL_OCC,
      GRID_SEL_BFAC,
      GRID_SEL_ELE,
      GRID_SEL_WITHIN,
      GRID_SEL_OBJECT};

enum {GRID_PROP_COLOR,
      GRID_PROP_STEP,
      GRID_PROP_ROT,
      GRID_PROP_TRANS,
      GRID_PROP_LSTART,
      GRID_PROP_LEND,
      GRID_PROP_LSTEP,
      GRID_PROP_SCALEZ,
      GRID_PROP_SCALEXY};

typedef struct GRID_POINT
{
  int n;    // position in array
  int x,y,z;
  union DBM_NODE *attach_node;
  int attach_element;
  unsigned char attach_flag,restrict;
}gridPoint;

typedef struct GRID_FIELD
{
  int width,height;
  float scale_x,scale_y,scale_z;
  float offset_x,offset_y,offset_z;
  gridPoint *point;
  int point_count;  // =width*height
}gridField;

// do not change as TIFFGet* expects this type
typedef unsigned char grid_t;

typedef struct GRID_TEXTURE
{
  int flag;
  char name[64];
  int width,height;
  grid_t *data;
}gridTexture;

typedef struct DBM_GRID_NODE {
  DBM_NODE_COMMON_HEADER
  gridObj *obj;
  unsigned char *obj_flag;
  int obj_max;
  float attach_cutoff;
  int attach_flag;
  gridField field;
  float scalx,scaly,scalz;

  // probably not necessary, since grid is regularly arranged
  // cubeArray *ca; 

  int smode;
  transMat transform_save;

  gridTexture *texture;
  int texture_count, texture_max;

}dbmGridNode;

int gridNewNode(dbmGridNode *node);

int gridCommand(dbmGridNode *,int wc, char **wl);
int gridComNew(dbmGridNode *,int wc, char **wl);
int gridComSet(dbmGridNode *,int wc, char **wl);
int gridComGet(dbmGridNode *,int wc, char **wl);
int gridComDel(dbmGridNode *,int wc, char **wl);
int gridComAttach(dbmGridNode *,int wc, char **wl);
int gridComRestrict(dbmGridNode *,int wc, char **wl);

int gridNew(dbmGridNode *node, char *name, int type, Set *set, Select *sel,int vflag);
int gridSet(dbmGridNode *node, Set *set);
int gridGet(dbmGridNode *node, char *prop);

gridObj *gridNewObj(dbmGridNode *node, char *name);
int gridDelObj(dbmGridNode *node, char *name);

int gridIsSelected(dbmGridNode *,gridPoint *, Select *);
int gridEvalPOV(dbmGridNode *, gridPoint *,  POV *);

int gridAttach(dbmGridNode *node, union DBM_NODE *attach, int);

int gridUVWtoXYZ(gridField *field, float *uvw, float *xyz);
int gridXYZtoUVW(gridField *field, float *xyz, float *uvw);

int gridBuildCA(dbmGridNode *node);

int gridSetDefault(dbmGridNode *,gridObj *obj);

int gridGetRangeVal(dbmGridNode *node, const char *, gridPoint *, float *r);
int gridGetRangeXYZVal(dbmGridNode *node, const char *prop, float *p, float *r);

int gridRead(int fd, char *fn, dbmGridNode *node);
int gridTiffRead(int fd, char *fn, dbmGridNode *node);
int gridPngRead(int fd, char *fn, dbmGridNode *node);
int gridTiffReadTex(char *fn, gridTexture *tex);

int gridLoad(dbmGridNode *node,char *fn, char *tn);
int gridLoadTexture(char *fn, gridTexture *tex);

int gridDraw(dbmGridNode *, int);
int gridDrawObj(gridObj *obj);

#endif
