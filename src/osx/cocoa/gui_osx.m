/************************************************

dino: gui_osx.m

Connects the Aqua user interface to Dino via CMI

*************************************************/
#import "gui_osx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#import  "Controller.h"
#import  "OMController.h"
#include "dino.h"
#include "gui_ext.h"

struct GUI gui;
static void check_redraw(void);
extern int debug_mode,gfx_flags;


/**********************************

  GUI Initialization

************************************/

int guiMainLoop()
{
    return 0;
}

int guiSetStereo(int m)
{
    return 0;
}

int guiQueryStereo(void)
{
    return 0;
}

int guiInit(int argc, char **argv)
{
    char major[8],minor[8];
    NSString *graphCard,*openGLVersion;

  // register cmi callbacks for GUI
    cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback);

  //reset redraw flag
    gui.redraw=0;

  //print info about graphics subsystem
    debmsg("guiInit: info");
    graphCard = [NSString stringWithFormat:@"Graphics Subsystem ID: %s %s\n", glGetString(GL_VENDOR),glGetString(GL_RENDERER)];

    sprintf(major,"%c",glGetString(GL_VERSION)[0]);
    major[1]='\0';
    sprintf(minor,"%c",glGetString(GL_VERSION)[2]);
    minor[1]='\0';

    if(atoi(major)<1 || (atoi(major)==1 && atoi(minor)<1)) {
	openGLVersion = [NSString stringWithFormat:@"OpenGL version %d.%d or above required, found %s.%s instead",1,1, major,minor];
	return -1;
    } else {
	openGLVersion = [NSString stringWithFormat:@"OpenGL Version %s.%s",major,minor];
    }

    [[Controller dinoController] notifyUser:[graphCard stringByAppendingString:openGLVersion] returnPrompt:NO];
    
  //initialization completed
    return 0;
}

int guiCreateOffscreenContext(int width, int height, int af)
{
    int n=0;

    n=[[Controller dinoController] offScreenContextWidth:width Height:height Accum:af];
    return n;
}

int guiDestroyOffscreenContext(int c)
{
    [[Controller dinoController] releaseOffScreenContext];
    return 0;
}


/**********************************

  CMI Callback function

***********************************/

void guiCMICallback(const cmiToken *t)
{
    const char **cp;
    cp=(const char **)t->value; // for DS and OBJ commands
    if(t->target==CMI_TARGET_GUI) {
	switch(t->command) {
	    case CMI_REFRESH: gui.redraw++; break;
	    case CMI_MESSAGE: guiMessage((char *)t->value); break;
	    case CMI_CHECKR: check_redraw(); break;
	    case CMI_DS_NEW: omAddDB(cp[0]); break;
	    case CMI_DS_DEL: omDelDB(cp[0]); break;
	    case CMI_DS_REN: /*TODO*/ break;
	    case CMI_OBJ_NEW: omAddObj(cp[0],cp[1]); break;
	    case CMI_OBJ_DEL: omDelObj(cp[0],cp[1]); break;
	    case CMI_OBJ_REN: /*TODO*/ break;
	    case CMI_OBJ_SHOW: omShowObj(cp[0],cp[1]); break;
	    case CMI_OBJ_HIDE: omHideObj(cp[0],cp[1]); break;
	}
    }
}


/*************************************

  The periodic scheduler

**************************************/

void guiTimeProc(void)
{
    check_redraw();
    cmiTimer();

}

static void check_redraw(void)
{
    if(gui.redraw) {
	gui.redraw=0;
	cmiRedraw();
    }
}


/***************************************

  Update DinoGFX display and terminal

****************************************/

void guiSwapBuffers(void)
{
    [[Controller dinoController] swapBuffers];
}

int guiMessage(char *m)
{
    [[Controller dinoController] updateStatusBox:[NSString stringWithCString:m]];

    return 0;
}

void guitWrite(const char *s)
{
    [[Controller dinoController] showCommandResult:[NSString stringWithCString:s]];
}


/***************************************

  Input Events

****************************************/

void gfx_mouse_input(int eventType, int mask, int x, int y)
{
    cmiToken t;
    int val[5];

    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[0]=CMI_INPUT_MOUSE;
    val[1]=eventType;
    val[2]=mask;

    val[3]=x;
    val[4]=y;

    cmiSubmit(&t);
}

/*void glx_keyboard_input(int key)
{
    cmiToken t;
    int val[3];

    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[0]=CMI_INPUT_KEYBOARD;
    val[1]=CMI_BUTTON_RELEASE;

    // somehow get the key value from the event into key

    // replace XK_* with OSX macros
    switch(key) {
	case XK_Return:
	case XK_Linefeed:
	case XK_KP_Enter:
	    val[2]=CMI_KEY_RETURN;
	    cmiSubmit(&t);
	    break;
	case XK_Delete:
	    val[2]=CMI_KEY_DELETE;
	    cmiSubmit(&t);
	    break;
	case XK_Up:
	    val[2]=CMI_KEY_UP;
	    cmiSubmit(&t);
	    break;
	case XK_Down:
	    val[2]=CMI_KEY_DOWN;
	    cmiSubmit(&t);
	    break;
	case XK_Left:
	    val[2]=CMI_KEY_LEFT;
	    cmiSubmit(&t);
	    break;
	case XK_Right:
	    val[2]=CMI_KEY_RIGHT;
	    cmiSubmit(&t);
	    break;
	default:
	{
	    for(i=0;i<kcount;i++) {
		val[2]=(int)kbuf[i];
		cmiSubmit(&t);
	    }
	}
    }    

}*/

/***************************************

Object Menu

****************************************/

int omAddDB(const char *name)
{
    [[OMController dinoOMController]  omAddDB:[NSString stringWithCString:name]];
    return 0;
}

int omDelDB(const char *name)
{
    [[OMController dinoOMController]  omDelDB:[NSString stringWithCString:name]];
    return 0;
}

int omAddObj(const char *db, const char *name)
{
    [[OMController dinoOMController]  omAddObj:[NSString stringWithCString:name] inDB:[NSString stringWithCString:db]];
    return 0;
}

int omDelObj(const char *db, const char *name)
{
    [[OMController dinoOMController]  omDelObj:[NSString stringWithCString:name] inDB:[NSString stringWithCString:db]];
    return 0;
}

int omHideObj(const char *db, const char *name)
{
    [[OMController dinoOMController]  omHideObj:[NSString stringWithCString:name] ofDB:[NSString stringWithCString:db]];
    return 0;
}

int omShowObj(const char *db, const char *name)
{
    [[OMController dinoOMController]  omShowObj:[NSString stringWithCString:name] ofDB:[NSString stringWithCString:db]];
    return 0;
}

