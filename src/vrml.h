#ifndef VRML_H
#define VRML_H

#include <stdio.h>

#include "dbm.h"

int writeVRML(FILE *f);
int writeVRMLHeader(FILE *f);
int writeVRMLFooter(FILE *f);
int writeVRMLStructObj(FILE *f,struct STRUCT_OBJ *obj);
int writeVRMLScalObj(FILE *f,struct SCAL_OBJ *obj);
int writeVRMLSurfObj(FILE *f,surfObj *obj);

#endif
