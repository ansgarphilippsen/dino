#include <math.h>

#include <X11/Xlib.h>

#include "om.h"
#include "com.h"
#include "gui.h"
#include "Cmalloc.h"

extern struct GUI gui;

struct OBJECT_MENU om;

static struct OM_POPUP_LIST om_scene_list[]={
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

static struct OM_POPUP_LIST om_ds_list[]={
  {"label",""},
  {"separator",""},
  {"center","scene center [% get center]"},
  {"show all","%.* show"},
  {"hide all","%.* hide"},
  {NULL,NULL}
};

static struct OM_POPUP_LIST om_obj_list[]={
  {"label",""},
  {"separator",""},
  {"center","scene center [% get center]"},
  {NULL,NULL}
};

static struct OM_POPUP_LIST om_user_list[]={
  {"title","User Menu"},
  {"separator",""},
  {"auto slab","scene autoslab"},
  {"center CS","scene center [$CS]"},
  {"center CP","scene center $CP"},
  {"separator",""},
  /*  {"dist","push [[scene pop] get xyz] [[scene pop] get xyz]; opr - abs; scene message Distance: [pop]"},*/
  {"calc dist (2)","push [[scene pop]] [[scene pop]]; opr dist; scene message Distance: [pop]"},
  {"calc angle (3)","push [[scene pop]] [[scene pop]] [[scene pop]]; opr angle; scene message Angle: [pop]"},
  {"calc torsion (4)","push [[scene pop]] [[scene pop]] [[scene pop]] [[scene pop]]; opr torsion; scene message Torsion: [pop]"},
  {NULL,NULL}
};

int omInit(void)
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

  om.dpy=gui.dpy;
  om.scrn=DefaultScreen(om.dpy);
  om.root=DefaultRootWindow(om.dpy);
  om.cm=DefaultColormap(om.dpy,om.scrn);
#ifdef EXPO
  om.width=400;
  om.height=800;
#else
  om.width=180;
  om.height=400;
#endif

  om.bd_color=BlackPixel(om.dpy,om.scrn);
  om.fg_color=BlackPixel(om.dpy,om.scrn);
  om.bg_color=WhitePixel(om.dpy,om.scrn);
  om.bg2_color=WhitePixel(om.dpy,om.scrn);
  om.s1_color=BlackPixel(om.dpy,om.scrn);
  om.s2_color=BlackPixel(om.dpy,om.scrn);

  if(XParseColor(om.dpy,om.cm,"slategray3",&color)!=0) {
    if(XAllocColor(om.dpy,om.cm,&color)!=0)
      om.bg_color=color.pixel;
  }
  if(XParseColor(om.dpy,om.cm,"slategray1",&color)!=0) {
    if(XAllocColor(om.dpy,om.cm,&color)!=0)
      om.bg2_color=color.pixel;
  }
  if(XParseColor(om.dpy,om.cm,"black",&color)!=0) {
    if(XAllocColor(om.dpy,om.cm,&color)!=0)
      om.bd_color=color.pixel;
  }
  if(XParseColor(om.dpy,om.cm,"slategray4",&color)!=0) {
    if(XAllocColor(om.dpy,om.cm,&color)!=0)
      om.s1_color=color.pixel;
  }
  if(XParseColor(om.dpy,om.cm,"slategray2",&color)!=0) {
    if(XAllocColor(om.dpy,om.cm,&color)!=0)
      om.s2_color=color.pixel;
  }

  om.ds_count=0;
  om.pflag=0;
  om.ncol=1;

  /* test */
  /*********************
  om.ds_count=1;
  strcpy(om.ds[0].name,"test1");
  om.ds[0].oc=2;
  strcpy(om.ds[0].obj[0].name,"aaaaa");
  om.ds[0].obj[0].show=1;
  strcpy(om.ds[0].obj[1].name,"bbbbb");
  om.ds[0].obj[1].show=0;
  *********************/


