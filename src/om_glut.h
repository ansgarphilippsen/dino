#ifndef _OM_GLUT
#define _OM_GLUT

#include <GL/gl.h>
#ifdef USE_FREEGLUT
#include "GL/freeglut.h"
#else
#include <GL/glut.h>
#endif

int omInit(void);

int omAddDB(const char *name);
int omDelDB(const char *name);
int omAddObj(const char *db, const char *name);
int omDelObj(const char *db, const char *name);
int omHideObj(const char *db, const char *name);
int omShowObj(const char *db, const char *name);

#endif
