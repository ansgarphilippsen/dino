#include <iostream>
using namespace std;

#include <stdio.h>

#include "gui_wx.h"

#include "dino.h"
#include "gfx.h"
#include "gui_terminal.h"

extern int debug_mode;

IMPLEMENT_APP(App);

// global vars (actuall bad style, but unavoidable for C and C++ combination
DinoShell* dino_shell=0;
DinoGLView* dino_glview=0;
wxFrame* dino_top=0;

void DinoGLView::OnChar(const wxKeyEvent& e)
{
  dino_shell->SendKeyToPrompt(e);
}

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
  dino_top = new wxFrame(0,-1, "dino");
  dino_top->CreateStatusBar();

  debmsg("searching best OpenGL config");
  FindBestConfig(c);

  debmsg("creating OpenGL Canvas");
  GLCanvas* glcanvas = new GLCanvas(dino_top,-1,c, wxDefaultPosition, wxDefaultSize);

  debmsg("creating actual OpenGL view");
  dino_glview=new DinoGLView(dino_top,-1,glcanvas);

  int scrw=wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
  int scrh=wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
  int delta=10;

  // set initial size, from root window
  dino_top->SetSize(scrw-scrh,0,scrh-delta,scrh-delta);
  dino_top->Show(TRUE);

  debmsg("registering callbacks");
  cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback); 

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
  dino_top->SetStatusText(m);
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