  /* define the font */
  if((om.xfs=XLoadQueryFont(om.dpy,font_name))==NULL)
    if((om.xfs=XLoadQueryFont(om.dpy,backup_font_name))==NULL) {
      fprintf(stderr,"\ncannot load font fixed");
      exit(1);
    }
  if((om.xfs2=XLoadQueryFont(om.dpy,font_name2))==NULL)
    if((om.xfs2=XLoadQueryFont(om.dpy,backup_font_name))==NULL) {
      fprintf(stderr,"\ncannot load font fixed");
      exit(1);
    }
  
  
  om.texth=om.xfs->max_bounds.ascent+om.xfs->max_bounds.descent;

  /* create window */
  om.top=XCreateSimpleWindow(om.dpy,om.root,
			  0,0,om.width,om.height,1,
			  om.bd_color,om.bg_color);


  /* set some attributes */
  xswa.colormap=om.cm;
  xswa.bit_gravity=NorthWestGravity;
  xswa.override_redirect=False;
  XChangeWindowAttributes(om.dpy,om.top,
			  CWColormap | CWBitGravity | CWOverrideRedirect,
			  &xswa);

  /* WM interactions */ 
  xwmh=XAllocWMHints();
  xwmh->flags=0;

  xsh=XAllocSizeHints();
  xsh->flags=PMinSize;
  xsh->min_height=40;
  xsh->min_width=om.width;
    
  xch=XAllocClassHint();
  xch->res_name=wname;
  xch->res_class=wname;
  XmbSetWMProperties(om.dpy,om.top,
		     wname,iname,
		     NULL,0,
		     xsh,xwmh,xch);



  /* get the GC */
  gcv.foreground=om.fg_color;
  gcv.background=om.bg_color;
  gcv.font=om.xfs->fid;
  om.gc=XCreateGC(om.dpy,om.top,
		  GCForeground | GCBackground | GCFont,
		  &gcv);

  XSetFont(om.dpy,om.gc,om.xfs->fid);

  /* set event mask */
  XSelectInput(om.dpy,om.top,
	       ExposureMask | 
	       ButtonPressMask | 
	       ButtonReleaseMask | 
	       OwnerGrabButtonMask |
	       StructureNotifyMask |
	       EnterWindowMask |
	       LeaveWindowMask);

  /* register window with main loop */
  guiRegisterCustomEvent(om.top,omEvent,NULL);

  XMapWindow(om.dpy,om.top);
#ifdef EXPO
  xwc.x=500;
  xwc.y=0;
#else
  xwc.x=80;
  xwc.y=30;
#endif
  XReconfigureWMWindow(om.dpy,om.top,om.scrn,
		       CWX | CWY , &xwc);

  omRecreate();

  /* 
     the popup windows for scene,
     the dataset and the objects
     need to be created, but not
     mapped yet !
  */
  omCreatePopup(&om.scene_popup,om_scene_list);
  omCreatePopup(&om.ds_popup,om_ds_list);
  omCreatePopup(&om.obj_popup,om_obj_list);
  omCreatePopup(&om.user_popup,om_user_list);
  guiRegisterUserMenu(om.user_popup.top);
  
  XFlush(om.dpy);
  return 0;
}




