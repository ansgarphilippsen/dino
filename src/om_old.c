#include <Xm/XmAll.h>
#include <X11/Shell.h>
#include <X11/Intrinsic.h>

#include "om_old.h"
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
  int i;
  Arg arg[10];
  char s[256];
  Widget form,frame,rc1,rc2,scene,db,obj,sep;
  XmString xms;

  om.entry_max=100;
  om.entry=Ccalloc(om.entry_max,sizeof(struct OBJECT_MENU_ENTRY));
  for(i=0;i<om.entry_max;i++)
    om.entry[i].type=OM_NONE;
 

  om.top=XmCreateDialogShell(gui.top,"objectmenu",arg,0);


  XtSetArg(arg[0],XmNorientation, XmVERTICAL);
  XtSetArg(arg[1],XmNnumColumns,1);
  XtSetArg(arg[2],XmNpacking, XmPACK_TIGHT);
  XtSetArg(arg[3],XmNradioBehavior,False);
  XtSetArg(arg[4],XmNresizeHeight,True);
  XtSetArg(arg[5],XmNresizeWidth,True);
  XtSetArg(arg[6],XmNspacing,0);
  XtSetArg(arg[7],XmNentryAlignment, XmALIGNMENT_BEGINNING);
  XtSetArg(arg[8],XmNadjustLast, True);
  om.rc=XtCreateWidget("omRC",xmRowColumnWidgetClass,om.top,arg,9);
  XtManageChild(om.rc);

  om.entry[0].type=OM_SCENE;
  strcpy(om.entry[0].db,"");
  strcpy(om.entry[0].obj,"");
  sprintf(s,"omFrame%d",0);
  om.entry[0].f=XtCreateManagedWidget(s,xmFrameWidgetClass,om.rc,arg,0);
  xms= XmStringCreateLtoR(" scene", XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  XtSetArg(arg[1],XmNalignment, XmALIGNMENT_BEGINNING);
  om.entry[0].w=XtCreateManagedWidget("scene",xmLabelWidgetClass,
				      om.entry[0].f,arg,2);
  XmStringFree(xms);

  /*
  XtPopup(om.top,XtGrabNone);
  */
  XtManageChild(om.top);

  omGenPopupMenu(om.entry[0].w,&om.entry[0].p,def_scene_menu,"scenemenu",&om.entry[0]);

  /* define the user menu */
  om.um.type=OM_USER;
  omGenPopupMenu(gui.top,&om.um.p,def_user_menu,"usermenu",&om.um);
}


int omAddDB(char *name)
{
  int i,c;
  Arg arg[10];
  XmString xms;
  char s[256];
  struct POPUP_MENU_ENTRY menu[100];


  for(i=0;i<om.entry_max;i++)
    if(om.entry[i].type==OM_NONE)
      break;

  if(i==om.entry_max)
    return -1;

  /* make submenu */
  c=0;
  while(strlen(def_db_menu[c].label)>0) {
    menu[c].w=def_db_menu[c].w;
    strcpy(menu[c].label,def_db_menu[c].label);
    strcpy(menu[c].command,def_db_menu[c].command);
    c++;
  }
  strcpy(menu[0].command,".");
  strcat(menu[0].command,name);
  strcpy(menu[c].label,"");

  om.entry[i].type=OM_DB;
  strcpy(om.entry[i].db,name);
  strcpy(om.entry[i].obj,"");

  sprintf(s,"omFrame%d",i);
  om.entry[i].f=XtCreateManagedWidget(s,xmFrameWidgetClass,om.rc,arg,0);

  sprintf(s,"omRC%d",i);
  XtSetArg(arg[0],XmNorientation, XmVERTICAL);
  XtSetArg(arg[1],XmNnumColumns,1);
  XtSetArg(arg[2],XmNpacking, XmPACK_COLUMN);
  XtSetArg(arg[3],XmNradioBehavior,False);
  XtSetArg(arg[4],XmNresizeHeight,True);
  XtSetArg(arg[5],XmNresizeWidth,True);
  XtSetArg(arg[6],XmNspacing,0);
  XtSetArg(arg[7],XmNentryAlignment, XmALIGNMENT_BEGINNING);
  om.entry[i].rc=XtCreateWidget(s,xmRowColumnWidgetClass,om.entry[i].f,arg,8);
  XtManageChild(om.entry[i].rc);

  xms= XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  om.entry[i].w=XtCreateManagedWidget(name,xmLabelWidgetClass,
				      om.entry[i].rc,arg,1);

  /* create submenu now after .w is defined */
  omGenPopupMenu(om.entry[i].w,&om.entry[i].p,menu,name,&om.entry[i]);

  XtAddEventHandler(om.entry[i].w,  ButtonPressMask, False,
		    (XtEventHandler) omActivate, &om.entry[i]);

  XmStringFree(xms);
}

