#include <wx/wxprec.h>
#include <wx/wx.h>

#include "gui_wx.h"

class MyApp: public wxApp
{
    virtual bool OnInit();
};

class MyFrame: public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:
    DECLARE_EVENT_TABLE()
};

enum
{
    ID_Quit = 1,
    ID_About,
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Quit,  MyFrame::OnQuit)
    EVT_MENU(ID_About, MyFrame::OnAbout)
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50,50), wxSize(450,340) );
    frame->Show( TRUE );
    SetTopWindow( frame );
    return TRUE;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
    wxMenu *menuFile = new wxMenu;

    menuFile->Append( ID_About, "&About..." );
    menuFile->AppendSeparator();
    menuFile->Append( ID_Quit, "E&xit" );

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append( menuFile, "&File" );

    SetMenuBar( menuBar );

    CreateStatusBar();
    SetStatusText( "Welcome to wxWindows!" );
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close( TRUE );
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox( "This is a wxWindows' Hello world sample",
                  "About Hello World", wxOK | wxICON_INFORMATION );
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

}

void guiCMICallback(const cmiToken *t)
{

}