int omRecreate(void)
{
  int i,j;
  int idummy;
  unsigned int udummy;
  Window wdummy;
  int boxx,boxy,boxw,boxh,boxs1,boxs2;

  /* first unmap all subwindows */
  XUnmapSubwindows(om.dpy,om.top);
  /* then destroy them */
  XDestroySubwindows(om.dpy,om.top);

  /* get geometry from top window */
  XGetGeometry(om.dpy,om.top,&wdummy,
	       &idummy,&idummy,&om.width,&om.height,
	       &udummy,&udummy);

  /*
    now go through the list and create all substructures
    All windows are created with x and y set to zero,
    those will be set afterwards by arrange
  */
  
  om.boxh=om.texth+6;
  om.boxw=om.width-6;
  boxs1=8;
  boxs2=1;
  boxx=2;
  boxy=boxs1;
  boxw=om.boxw;
  boxh=om.boxh;

  /* first the scene */
  om.scene.w=XCreateSimpleWindow(om.dpy,om.top,
				 0,0,
				 boxw,boxh,
				 2,
				 om.bd_color,om.bg_color);

  om.scene.width=boxw;
  om.scene.height=boxh;

  /* the ds and obj */
  for(i=0;i<om.ds_count;i++) {
    /*
      create a box that can take
      all objects and the ds name
    */
    om.ds[i].width=boxw;
    om.ds[i].height=(boxh+boxs2)*(om.ds[i].oc+1)-boxs2+2;
    om.ds[i].w=XCreateSimpleWindow(om.dpy,om.top,
				   0,0,
				   om.ds[i].width,om.ds[i].height,
				   2,
				   om.bd_color,om.bg_color);
				   
    /* creat the subwindows that take the objects */ 
    for(j=0;j<om.ds[i].oc;j++) {
      om.ds[i].obj[j].width=boxw-10;
      om.ds[i].obj[j].height=boxh;
      om.ds[i].obj[j].w=XCreateSimpleWindow(om.dpy,om.top,
					    0,0,boxw-10,boxh,0,
					    om.bd_color,om.bg_color);
    }
  }

  /* now map the subwindows */
  XMapSubwindows(om.dpy,om.top);

  omArrange();

  return 0;
}

int omArrange(void)
{
  int i, j, col, ncol;
  Window wdummy;
  int idummy;
  unsigned int udummy;
  int theight;
  int x,y,width,height;
  int boxh,boxs2;

  boxh=om.boxh;
  boxs2=1;

  XGetGeometry(om.dpy,om.top,&wdummy,
	       &idummy,&idummy,&om.width,&om.height,
	       &udummy,&udummy);

  om.width-=2;
  om.height-=2;

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
  theight=om.scene.height;
  col=0;

  for(i=0;i<om.ds_count;i++) {
    if(theight+om.ds[i].height>=om.height) {
      col++;
      om.ds[i].c=col;
      om.ds[i].y=1;
      theight=om.ds[i].height;
    } else {
      om.ds[i].c=col;
      om.ds[i].y=theight;
      theight+=om.ds[i].height;
    }
  }
  ncol=col+1;
  om.ncol=ncol;

  /*
    now assign the x coordinates and the width
  */
  om.scene.x=1;
  om.scene.y=1;
  XMoveResizeWindow(om.dpy,om.scene.w,
		    om.scene.x,om.scene.y,
		    om.width/ncol-2,om.scene.height);

  om.boxw=om.width/ncol;

  for(i=0;i<om.ds_count;i++) {
    x=(int)((float)om.ds[i].c*(float)(om.width)/(float)(ncol))+1;
    y=om.ds[i].y;
    width=om.boxw-2;
    height=om.ds[i].height;
    XMoveResizeWindow(om.dpy,om.ds[i].w,x,y,width,height);
    for(j=0;j<om.ds[i].oc;j++) {
      y=j*(om.boxh+1)+boxh+1+om.ds[i].y;
      XMoveResizeWindow(om.dpy,om.ds[i].obj[j].w,x+4,y,om.boxw-6,om.boxh);
    }
  }
  return 0;
}


void omEvent(Window w, XEvent *event, void *ptr)
{
  switch(event->type) {
  case Expose:
    omExposeEvent();
    break;
  case ConfigureNotify:
    /* should now be handled by omArrange */
    omArrange();
    /*
    if(event->xconfigure.width !=om.width ||
       event->xconfigure.height!=om.height)
      omResizeEvent(event->xconfigure.width,event->xconfigure.height);
    */
    break;
  case ButtonPress:
    omButtonEvent(&event->xbutton,1);
    break;
  case ButtonRelease:
    omButtonEvent(&event->xbutton,0);
    break;
  case EnterNotify:
  case LeaveNotify:
    omCrossEvent(&event->xcrossing);
    break;
  }

}