int omAddObj(char *db, char *name)
{
  int i,p,c;
  Arg arg[10];
  Widget parent;
  XmString xms;
  char s[256];
  struct POPUP_MENU_ENTRY menu[100];

  for(p=0;p<om.entry_max;p++)
    if(!strcmp(db,om.entry[p].db))
      break;

  if(p==om.entry_max)
    return -1;

  parent=om.entry[p].rc;

  for(i=0;i<om.entry_max;i++)
    if(om.entry[i].type==OM_NONE)
      break;

  if(i==om.entry_max)
    return -1;

  /* make submenu */
  c=0;
  while(strlen(def_obj_menu[c].label)>0) {
    menu[c].w=def_obj_menu[c].w;
    strcpy(menu[c].label,def_obj_menu[c].label);
    strcpy(menu[c].command,def_obj_menu[c].command);
    c++;
  }
  strcpy(menu[0].command,".");
  strcat(menu[0].command,db);
  strcat(menu[0].command,".");
  strcat(menu[0].command,name);
  strcpy(menu[c].label,"");

  om.entry[i].type=OM_OBJ;
  strcpy(om.entry[i].db,om.entry[p].db);
  strcpy(om.entry[i].obj,name);
  xms= XmStringCreateLtoR(name, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg[0],XmNlabelString,xms);
  XtSetArg(arg[1],XmNindicatorOn, False);
  XtSetArg(arg[2],XmNfillOnSelect, True);
  XtSetArg(arg[3],XmNset,True);
  XtSetArg(arg[4],XmNhighlightOnEnter, False);
  XtSetArg(arg[5],XmNhighlightThickness,0);
  XtSetArg(arg[6],XmNshadowThickness,5);
  om.entry[i].w=XtCreateManagedWidget(name,xmToggleButtonWidgetClass,
				      parent,arg,7);

  omGenPopupMenu(om.entry[i].w,&om.entry[i].p,menu,name,&om.entry[i]);

  XtInsertEventHandler(om.entry[i].w,ButtonPressMask, False,
		       (XtEventHandler) omActivate, &om.entry[i], XtListHead);  
}

int omDelDB(char *name)
{
  int i;
  
  for(i=0;i<om.entry_max;i++)
    if(!strcmp(name,om.entry[i].db)) {
      if(om.entry[i].type==OM_DB) {
	XtDestroyWidget(om.entry[i].f);
      }
      om.entry[i].type=OM_NONE;
    }
  return 0;
}


int omDelObj(char *db, char *name)
{
  int i;

  for(i=0;i<om.entry_max;i++)
    if(!strcmp(name,om.entry[i].obj) &&
       !strcmp(db,om.entry[i].db))
      break;

  if(i==om.entry_max)
    return -1;

  om.entry[i].type=OM_NONE;
  XtDestroyWidget(om.entry[i].w);
}

int omHideObj(char *db, char *name)
{
  int i;
  Arg arg[5];

  for(i=0;i<om.entry_max;i++)
    if(!strcmp(name,om.entry[i].obj) &&
       !strcmp(db,om.entry[i].db))
      break;

  if(i==om.entry_max)
    return -1;

  XtSetArg(arg[0],XmNset,False);
  XtSetValues(om.entry[i].w,arg,1);
  XtManageChild(om.entry[i].w);
  XtUnmapWidget(om.entry[i].w);
  XtMapWidget(om.entry[i].w);
}

int omShowObj(char *db, char *name)
{
  int i;
  Arg arg[5];

  for(i=0;i<om.entry_max;i++)
    if(!strcmp(name,om.entry[i].obj) &&
       !strcmp(db,om.entry[i].db))
      break;

  if(i==om.entry_max)
    return -1;

  XtSetArg(arg[0],XmNset,True);
  XtSetValues(om.entry[i].w,arg,1);
  XtUnmapWidget(om.entry[i].w);
  XtMapWidget(om.entry[i].w);
}


