#include <X11/Intrinsic.h>

#ifndef _OM_H
#define _OM_H

const enum OM_TYPE { OM_NONE, OM_SCENE, OM_DB, OM_OBJ, OM_USER};


struct POPUP_MENU_ENTRY {
  Widget w;
  struct OBJECT_MENU_ENTRY *ome;
  char label[64];
  char command[256];
};


struct POPUP_MENU {
  Widget menu;
  struct POPUP_MENU_ENTRY entry[64];
  int entry_count;
};

struct OBJECT_MENU_ENTRY {
  int type;
  char db[64];
  char obj[64];
  Widget f,rc,w;
  struct POPUP_MENU p;
};

struct OM_SCENE_ENTRY {
  Window frame;
  Window scene;
};

struct OM_DS_ENTRY {
  Window win;
};

struct OBJECT_MENU {
  Display *dpy;
  int scrn;

  int width,height;

  Window root,top;

  /* these are the entries for the substructures */
  struct OM_SCENE_ENTRY scene;
  struct OM_DS_ENTRY ds[256];

  long bd_color,bg_color;

};

int omInit(void);

int omRecreate(void);

int omSceneMenuInit(void);
int omAddDB(char *name);
int omDelDB(char *name);
int omAddObj(char *db, char *name);
int omDelObj(char *db, char *name);
int omHideObj(char *db, char *name);
int omShowObj(char *db, char *name);

void omCallback(Widget ww, XtPointer clientData, XtPointer call);

void omActivate(Widget w, XtPointer clientData, XEvent *event, Boolean *cont);
void omPopupCallback(Widget ww, XtPointer clientData, XtPointer call);

void omGenPopupMenu(Widget parent, struct POPUP_MENU *p,struct POPUP_MENU_ENTRY *def,char *n,struct OBJECT_MENU_ENTRY *entry);

#endif