int omExposeEvent()
{
  int i,j,tx,ty;
  char label[256];
  int objw,objh;

  objw=om.boxw-8;
  objh=om.boxh;
  
  strcpy(label,"scene");
  tx=(om.boxw-XTextWidth(om.xfs,label,strlen(label)))/2;
  tx=4;
  ty=om.boxh-(om.boxh-om.xfs->ascent+om.xfs->descent)/2;

  XClearWindow(om.dpy,om.scene.w);
  XDrawString(om.dpy,om.scene.w,om.gc,tx,ty,label,strlen(label));

  for(i=0;i<om.ds_count;i++) {
    tx=4;
    strcpy(label,om.ds[i].name);
    XClearWindow(om.dpy,om.ds[i].w);
    XDrawString(om.dpy,om.ds[i].w,om.gc,tx,ty,label,strlen(label));
    
    XSetLineAttributes(om.dpy,om.gc,2,LineSolid,CapRound,JoinRound);
    for(j=0;j<om.ds[i].oc;j++) {
      tx=12;
      strcpy(label,om.ds[i].obj[j].name);
      XClearWindow(om.dpy,om.ds[i].obj[j].w);
      /* draw the label */
      XDrawString(om.dpy,om.ds[i].obj[j].w,om.gc,tx,ty,label,strlen(label));
      /* draw the shadow box */
      if(om.ds[i].obj[j].show) {
	XSetForeground(om.dpy,om.gc,om.s1_color);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  1,0,1,objh);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  1,0,objw,0);
	XSetForeground(om.dpy,om.gc,om.s2_color);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  1,objh,objw,objh);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  objw,0,objw,objh);
      } else {
	XSetForeground(om.dpy,om.gc,om.s2_color);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  1,0,1,objh);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  1,0,objw,0);
	XSetForeground(om.dpy,om.gc,om.s1_color);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  1,objh,objw,objh);
	XDrawLine(om.dpy,om.ds[i].obj[j].w,om.gc,
		  objw,0,objw,objh);
      }
      XSetForeground(om.dpy,om.gc,om.fg_color);

    }
  }
  XSetLineAttributes(om.dpy,om.gc,1,LineSolid,CapButt,JoinMiter);
  return 0;
}



int omCrossEvent(XCrossingEvent *event)
{
  return 0;
}

