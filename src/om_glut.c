#include <stdio.h>

#include "gui.h"
#include "om_glut.h"
#include "cl.h"
#include "com.h"

extern struct GUI gui;

struct OM_OBJ_ENTRY {
  int x1,y1,x2,y2,w,h;
  char name[256];
  int show;
};

struct OM_DS_ENTRY {
  int x1,y1,x2,y2,w,h;
  char name[256];
  struct OM_OBJ_ENTRY obj[256];
  int oc;
};

struct OM {
  int width,height,bheight;
  struct OM_DS_ENTRY ds[256];
  int ds_count;
  int popup;
  char **popup_list;
  int popup_list_count;
  struct OM_DS_ENTRY *dh;
  struct OM_OBJ_ENTRY *oh;
}om;



static void draw_string(const char *s)
{
  int i;
  for(i=0;i<clStrlen(s);i++) 
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15,s[i]);
}

static void draw_label(int x,int y, int w, int h, int f, const char *s)
{
  w-=2;
  if(f) {
    glColor3f(0.0,0.0,0.0);
    glRectd(x,y,x+w,y+h);
    glColor3f(0.85,0.85,0.95);
    glRectd(x+1,y+1,x+w-1,y+h-1);
  } else {
    glColor3f(0.85,0.85,0.95);
    glRectd(x,y,x+w,y+h);
  }

  glColor3f(0.0,0.0,0.0);
  glRasterPos2d(x+5,y+h-(h-16)/2-3);
  draw_string(s);
}

static void draw_panel(int x,int y, int w, int h, int f, const char *s)
{
  w-=2;
  if(f) {
    glColor3f(0.0,0.0,0.0);
    glRectd(x,y,x+w,y+h);
    glColor3f(0.85,0.85,0.95);
    glRectd(x+1,y+1,x+w-1,y+h-1);
  } else {
    glColor3f(0.85,0.85,0.95);
    glRectd(x,y,x+w,y+h);
  }

  glColor3f(0.0,0.0,0.0);
  glRasterPos2d(x+5,y+17);
  draw_string(s);
}

static void draw_button(int x,int y, int w, int h, int t, const char *s)
{
  w-=2;
  glColor3f(0.0,0.0,0.0);
  glRectd(x,y,x+w,y+h);
  if(!t) {
    glColor3f(0.5,0.5,0.6);
    glRectd(x+1,y+1,x+w-1,y+h-1);
    glColor3f(0.9,0.9,1.0);
    glRectd(x+1,y+1,x+w-2,y+h-2);
    glColor3f(0.8,0.8,0.9);
    glRectd(x+2,y+2,x+w-2,y+h-2);
  } else {
    glColor3f(0.5,0.5,0.6);
    glRectd(x+1,y+1,x+w-1,y+h-1);
    glColor3f(0.9,0.9,1.0);
    glRectd(x+2,y+2,x+w-1,y+h-1);
    glColor3f(0.75,0.75,0.85);
    glRectd(x+2,y+2,x+w-2,y+h-2);
  }

  glColor3f(0.0,0.0,0.0);
  glRasterPos2d(x+7,y+h-(h-16)/2-3);
  draw_string(s);
}


static int popup_hilight;
static int popup_fh=15;
static int popup_bh=17;

static char *pl_scene[]={
  "autoslab","scene autoslab",
  "view left","scene set view=left",
  "view right","scene set view=right",
  "view center","scene set view=center",
  "hide all","scene hide",
  "show all","scene show",
  "toggle stereo","scene stereo"
};

static int plc_scene=7;

static char *ds_list[]={
  "center","scene center [.%s]",
  "hide all",".%s.* hide",
  "show all",".%s.* show"
};

static int dsc_list=3;

static char *obj_list[]={
  "center","scene center [.%s.%s]"
};

static int objc_list=1;

static void popup_redisplay()
{
  glutSetWindow(om.popup);
  glutPostRedisplay();
  glutSetWindow(gui.glut_main);
}

static void popup_close()
{
  glutSetWindow(om.popup);
  glutHideWindow();
  glutSetWindow(gui.glut_main);
}

static void popup_entry(int state)
{
  if(state==GLUT_LEFT) {
    popup_close();
  }
}

static void popup_motion(int x, int y)
{
  popup_hilight=y/popup_bh;
  glutSetWindow(om.popup);
  glutPostRedisplay();
  glutSetWindow(gui.glut_main);
}

