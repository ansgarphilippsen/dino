/************************************
  
  these are the _required_ functions
  that must be provided by any GUI

*************************************/			 

#ifndef _GUI_EXT_H
#define _GUI_EXT_H

#include "cmi.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "write.h"

// initialize the gui with the command line args
int guiInit(int , char **);

// main application loop
int guiMainLoop(void);

// message to appear in status bar
int guiMessage(char *m);

//int guiGetImage(struct WRITE_IMAGE *img);
// opengl offscreen context creation and destruction
int guiCreateOffscreenContext(int w, int h, int af);
int guiDestroyOffscreenContext(int n);

// swap buffers (necessary, could be provided by backend)
void guiSwapBuffers(void);

// returns value !=0 if stereo is available
int guiQueryStereo(void);
// turns stereo mode on (m!=0) or off(m=0), returns <0 on error!
int guiSetStereo(int m);

// interface for CMI functionality
void guiCMICallback(const cmiToken *t);


#ifndef INTERNAL_COLOR
int guiResolveColor(const char *name, float *r, float *g, float *b);
#endif

#ifdef __cplusplus
}
#endif



#endif
