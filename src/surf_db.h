#include <stdio.h>

#ifndef SURF_H
#define SURF_H

#include "dbm_common.h"
#include "render.h"
#include "rex.h"
#include "surf_obj.h"
#include "cubearray.h"
#include "set.h"
#include "transform.h"
#include "prop.h"

#ifdef LINUX
extern union DBM_NODE _DBM_NODE;
#endif
#ifdef SGI
//extern union DBM_NODE;
#endif
#ifdef DEC
extern union DBM_NODE;
#endif

enum {SURF_SMODE_ALL, SURF_SMODE_ANY};

enum {SURF_SEL_ANUM,
      SURF_SEL_ANAME,
      SURF_SEL_RNUM,
      SURF_SEL_RNAME,
      SURF_SEL_CHAIN,
      SURF_SEL_MODEL,
      SURF_SEL_OCC,
      SURF_SEL_BFAC,
      SURF_SEL_ELE,
      SURF_SEL_WITHIN,
      SURF_SEL_OBJECT,
      SURF_SEL_CP,
      SURF_SEL_X,
      SURF_SEL_Y,
      SURF_SEL_Z};

enum {SURF_PROP_COLOR,
      SURF_PROP_SMODE,
      SURF_PROP_ROT,
      SURF_PROP_TRANS,
      SURF_PROP_RTC,
      SURF_PROP_RCEN};

typedef struct DBM_SURF_NODE {
  DBM_NODE_COMMON_HEADER
  Select *restrict;
  float attach_cutoff;
  int attach_flag;
  union DBM_NODE *last_attach;
  surfObj *obj;
  unsigned char *obj_flag;
  int obj_max;
  struct SURF_VERTICE *v;
  int vc;
  struct SURF_FACE *f;
  int fc;
  cubeArray *ca;
  int smode;

  PropxTable pt;
  
  float cprop_min[PROP_MAX_VALUES],cprop_max[PROP_MAX_VALUES];

}dbmSurfNode;

#define SURF_RENORMAL_MAX_FACEI 16

struct SURF_RENORMAL_VERTICE {
  struct SURF_VERTICE *v;
  float n[3];
  int facei[SURF_RENORMAL_MAX_FACEI],facec;
};


int surfNewNode(dbmSurfNode *node);

int surfCommand(dbmSurfNode *,int wc, char **wl);
int surfComNew(dbmSurfNode *,int wc, char **wl);
int surfComSet(dbmSurfNode *,int wc, char **wl);
int surfComGet(dbmSurfNode *,int wc, char **wl);
int surfComDel(dbmSurfNode *,int wc, char **wl);
int surfComAttach(dbmSurfNode *,int wc, char **wl);
int surfComRestrict(dbmSurfNode *,int wc, char **wl);

int surfNew(dbmSurfNode *node, char *name, int type, Set *set, Select *sel,int vflag);
int surfSet(dbmSurfNode *node, Set *set);
int surfGet(dbmSurfNode *node, char *prop);

surfObj *surfNewObj(dbmSurfNode *node, char *name);
int surfDelObj(dbmSurfNode *node, char *name);

//int surfRecolor(dbmSurfNode *,surfObj *obj);

int surfIsSelected(dbmSurfNode *,struct SURF_VERTICE *, Select *);
int surfEvalPOV(dbmSurfNode *, struct SURF_VERTICE *,  POV *);

int surfAttach(dbmSurfNode *node, union DBM_NODE *attach, int);

int surfFix(dbmSurfNode *node);

int surfBuildCA(dbmSurfNode *node);

int surfSetDefault(dbmSurfNode *,surfObj *obj);

int surfGetRangeVal(dbmSurfNode *node, struct SURF_VERTICE *, const char *prop, float *rval);

int surfCalcMinMax(dbmSurfNode *);
int surfGetMinMax(dbmSurfNode *, const char *, float *, float *);

int surfGetRangeXYZVal(dbmSurfNode *node, const char *prop, float *p, float *r);

int surfDraw(dbmSurfNode *, int);
int surfDrawObj(surfObj *obj);

int surfPrepObj(surfObj *obj);

int surfRenormalize(dbmSurfNode *);
#endif
