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
#include "dino.h"

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

int guiCreateOffscreenContext(int w, int h, int af)
{
    return 0;
}

int guiDestroyOffscreenContext(int c)
{
    return 0;
}

int guiInit(int argc, char **argv)
{
    char major[8],minor[8];
    
  // register cmi callbacks for GUI
    cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback);

  //reset redraw flag
    gui.redraw=0;

  //print info about graphics subsystem
    debmsg("guiInit: info");
    fprintf(stderr,"Graphics Subsystem ID: %s %s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER));
    sprintf(major,"%c",glGetString(GL_VERSION)[0]);
    major[1]='\0';
    sprintf(minor,"%c",glGetString(GL_VERSION)[2]);
    minor[1]='\0';

    if(atoi(major)<1 || (atoi(major)==1 && atoi(minor)<1)) {
	fprintf(stderr,"OpenGL version %d.%d or above required, found %s.%s instead\n",
	 1,1,major,minor);
	return -1;
    } else {
	fprintf(stderr,"OpenGL Version %s.%s\n",major,minor);
    }

  //initialization completed
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
	    case CMI_DS_NEW:  break;
	    case CMI_DS_DEL:  break;
	    case CMI_DS_REN: /*TODO*/ break;
	    case CMI_OBJ_NEW:  break;
	    case CMI_OBJ_DEL:  break;
	    case CMI_OBJ_REN: /*TODO*/ break;
	    case CMI_OBJ_SHOW:  break;
	    case CMI_OBJ_HIDE:  break;
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
    [[Controller dinoController] commandResult:s];
}


/***************************************

  Input Events

****************************************/


void gui_mouse_input(int eventType, int mask, int x, int y)
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