static void popup_mouse(int button, int state, int x, int y)
{
  int i;
  char com[256],com2[256];
  if(state==GLUT_UP) {
    i=y/popup_bh;
    if(om.dh!=NULL) {
      if(om.oh!=NULL) {
	sprintf(com2,"%s",om.popup_list[i*2+1]);
	sprintf(com,com2,om.dh->name,om.oh->name);
      } else {
	sprintf(com2,"%s",om.popup_list[i*2+1]);
	sprintf(com,com2,om.dh->name);
      }
    } else {
      sprintf(com,"%s",om.popup_list[i*2+1]);
    }
    glutSetWindow(gui.glut_main);
    comRawCommand(com);
    popup_close();
  }
}

static void popup_expose()
{
  int width,height;
  int theight=15;
  int i;
  
  glutSetWindow(om.popup);
  glClear(GL_COLOR_BUFFER_BIT);
  width=glutGet(GLUT_WINDOW_WIDTH);
  height=glutGet(GLUT_WINDOW_HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,width,height,0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // background
  glColor3f(0.3,0.3,0.3);
  glRectd(0,0,width,height);
  glColor3f(0.85,0.90,0.90);
  glRectd(1,1,width-2,height-2);

  // hilight
  glColor3f(0.95,0.99,0.99);
  glRectd(1,popup_hilight*popup_bh+1,width-2,(popup_hilight+1)*popup_bh-2);

  // text entries
  glColor3f(0.0,0.0,0.0);
  for(i=0;i<om.popup_list_count;i++) {
    glRasterPos2d(4,(i+1)*popup_bh-3);
    draw_string(om.popup_list[i*2]);
  }

  // finish
  glutSwapBuffers();
  glutSetWindow(gui.glut_main);
}

static void om_redisplay()
{
  glutSetWindow(gui.glut_om);
  glutPostRedisplay();
  glutSetWindow(gui.glut_main);
}


static void om_mouse(int button, int state, int x, int y)
{
  int i,j,pflag=0;
  struct OM_DS_ENTRY *de,*dh;
  struct OM_OBJ_ENTRY *oe,*oh=NULL;
  char raw_com[256];

  if(button==GLUT_RIGHT_BUTTON) {
    if(state==GLUT_DOWN) {
      // check wether popup should be activated
      // and if yes, which list to use

      // scene
      if(y>0 && y< om.bheight) {
	om.popup_list=pl_scene;
	om.popup_list_count=plc_scene;
	om.dh=NULL;
	om.oh=NULL;
	pflag=1;
      } else {
	// one of the ds labels
	for(i=0;i<om.ds_count;i++) {
	  de=&om.ds[i];
	  if(y>de->y1 && y<de->y2) {
	    // check wether obj
	    oh=NULL;
	    for(j=0;j<de->oc;j++) {
	      oe=&de->obj[j];
	      if(y>oe->y1 && y<oe->y2) {
		oh=oe;
		break;
	      }
	    }
	    if(oh) {
	      // popup obj menu
	      om.dh=de;
	      om.oh=oh;
	      om.popup_list=obj_list;
	      om.popup_list_count=objc_list;
	      pflag=1;
	    } else {
	      // popup ds menu
	      om.dh=de;
	      om.oh=NULL;
	      om.popup_list=ds_list;
	      om.popup_list_count=dsc_list;
	      pflag=1;
	    }
	    break;
	  }
	}
      }
      if(pflag) {
	glutSetWindow(om.popup);
	glutPositionWindow(4,y);
	glutReshapeWindow(om.width-4,popup_bh*om.popup_list_count);
	glutShowWindow();
	glutSetWindow(gui.glut_main);
      } else {
	// in case its up hide it
	popup_close();
      }
    }
  } else if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN) {
    // in case popup is still up, close it
    popup_close();

    // check wether an object entry was clicked
    for(i=0;i<om.ds_count;i++) {
      de=&om.ds[i];
      for(j=0;j<de->oc;j++) {
	oe=&de->obj[j];
	if(x>oe->x1 && x<oe->x2 && y>oe->y1 && y<oe->y2) {
	  dh=de;
	  oh=oe;
	}
      }
    }

    // object found ?
    if(oh!=NULL) {
      if(oh->show) {
	oh->show=0;
	sprintf(raw_com,".%s.%s hide",dh->name,oh->name);
	comRawCommand(raw_com);
      } else {
	oh->show=1;
	sprintf(raw_com,".%s.%s show",dh->name,oh->name);
	comRawCommand(raw_com);
      }
      om_redisplay();
    }
  }
}

