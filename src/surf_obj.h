#ifndef SURF_OBJ_H
#define SURF_OBJ_H

#include "set.h"
#include "render.h"
#include "prop.h"

enum {SURF_DOTS, SURF_LINES, SURF_FACES};

#ifdef LINUX
extern struct DBM_SURF_NODE _DBM_SURF_NODE;
extern struct DBM_SET _DBM_SET;
#endif
#ifdef SGI
//extern struct DBM_SURF_NODE;
//extern struct DBM_SET;
#endif
#ifdef DEC
extern struct DBM_SURF_NODE;
extern struct DBM_SET;
#endif

struct SURF_VERTICE {
  int num;
  float p[3],n[3];
  int id;
  union DBM_NODE *attach_node;
  int attach_element;
  //  struct STRUCT_ATOM *ap;
  int restrict;
  /*
    this is the ideal implementation 
    of custom properties, but not 
    done yet
    Prop prop;
  */
  float cprop[10];
};

struct SURF_FACE {
  int v[3];
};

struct SURF_OBJ_V {
  float c[4], n[3], p[3]; // allows efficient packing for interleaved arrays
  struct SURF_VERTICE *vp; // reference pointer
};

typedef struct SURF_OBJ {
  int type;
  char name[256];
  struct DBM_SURF_NODE *node;
  Select select;
  struct RENDER render;
  double r,g,b;

  struct SURF_OBJ_V *vert;
  int vertc;

  unsigned int list;

  int *face,facec;
  /*
  struct SURF_OBJ_F {
    int v[3];
  }*face;
  int facec;
  */

  unsigned char *vert_flag;
}surfObj;


int surfObjCommand(struct DBM_SURF_NODE *node, surfObj *obj, int wc, char **wl);
int surfObjComSet(surfObj *obj, int wc, char **wl);
int surfObjComGet(surfObj *obj, int wc, char **wl);
int surfObjComRenew(surfObj *obj, int wc, char **wl);

int surfObjSet(surfObj *obj, Set *set, int flag);
int surfObjGet(surfObj *obj, char *prop);
int surfObjRenew(surfObj *obj, Set *set, Select *sel, int vflag);

int surfObjRender(struct DBM_SURF_NODE *,surfObj *obj, int wc, char **wl);

int surfGenerate(surfObj *obj, Select *sel);

int surfObjIsWithin(surfObj *obj, float *p, float d2);

#endif
