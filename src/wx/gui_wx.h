#ifndef _GUI_WX_H
#define _GUI_WX_H

extern "C" {

#include "cmi.h"

int guiInit(int *argc, char ***argv);
int guiMainLoop(void);
int guiMessage(char *m);
void guiSwapBuffers(void);
void guiCMICallback(const cmiToken *t);

}

#endif
