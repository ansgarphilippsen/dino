#ifndef STRUCT_OBJ_H
#define STRUCT_OBJ_H

#include "rex.h"
#include "render.h"
#include "struct_common.h"
#include "cgfx.h"
#include "set.h"

#ifdef LINUX
extern struct DBM_SET _DBM_SET;
#endif
#ifdef SGI
//extern struct DBM_SET;
#endif
#ifdef DEC
extern struct DBM_SET;
#endif

typedef float structVect[3];

typedef struct STRUCT_OBJ {
  int type;
  char name[256];
  struct DBM_STRUCT_NODE *node;
  Select select;
  struct RENDER render;
  double r,g,b;
  struct STRUCT_OBJ_MODEL {
    float r,g,b;
    struct STRUCT_DB *mp;
  }*model;
  int model_count;
  struct STRUCT_OBJ_CHAIN {
    float r,g,b;
    struct STRUCT_CHAIN *cp;
  }*chain;
  int chain_count;
  struct STRUCT_OBJ_RESIDUE {
    float r,g,b,rad;
    struct STRUCT_RESIDUE *rp;
  }*residue;
  int residue_count;
  struct STRUCT_OBJ_ATOM {
    int label;
    int cc;
    struct STRUCT_ATOM_PROP prop;
    struct STRUCT_ATOM *ap;
  }*atom;
  int atom_count;
  unsigned char *atom_flag;
  /*
  structVect *trace_v1;
  structVect *trace_v2;
  */
  struct STRUCT_BOND *bond;
  int bond_count;
  struct STRUCT_SINGULAR_BOND *s_bond;
  int s_bond_count;
  struct STRUCT_NBOND *nbond;
  int nbond_count;
  cgfxVA va;  /* vertex array for spline objects */

  cgfxVA sphere; // generic sphere for CPK speedup

  unsigned int sphere_list;
  
}structObj;


int structObjCommand(struct DBM_STRUCT_NODE *n,structObj *obj,int wc,char **wl);
int structObjComSet(structObj *obj, int wc, char **wl);
int structObjComGet(structObj *obj, int wc, char **wl);
int structObjComRenew(structObj *obj, int wc, char **wl);

int structObjSet(structObj *obj, Set *set, int flag);
int structObjGet(structObj *obj, char *prop);
int structObjRenew(structObj *obj, Set *set, Select *sel);

int structObjRender(structObj *obj, int wc, char **wl);
int structObjCp(structObj *o1, structObj *o2);

int structObjConnect(struct DBM_STRUCT_NODE *n, structObj *o, Select *);
int structObjTrace(struct DBM_STRUCT_NODE *n, structObj *o, Select *);
int structObjNbond(struct DBM_STRUCT_NODE *n, structObj *o, Select *);

int structObjIsWithin(structObj *obj, float *p, float d2);

int structSmooth(structObj *obj);

int structObjGenVA(structObj *obj);

#endif
