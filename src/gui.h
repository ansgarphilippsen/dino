#ifndef _GUI_H
#define _GUI_H

#include "gui_ext.h"

#ifdef GLUT_GUI
#include "gui_glut.h"
#endif
#ifdef SDL_GUI
#include "gui_sdl.h"
#endif
#ifdef X11_GUI
#include "gui_x11.h"
#endif

// some generic stuff
#define GUI_SHIFT_MASK   (1<<0)
#define GUI_LOCK_MASK    (1<<1)
#define GUI_CNTRL_MASK   (1<<2)
#define GUI_MOD1_MASK    (1<<3)
#define GUI_MOD2_MASK    (1<<4)
#define GUI_MOD3_MASK    (1<<5)
#define GUI_MOD4_MASK    (1<<6)
#define GUI_MOD5_MASK    (1<<7)
#define GUI_BUTTON1_MASK (1<<8)
#define GUI_BUTTON2_MASK (1<<9)
#define GUI_BUTTON3_MASK (1<<10)
#define GUI_BUTTON4_MASK (1<<11)
#define GUI_BUTTON5_MASK (1<<12)


#endif
