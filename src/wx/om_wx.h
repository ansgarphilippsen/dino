#ifndef _OBJ_MENU_H
#define _OBJ_MENU_H

#include <string>
using std::string;

#include <wx/wx.h>
#include <wx/checkbox.h>


namespace {

// Dataset entry in OM
class OMDataset: public wxPanel {
public:
  OMDataset(wxWindow* p, const string& name);
  
  // add object
  void AddObj(const string& name);
  // remove object
  void RemoveObj(const string& name);

private:
  wxBoxSizer* _sizer;
};

// Obj entry within Dataset entry
class OMObject: public wxCheckBox {
public:
  OMObject(wxWindow* p, const string& name);

private:
};


} // anon namespace

class ObjMenu: public wxFrame {
public:
  ObjMenu();
  ~ObjMenu();

  // add dataset
  void AddDataset(const string& dsname);
  // remove dataset
  void RemoveDataset(const string& dsname);

  // add object to existing dataset
  void AddObject(const string& dsname, const string& oname);
  // remove object from dataset
  void RemoveObject(const string& dsname, const string& oname);

private:
  wxScrolledWindow* _swin;
  wxBoxSizer* _sizer;

  OMDataset* get_dataset(const string& name);
};

#endif
