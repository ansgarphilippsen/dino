#include <math.h>

#include <X11/Xlib.h>
#include <Xm/Frame.h>

#include "expom.h"
#include "com.h"
#include "gui.h"
#include "Cmalloc.h"

extern struct GUI gui;

struct EXPO_MENU expom;

static struct EXPOM_POPUP_LIST expom_scene_list[]={
  {"label",""},
  {"separator",""},
  {"center CS","% center [$CS get xyz]"},
  {"center CP","% center $CP"},
  {"auto slab","% autoslab"},
  {"toggle stereo","% stereo"},
  {"view left","% set view=left"},
  {"view right","% set view=right"},
  {"view center","% set view=center"},
  {"reset","% reset"},
  {NULL,NULL}
};

static struct EXPOM_POPUP_LIST expom_ds_list[]={
  {"label",""},
  {"separator",""},
  {"center","scene center [% get center]"},
  {"show all","%.* show"},
  {"hide all","%.* hide"},
  {NULL,NULL}
};

static struct EXPOM_POPUP_LIST expom_obj_list[]={
  {"label",""},
  {"separator",""},
  {"center","scene center [% get center]"},
  {NULL,NULL}
};

static struct EXPOM_POPUP_LIST expom_user_list[]={
  {"title","User Menu"},
  {"separator",""},
  {"dist","push [[scene pop] get xyz] [[scene pop] get xyz]; opr - abs; scene message Distance: [pop]"},
  {"auto slab","scene autoslab"},
  {"center CS","scene center [$CS]"},
  {"center CP","scene center $CP"},
  {NULL,NULL}
};

