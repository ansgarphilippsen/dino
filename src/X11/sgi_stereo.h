#ifndef _SGI_STEREO_H
#define _SGI_STEREO_H

#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/GLw/GLwMDrawA.h>
#include <X11/extensions/SGIStereo.h>
#include <X11/extensions/XSGIvc.h>

enum {SGI_STEREO_NONE=0,
      SGI_STEREO_LOW=1,
      SGI_STEREO_HIGH=2};

#define SGI_STEREO_ON  1
#define SGI_STEREO_OFF 0

struct SGI_STEREO_INFO {
  Display *display;
  GLXDrawable drawable;

  int active;
  int available_mode;

  XSGIvcVideoFormatInfo vc_stereo,vc_mono;

  long mask;
  char stereo_high[256];
  int resx,resy,y_offset;
};


int SGIStereoInit(Display *display, GLXDrawable drawable, int m);
int SGIStereoCommand(int mode);
void SGIStereoDrawBuffer(GLenum mode);

#endif






