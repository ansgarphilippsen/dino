#ifndef _GUI_OSX_H
#define _GUI_OSX_H

#import <Cocoa/Cocoa.h>
#include <sys/time.h>

#include "gui_ext.h"

enum {GUI_STEREO_OFF=0, GUI_STEREO_NORMAL=1, GUI_STEREO_SPLIT=2};

struct GUI
{
    int redraw;
};

void guiTimeProc();
void gui_mouse_down(int s, int x, int y);
void gui_mouse_up(int s, int x, int y);
void gui_mouse_drag(int s, int x, int y);

#endif
