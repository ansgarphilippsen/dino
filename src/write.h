#ifndef _WRITE_H
#define _WRITE_H

#ifdef FORMAT_TIFF
#include <tiff.h>
#include <tiffio.h>
#endif

enum             {WRITE_CENTER_VIEW,
		  WRITE_LEFT_VIEW,
		  WRITE_RIGHT_VIEW};

enum {WRITE_TYPE_TIFF, WRITE_TYPE_PNG};

struct WRITE_IMAGE {
  unsigned char *data;
  int width,height;
};

int writeFile(char *name, int type, int accum, float scale, int dump);

#endif

