#include <iostream>
using namespace std;

#include <stdio.h>

#include "gui_wx.h"

#include "dino.h"
#include "gfx.h"
#include "gui_terminal.h"

#include "glix/app.h"
#include "glix/glcanvas.h"
#include "glix/glview.h"
#include "glix/shell.h"

using namespace glix;

extern int debug_mode;

IMPLEMENT_APP(App);

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

  virtual void OnChar(int kc) {
  }

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

DinoShell* dino_shell=0;
DinoGLView* dino_glview=0;

bool App::OnInit()
{
  dinoParseArgs(argc,argv);

  cout << "startup" << endl;
  // initialize dino
  cout << "initializing CMI" << endl;
  cmiInit();

  cout << "initializing gfx" << endl;
  gfxInit();

  cout << "initializing GUI" << endl;

  debmsg("opening shell window");
  // create the shell window
  dino_shell=new DinoShell();
  
  dino_shell->Show(True);

  // Create the gl window
  GLConfig c;

  debmsg("opening top level frame");
  wxFrame* top = new wxFrame(0,-1, "dino");

  debmsg("searching best OpenGL config");
  FindBestConfig(c);

  debmsg("creating OpenGL Canvas");
  GLCanvas* glcanvas = new GLCanvas(top,-1,c,wxDefaultPosition,wxSize(500,500));

  debmsg("creating actual OpenGL view");
  dino_glview=new DinoGLView(top,-1,glcanvas);

  top->Show(TRUE);

  debmsg("registering callbacks");
  cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback); 
  cmiResize(500,500);

  cout << "initializing Main" << endl;
  dinoMain(argc,argv);

  cout << "init done, proceeding to main loop" << endl;
  return TRUE;
}


/* frontend/backend communication */


int guiInit(int *argc, char ***argv)
{
  return 0;
}

int guiMainLoop(void)
{
  return 0;
}

int guiMessage(char *m)
{
  // update status bar
  return 0;
}

void guiSwapBuffers(void)
{
  // handled automatically
}

void guiCMICallback(const cmiToken *t)
{
  if(t->target==CMI_TARGET_GUI) {
    switch(t->command) {
    case CMI_REFRESH: dino_glview->RefreshGL(); break;
    case CMI_MESSAGE: guiMessage((char *)t->value); break;
    case CMI_CHECKR: dino_glview->RefreshGL(); break;
    }
  }
}

int guiGetImage(struct WRITE_IMAGE *i)
{
  i=NULL;
  return -1;
}

int guiCreateOffscreenContext(int w, int h, int af)
{
  return -1;
}

int guiDestroyOffscreenContext(int n)
{
  return -1;
}

int guiQueryStereo(void)
{
  return 0;
}

int guiSetStereo(int m)
{
  return 0;
}

int guitInit(void)
{
  return 0;
}

void guitOutit(void)
{
}

void guitTimeProc(void)
{
}

void guitWrite(const char *s)
{
  dino_shell->PrintMessage2(s);
}

void guitAddChar(unsigned char c)
{
}

void guitSuspend(int f)
{
}