int omButtonEvent(XButtonEvent *event,int mp)
{
  int i,j;
  char command[1024];

  if(event->subwindow==om.scene.w) {
    strcpy(om.pbase,"scene");
    if(event->button==3) {
      if(mp==1) {
	om.pflag=1;
	om.pwin=om.scene_popup.top;
	XMoveWindow(om.dpy,om.pwin,event->x_root,event->y_root);
	XMapRaised(om.dpy,om.pwin);
      }
    }
  } else {
    for(i=0;i<om.ds_count;i++) {
      if(event->subwindow==om.ds[i].w) {
	sprintf(om.pbase,".%s",om.ds[i].name);
	if(event->button==3) {
	  if(mp==1) {
	    om.pflag=1;
	    om.pwin=om.ds_popup.top;
	    XMoveWindow(om.dpy,om.pwin,event->x_root,event->y_root);
	    XMapRaised(om.dpy,om.pwin);
	  }
	}
	break;
      } else {
	for(j=0;j<om.ds[i].oc;j++)
	  if(event->subwindow==om.ds[i].obj[j].w) {
	    sprintf(om.pbase,".%s.%s",om.ds[i].name,om.ds[i].obj[j].name);
	    if(mp==1 && event->button==1) {
	      /* toggle the state */
	      if(om.ds[i].obj[j].show) {
		sprintf(command,"%s hide",om.pbase);
		comRawCommand(command);
	      } else {
		sprintf(command,"%s show",om.pbase);
		comRawCommand(command);
	      }
	    }
	    if(mp==1 && event->button==3) {
	      om.pflag=1;
	      om.pwin=om.obj_popup.top;
	      XMoveWindow(om.dpy,om.pwin,event->x_root,event->y_root);
	      XMapRaised(om.dpy,om.pwin);
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

int omResizeEvent(int width, int height)
{
  int i,j;
  int boxw,boxh,boxs1,boxs2;

  om.width=width;
  om.height=height;

  om.boxw=om.width-6;
  boxh=om.boxh;
  boxw=om.boxw;
  boxs1=8;
  boxs2=1;

  XResizeWindow(om.dpy,om.scene.w,boxw,boxh);

  for(i=0;i<om.ds_count;i++) {
    XResizeWindow(om.dpy,om.ds[i].w,boxw,(boxh+boxs2)*(om.ds[i].oc+1)-boxs2+2);
    for(j=0;j<om.ds[i].oc;j++) {
      XResizeWindow(om.dpy,om.ds[i].obj[j].w,boxw-4,boxh);
    }
  }
  return 0;
}




int omAddDB(const char *name)
{
  int i;
  for(i=0;i<om.ds_count;i++)
    if(!strcmp(om.ds[i].name,name)) {
      /* that name is already used, should not
	 happen, so just return
      */
      return -1;
    }
  strcpy(om.ds[om.ds_count].name,name);
  om.ds[om.ds_count].oc=0;
  om.ds_count++;
  omRecreate();
  return 0;
}

int omAddObj(const char *db, const char *name)
{
  int i,j;

  for(i=0;i<om.ds_count;i++)
    if(!strcmp(om.ds[i].name,db)) {
      for(j=0;j<om.ds[i].oc;j++)
	if(!strcmp(om.ds[i].obj[j].name,name)) {
	  /* name already found! this should
	     not happen, just return */
	  return -1;
	}
      strcpy(om.ds[i].obj[om.ds[i].oc].name,name);
      om.ds[i].obj[om.ds[i].oc].show=1;
      om.ds[i].oc++;
    }
  omRecreate();
  return 0;
}

int omDelDB(const char *name)
{
  int i,j;
  for(i=0;i<om.ds_count;i++)
    if(!strcmp(om.ds[i].name,name)) {
      for(j=i+1;j<om.ds_count;j++) {
	memcpy(&om.ds[j-1],&om.ds[j],sizeof(struct OM_DS_ENTRY));
      }
      om.ds_count--;
    }
  omRecreate();
  return 0;
}

int omDelObj(const char *db, const char *name)
{
  int i,j,k;

  for(i=0;i<om.ds_count;i++)
    if(!strcmp(om.ds[i].name,db)) {
      for(j=0;j<om.ds[i].oc;j++)
	if(!strcmp(om.ds[i].obj[j].name,name)) {
	  for(k=j+1;k<om.ds[i].oc;k++) {
	    memcpy(&om.ds[i].obj[k-1],&om.ds[i].obj[k],sizeof(struct OM_OBJ_ENTRY));
	  }
	  om.ds[i].oc--;
	}
    }
  omRecreate();
  return 0;
}

int omHideObj(const char *db, const char *name)
{
  int i,j;
  for(i=0;i<om.ds_count;i++)
    if(!strcmp(om.ds[i].name,db))
      for(j=0;j<om.ds[i].oc;j++)
	if(!strcmp(om.ds[i].obj[j].name,name))
	  om.ds[i].obj[j].show=0;
  omExposeEvent();
  return 0;
}

int omShowObj(const char *db, const char *name)
{
  int i,j;
  for(i=0;i<om.ds_count;i++)
    if(!strcmp(om.ds[i].name,db))
      for(j=0;j<om.ds[i].oc;j++)
	if(!strcmp(om.ds[i].obj[j].name,name))
	  om.ds[i].obj[j].show=1;
  omExposeEvent();
  return 0;
}


int omCreatePopup(struct OM_POPUP *popup,struct OM_POPUP_LIST *list)
{
  int i,lc,lc2, ls,lm;
  int width,height,boxw,boxh;
  XSetWindowAttributes xswa;
  int px,py,pw,ph;
  char *mins="xxxxxxxxxxxxxxx";

  /* create top level window */

  lm=XTextWidth(om.xfs,mins,strlen(mins));

  lc2=0;
  for(lc=0;strlen(list[lc].label)>0;lc++) {
    if(!strcmp(list[lc].label,"separator")) {
      lc2++;
    } else {
      ls=XTextWidth(om.xfs,list[lc].label,strlen(list[lc].label));
      lm=(ls>lm) ? ls : lm;
    }
  }
  /*
    should now have number of labels 
    and length of largest label
  */
  boxh=om.texth+4;
  boxw=lm+12;
  
  height=(lc-lc2)*(boxh+2)+(lc2*4)+6;
  width=boxw+6;
  popup->height=height;
  popup->width=width;

  popup->boxw=boxw;
  popup->boxh=boxh;

  popup->top=XCreateSimpleWindow(om.dpy,om.root,0,0,width,height,0,
				 om.bd_color,om.bg_color);
  XSelectInput(om.dpy,popup->top,
	       ExposureMask |
	       OwnerGrabButtonMask);
  
  /* register event handling */
  guiRegisterCustomEvent(popup->top,omPopupEvent,popup);

  xswa.override_redirect=True;
  XChangeWindowAttributes(om.dpy,popup->top,
			  CWOverrideRedirect,
			  &xswa);

  XSetTransientForHint(om.dpy,popup->top,popup->top);

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
      popup->entry[i].type=OM_POPUP_LABEL;
      popup->entry[i].w=XCreateSimpleWindow(om.dpy,popup->top,
					    px,py,pw,ph,0,
					    om.bd_color,om.bg_color);
      XSelectInput(om.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask);
    } else if(!strcmp(list[i].label,"title")) {
      strcpy(popup->entry[i].label,list[i].command);
      popup->entry[i].type=OM_POPUP_TITLE;
      popup->entry[i].w=XCreateSimpleWindow(om.dpy,popup->top,
					    px,py,pw,ph,0,
					    om.bd_color,om.bg_color);
      XSelectInput(om.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask);

    } else if(!strcmp(list[i].label,"separator")) {
      strcpy(popup->entry[i].label,"---");
      popup->entry[i].type=OM_POPUP_SEPARATOR;
      popup->entry[i].w=XCreateSimpleWindow(om.dpy,popup->top,
					    px,py,pw,4,0,
					    om.bd_color,om.bg_color);
      py-=boxh;
      py+=4;
      popup->entry[i].height=4;
      XSelectInput(om.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask);
    } else {
      strcpy(popup->entry[i].label,list[i].label);
      strcpy(popup->entry[i].command,list[i].command);
      popup->entry[i].type=OM_POPUP_BUTTON;
      popup->entry[i].w=XCreateSimpleWindow(om.dpy,popup->top,
					    px,py,pw,ph,0,
					    om.bd_color,om.bg_color);
      XSelectInput(om.dpy,popup->entry[i].w,
		   ButtonPressMask | 
		   ButtonReleaseMask | 
		   OwnerGrabButtonMask |
		   EnterWindowMask |
		   LeaveWindowMask);
      guiRegisterCustomEvent(popup->entry[i].w,omPopupEvent,&popup->entry[i]);
    }
    py+=boxh+2;

    
  }
  popup->entry_count=lc;

  XMapSubwindows(om.dpy,popup->top);
  return 0;
}


void omPopupEvent(Window w, XEvent *event, void *ptr)
{
  char c1[1024],c2[1024],*cp;

  struct OM_POPUP_ENTRY *entry;
  switch(event->type) {
  case Expose:
    omPopupExpose(w,&event->xexpose,(struct OM_POPUP *)ptr);
    break;
  case EnterNotify:
    entry=(struct OM_POPUP_ENTRY *)ptr;
    XSetFont(om.dpy,om.gc,om.xfs->fid);
    XSetForeground(om.dpy,om.gc,om.bg2_color);
    XFillRectangle(om.dpy,entry->w,om.gc,
		   0,0,entry->width,entry->height);
    XSetForeground(om.dpy,om.gc,om.fg_color);
    XDrawString(om.dpy,entry->w,om.gc,entry->tx,entry->ty,
		entry->label,strlen(entry->label));
    break;
  case LeaveNotify:
    entry=(struct OM_POPUP_ENTRY *)ptr;
    XSetFont(om.dpy,om.gc,om.xfs->fid);
    XClearWindow(om.dpy,entry->w);
    XDrawString(om.dpy,entry->w,om.gc,entry->tx,entry->ty,
		entry->label,strlen(entry->label));
    break;
  case ButtonPress:
    break;
  case ButtonRelease:
    entry=(struct OM_POPUP_ENTRY *)ptr;
    if(event->xbutton.button==3) {
      if(entry->type==OM_POPUP_BUTTON) {
	/* go through command and scan for % */
	strcpy(c1,entry->command);
	cp=strchr(c1,'%');
	if(cp==NULL) {
	  strcpy(c2,c1);
	} else {
	  cp[0]=0;
	  sprintf(c2,"%s%s%s",c1,om.pbase,cp+1);
	}
	comRawCommand(c2);
	om.pflag=0;
      }
    }
    
    break;
  }
}


int omPopupExpose(Window w, XExposeEvent *e, struct OM_POPUP *popup)
{
  int i,tx,ty,ty2;

  /* draw 3D around popup window */
  XSetLineAttributes(om.dpy,om.gc,2,LineSolid,CapRound,JoinRound);
  XSetForeground(om.dpy,om.gc,om.s2_color);
  XDrawLine(om.dpy,popup->top,om.gc,
	    1,1,1,popup->height-1);
  XDrawLine(om.dpy,popup->top,om.gc,
	    1,1,popup->width-1,1);
  XSetForeground(om.dpy,om.gc,om.s1_color);
  XDrawLine(om.dpy,popup->top,om.gc,
	    popup->width-1,1,popup->width-1,popup->height-1);
  XDrawLine(om.dpy,popup->top,om.gc,
	    1,popup->height-1,popup->width-1,popup->height-1);
  XSetForeground(om.dpy,om.gc,om.fg_color);
  

  tx=4;
  ty=popup->boxh-(popup->boxh-om.xfs->ascent+om.xfs->descent)/2;
  ty2=popup->boxh-(popup->boxh-om.xfs2->ascent+om.xfs2->descent)/2;

  for(i=0;i<popup->entry_count;i++) {
    XClearWindow(om.dpy,popup->entry[i].w);
    switch(popup->entry[i].type) {
    case OM_POPUP_TITLE:
      XSetFont(om.dpy,om.gc,om.xfs2->fid);
      XDrawString(om.dpy,popup->entry[i].w,om.gc,tx,ty2,
		  popup->entry[i].label,strlen(popup->entry[i].label));
      popup->entry[i].tx=tx;
      popup->entry[i].ty=ty;
    break;
    case OM_POPUP_LABEL:
      XSetFont(om.dpy,om.gc,om.xfs2->fid);
      /*
      XDrawString(om.dpy,popup->entry[i].w,om.gc,tx,ty2,
		  popup->entry[i].label,strlen(popup->entry[i].label));
      */
      XDrawString(om.dpy,popup->entry[i].w,om.gc,tx,ty2,
		  om.pbase,strlen(om.pbase));
      popup->entry[i].tx=tx;
      popup->entry[i].ty=ty;
      break;
    case OM_POPUP_SEPARATOR:
      XSetLineAttributes(om.dpy,om.gc,1,LineOnOffDash,CapRound,JoinRound);
      XDrawLine(om.dpy,popup->entry[i].w,om.gc,0,2,popup->boxw,2);
      XSetLineAttributes(om.dpy,om.gc,1,LineSolid,CapRound,JoinRound);
      popup->entry[i].tx=0;
      popup->entry[i].ty=0;
      break;
    case OM_POPUP_BUTTON:
      XSetFont(om.dpy,om.gc,om.xfs->fid);
      XDrawString(om.dpy,popup->entry[i].w,om.gc,tx,ty2,
		  popup->entry[i].label,strlen(popup->entry[i].label));
      popup->entry[i].tx=tx;
      popup->entry[i].ty=ty2;
      break;
    }
  }
  XSetFont(om.dpy,om.gc,om.xfs->fid);
  return 0;
}
