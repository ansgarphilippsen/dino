#ifndef _GUI_OSX_H
#define _GUI_OSX_H

#import <Cocoa/Cocoa.h>

enum {GUI_STEREO_OFF=0, GUI_STEREO_NORMAL=1, GUI_STEREO_SPLIT=2};

struct GUI
{
    int redraw;
};

void guiTimeProc();
void gui_mouse_input(int eventType, int mask, int x, int y);
//Object Menu
int omAddDB(const char *name);
int omDelDB(const char *name);
int omAddObj(const char *db, const char *name);
int omDelObj(const char *db, const char *name);
int omHideObj(const char *db, const char *name);
int omShowObj(const char *db, const char *name);

#endif
