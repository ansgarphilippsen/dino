#include <X11/Intrinsic.h>

#ifndef _EXPOM_H
#define _EXPOM_H

enum                     {EXPOM_POPUP_LABEL,
			  EXPOM_POPUP_TITLE,
			  EXPOM_POPUP_SEPARATOR,
			  EXPOM_POPUP_BUTTON};

struct EXPOM_SCENE_ENTRY {
  Window w;
  int x,y,width,height;
};

struct EXPOM_OBJ_ENTRY {
  Window w;
  int x,y,width,height,c;
  char name[256];
  int show;
};

struct EXPOM_DS_ENTRY {
  Window w;
  int x,y,width,height,c;
  char name[256];
  struct EXPOM_OBJ_ENTRY obj[256];
  int oc, ncol;
};

struct EXPOM_POPUP_ENTRY {
  Window w;
  char label[256];
  char command[1024];
  int type;
  unsigned int width,height;
  int tx,ty;
  struct EXPOM_POPUP *parent;
};

struct EXPOM_POPUP {
  Window top;
  struct EXPOM_POPUP_ENTRY entry[64];
  int entry_count;
  int boxw,boxh;
  int width,height;
};

struct EXPOM_POPUP_LIST {
  char label[64],command[1024];
};

struct EXPO_MENU {
  Display *dpy;
  int scrn;
  GC gc;
  XFontStruct *xfs;
  XFontStruct *xfs2;
  Colormap cm;

  unsigned int width,height;
  int max_width,max_height,ncol;

  Widget topw;
  Window root,top;

  /* these are the entries for the substructures */
  struct EXPOM_SCENE_ENTRY scene;
  struct EXPOM_DS_ENTRY ds[256];
  int ds_count;

  struct EXPOM_POPUP scene_popup;
  struct EXPOM_POPUP ds_popup;
  struct EXPOM_POPUP obj_popup;
  struct EXPOM_POPUP user_popup;

  unsigned long bd_color,fg_color,bg_color,bg2_color;
  unsigned long s1_color,s2_color;

  unsigned int texth,boxh,boxw;

  int pflag;
  char *pcom;
  Window pwin;
  char pbase[256];
};

int expomInit(void);

int expomRecreate(void);
int expomArrange(void);
void expomEvent(Window w, XEvent *event, void *ptr);
int expomResizeEvent(int width, int height);
int expomExposeEvent(void);
int expomButtonEvent(XButtonEvent *event,int mp);
int expomCrossEvent(XCrossingEvent *event);

int expomCreatePopup(struct EXPOM_POPUP *popup,struct EXPOM_POPUP_LIST *list);
void expomPopupEvent(Window w, XEvent *event, void *ptr);
int expomPopupExpose(Window w, XExposeEvent *e, struct EXPOM_POPUP *p);
void expomUserEvent(Window w, XEvent *e, void *p);

int expomSceneMenuInit(void);
int expomAddDB(const char *name);
int expomDelDB(const char *name);
int expomAddObj(const char *db, const char *name);
int expomDelObj(const char *db, const char *name);
int expomHideObj(const char *db, const char *name);
int expomShowObj(const char *db, const char *name);

#endif
