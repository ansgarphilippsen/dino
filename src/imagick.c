#ifdef IMAGICK
#include "imagick.h"

int imagickWriteImage(imagickImage *img, char *name)
{
  ExceptionInfo exception;

  Image *image;

  ImageInfo *image_info;

  MagickIncarnate(NULL);
  GetExceptionInfo(&exception);

  return 0;
}


#endif
