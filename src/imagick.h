#ifdef IMAGICK

#ifndef _IMAGICK_H
#define _IMAGICK_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
/*
  UGLY UGLY UGLY undefs
  to get image magick and
  motif/x11 to like each 
  other
*/
#undef NoValue
#undef XValue
#undef YValue
#undef WidthValue
#undef HeightValue
#undef AllValues
#undef XNegative
#undef YNegative
#undef AspectValue
#undef GreaterValue
#undef LessValue
#undef ForgetGravity
#undef NorthWestGravity
#undef NorthGravity
#undef NorthEastGravity
#undef WestGravity
#undef CenterGravity
#undef EastGravity
#undef SouthWestGravity
#undef SouthGravity
#undef SouthEastGravity
#undef StaticGravity
#undef DoRed
#undef DoGreen
#undef DoBlue
#undef DoMatte

#include <mini-magick/api.h>

typedef struct IMAGICK_IMAGE {
  int width,height;
  unsigned char *rgba;
}imagickImage;

int imagickWriteImage(imagickImage *image, char *name);


#endif

#endif
