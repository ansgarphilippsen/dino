#include <stdio.h>

#ifdef USE_MESA
#include <MesaGL/gl.h>
#include <MesaGL/glu.h>
#include <MesaGL/glx.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#ifdef LINUX
#include <tiff.h>
#include <tiffio.h>
#endif
#ifdef SGI
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif
#ifdef DEC
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif

#include "dbm.h"
#include "struct_db.h"
#include "scal_db.h"
#include "surf_db.h"
#include "xtal.h"
#include "cgfx.h"

#ifndef WRITE_H
#define WRITE_H


enum             {WRITE_CENTER_VIEW,
		  WRITE_LEFT_VIEW,
		  WRITE_RIGHT_VIEW};

enum {WRITE_TYPE_TIFF, WRITE_TYPE_PNG};

XVisualInfo* writeGetVis(int accum_flag);

int writeFile(char *name, int type, int accum, float scale, int dump);

int writeImage2RGB(XImage *image,char *name);

int writeImage2Tiff(XImage *image,char *name);

int writeImage2PNG(XImage *image,char *name);

int writeRedraw(int mode, int accum, int flag);
int writeXGetPixel(XImage *image,int x,int y);


#endif