/*****************************

  glx_input
  -----------

  called after the GLX window
  has received an input
  event, like  mouse or keyb
  event.

*****************************/
/*
static void glx_input(Widget ww, XtPointer clientData, XtPointer call)
{
  cmiToken t;
  int val[5];
  int i;
  int dx,dy;
  //  double val;
  //struct timeval tp;
  //struct timezone tzp;
  //long dt;
  char kbuf[32];
  int kcount;
  KeySym key;


//    reading pointer

     XmDrawingAreaCallbackStruct *cd =
    (XmDrawingAreaCallbackStruct *) call;

  //gettimeofday(&tp,&tzp);

//     switch depending on event type

  switch(cd->event->type){

  case KeyPress:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;
    
    val[0]=CMI_INPUT_KEYBOARD;
    val[1]=CMI_BUTTON_PRESS;

    kcount=XLookupString(&(cd->event->xkey),kbuf,sizeof(kbuf),&key,NULL);
    kbuf[kcount]='\0';
    for(i=0;i<kcount;i++) {
      val[2]=(int)kbuf[i];
      cmiSubmit(&t);
      //comWriteCharBuf(kbuf[i]);
    }
    break;
  case KeyRelease:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;
    
    val[0]=CMI_INPUT_KEYBOARD;
    val[1]=CMI_BUTTON_RELEASE;

    kcount=XLookupString(&(cd->event->xkey),kbuf,sizeof(kbuf),&key,NULL);
    kbuf[kcount]='\0';
    switch(key) {
    case XK_Return:
    case XK_Linefeed:
    case XK_KP_Enter:
      val[2]=CMI_KEY_RETURN;
      cmiSubmit(&t);
      //      comWriteCharBuf(13);
      break;
    case XK_Delete:
      val[2]=CMI_KEY_DELETE;
      cmiSubmit(&t);
      //comWriteCharBuf(8);
      break;
    case XK_Up:
      val[2]=CMI_KEY_UP;
      cmiSubmit(&t);
      //comWriteCharBuf(27);
      //comWriteCharBuf('[');
      //comWriteCharBuf('A');
      break;
    case XK_Down:
      val[2]=CMI_KEY_DOWN;
      cmiSubmit(&t);
      //comWriteCharBuf(27);
      //comWriteCharBuf('[');
      //comWriteCharBuf('B');
      break;
    case XK_Left:
      val[2]=CMI_KEY_LEFT;
      cmiSubmit(&t);
      //comWriteCharBuf(27);
      //comWriteCharBuf('[');
      //comWriteCharBuf('D');
      break;
    case XK_Right:
      val[2]=CMI_KEY_RIGHT;
      cmiSubmit(&t);
      //comWriteCharBuf(27);
      //comWriteCharBuf('[');
      //comWriteCharBuf('C');
      break;
    }
    break;

  case ButtonPress:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[0]=CMI_INPUT_MOUSE;
    val[1]=CMI_BUTTON_PRESS;
    val[2]=0;
    if(cd->event->xmotion.state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(cd->event->xmotion.state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(cd->event->xmotion.state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;
    val[3]=cd->event->xbutton.x;
    val[4]=cd->event->xbutton.y;

    cmiSubmit(&t);

    gui.last_x=cd->event->xbutton.x;
    gui.last_y=cd->event->xbutton.y;
    //    memcpy(&gui.tp_button,&tp,sizeof(struct timezone));
    if(cd->event->xbutton.button==3) {

      om.pwin=gui.user_menu;
      om.pflag=1;
      XMoveWindow(gui.dpy,gui.user_menu,
		  cd->event->xbutton.x_root,cd->event->xbutton.y_root);
      XMapRaised(gui.dpy,gui.user_menu);

    }
    break;
  case ButtonRelease:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;

    val[0]=CMI_INPUT_MOUSE;
    val[1]=CMI_BUTTON_RELEASE;
    val[2]=0;
    if(cd->event->xmotion.state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(cd->event->xmotion.state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(cd->event->xmotion.state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;
    val[3]=cd->event->xbutton.x;
    val[4]=cd->event->xbutton.y;

    cmiSubmit(&t);

    dx=gui.last_x-cd->event->xbutton.x;
    dy=gui.last_y-cd->event->xbutton.y;

//	      dt=(long)(tp.tv_sec-gui.tp_button.tv_sec)*1000000+(tp.tv_usec-gui.tp_button.tv_usec);
//	      if(dt<200000 ||
//	      (dx==0 && dy==0 && dt<300000)) {
//	      if(cd->event->xbutton.state & Button1Mask) {
//	      if(cd->event->xbutton.state & ShiftMask)
//	      comPick(cd->event->xmotion.x,cd->event->xmotion.y,1);
//	      else
//	      comPick(cd->event->xmotion.x,cd->event->xmotion.y,0);
//	      }
//	      }

    if(cd->event->xbutton.button==3) {
      om.pflag=0;
      XUnmapWindow(gui.dpy,gui.user_menu);
    }

    break;
  case MotionNotify:
    t.target=CMI_TARGET_COM;
    t.command=CMI_INPUT;
    t.value=val;
    
    val[0]=CMI_INPUT_MOUSE;
    val[1]=CMI_MOTION;
    val[2]=0;

    if(cd->event->xmotion.state & GUI_BUTTON1_MASK)
      val[2] += CMI_BUTTON1_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON2_MASK)
      val[2] += CMI_BUTTON2_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON3_MASK)
      val[2] += CMI_BUTTON3_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON4_MASK)
      val[2] += CMI_BUTTON4_MASK;
    if(cd->event->xmotion.state & GUI_BUTTON5_MASK)
      val[2] += CMI_BUTTON5_MASK;
    if(cd->event->xmotion.state & GUI_SHIFT_MASK)
      val[2] += CMI_SHIFT_MASK;
    if(cd->event->xmotion.state & GUI_CNTRL_MASK)
      val[2] += CMI_CNTRL_MASK;

    val[3]=cd->event->xbutton.x;
    val[4]=cd->event->xbutton.y;

    cmiSubmit(&t);

    dx=gui.last_x-cd->event->xmotion.x;
    dy=gui.last_y-cd->event->xmotion.y;
    gui.last_x=cd->event->xmotion.x;
    gui.last_y=cd->event->xmotion.y;

    //comTransform(TRANS_MOUSE,cd->event->xmotion.state,0,dx);
    //comTransform(TRANS_MOUSE,cd->event->xmotion.state,1,dy);
    
    break;
  case EnterNotify:
    gui.inside=1;
    break;
  case LeaveNotify: 
    gui.inside=0;
    break;
  }
}

*/