int expomInit(void)
{
  XSetWindowAttributes xswa;
  XGCValues gcv;
  XColor color;
  char *font_name="-*-helvetica-*-r-*-*-14-*-75-75-*-*-*-*";
  char *font_name2="-*-helvetica-*-o-*-*-14-*-75-75-*-*-*-*";
  char *backup_font_name="fixed";
  XClassHint *xch;
  XWMHints *xwmh;
  XSizeHints *xsh;
  char *wname="objectmenu";
  char *iname="o-menu";
  XWindowChanges xwc;

  expom.dpy=gui.dpy;
  expom.scrn=DefaultScreen(expom.dpy);
  expom.root=DefaultRootWindow(expom.dpy);
  expom.cm=DefaultColormap(expom.dpy,expom.scrn);
  expom.width=256;
  expom.height=1024;

  expom.bd_color=BlackPixel(expom.dpy,expom.scrn);
  expom.fg_color=BlackPixel(expom.dpy,expom.scrn);
  expom.bg_color=WhitePixel(expom.dpy,expom.scrn);
  expom.bg2_color=WhitePixel(expom.dpy,expom.scrn);
  expom.s1_color=BlackPixel(expom.dpy,expom.scrn);
  expom.s2_color=BlackPixel(expom.dpy,expom.scrn);

  if(XParseColor(expom.dpy,expom.cm,"slategray3",&color)!=0) {
    if(XAllocColor(expom.dpy,expom.cm,&color)!=0)
      expom.bg_color=color.pixel;
  }
  if(XParseColor(expom.dpy,expom.cm,"slategray1",&color)!=0) {
    if(XAllocColor(expom.dpy,expom.cm,&color)!=0)
      expom.bg2_color=color.pixel;
  }
  if(XParseColor(expom.dpy,expom.cm,"black",&color)!=0) {
    if(XAllocColor(expom.dpy,expom.cm,&color)!=0)
      expom.bd_color=color.pixel;
  }
  if(XParseColor(expom.dpy,expom.cm,"slategray4",&color)!=0) {
    if(XAllocColor(expom.dpy,expom.cm,&color)!=0)
      expom.s1_color=color.pixel;
  }
  if(XParseColor(expom.dpy,expom.cm,"slategray2",&color)!=0) {
    if(XAllocColor(expom.dpy,expom.cm,&color)!=0)
      expom.s2_color=color.pixel;
  }

  expom.ds_count=0;
  expom.pflag=0;
  expom.ncol=1;


  expom.ds_count=1;
  strcpy(expom.ds[0].name,"test1");
  expom.ds[0].oc=2;
  strcpy(expom.ds[0].obj[0].name,"aaaaa");
  expom.ds[0].obj[0].show=1;
  strcpy(expom.ds[0].obj[1].name,"bbbbb");
  expom.ds[0].obj[1].show=0;


  /* define the font */
  if((expom.xfs=XLoadQueryFont(expom.dpy,font_name))==NULL)
    if((expom.xfs=XLoadQueryFont(expom.dpy,backup_font_name))==NULL) {
      fprintf(stderr,"\ncannot load font fixed");
      exit(1);
    }
  if((expom.xfs2=XLoadQueryFont(expom.dpy,font_name2))==NULL)
    if((expom.xfs2=XLoadQueryFont(expom.dpy,backup_font_name))==NULL) {
      fprintf(stderr,"\ncannot load font fixed");
      exit(1);
    }
  
  
  expom.texth=expom.xfs->max_bounds.ascent+expom.xfs->max_bounds.descent;

  /* create top level widget */
  expom.top=XCreateSimpleWindow(expom.dpy,expom.root,
				0,0,expom.width,expom.height,1,
				expom.bd_color,expom.bg_color);
  
  
  /* set some attributes */
  xswa.colormap=expom.cm;
  xswa.bit_gravity=NorthWestGravity;
  xswa.override_redirect=True;

  XChangeWindowAttributes(expom.dpy,expom.top,
			  CWColormap | CWBitGravity | CWOverrideRedirect,
			  &xswa);

  /* WM interactions */ 
  xwmh=XAllocWMHints();
  xwmh->flags=0;

  xsh=XAllocSizeHints();
  xsh->flags=PMinSize;
  xsh->min_height=1024;
  xsh->min_width=expom.width;
    
  xch=XAllocClassHint();
  xch->res_name=wname;
  xch->res_class=wname;
  XmbSetWMProperties(expom.dpy,expom.top,
		     wname,iname,
		     NULL,0,
		     xsh,xwmh,xch);



  /* get the GC */
  gcv.foreground=expom.fg_color;
  gcv.background=expom.bg_color;
  gcv.font=expom.xfs->fid;
  expom.gc=XCreateGC(expom.dpy,expom.top,
		  GCForeground | GCBackground | GCFont,
		  &gcv);

  XSetFont(expom.dpy,expom.gc,expom.xfs->fid);

  /* set event mask */
  XSelectInput(expom.dpy,expom.top,
	       ExposureMask | 
	       ButtonPressMask | 
	       ButtonReleaseMask | 
	       OwnerGrabButtonMask |
	       StructureNotifyMask |
	       EnterWindowMask |
	       LeaveWindowMask);

  /* register window with main loop */
  guiRegisterCustomEvent(expom.top,expomEvent,NULL);

  XMapWindow(expom.dpy,expom.top);
  xwc.x=1024;
  xwc.y=0;
  XReconfigureWMWindow(expom.dpy,expom.top,expom.scrn,
		       CWX | CWY, &xwc);

  expomRecreate();

  /* 
     the popup windows for scene,
     the dataset and the objects
     need to be created, but not
     mapped yet !
  */
  expomCreatePopup(&expom.scene_popup,expom_scene_list);
  expomCreatePopup(&expom.ds_popup,expom_ds_list);
  expomCreatePopup(&expom.obj_popup,expom_obj_list);
  expomCreatePopup(&expom.user_popup,expom_user_list);
  guiRegisterUserMenu(expom.user_popup.top);
  
  XFlush(expom.dpy);
  return 0;
}




