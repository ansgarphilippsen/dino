#include <stdio.h>

#include <GL/gl.h>
#include <GL/glu.h>
#ifdef X11_GUI
#include <GL/glx.h>
#endif

#ifdef FORMAT_TIFF
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

int writeFile(char *name, int type, int accum, float scale, int dump);

#endif