void omGenPopupMenu(Widget parent, struct POPUP_MENU *p,struct POPUP_MENU_ENTRY *def,char *n,struct OBJECT_MENU_ENTRY *entry)
{
  Arg arg[10];
  XmString xms;
  int c;

  XtSetArg(arg[0],XmNresizeWidth,True);
  p->menu=XmCreatePopupMenu(parent,n,arg,1);

  c=0;
  while(strlen(def[c].label)>0) {
    strcpy(p->entry[c].label,def[c].label);
    strcpy(p->entry[c].command,def[c].command);
    if(!strcmp(p->entry[c].label,"separator")) {
      p->entry[c].w=XtCreateManagedWidget(p->entry[c].label,
					  xmSeparatorWidgetClass,
					  p->menu,
					  arg,0);
    } else if(!strcmp(p->entry[c].label,"label")) {
      xms=XmStringCreateLtoR(p->entry[c].command, XmSTRING_DEFAULT_CHARSET);
      XtSetArg(arg[0],XmNlabelString,xms);
      p->entry[c].w=XtCreateManagedWidget(p->entry[c].label,
					  xmLabelWidgetClass,
					  p->menu,
					  arg,1);
      XmStringFree(xms);
    } else {
      xms=XmStringCreateLtoR(p->entry[c].label, XmSTRING_DEFAULT_CHARSET);
      XtSetArg(arg[0],XmNlabelString,xms);
      p->entry[c].w=XtCreateManagedWidget(p->entry[c].label,
					  xmCascadeButtonWidgetClass,
					  p->menu,
					  arg,1);

      p->entry[c].ome=entry;

      XtAddCallback(p->entry[c].w, 
		    XmNactivateCallback,omPopupCallback,&p->entry[c]); 
      
      XmStringFree(xms);
    }
    c++;
  }
  XtAddEventHandler(parent, ButtonPressMask, False, omActivate, entry);  
  p->entry_count=c;
}


void omActivate(Widget w, XtPointer clientData, XEvent *event, Boolean *cont)
{
  Arg arg[10];
  struct OBJECT_MENU_ENTRY *ome;
  Boolean state;

  ome=(struct OBJECT_MENU_ENTRY *)clientData;


  if (event->type == ButtonPress) { 
    if(event->xbutton.button == Button3) {
      if(ome->p.menu!=NULL) {
	XmMenuPosition(ome->p.menu, &event->xbutton);
	XtManageChild(ome->p.menu);
      }
    } else if(event->xbutton.button == Button1){
      if(ome->type==OM_OBJ) {
        XtSetArg(arg[0],XmNset,&state);
        XtGetValues(w,arg,1);
        if(state==False) {
          XtSetArg(arg[0],XmNset,False);
          comObjCommand(ome->db,ome->obj,"show");
          XtSetValues(ome->w,arg,1);
        }
        else {
          XtSetArg(arg[0],XmNset,True);
          comObjCommand(ome->db,ome->obj,"hide");
          XtSetValues(ome->w,arg,1);
        }
      }
    }
  }
}

void omPopupCallback(Widget ww, XtPointer clientData, XtPointer call)
{
  Arg args[10];
  char expr[256],db[64],ob[64];
  struct POPUP_MENU_ENTRY *entry;
  Widget p;
  int i,t,e;

  entry=(struct POPUP_MENU_ENTRY *)clientData;

  if(entry->ome->type==OM_SCENE ||
     entry->ome->type==OM_USER) {
    strcpy(expr,entry->command);
    comRawCommand(expr);
  } else if(entry->ome->type==OM_DB) {
    i=0;
    e=0;
    while(i<strlen(entry->command)) {
      if(entry->command[i]=='%') {
	expr[e]='\0';
	strcat(expr,".");
	strcat(expr,entry->ome->db);
	e+=strlen(entry->ome->db)+1;
	i++;
      } else {
	expr[e++]=entry->command[i++];
      }
    }
    expr[e]='\0';

    comRawCommand(expr);

  } else if(entry->ome->type==OM_OBJ) {
    i=0;
    e=0;
    while(i<strlen(entry->command)) {
      if(entry->command[i]=='%') {
	expr[e]='\0';
	strcat(expr,".");
	strcat(expr,entry->ome->db);
	strcat(expr,".");
	strcat(expr,entry->ome->obj);
	e+=strlen(entry->ome->db)+1+strlen(entry->ome->obj)+1;
	i++;
      } else {
	expr[e++]=entry->command[i++];
      }
    }
    expr[e]='\0';

    comRawCommand(expr);

  } else {
  }
}

int omEvent(void)
{
  return 0;
}
