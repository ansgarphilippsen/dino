#ifndef _RASTER_H
#define _RASTER_H

#include "struct_db.h"
#include "scal_db.h"
#include "surf_db.h"
#include "grid_db.h"
#include "geom_db.h"
#include "cgfx.h"
#include "transform.h"

int writeRaster(FILE *f);
int writeRasterStructObj(FILE *f,struct STRUCT_OBJ *obj);
int writeRasterScalObj(FILE *f,struct SCAL_OBJ *obj);
int writeRasterSurfObj(FILE *f,struct SURF_OBJ *obj);
int writeRasterGridObj(FILE *f,struct GRID_OBJ *obj);
int writeRasterGeomObj(FILE *f,struct GEOM_OBJ *obj);
int writeRasterCell(FILE *f,struct XTAL *xtal);

int writeRasterTransform( transMat *m, float *v);
int writeRasterTransformd( transMat *m, double *v);

int writeRasterTriangle(FILE *f, transMat *m,
			float *v1, float *v2, float *v3,
			float *n1, float *n2, float *n3, 
			float *c1, float *c2, float *c3);
int writeRasterSphere(FILE *f, transMat *m, int t, float *v1,float r, float *c);
int writeRasterCylinder(FILE *f, transMat *m,int t, float *v1,float *v2, float r, float *c);
int writeRasterMaterial(FILE *f, transMat *m,int t, float mp, float ms, float *c, float cl);

#endif
