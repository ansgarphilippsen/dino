#ifndef SURF_READ_H
#define SURF_READ_H

#include "surf_db.h"

struct ADS_ENTRY {
  int flag;
  int u,v;
  float x,y,z;
  int vert_p,vertm_p;
  float c[10];
};

struct GRASP_HEADER {
  int d1;
  char format[80];
  char d2[4];
  char d3[4];
  char content1[80];
  char d4[4];
  char d5[4];
  char content2[80];
  char d6[4];
  char d7[4];
  char count[80];
  char d8[4];
  char d9[4];
  char midpoint[80];
  char d10[4];
};

int msmsRead(FILE *f1, FILE *f2, union DBM_NODE *node, int);
int mspRead(FILE *f1, union DBM_NODE *node);
int adsRead(FILE *f, union DBM_NODE *node);
int graspRead(FILE *f, union DBM_NODE *node,int);


#endif
