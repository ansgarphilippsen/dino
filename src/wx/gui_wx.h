#ifndef _GUI_WX_H
#define _GUI_WX_H

#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/glcanvas.h>

#include <GL/gl.h>
#include <GL/glu.h>


class MyApp: public wxApp
{
  virtual bool OnInit();
};

class MyTimer: public wxTimer
{
  void Notify();
};

class DinoGLCanvas: public wxGLCanvas
{
 public:
  DinoGLCanvas(wxWindow *parent, const wxWindowID id = -1, 
	       const wxPoint& pos = wxDefaultPosition,
	       const wxSize& size = wxDefaultSize, long style = 0,
	       const wxString& name = "DinoGLCanvas",
	       int* gl_attrib = NULL);
  ~DinoGLCanvas(void);
  
  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnChar(wxKeyEvent& event);
  void OnMouseEvent(wxMouseEvent& event);
 private:
  
DECLARE_EVENT_TABLE()
};
    
    
class MyFrame: public wxFrame
{
 public:
  MyFrame(wxFrame *frame, const wxString& title, 
	  const wxPoint& pos, const wxSize& size,
	  long style = wxDEFAULT_FRAME_STYLE);
  
  void OnExit(wxCommandEvent& event);
  
DECLARE_EVENT_TABLE()
};



extern "C" {

#include "cmi.h"
#include "write.h"

int guiInit(int *argc, char ***argv);
int guiMainLoop(void);
int guiMessage(char *m);
void guiSwapBuffers(void);
int guiGetImage(struct WRITE_IMAGE *i);
void guiCMICallback(const cmiToken *t);

}

#endif
