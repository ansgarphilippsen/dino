#include <X11/Xlib.h>

#include "om.h"
#include "com.h"
#include "gui.h"
#include "Cmalloc.h"

extern struct GUI gui;

struct OBJECT_MENU om;

static struct POPUP_MENU_ENTRY def_scene_menu[]={
  {NULL,NULL,"label","Scene Menu"},
  {NULL,NULL,"separator",""},
  {NULL,NULL,"center CS","scene center [$CS get xyz]"},
  {NULL,NULL,"auto slab","scene autoslab"},
  {NULL,NULL,"toggle stereo","scene stereo"},
  {NULL,NULL,"view left","scene set view=left"},
  {NULL,NULL,"view right","scene set view=right"},
  {NULL,NULL,"view center","scene set view=center"},
  {NULL,NULL,NULL,NULL}
};

static struct POPUP_MENU_ENTRY def_db_menu[]={
  {NULL,NULL,"label",""},
  {NULL,NULL,"separator",""},
  {NULL,NULL,"center","scene center [% get center]"},
  {NULL,NULL,"show all","%.* show"},
  {NULL,NULL,"hide all","%.* hide"},
  {NULL,NULL,NULL,NULL},
};

static struct POPUP_MENU_ENTRY def_obj_menu[]={
  {NULL,NULL,"label",""},
  {NULL,NULL,"separator",""},
  {NULL,NULL,"center","scene center [% get center]"},
  {NULL,NULL,NULL,NULL}
};

static struct POPUP_MENU_ENTRY def_user_menu[]={
  {NULL,NULL,"label","User Menu"},
  {NULL,NULL,"separator",""},
  {NULL,NULL,"dist","push [[scene pop] get xyz] [[scene pop] get xyz]; opr -; scene message Distance: [opr abs]"},
  {NULL,NULL,"separator",""},
  {NULL,NULL,NULL,NULL}
};

int omInit(void)
{
  XSetWindowAttributes xswa;

  om.dpy=gui.dpy;
  om.scrn=DefaultScreen(om.dpy);
  om.root=DefaultRootWindow(om.dpy);

  om.width=100;
  om.height=300;

  om.bd_color=BlackPixel(om.dpy,om.scrn);
  om.bg_color=WhitePixel(om.dpy,om.scrn);

  /* create window */
  om.top=XCreateSimpleWindow(om.dpy,om.root,
			  0,0,om.width,om.height,1,
			  om.bd_color,om.bg_color);

  /* set some attributes */
  xswa.colormap=DefaultColormap(om.dpy,om.scrn);
  xswa.bit_gravity=NorthWestGravity;
  XChangeWindowAttributes(om.dpy,om.top,
			  CWColormap | CWBitGravity,
			  &xswa);

  /* set event mask */
  XSelectInput(om.dpy,om.top,ExposureMask | ButtonPressMask);

  XMapWindow(om.dpy,om.top);

  omRecreate();

  XFlush(om.dpy);
}


int omRecreate(void)
{
  int pos=0,def_h;
  /* first unmap all subwindows */
  XUnmapSubwindows(om.dpy,om.top);

  /* now go through the list and create all substructures */

  def_h=20;

  /* first the scene */
  om.scene.frame=XCreateSimpleWindow(om.dpy,om.top,
				     0,0,om.width,def_h,
				     2,om.bd_color,om.bg_color);

  om.scene.scene=XCreateSimpleWindow(om.dpy,om.scene.frame,
				     0,0,om.width,def_h,
				     1,om.bd_color,om.bg_color);

  pos+=def_h;

  /* now map all subwindows */
  XMapSubwindows(om.dpy,om.top);
}


int omAddDB(char *name)
{
}

int omAddObj(char *db, char *name)
{
}

int omDelDB(char *name)
{
}

int omDelObj(char *db, char *name)
{
}

int omHideObj(char *db, char *name)
{
}

int omShowObj(char *db, char *name)
{
}


void omGenPopupMenu(Widget parent, struct POPUP_MENU *p,struct POPUP_MENU_ENTRY *def,char *n,struct OBJECT_MENU_ENTRY *entry)
{
}


void omActivate(Widget w, XtPointer clientData, XEvent *event, Boolean *cont)
{
}

void omPopupCallback(Widget ww, XtPointer clientData, XtPointer call)
{
}

