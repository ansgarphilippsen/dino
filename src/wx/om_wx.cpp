#include <iostream>

#include "om_wx.h"

// om dataset 
OMDataset::OMDataset(wxWindow* p, const string& name):
  wxPanel(p,-1)
{
  SetName(name.c_str());
  SetBackgroundColour(wxColour(255,255,255));
  _sizer = new wxBoxSizer(wxVERTICAL);
  _sizer->Add(new wxStaticText(this, -1, name.c_str()), 
	      1, wxEXPAND | wxALIGN_TOP, 1);
  SetSizerAndFit(_sizer);
}

void OMDataset::AddObj(const string& name)
{
  _sizer->Add(new OMObject(this, name), 
	      1, wxEXPAND | wxALL | wxALIGN_TOP, 5);
  //_sizer->SetSizeHints(this);
}

void OMDataset::RemoveObj(const string& name)
{
}

// om object
OMObject::OMObject(wxWindow* p, const string& name):
  wxCheckBox(p, -1, name.c_str())
{
  SetName(name.c_str());
}

// actual object menu

ObjMenu::ObjMenu():
  wxFrame(0,-1,"DINO Object Menu", 
	  wxPoint(10,10), wxSize(100,400))
{
  _swin = new wxScrolledWindow(this,-1);
  _sizer = new wxBoxSizer(wxVERTICAL);
  _swin->SetSizer(_sizer);
  _swin->FitInside();
  _swin->SetBackgroundColour(wxColour(1,0,0));
  Show(true);
}

ObjMenu::~ObjMenu()
{
}

void ObjMenu::AddDataset(const string& dsname)
{
  _sizer->Add(new OMDataset(_swin,dsname), 0, wxEXPAND | wxALL | wxALIGN_TOP | wxADJUST_MINSIZE, 5);
}

void ObjMenu::RemoveDataset(const string& dsname)
{
  
}

void ObjMenu::AddObject(const string& dsname, const string& oname)
{
  OMDataset* dsp = get_dataset(dsname);
  if(dsp) {
    dsp->AddObj(oname);
    _swin->FitInside();
  }
}

void ObjMenu::RemoveObject(const string& dsname, const string& oname)
{
}

OMDataset* ObjMenu::get_dataset(const string& name)
{
  wxWindowList &l = _swin->GetChildren();

  wxWindowListNode *node = l.GetFirst();
  while (node) {
    wxWindow *win = node->GetData();
    if(string(win->GetName())==name) {
      return static_cast<OMDataset*>(win);
    }
    node = node->GetNext();
  }
  return 0;
}

