#ifndef _BSPLINE_H
#define _BSPLINE_H

#include "cgfx.h"

int bsplineGenerate(cgfxSplinePoint *sp, cgfxPoint **pp, int n, int detail, int intpol);

int bsplineGen(float *x,float *y, int n, float yp1, float ypn, float *y2);
int bsplineGet(float *xa, float *ya, float *y2a, int n, float x, float *y);
int bspline_c2p(float *v[3], float *p[3]);

#endif
