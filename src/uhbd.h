#include <stdio.h>

#ifndef UHBD_H
#define UHBD_H

#include "dbm.h"

struct UHBD_GRID_HEADER {
  char title[72];
  float scale;
  float res1;
  int grdflg;
  int res2;
  int kmd1;
  int one;
  int km,im,jm,kmd2;
  float h,ox,oy,oz;
  float res3,res4,res5,res6,res7,res8;
  int res9,res10;
};

int uhbdRead(FILE *f, struct DBM_SCAL_NODE *node, int cflag);

#endif
