#ifndef _GUI_WX_H
#define _GUI_WX_H

extern "C" {

#include "cmi.h"
#include "write.h"

int guiInit(int *argc, char ***argv);
int guiMainLoop(void);
int guiMessage(char *m);
void guiSwapBuffers(void);
int guiGetImage(struct WRITE_IMAGE *i);
void guiCMICallback(const cmiToken *t);

int guiCreateOffscreenContext(int w, int h, int af);
int guiDestroyOffscreenContext(int n);

int guiQueryStereo(void);
int guiSetStereo(int m);


}

#endif
