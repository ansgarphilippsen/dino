#ifndef _SGI_STEREO_H
#define _SGI_STEREO_H

#include <X11/extensions/SGIStereo.h>
#include <X11/extensions/XSGIvc.h>

enum {SGI_STEREO_NONE=0,
      SGI_STEREO_LOW=1,
      SGI_STEREO_HIGH=2};


struct SGI_STEREO {
  Display *display;
  GLXDrawable drawable;

  int mode;

  XSGIvcVideoFormatInfo vc_stereo,vc_mono;

  long mask;
  char stereo_high[256];
  int resx,resy,y_offset;
};


int SGIStereoInit(Display *display, GLXDrawable drawable);
int SGIStereoCommand(int mode);
void SGIStereoDrawBuffer(GLenum mode);

#endif






