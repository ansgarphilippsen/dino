#ifndef _SCAL_MC_NEW_H
#define _SCAL_MC_NEW_H

#include "scal_db.h"
#include "set.h"

typedef struct SCAL_MC_NEW_VERT {
  float p[3]; /* position */
  int c[8];  /* neighbour list */
  int cc;     /* neighbour count */
  int uvw[3]; /* index of nearest grid point */
}scalMCNVert;

typedef struct SCAL_MC_NEW_FACE {
  int v1,v2,v3;  /* index of three vertices */
  float n[3];
}scalMCNFace;

typedef struct SCAL_MC_NEW_LINE {
  int v1,v2;  /* index of two vertices */
}scalMCNLine;

typedef struct SCAL_MC_NEW_SAVE_CELL {
  int p[12];
  int c[8];  /* the corners */
}scalMCNSaveCell;

struct SCAL_MC_NEW_ORG {
  int vert_count;
  int vert_max,vert_add;
  scalMCNVert *vert;

  int line_count;
  int line_max,line_add;
  scalMCNLine *line;

  int face_count;
  int face_max,face_add;
  scalMCNFace *face;

  scalMCNSaveCell sc;

  scalObj *obj;
  scalField *field;
  float level;

  Select *select;

  int error;
};

int scalMCN(scalObj *obj, Select *sel);

int scalMCNCell(void);

int scalMCNCalcVert(int u, int v, int w, int id);

int scalMCNAddVert(float *p, int u, int v, int w);

int scalMCNAddLine(int v1, int v2);
int scalMCNAddFace(int v1, int v2, int v3);

int scalMCN2Obj(void);

void scalMCNFaceNormal(float *a, float *b, float *c, float *n);

#endif
