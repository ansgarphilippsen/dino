#include <stdio.h>

#include "gui_wx.h"
#include "Shell.h"

#include "dino.h"

MyFrame *frame = NULL;
MyTimer *timer=NULL;
DinoGLCanvas *gl_canvas=NULL;

Shell *shell=NULL;

static int refresh_flag=1;


IMPLEMENT_APP(MyApp)

// this is the equivalent to main

bool MyApp::OnInit()
{
  // initialize dino
  cmiInit();
  dinoMain(argc,argv);

  // create the shell window
  shell=new Shell(NULL,"DINO Shell", 
		  wxPoint(10,100),wxSize(500,300));
  
  shell->Show(True);

    // Create the main frame window
  frame = new MyFrame(NULL, "DINO", 
		      wxPoint(100, 100), wxSize(500, 500));

  // Make a menubar
  /*
  wxMenu *fileMenu = new wxMenu;

  fileMenu->Append(wxID_EXIT, "E&xit");
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  frame->SetMenuBar(menuBar);
  */

#ifdef __WXMSW__
  int *gl_attrib = NULL;
#else
  int gl_attrib[20] = { GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8,
			GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 16,
			GLX_DOUBLEBUFFER, None };
#endif

  gl_canvas = new DinoGLCanvas(frame, -1, 
			       wxPoint(0, 0), wxSize(200, 200),
			       0, "DinoGLCanvas", gl_attrib);
  
  // Show the frame
  frame->Show(TRUE);

  gl_canvas->SetCurrent();

  cmiRegisterCallback(CMI_TARGET_GUI, guiCMICallback); 
  cmiResize(500,500);
  cmiInitGL();

  timer=new MyTimer();
  timer->Start(100, True);
  
  return TRUE;
}

void MyTimer::Notify()
{
  if(refresh_flag) {
    refresh_flag=0;
    fprintf(stderr,"redraw\n");
    cmiRedraw();
  }
  cmiTimer();

  this->Start(50,True);
}

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
END_EVENT_TABLE()


// Frame Stuff
MyFrame::MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
		 const wxSize& size, long style):
  wxFrame(frame, -1, title, pos, size, style)
{
  gl_canvas = NULL;
}

void MyFrame::OnExit(wxCommandEvent& event)
{
  Destroy();
}


// Timer Stuff


// DinoGLCanvas Stuff

BEGIN_EVENT_TABLE(DinoGLCanvas, wxGLCanvas)
    EVT_SIZE(DinoGLCanvas::OnSize)
    EVT_PAINT(DinoGLCanvas::OnPaint)
    EVT_CHAR(DinoGLCanvas::OnChar)
    EVT_MOUSE_EVENTS(DinoGLCanvas::OnMouseEvent)
    EVT_ERASE_BACKGROUND(DinoGLCanvas::OnEraseBackground)
END_EVENT_TABLE()

DinoGLCanvas::DinoGLCanvas(wxWindow *parent, wxWindowID id,
			   const wxPoint& pos, const wxSize& size, 
			   long style, const wxString& name,
			   int* gl_attrib):
  wxGLCanvas(parent, id, pos, size, style, name, gl_attrib)
{
  parent->Show(TRUE);
  SetCurrent();
}


DinoGLCanvas::~DinoGLCanvas(void)
{
}

void DinoGLCanvas::OnPaint( wxPaintEvent& event )
{
  // This is a dummy, to avoid an endless succession of paint messages.
  // OnPaint handlers must always create a wxPaintDC.
  wxPaintDC dc(this);
  
#ifndef __WXMOTIF__
  if (!GetContext()) return;
#endif
  
  cmiRefresh();
}

void DinoGLCanvas::OnSize(wxSizeEvent& event)
{
#ifndef __WXMOTIF__
  if (!GetContext()) return;
#endif
  
  SetCurrent();
  int width, height;
  GetClientSize(&width, &height);
  cmiResize(width, height);
}

void DinoGLCanvas::OnChar(wxKeyEvent& event)
{
  switch(event.KeyCode()) {
  case WXK_LEFT:
    break;
  case WXK_RIGHT:
    break;
  case WXK_UP:
    break;
  case WXK_DOWN:
    break;
  default:
    {
      event.Skip();
      return;
    }
  }
  
  Refresh(FALSE);
}

void DinoGLCanvas::OnMouseEvent(wxMouseEvent& event)
{
}

void DinoGLCanvas::OnEraseBackground(wxEraseEvent& event)
{
    // Do nothing, to avoid flashing.
}



/*
  external entry functions
*/

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
  gl_canvas->SwapBuffers();
}

void guiCMICallback(const cmiToken *t)
{
  if(t->target==CMI_TARGET_GUI) {
    switch(t->command) {
    case CMI_REFRESH: refresh_flag++; break;
    case CMI_MESSAGE: guiMessage((char *)t->value); break;
    }
  }
}

int guiGetImage(struct WRITE_IMAGE *i)
{
  i=NULL;
  return -1;
}
