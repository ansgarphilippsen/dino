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

#define WRITE_POV_DEFAULT 1
#define WRITE_POV_PATCH   2
#define WRITE_POV_SMOOTH  3
#define WRITE_POV_NOCOLOR 4

#define WRITE_POV_V31 1
#define WRITE_POV_V35 2

#define WRITE_POV_PLANE 0x1
#define WRITE_POV_BOX   0x2
#define WRITE_POV_RAW   0x4

#define WRITE_VA_POINTS 1
#define WRITE_VA_LINES 2
#define WRITE_VA_TRIS 3

int writePOV(FILE *fini,FILE *fpov,char *, int, int, int);

#endif
