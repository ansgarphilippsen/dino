#ifndef _GUI_WX_H
#define _GUI_WX_H

#include "cmi.h"
#include "write.h"
#include "gui_ext.h"
#include "gui_terminal.h"

#include "glix/app.h"
#include "glix/glcanvas.h"
#include "glix/glview.h"
#include "glix/shell.h"

using namespace glix;

// implementation of GLView
class DinoGLView: public GLView {
public:
  DinoGLView(wxWindow* p, wxWindowID id, GLCanvas* c):
    GLView(p,id,c) {
    SetTimer(10);
  }

  virtual void InitGL() {
    cmiInitGL();
  }

  virtual void Render() {
    cmiRedraw();
  }

  virtual void Resize(int w, int h) {
    cmiResize(w,h);
  }

  virtual void OnMouse(wxMouseEvent& e) {
    if(e.ButtonUp() || e.ButtonDown() || e.Dragging() ) {
      cmiToken t;
      int val[5];
      int i;
      int dx,dy;
      
      t.target=CMI_TARGET_COM;
      t.command=CMI_INPUT;
      t.value=val;
      
      val[0]=CMI_INPUT_MOUSE;

      val[2]=0;

      if(e.ButtonDown()) {
	val[1]=CMI_BUTTON_PRESS;
	if(e.LeftDown())   val[2] += CMI_BUTTON1_MASK;
	if(e.RightDown())  val[2] += CMI_BUTTON3_MASK;
	if(e.MiddleDown()) val[2] += CMI_BUTTON2_MASK;
      } else if(e.ButtonUp()) {
	val[1]=CMI_BUTTON_RELEASE;
	if(e.LeftUp())   val[2] += CMI_BUTTON1_MASK;
	if(e.RightUp())  val[2] += CMI_BUTTON3_MASK;
	if(e.MiddleUp()) val[2] += CMI_BUTTON2_MASK;
      } else {
	val[1]=CMI_MOTION;
	if(e.LeftIsDown())   val[2] += CMI_BUTTON1_MASK;
	if(e.RightIsDown())  val[2] += CMI_BUTTON3_MASK;
	if(e.MiddleIsDown()) val[2] += CMI_BUTTON2_MASK;
      }

      if(e.ShiftDown())    val[2] += CMI_SHIFT_MASK;
      if(e.ControlDown())  val[2] += CMI_CNTRL_MASK;
      
      val[3]=e.GetX();
      val[4]=e.GetY();
      
      cmiSubmit(&t);
    }    
  }

  virtual void OnKeyPress(wxKeyEvent& e);

  virtual void OnTimer() {
    cmiTimer();
  }

private:
};

class DinoShell: public Shell {
public:
  DinoShell(): Shell("DINO Shell", "dino> ", 0, wxPoint(10,100),wxSize(500,300)) {
    SetInfoAttr(wxColour(63,0,191),wxNullColour, GetFont(), "dino> ");
  }

  virtual void OnInput(const string& in) {
    AddToHistory(in);
    PrintInfo(in);
    cmiCommand(in.c_str());
  }

};





#endif