int expomRecreate(void)
{
  int i,j;
  int idummy;
  unsigned int udummy;
  Window wdummy;
  int boxx,boxy,boxw,boxh,boxs1,boxs2;

  /* first unmap all subwindows */
  XUnmapSubwindows(expom.dpy,expom.top);
  /* then destroy them */
  XDestroySubwindows(expom.dpy,expom.top);

  /* get geometry from top window */
  XGetGeometry(expom.dpy,expom.top,&wdummy,
	       &idummy,&idummy,&expom.width,&expom.height,
	       &udummy,&udummy);

  /*
    now go through the list and create all substructures
    All windows are created with x and y set to zero,
    those will be set afterwards by arrange
  */
  
  expom.boxh=expom.texth+6;
  expom.boxw=expom.width-6;
  boxs1=8;
  boxs2=1;
  boxx=2;
  boxy=boxs1;
  boxw=expom.boxw;
  boxh=expom.boxh;

  /* first the scene */
  expom.scene.w=XCreateSimpleWindow(expom.dpy,expom.top,
				 0,0,
				 boxw,boxh,
				 2,
				 expom.bd_color,expom.bg_color);

  expom.scene.width=boxw;
  expom.scene.height=boxh;

  /* the ds and obj */
  for(i=0;i<expom.ds_count;i++) {
    /*
      create a box that can take
      all objects and the ds name
    */
    expom.ds[i].width=boxw;
    expom.ds[i].height=(boxh+boxs2)*(expom.ds[i].oc+1)-boxs2+2;
    expom.ds[i].w=XCreateSimpleWindow(expom.dpy,expom.top,
				   0,0,
				   expom.ds[i].width,expom.ds[i].height,
				   2,
				   expom.bd_color,expom.bg_color);
				   
    /* creat the subwindows that take the objects */ 
    for(j=0;j<expom.ds[i].oc;j++) {
      expom.ds[i].obj[j].width=boxw-10;
      expom.ds[i].obj[j].height=boxh;
      expom.ds[i].obj[j].w=XCreateSimpleWindow(expom.dpy,expom.top,
					    0,0,boxw-10,boxh,0,
					    expom.bd_color,expom.bg_color);
    }
  }

  /* now map the subwindows */
  XMapSubwindows(expom.dpy,expom.top);

  expomArrange();

  return 0;
}

int expomArrange(void)
{
  int i, j, col, ncol;
  Window wdummy;
  int idummy;
  unsigned int udummy;
  int theight;
  int x,y,width,height;
  int boxh,boxs2;

  boxh=expom.boxh;
  boxs2=1;

  XGetGeometry(expom.dpy,expom.top,&wdummy,
	       &idummy,&idummy,&expom.width,&expom.height,
	       &udummy,&udummy);

  expom.width-=2;
  expom.height-=2;

  /*
    all windows and subwindows are assumed
    to have x and y zero, and the width and
    height the original values.
    Arrange will first go through all entries,
    assigning column numbers to all ds and all
    object entries. then it will calculate the 
    x and y and relative width and height
  */

  /* assume that a linear ds can be arranged */
  theight=expom.scene.height;
  col=0;

  for(i=0;i<expom.ds_count;i++) {
    if(theight+expom.ds[i].height>=expom.height) {
      col++;
      expom.ds[i].c=col;
      expom.ds[i].y=1;
      theight=expom.ds[i].height;
    } else {
      expom.ds[i].c=col;
      expom.ds[i].y=theight;
      theight+=expom.ds[i].height;
    }
  }
  ncol=col+1;
  expom.ncol=ncol;

  /*
    now assign the x coordinates and the width
  */
  expom.scene.x=1;
  expom.scene.y=1;
  XMoveResizeWindow(expom.dpy,expom.scene.w,
		    expom.scene.x,expom.scene.y,
		    expom.width/ncol-2,expom.scene.height);

  expom.boxw=expom.width/ncol;

  for(i=0;i<expom.ds_count;i++) {
    x=(int)((float)expom.ds[i].c*(float)(expom.width)/(float)(ncol))+1;
    y=expom.ds[i].y;
    width=expom.boxw-2;
    height=expom.ds[i].height;
    XMoveResizeWindow(expom.dpy,expom.ds[i].w,x,y,width,height);
    for(j=0;j<expom.ds[i].oc;j++) {
      y=j*(expom.boxh+1)+boxh+1+expom.ds[i].y;
      XMoveResizeWindow(expom.dpy,expom.ds[i].obj[j].w,x+4,y,expom.boxw-6,expom.boxh);
    }
  }
  return 0;
}


void expomEvent(Window w, XEvent *event, void *ptr)
{
  switch(event->type) {
  case Expose:
    expomExposeEvent();
    break;
  case ConfigureNotify:
    /* should now be handled by expomArrange */
    expomArrange();
    /*
    if(event->xconfigure.width !=expom.width ||
       event->xconfigure.height!=expom.height)
      expomResizeEvent(event->xconfigure.width,event->xconfigure.height);
    */
    break;
  case ButtonPress:
    expomButtonEvent(&event->xbutton,1);
    break;
  case ButtonRelease:
    expomButtonEvent(&event->xbutton,0);
    break;
  case EnterNotify:
  case LeaveNotify:
    expomCrossEvent(&event->xcrossing);
    break;
  }

}

