#include <X11/Intrinsic.h>

#ifndef _OM_X11_H
#define _OM_X11_H

enum                     {OM_POPUP_LABEL,
			  OM_POPUP_TITLE,
			  OM_POPUP_SEPARATOR,
			  OM_POPUP_BUTTON};

struct OM_SCENE_ENTRY {
  Window w;
  int x,y,width,height;
};

struct OM_OBJ_ENTRY {
  Window w;
  int x,y,width,height,c;
  char name[256];
  int show;
};

struct OM_DS_ENTRY {
  Window w;
  int x,y,width,height,c;
  char name[256];
  struct OM_OBJ_ENTRY obj[256];
  int oc, ncol;
  int obj_show;
};

struct OM_POPUP_ENTRY {
  Window w;
  char label[256];
  char command[1024];
  int type;
  unsigned int width,height;
  int tx,ty;
  struct OM_POPUP *parent;
};

struct OM_POPUP {
  Window top;
  struct OM_POPUP_ENTRY entry[64];
  int entry_count;
  int boxw,boxh;
  int width,height;
};

struct OM_POPUP_LIST {
  char label[64],command[128];
  //  char *label,*command;
};

struct OBJECT_MENU {
  Display *dpy;
  int scrn;
  GC gc;
  XFontStruct *xfs;
  XFontStruct *xfs2;
  Colormap cm;

  unsigned int width,height;
  int max_width,max_height,ncol;

  Window root,top;

  /* these are the entries for the substructures */
  struct OM_SCENE_ENTRY scene;
  struct OM_DS_ENTRY ds[256];
  int ds_count;

  struct OM_POPUP scene_popup;
  struct OM_POPUP ds_popup;
  struct OM_POPUP obj_popup;
  struct OM_POPUP user_popup;

  unsigned long bd_color,fg_color,bg_color,bg2_color;
  unsigned long s1_color,s2_color;

  unsigned int texth,boxh,boxw;

  int pflag;
  char *pcom;
  Window pwin;
  char pbase[256];
};

int omInit(int icf);

int omRecreate(void);
int omArrange(void);
void omEvent(Window w, XEvent *event, void *ptr);
int omResizeEvent(int width, int height);
int omExposeEvent(void);
int omButtonEvent(XButtonEvent *event,int mp);
int omCrossEvent(XCrossingEvent *event);

int omCreatePopup(struct OM_POPUP *popup,struct OM_POPUP_LIST *list);
void omPopupEvent(Window w, XEvent *event, void *ptr);
int omPopupExpose(Window w, XExposeEvent *e, struct OM_POPUP *p);
void omUserEvent(Window w, XEvent *e, void *p);

int omSceneMenuInit(void);
int omAddDB(const char *name);
int omDelDB(const char *name);
int omAddObj(const char *db, const char *name);
int omDelObj(const char *db, const char *name);
int omHideObj(const char *db, const char *name);
int omShowObj(const char *db, const char *name);

#endif
