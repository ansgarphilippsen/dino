#include <iostream>
using namespace std;

#include <stdio.h>

#include "gui_wx.h"

#include "dino.h"
#include "gfx.h"

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
    GLView(p,id,c) {}

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
  }

  virtual void OnChar(int kc) {
  }

  virtual void OnTimer() {
    cmiTimer();
  }
};

Shell* dino_shell=0;
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
  dino_shell=new Shell("DINO Shell", "dino> ", 0, 
		       wxPoint(10,100),wxSize(500,300));
  
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
  return 0;
}

void guiSwapBuffers(void)
{
}

void guiCMICallback(const cmiToken *t)
{
  if(t->target==CMI_TARGET_GUI) {
    switch(t->command) {
    case CMI_REFRESH: dino_glview->Refresh(false); break;
      //case CMI_MESSAGE: guiMessage((char *)t->value); break;
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