int expomExposeEvent()
{
  int i,j,tx,ty;
  char label[256];
  int objw,objh;

  objw=expom.boxw-8;
  objh=expom.boxh;
  
  strcpy(label,"scene");
  tx=(expom.boxw-XTextWidth(expom.xfs,label,strlen(label)))/2;
  tx=4;
  ty=expom.boxh-(expom.boxh-expom.xfs->ascent+expom.xfs->descent)/2;

  XClearWindow(expom.dpy,expom.scene.w);
  XDrawString(expom.dpy,expom.scene.w,expom.gc,tx,ty,label,strlen(label));

  for(i=0;i<expom.ds_count;i++) {
    tx=4;
    strcpy(label,expom.ds[i].name);
    XClearWindow(expom.dpy,expom.ds[i].w);
    XDrawString(expom.dpy,expom.ds[i].w,expom.gc,tx,ty,label,strlen(label));
    
    XSetLineAttributes(expom.dpy,expom.gc,2,LineSolid,CapRound,JoinRound);
    for(j=0;j<expom.ds[i].oc;j++) {
      tx=12;
      strcpy(label,expom.ds[i].obj[j].name);
      XClearWindow(expom.dpy,expom.ds[i].obj[j].w);
      /* draw the label */
      XDrawString(expom.dpy,expom.ds[i].obj[j].w,expom.gc,tx,ty,label,strlen(label));
      /* draw the shadow box */
      if(expom.ds[i].obj[j].show) {
	XSetForeground(expom.dpy,expom.gc,expom.s1_color);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  1,0,1,objh);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  1,0,objw,0);
	XSetForeground(expom.dpy,expom.gc,expom.s2_color);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  1,objh,objw,objh);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  objw,0,objw,objh);
      } else {
	XSetForeground(expom.dpy,expom.gc,expom.s2_color);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  1,0,1,objh);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  1,0,objw,0);
	XSetForeground(expom.dpy,expom.gc,expom.s1_color);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  1,objh,objw,objh);
	XDrawLine(expom.dpy,expom.ds[i].obj[j].w,expom.gc,
		  objw,0,objw,objh);
      }
      XSetForeground(expom.dpy,expom.gc,expom.fg_color);

    }
  }
  XSetLineAttributes(expom.dpy,expom.gc,1,LineSolid,CapButt,JoinMiter);
  return 0;
}



int expomCrossEvent(XCrossingEvent *event)
{
  return 0;
}

int expomButtonEvent(XButtonEvent *event,int mp)
{
  int i,j;
  char command[1024];

  if(event->subwindow==expom.scene.w) {
    strcpy(expom.pbase,"scene");
    if(event->button==3) {
      if(mp==1) {
	expom.pflag=1;
	expom.pwin=expom.scene_popup.top;
	XMoveWindow(expom.dpy,expom.pwin,event->x_root,event->y_root);
	XMapRaised(expom.dpy,expom.pwin);
      }
    }
  } else {
    for(i=0;i<expom.ds_count;i++) {
      if(event->subwindow==expom.ds[i].w) {
	sprintf(expom.pbase,".%s",expom.ds[i].name);
	if(event->button==3) {
	  if(mp==1) {
	    expom.pflag=1;
	    expom.pwin=expom.ds_popup.top;
	    XMoveWindow(expom.dpy,expom.pwin,event->x_root,event->y_root);
	    XMapRaised(expom.dpy,expom.pwin);
	  }
	}
	break;
      } else {
	for(j=0;j<expom.ds[i].oc;j++)
	  if(event->subwindow==expom.ds[i].obj[j].w) {
	    sprintf(expom.pbase,".%s.%s",expom.ds[i].name,expom.ds[i].obj[j].name);
	    if(mp==1 && event->button==1) {
	      /* toggle the state */
	      if(expom.ds[i].obj[j].show) {
		sprintf(command,"%s hide",expom.pbase);
		comRawCommand(command);
	      } else {
		sprintf(command,"%s show",expom.pbase);
		comRawCommand(command);
	      }
	    }
	    if(mp==1 && event->button==3) {
	      expom.pflag=1;
	      expom.pwin=expom.obj_popup.top;
	      XMoveWindow(expom.dpy,expom.pwin,event->x_root,event->y_root);
	      XMapRaised(expom.dpy,expom.pwin);
	    }
	    break; /* if subwindow was found */
	  }
      }
    }
  }
  /*
  fprintf(stderr,"\n%s %d %d",base,event->button,event->state);
  */
  return 0;
}

