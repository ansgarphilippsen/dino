/************************************
  
  these are the _required_ functions
  that must be provided by any GUI

*************************************/			 

#ifndef _GUI_EXT_H
#define _GUI_EXT_H

#ifdef USE_CMI
#include "cmi.h"
#endif

#ifdef USE_CMI
int guiInit(int *, char ***);
#else
int guiInit(void (*)(int, char **), int*, char ***);
#endif

int guiMainLoop(void);

int guiMessage(char *m);

int guiResolveColor(const char *name, float *r, float *g, float *b);

void guiSwapBuffers(void);


#ifdef USE_CMI
void guiCMICallback(const cmiToken *t);
#endif

#endif
