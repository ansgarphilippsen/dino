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


int guiInit(int , char **);

int guiMainLoop(void);

int guiMessage(char *m);

//int guiGetImage(struct WRITE_IMAGE *img);
int guiCreateOffscreenContext(int w, int h, int af);
int guiDestroyOffscreenContext(int n);

void guiSwapBuffers(void);

int guiQueryStereo(void);

void guiCMICallback(const cmiToken *t);

#ifdef SGI_STEREO
int guiSetStereo(int m);
#endif

#ifndef INTERNAL_COLOR
int guiResolveColor(const char *name, float *r, float *g, float *b);
#endif

#ifdef __cplusplus
}
#endif



#endif