int expomResizeEvent(int width, int height)
{
  int i,j;
  int boxw,boxh,boxs1,boxs2;

  expom.width=width;
  expom.height=height;

  expom.boxw=expom.width-6;
  boxh=expom.boxh;
  boxw=expom.boxw;
  boxs1=8;
  boxs2=1;

  XResizeWindow(expom.dpy,expom.scene.w,boxw,boxh);

  for(i=0;i<expom.ds_count;i++) {
    XResizeWindow(expom.dpy,expom.ds[i].w,boxw,(boxh+boxs2)*(expom.ds[i].oc+1)-boxs2+2);
    for(j=0;j<expom.ds[i].oc;j++) {
      XResizeWindow(expom.dpy,expom.ds[i].obj[j].w,boxw-4,boxh);
    }
  }
  return 0;
}




int expomAddDB(const char *name)
{
  int i;
  for(i=0;i<expom.ds_count;i++)
    if(!strcmp(expom.ds[i].name,name)) {
      /* that name is already used, should not
	 happen, so just return
      */
      return -1;
    }
  strcpy(expom.ds[expom.ds_count].name,name);
  expom.ds[expom.ds_count].oc=0;
  expom.ds_count++;
  expomRecreate();
  return 0;
}

int expomAddObj(const char *db, const char *name)
{
  int i,j;

  for(i=0;i<expom.ds_count;i++)
    if(!strcmp(expom.ds[i].name,db)) {
      for(j=0;j<expom.ds[i].oc;j++)
	if(!strcmp(expom.ds[i].obj[j].name,name)) {
	  /* name already found! this should
	     not happen, just return */
	  return -1;
	}
      strcpy(expom.ds[i].obj[expom.ds[i].oc].name,name);
      expom.ds[i].obj[expom.ds[i].oc].show=1;
      expom.ds[i].oc++;
    }
  expomRecreate();
  return 0;
}

int expomDelDB(const char *name)
{
  int i,j;
  for(i=0;i<expom.ds_count;i++)
    if(!strcmp(expom.ds[i].name,name)) {
      for(j=i+1;j<expom.ds_count;j++) {
	memcpy(&expom.ds[j-1],&expom.ds[j],sizeof(struct EXPOM_DS_ENTRY));
      }
      expom.ds_count--;
    }
  expomRecreate();
  return 0;
}

int expomDelObj(const char *db, const char *name)
{
  int i,j,k;

  for(i=0;i<expom.ds_count;i++)
    if(!strcmp(expom.ds[i].name,db)) {
      for(j=0;j<expom.ds[i].oc;j++)
	if(!strcmp(expom.ds[i].obj[j].name,name)) {
	  for(k=j+1;k<expom.ds[i].oc;k++) {
	    memcpy(&expom.ds[i].obj[k-1],&expom.ds[i].obj[k],sizeof(struct EXPOM_OBJ_ENTRY));
	  }
	  expom.ds[i].oc--;
	}
    }
  expomRecreate();
  return 0;
}

int expomHideObj(const char *db, const char *name)
{
  int i,j;
  for(i=0;i<expom.ds_count;i++)
    if(!strcmp(expom.ds[i].name,db))
      for(j=0;j<expom.ds[i].oc;j++)
	if(!strcmp(expom.ds[i].obj[j].name,name))
	  expom.ds[i].obj[j].show=0;
  expomExposeEvent();
  return 0;
}

int expomShowObj(const char *db, const char *name)
{
  int i,j;
  for(i=0;i<expom.ds_count;i++)
    if(!strcmp(expom.ds[i].name,db))
      for(j=0;j<expom.ds[i].oc;j++)
	if(!strcmp(expom.ds[i].obj[j].name,name))
	  expom.ds[i].obj[j].show=1;
  expomExposeEvent();
  return 0;
}


