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

struct WRITE_PARAM {
  int type;  // output type
  int accum; // if non-zero, the amount of accumulation rounds
  int width,height;  // dimensions of image
  int dump;  // if non-zero, take pixels from screen
};

struct WRITE_IMAGE {
  unsigned char *data;
  struct WRITE_PARAM param;
};

int writeFile(char *name, struct WRITE_PARAM *p);

#endif

