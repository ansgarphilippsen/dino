#ifndef _HSC_H
#define _HSC_H

#include "cgfx.h"
#include "render.h"

typedef struct _HSC {
  cgfxSplinePoint *spoint;
  int spoint_count;
  splinePoint *point;
  int point_count;
  cgfxVA va;

  struct RENDER *render;
}HSC;

int hscGenerate(HSC *hsc);

int hscGenSpline(HSC *hsc);
int hscBuild(HSC *hsc);
int hscGenEndPoints(HSC *hsc);

#endif