int expomCreatePopup(struct EXPOM_POPUP *popup,struct EXPOM_POPUP_LIST *list)
{
  int i,lc,lc2, ls,lm;
  int width,height,boxw,boxh;
  XSetWindowAttributes xswa;
  int px,py,pw,ph;
  char *mins="xxxxxxxxxxxxxxx";

  /* create top level window */

  lm=XTextWidth(expom.xfs,mins,strlen(mins));

  lc2=0;
  for(lc=0;strlen(list[lc].label)>0;lc++) {
    if(!strcmp(list[lc].label,"separator")) {
      lc2++;
    } else {
      ls=XTextWidth(expom.xfs,list[lc].label,strlen(list[lc].label));
      lm=(ls>lm) ? ls : lm;
    }
  }
  /*
    should now have number of labels 
    and length of largest label
  */
  boxh=expom.texth+4;
  boxw=lm+12;
  
  height=(lc-lc2)*(boxh+2)+(lc2*4)+6;
  width=boxw+6;
  popup->height=height;
  popup->width=width;

  popup->boxw=boxw;
  popup->boxh=boxh;

  popup->top=XCreateSimpleWindow(expom.dpy,expom.root,0,0,width,height,0,
				 expom.bd_color,expom.bg_color);
  XSelectInput(expom.dpy,popup->top,
	       ExposureMask |
	       OwnerGrabButtonMask);
  
  /* register event handling */
  guiRegisterCustomEvent(popup->top,expomPopupEvent,popup);

  xswa.override_redirect=True;
  XChangeWindowAttributes(expom.dpy,popup->top,
			  CWOverrideRedirect,
			  &xswa);

  XSetTransientForHint(expom.dpy,popup->top,popup->top);

  px=3;
  py=3;
  pw=boxw;
  ph=boxh;
  for(i=0;i<lc;i++) {
    /* 
       check for the different type
       of entries
    */
    popup->entry[i].width=pw;
    popup->entry[i].height=ph;
    if(!strcmp(list[i].label,"label")) {
      strcpy(popup->entry[i].label,list[i].command);
      popup->entry[i].type=EXPOM_POPUP_LABEL;
      popup->entry[i].w=XCreateSimpleWindow(expom.dpy,popup->top,
					    px,py,pw,ph,0,
					    expom.bd_color,expom.bg_color);
      XSelectInput(expom.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask);
    } else if(!strcmp(list[i].label,"title")) {
      strcpy(popup->entry[i].label,list[i].command);
      popup->entry[i].type=EXPOM_POPUP_TITLE;
      popup->entry[i].w=XCreateSimpleWindow(expom.dpy,popup->top,
					    px,py,pw,ph,0,
					    expom.bd_color,expom.bg_color);
      XSelectInput(expom.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask);

    } else if(!strcmp(list[i].label,"separator")) {
      strcpy(popup->entry[i].label,"---");
      popup->entry[i].type=EXPOM_POPUP_SEPARATOR;
      popup->entry[i].w=XCreateSimpleWindow(expom.dpy,popup->top,
					    px,py,pw,4,0,
					    expom.bd_color,expom.bg_color);
      py-=boxh;
      py+=4;
      popup->entry[i].height=4;
      XSelectInput(expom.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask);
    } else {
      strcpy(popup->entry[i].label,list[i].label);
      strcpy(popup->entry[i].command,list[i].command);
      popup->entry[i].type=EXPOM_POPUP_BUTTON;
      popup->entry[i].w=XCreateSimpleWindow(expom.dpy,popup->top,
					    px,py,pw,ph,0,
					    expom.bd_color,expom.bg_color);
      XSelectInput(expom.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask |
		   EnterWindowMask |
		   LeaveWindowMask);
      guiRegisterCustomEvent(popup->entry[i].w,expomPopupEvent,&popup->entry[i]);
    }
    py+=boxh+2;

    
  }
  popup->entry_count=lc;

  XMapSubwindows(expom.dpy,popup->top);
  return 0;
}


