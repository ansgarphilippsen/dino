#ifndef _SPACETEC_H
#define _SPACETEC_H

#include <X11/Xlib.h>

#ifdef SPACETEC

#include <spwdata.h>
#include <spwinput.h>
#include <spwx11.h>

#endif

int spacetecInit(Display *dpy, Window w, char *n);
void spacetecEventHandler(Display *dpy, XEvent *e);
void spacetecMotion(Display *dpy, float data[7], void *appdata);
void spacetecButtonPress(Display *dpy, int button, void *appdata);
void spacetecButtonRelease(Display *dpy, int button, void *appdata);

#endif
