#include <stdio.h>

#ifndef _POV_H
#define _POV_H

#include "dbm.h"
#include "struct_obj.h"
#include "surf_obj.h"
#include "scal_obj.h"
#include "grid_obj.h"
#include "geom_obj.h"
#include "render.h"
#include "transform.h"

#define WRITE_POV_DEFAULT 0
#define WRITE_POV_NEW   1
#define WRITE_POV_SMOOTH 2

#define WRITE_POV_PLANE 0x1
#define WRITE_POV_BOX   0x2

int writePOV(FILE *fini,FILE *fpov,char *, int, int);

int writePOVScene(FILE *f);
int writePOVStructObj(FILE *f, structObj *o, int k, float *lim);
int writePOVSurfObj(FILE *f, surfObj *o, int k, float *lim);
int writePOVScalObj(FILE *f, scalObj *o, int k, float *lim);
int writePOVGridObj(FILE *f, gridObj *o, int k, float *lim);
int writePOVGeomObj(FILE *f, geomObj *o, int k, float *lim);

int writePOVTransform(transMat *m,float *v1, float *v2,int );

int writePOVCheckLim(float *v, float *lim);

int writePOVCheckName(char *name);

#endif
