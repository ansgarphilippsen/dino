#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

#ifndef EXTENSION_H
#define EXTENSION_H

/* new API */
XDevice *extFindDevice(Display *dpy, char *name);

/* deprecated */
XDevice *extDialBoxInit(Display *dpy);
XDevice *extSpaceballInit(Display *dpy);

#endif