static void om_regenerate()
{
  int i,j,offset=om.bheight;
  struct OM_DS_ENTRY *de;
  struct OM_OBJ_ENTRY *oe;
  for(i=0;i<om.ds_count;i++) {
    offset+=2;
    de=&om.ds[i];
    de->x1=1;
    de->y1=offset;
    de->w=om.width;
    de->h=om.bheight*(de->oc+1);
    de->x2=de->x1+de->w;
    de->y2=de->y1+de->h;
    offset+=om.bheight;
    for(j=0;j<de->oc;j++) {
      oe=&de->obj[j];
      oe->x1=2;
      oe->y1=offset;
      oe->w=om.width-2;
      oe->h=om.bheight;
      oe->x2=oe->x1+oe->w;
      oe->y2=oe->y1+oe->h;
      offset+=om.bheight;
    }
  }
  om_redisplay();
}

static void om_reshape(int w, int h)
{
  glutSetWindow(gui.glut_om);
  om.width=w;
  om.height=h;
  glViewport(0,0,w,h);
  om_regenerate();
  glutSetWindow(gui.glut_main);
}

static void om_expose()
{
  int fheight=15,bheight=20;
  int i,j,offset;
  struct OM_DS_ENTRY *de;
  struct OM_OBJ_ENTRY *oe;

  glutSetWindow(gui.glut_om);
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,om.width,om.height,0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // background
  glColor3f(1.0,1.0,1.0);
  glRectd(0,0,om.width,om.height);

  // scene label
  draw_label(1,1,om.width,om.bheight,1,"Scene");

  for(i=0;i<om.ds_count;i++) {
    de=&om.ds[i];
    draw_panel(de->x1,de->y1,de->w,de->h,1,de->name);
    for(j=0;j<de->oc;j++) {
      oe=&de->obj[j];
      draw_button(oe->x1,oe->y1,oe->w,oe->h,oe->show,oe->name);
    }
  }

  glutSwapBuffers();
  glutSetWindow(gui.glut_main);
}


int omInit()
{
  om.width=150;
  om.height=400;
  om.bheight=20;

  om.ds_count=0;


  // generate object menu
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  gui.glut_om=glutCreateWindow("dino om");
  glutReshapeWindow(om.width,om.height);
  glutPositionWindow(10,10);
  glutDisplayFunc(om_expose);
  glutReshapeFunc(om_reshape);
  glutMouseFunc(om_mouse);

  // generate popup window 
  om.popup_list=pl_scene;
  om.popup_list_count=3;

  om.popup=glutCreateSubWindow(gui.glut_om,0,0,om.width-2,100);
  glutHideWindow();
  glutDisplayFunc(popup_expose);
  glutMouseFunc(popup_mouse);
  glutMotionFunc(popup_motion);
  glutPassiveMotionFunc(popup_motion);
  glutEntryFunc(popup_entry);

  om_regenerate();

  return 0;
}

int omAddDB(const char *name)
{
  struct OM_DS_ENTRY *de=&om.ds[om.ds_count];

  clStrncpy(de->name,name,256);
  de->oc=0;

  om.ds_count++;
  om_regenerate();
  return 0;
}

int omDelDB(const char *name)
{
  return 0;
}

int omAddObj(const char *db, const char *name)
{
  int i;
  struct OM_DS_ENTRY *de;

  for(i=0;i<om.ds_count;i++) {
    de=&om.ds[i];
    if(clStrcmp(db,de->name)) {
      clStrcpy(de->obj[de->oc].name,name);
      de->obj[de->oc].show=1;
      de->oc++;
    }
  }
  om_regenerate();
  return 0;
}

int omDelObj(const char *db, const char *name)
{
  return 0;
}

int omHideObj(const char *db, const char *name)
{
  int i,j;
  for(i=0;i<om.ds_count;i++) {
    if(clStrcmp(om.ds[i].name,db)) {
      for(j=0;j<om.ds[i].oc;j++) {
	if(clStrcmp(om.ds[i].obj[j].name,name)) {
	  om.ds[i].obj[j].show=0;
	  om_redisplay();
	}
      }
    }
  }
  return 0;
}

int omShowObj(const char *db, const char *name)
{
  int i,j;
  for(i=0;i<om.ds_count;i++) {
    if(clStrcmp(om.ds[i].name,db)) {
      for(j=0;j<om.ds[i].oc;j++) {
	if(clStrcmp(om.ds[i].obj[j].name,name)) {
	  om.ds[i].obj[j].show=1;
	  om_redisplay();
	}
      }
    }
  }
  return 0;
}