void expomPopupEvent(Window w, XEvent *event, void *ptr)
{
  char c1[1024],c2[1024],*cp;

  struct EXPOM_POPUP_ENTRY *entry;
  switch(event->type) {
  case Expose:
    expomPopupExpose(w,&event->xexpose,(struct EXPOM_POPUP *)ptr);
    break;
  case EnterNotify:
    entry=(struct EXPOM_POPUP_ENTRY *)ptr;
    XSetFont(expom.dpy,expom.gc,expom.xfs->fid);
    XSetForeground(expom.dpy,expom.gc,expom.bg2_color);
    XFillRectangle(expom.dpy,entry->w,expom.gc,
		   0,0,entry->width,entry->height);
    XSetForeground(expom.dpy,expom.gc,expom.fg_color);
    XDrawString(expom.dpy,entry->w,expom.gc,entry->tx,entry->ty,
		entry->label,strlen(entry->label));
    break;
  case LeaveNotify:
    entry=(struct EXPOM_POPUP_ENTRY *)ptr;
    XSetFont(expom.dpy,expom.gc,expom.xfs->fid);
    XClearWindow(expom.dpy,entry->w);
    XDrawString(expom.dpy,entry->w,expom.gc,entry->tx,entry->ty,
		entry->label,strlen(entry->label));
    break;
  case ButtonPress:
    break;
  case ButtonRelease:
    entry=(struct EXPOM_POPUP_ENTRY *)ptr;
    if(event->xbutton.button==3) {
      if(entry->type==EXPOM_POPUP_BUTTON) {
	/* go through command and scan for % */
	strcpy(c1,entry->command);
	cp=strchr(c1,'%');
	if(cp==NULL) {
	  strcpy(c2,c1);
	} else {
	  cp[0]=0;
	  sprintf(c2,"%s%s%s",c1,expom.pbase,cp+1);
	}
	comRawCommand(c2);
	expom.pflag=0;
      }
    }
    
    break;
  }
}


int expomPopupExpose(Window w, XExposeEvent *e, struct EXPOM_POPUP *popup)
{
  int i,tx,ty,ty2;

  /* draw 3D around popup window */
  XSetLineAttributes(expom.dpy,expom.gc,2,LineSolid,CapRound,JoinRound);
  XSetForeground(expom.dpy,expom.gc,expom.s2_color);
  XDrawLine(expom.dpy,popup->top,expom.gc,
	    1,1,1,popup->height-1);
  XDrawLine(expom.dpy,popup->top,expom.gc,
	    1,1,popup->width-1,1);
  XSetForeground(expom.dpy,expom.gc,expom.s1_color);
  XDrawLine(expom.dpy,popup->top,expom.gc,
	    popup->width-1,1,popup->width-1,popup->height-1);
  XDrawLine(expom.dpy,popup->top,expom.gc,
	    1,popup->height-1,popup->width-1,popup->height-1);
  XSetForeground(expom.dpy,expom.gc,expom.fg_color);
  

  tx=4;
  ty=popup->boxh-(popup->boxh-expom.xfs->ascent+expom.xfs->descent)/2;
  ty2=popup->boxh-(popup->boxh-expom.xfs2->ascent+expom.xfs2->descent)/2;

  for(i=0;i<popup->entry_count;i++) {
    XClearWindow(expom.dpy,popup->entry[i].w);
    switch(popup->entry[i].type) {
    case EXPOM_POPUP_TITLE:
      XSetFont(expom.dpy,expom.gc,expom.xfs2->fid);
      XDrawString(expom.dpy,popup->entry[i].w,expom.gc,tx,ty2,
		  popup->entry[i].label,strlen(popup->entry[i].label));
      popup->entry[i].tx=tx;
      popup->entry[i].ty=ty;
    break;
    case EXPOM_POPUP_LABEL:
      XSetFont(expom.dpy,expom.gc,expom.xfs2->fid);
      /*
      XDrawString(expom.dpy,popup->entry[i].w,expom.gc,tx,ty2,
		  popup->entry[i].label,strlen(popup->entry[i].label));
      */
      XDrawString(expom.dpy,popup->entry[i].w,expom.gc,tx,ty2,
		  expom.pbase,strlen(expom.pbase));
      popup->entry[i].tx=tx;
      popup->entry[i].ty=ty;
      break;
    case EXPOM_POPUP_SEPARATOR:
      XSetLineAttributes(expom.dpy,expom.gc,1,LineOnOffDash,CapRound,JoinRound);
      XDrawLine(expom.dpy,popup->entry[i].w,expom.gc,0,2,popup->boxw,2);
      XSetLineAttributes(expom.dpy,expom.gc,1,LineSolid,CapRound,JoinRound);
      popup->entry[i].tx=0;
      popup->entry[i].ty=0;
      break;
    case EXPOM_POPUP_BUTTON:
      XSetFont(expom.dpy,expom.gc,expom.xfs->fid);
      XDrawString(expom.dpy,popup->entry[i].w,expom.gc,tx,ty2,
		  popup->entry[i].label,strlen(popup->entry[i].label));
      popup->entry[i].tx=tx;
      popup->entry[i].ty=ty2;
      break;
    }
  }
  XSetFont(expom.dpy,expom.gc,expom.xfs->fid);
  return 0;
}
