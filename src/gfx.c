/**************************************

  gfx.c

  handles all direct graphical output
  via OpenGL calls, especially the display
  of all currently visible objects

  those are rerouted to the gfx routines
  of the individual dataset objects !

***************************************/

#include <stdio.h>
#include <math.h>

#include "dino.h"
#include "gfx.h"
#include "gui.h"
#include "com.h"
#include "dbm.h"
#include "xtal.h"
#include "GLwStereo.h"
#include "render.h"
#include "mat.h"
#include "cgfx.h"
#include "glf.h"
#include "Cmalloc.h"
#include "scene.h"

extern struct SCENE scene;
extern struct GUI gui;
struct GFX gfx;

extern int debug_mode,gfx_mode;

struct GFX_LIGHT gfx_def_light[]={
#if 1
  {1,0,
   {0.5, 0.7, 1.0, 0.0},
   {0.05, 0.05, 0.05, 1.0},
   {0.45, 0.45, 0.45, 1.0},
   {0.3, 0.3, 0.3, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {1,0,
   {-0.7, -0.4, 1.0, 0.0},
   {0.05, 0.05, 0.05, 0.0},
   {0.45, 0.45, 0.45, 1.0},
   {0.3, 0.3, 0.3, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
#else
  {1,0,
   {0.1, 0.2, 1.0, 0.0},
   {0.05, 0.05, 0.05, 1.0},
   {0.6, 0.6, 0.6, 1.0},
   {0.3, 0.3, 0.3, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
#endif
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  },
  {0,0,
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 0.0},
   {1.0, 1.0, 1.0, 1.0},
   {1.0, 1.0, 1.0, 1.0},
   1.0,0.0,0.0,
   180.0,{0,0,-1},0.0
  }
};

struct GFX_CLIP gfx_default_clip =
{
  0,
  {{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1},
   {0,0,0,0}, 
   {0,0,0,0},
   0,0
  },
  {0,0,1},{0,0,0},{0,0,1,0}
};

static GLfloat gfx_line_width = 1.0;
static GLfloat gfx_fog_color[]={0.0,0.0,0.0,0.0};

static GLfloat gfx_mat_ambient[] = {0.19225,0.19225,0.19225,1.0};
static GLfloat gfx_mat_diffuse[] = {0.40754,0.40754,0.40754,1.0};
static GLfloat gfx_mat_specular[] = {0.508273,0.508273,0.508273,1.0};
static GLfloat gfx_mat_shininess[]= {51.2};

static struct RENDER_MATERIAL gfx_def_mat={
  {0.25,0.25,0.25,1.0},
  {0.55,0.55,0.55,1.0},
  {0.20,0.20,0.20,1.0},
  {0.0,0.0,0.0,1.0},
  16
};



/******************************

  gfxInit
  -------

  set all parameters to defaults

*******************************/

int gfxInit()
{
  int i;
  debmsg("gfxInit: filling defaults");
  gfx.mode=GFX_PERSP;

  transReset(&gfx.transform);
  gfx.transform.tra[2]=-100.0;
  gfx.transform.slabn=5.0;
  gfx.transform.slabf=500.0;
  gfx.transform.slabn2=5.0;
  gfx.transform.slabf2=500.0;

  gfx.scale=1.0;
  gfx.fovy=25.0;
  gfx.aspect=1.0;
  gfx.r=0.0;
  gfx.g=0.0;
  gfx.b=0.0;
  
  gfx.axisflag=1;
  gfx.stereo_display=GFX_CENTER;
  gfx.slab_flag=0;
  gfx.spin=0;

  gfx.show=1;

  gfx.fog_color[0]=gfx_fog_color[0];
  gfx.fog_color[1]=gfx_fog_color[1];
  gfx.fog_color[2]=gfx_fog_color[2];
  gfx.fog_color[3]=gfx_fog_color[3];

  gfx.fog=1;
  gfx.fog_dist=0.0;
  gfx.fog_near=gfx.transform.slabn;
  gfx.fog_far=gfx.transform.slabf;

  gfx.offline=0;

  for(i=0;i<8;i++)
    memcpy(&gfx.light[i],&gfx_def_light[i],sizeof(struct GFX_LIGHT));

  memcpy(&gfx.defmat,&gfx_def_mat,sizeof(struct RENDER_MATERIAL));

  // clipping plane
  for(i=0;i<6;i++)
    memcpy(gfx.clip+i,&gfx_default_clip,sizeof(struct GFX_CLIP));


  gfx.save_buf_size=0;
  gfx.save_buf=NULL;

  gfx.sort=0;
  gfx.fbuf_size=100000;
  gfx.fbuf=Crecalloc(NULL,gfx.fbuf_size,sizeof(float));

  gfx.dither=1;
  gfx.smooth=1;

#ifdef EXPO
  gfx.limx1=0;
  gfx.limx2=0;
  gfx.limy1=0;
  gfx.limy2=0;
  gfx.limz1=0;
  gfx.limz2=0;
#endif

  return 0;
}


/******************************************

  gfxGLInit
  ---------

  initialize the OpenGL state machine
  this routine is called from the
  event loop after a valid OpenGL
  rendering context has been established

********************************************/

int gfxGLInit(void)
{
  int i;
  GLfloat ambient[]={0.0,0.0,0.0,1};
  /* 
     lighting
  */
  debmsg("gfxGLInit: setting default GL params");

  for(i=0;i<8;i++) {
    glLightfv(GL_LIGHT0+i, GL_AMBIENT, gfx.light[i].amb);
    glLightfv(GL_LIGHT0+i, GL_DIFFUSE, gfx.light[i].diff);
    glLightfv(GL_LIGHT0+i, GL_SPECULAR, gfx.light[i].spec);
    glLightfv(GL_LIGHT0+i, GL_POSITION, gfx.light[i].pos);
    glLightf(GL_LIGHT0+i, GL_CONSTANT_ATTENUATION, gfx.light[i].kc);
    glLightf(GL_LIGHT0+i, GL_LINEAR_ATTENUATION, gfx.light[i].kl);
    glLightf(GL_LIGHT0+i, GL_QUADRATIC_ATTENUATION, gfx.light[i].kq);
    glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, gfx.light[i].spotc);
    glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, gfx.light[i].spotd);
    glLightf(GL_LIGHT0+i, GL_SPOT_EXPONENT, gfx.light[i].spote);
    if(gfx.light[i].on)
      glEnable(GL_LIGHT0+i);
    else
      glDisable(GL_LIGHT0+i);
  }
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
  //glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  glEnable(GL_LIGHTING);

  /*
    material 
  */
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gfx.defmat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, gfx.defmat.diff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx.defmat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, gfx.defmat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, gfx.defmat.shin);

  /*
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gfx_mat_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, gfx_mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx_mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, gfx_mat_shininess);
  */
  //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

  /*
    depth buffer
  */
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearDepth(1.0);

  /* 
     background color
  */
  glClearColor(gfx.r,gfx.g,gfx.b,0.0);


  /*
    polygon settings
  */
  glFrontFace(GL_CCW);

  /*
    blending
  */
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  /*
    depth effect thru fog
  */
  glFogfv(GL_FOG_COLOR,gfx.fog_color);
  glFogi(GL_FOG_MODE,GL_LINEAR);
  glEnable(GL_FOG);

  /*
    stencil buffer
  */
  glClearStencil(0x0);
  glStencilMask(0xf);
  glDisable(GL_STENCIL_TEST);

  /*
    misc default settings
  */
  glLineWidth(gfx_line_width);
  glPointSize(gfx_line_width);

  glDisable(GL_CULL_FACE);

  if(gfx.smooth)
    glShadeModel(GL_SMOOTH);
  else
    glShadeModel(GL_FLAT);
  
#ifndef EXPO
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POINT_SMOOTH);
#endif

  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  glHint(GL_FOG_HINT, GL_FASTEST);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

  if(gfx.dither)
    glEnable(GL_DITHER);
  else
    glDisable(GL_DITHER);

  glDisable(GL_NORMALIZE);

  /*
    init the fonts
  */

  debmsg("gfxGLInit: calling glfInit");
  glfInit();
  debmsg("gfxGLInit: calling glfGenFont");
  glfGenFont();

  // set projection matrix
  gfxSetProjection(gfx.stereo_display);
  // set fog
  gfxSetFog();

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);


  gfxResizeEvent();

#ifdef VR
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP);

  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

  glEnable(GL_TEXTURE_3D_EXT);
#endif

  return 0;
}


/*********************************

  gfxRedraw
  ---------

  top level redraw routine

**********************************/
  
int gfxRedraw() 
{
  if(gui.stereo_mode==GUI_STEREO_NORMAL) {
    /*
      the projection matrix needs to be set
      for each redraw
    */
    if(gui.stereo_available==GLW_STEREO_LOW)
      gfx.aspect=(double)gui.win_width/(double)gui.win_height*0.5;
    else
      gfx.aspect=(double)gui.win_width/(double)gui.win_height;
    GLwStereoDrawBuffer(GLW_STEREO_RIGHT);
    gfxSetProjection(GFX_RIGHT);
    gfxSceneRedraw(1);
    comDBRedraw();
    GLwStereoDrawBuffer(GLW_STEREO_LEFT);
    gfxSetProjection(GFX_LEFT);
    gfxSceneRedraw(1);
    comDBRedraw();
  } else if(gui.stereo_mode==GUI_STEREO_SPLIT) {
    GLwStereoDrawBuffer(GLW_STEREO_RIGHT);
    gfx.aspect=0.5*(double)gui.win_width/(double)gui.win_height;
    glViewport(0,0,gui.win_width/2, gui.win_height);
    gfxSetProjection(GFX_RIGHT);
    gfxSceneRedraw(1);
    comDBRedraw();
    glViewport(gui.win_width/2+1,0,gui.win_width/2, gui.win_height);
    gfxSetProjection(GFX_LEFT);
    gfxSceneRedraw(0);
    comDBRedraw();
  } else {
    GLwStereoDrawBuffer(GLW_STEREO_LEFT);
    gfxSetProjection(gfx.stereo_display);
    gfxSceneRedraw(1);
    
    comDBRedraw();
  }
  
#ifdef GLUT_GUI
  glutSwapBuffers();
#else
  glXSwapBuffers(gui.dpy, gui.glxwindow);
#endif  

  return 0;
}

int gfxSceneRedraw(int clear)
{
  int i;
  double eye_dist;
  double v[4];

  if(clear)
    GLwStereoClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  for(i=0;i<8;i++)
    if(gfx.light[i].on && !gfx.light[i].local)
      glLightfv(GL_LIGHT0+i,GL_POSITION,gfx.light[i].pos);

  for(i=0;i<1;i++)
    if(gfx.clip[i].on) {
      glPushMatrix();
      glTranslated(gfx.transform.tra[0],
		   gfx.transform.tra[1],
		   gfx.transform.tra[2]);
      
      glTranslated(gfx.transform.cen[0],
		   gfx.transform.cen[1],
		   gfx.transform.cen[2]);
      glTranslatef(gfx.clip[i].pos[0],
		   gfx.clip[i].pos[1],
		   gfx.clip[i].pos[2]);
      v[0]=gfx.clip[i].dir[0];
      v[1]=gfx.clip[i].dir[1];
      v[2]=gfx.clip[i].dir[2];
      transApplyRot(&gfx.clip[i].transform,v);
      v[3]=0.0;
      glClipPlane(GL_CLIP_PLANE0+i,v);
      glEnable(GL_CLIP_PLANE0+i);
      glPopMatrix();
    } else {
      glDisable(GL_CLIP_PLANE0+i);
    }
  
  
  if(gfx.mode==GFX_ORTHO)
    if(gfx.scale!=1.0)
      glScaled(gfx.scale,gfx.scale,gfx.scale);
  
  glTranslated(gfx.transform.tra[0],
	       gfx.transform.tra[1],
	       gfx.transform.tra[2]);

  glMultMatrixd(gfx.transform.rot);

  glTranslated(gfx.transform.cen[0],
	       gfx.transform.cen[1],
	       gfx.transform.cen[2]);

  for(i=0;i<8;i++)
    if(gfx.light[i].on && gfx.light[i].local)
      glLightfv(GL_LIGHT0+i,GL_POSITION,gfx.light[i].pos);

#ifdef EXPO
  glDisable(GL_LIGHTING);
  for(i=0;i<scene.label_c;i++) {
    glRasterPos3fv(scene.label[i].p);
    glColor3fv(scene.label[i].c);
    glfDrawString(scene.label[i].s);
    
  }

  glColor3f(0.8,0.8,0.0);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(3.0);
  glBegin(GL_POINTS);
  glVertex3f(-gfx.transform.cen[0],
	     -gfx.transform.cen[1],
	     -gfx.transform.cen[2]);
  glEnd();
	     
  glEnable(GL_LIGHTING);


#endif
  if(scene.cpflag) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0,1.0,0.5);
    glBegin(GL_LINES);
    glVertex3f(scene.cp[0]-1.0,scene.cp[1],scene.cp[2]);
    glVertex3f(scene.cp[0]+1.0,scene.cp[1],scene.cp[2]);
    glVertex3f(scene.cp[0],scene.cp[1]-1.0,scene.cp[2]);
    glVertex3f(scene.cp[0],scene.cp[1]+1.0,scene.cp[2]);
    glVertex3f(scene.cp[0],scene.cp[1],scene.cp[2]-1.0);
    glVertex3f(scene.cp[0],scene.cp[1],scene.cp[2]+1.0);
    glEnd();
    glEnable(GL_LIGHTING);
  }

  return 0;
}

int gfxSetViewport(void)
{
  glViewport(0,0,gui.win_width, gui.win_height);
  gfx.aspect=(double)gui.win_width/(double)gui.win_height;
  gfxSetProjection(gfx.stereo_display);
  return 0;
}

int gfxSetProjection(int view)
{
  GLdouble iod,fd;
  if(view==GFX_LEFT) {
    iod=-gui.eye_dist;
    //    fd=gfx.transform.tra[2];
    fd=gui.eye_offset;
  } else if(view==GFX_RIGHT) {
    iod=gui.eye_dist;
    //    fd=gfx.transform.tra[2];
    fd=gui.eye_offset;
  } else {
    iod=0.0;
    fd=0.0;
  }

  //  fd=gfx.transform.slabn;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if(gfx.mode==GFX_PERSP) {
    /*
    gluPerspective(gfx.fovy,gfx.aspect,
		   gfx.transform.slabn,gfx.transform.slabf);
    */
    GLwStereoPerspective(gfx.fovy,gfx.aspect,
			 gfx.transform.slabn2,gfx.transform.slabf2,
			 iod,fd);
  } else {
    GLwStereoOrtho(gfx.left,gfx.right,gfx.bottom,gfx.top,
		   gfx.transform.slabn2,gfx.transform.slabf2,
		   0.0, 0.0);
  }
  return 0;
}

int gfxSetSlab(float near, float far)
{
  gfx.transform.slabn=near;
  gfx.transform.slabf=far;
  if(near<1.0)
    near=1.0;
  if(far<near+0.1)
    far=near+0.1;
  gfx.transform.slabn2=near;
  gfx.transform.slabf2=far;

  gfxSetProjection(gfx.stereo_display);
  gfxSetFog();
  return 0;
}

int gfxSetFog(void)
{
  int slab_near_far_dist=gfx.transform.slabf-gfx.transform.slabn;

  gfx.fog_near=gfx.transform.slabn;
  gfx.fog_far=slab_near_far_dist*gfx.fog_dist+gfx.transform.slabf;

  glFogf(GL_FOG_START,gfx.fog_near);
  glFogf(GL_FOG_END,gfx.fog_far);

  return 0;
}


int gfxDrawCell(struct XTAL *xtal)
{
  const int conn[][3]={
    {0,1},
    {0,2},
    {0,4},
    {1,3},
    {1,5},
    {2,3},
    {2,6},
    {3,7},
    {4,5},
    {4,6},
    {5,7},
    {6,7},
  };
  int i;


  glPushAttrib(GL_ENABLE_BIT);

  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_LINE_SMOOTH);


  glBegin(GL_LINES);
  glColor3f(1.0,1.0,1.0);
  for(i=0;i<12;i++) {
    glVertex3dv(xtal->cc[conn[i][0]]);
    glVertex3dv(xtal->cc[conn[i][1]]);
  }
  glEnd();

  glPopAttrib();

  return 0;
}

int gfxSetup(void)
{
  return 0;
}


int gfxResizeEvent(void)
{
#ifdef TRANSP_NEW
  gfx.save_buf_size=(gui.win_width)*gui.win_height;
  gfx.save_buf=Crecalloc(gfx.save_buf,gfx.save_buf_size,sizeof(float));
  //  fprintf(stderr,"\n%d", gfx.save_buf_size);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glOrtho(0,0,gui.win_width,gui.win_height,1,1);
  glGetDoublev(GL_PROJECTION_MATRIX,gfx.pmat);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
#endif
  return 0;
}

int gfxSaveDepthBuffer(void)
{
  //  fprintf(stderr,"r[");
  //  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  //  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0,0,gui.win_width,gui.win_height,
	       GL_ALPHA,GL_FLOAT,
	       gfx.save_buf);
  //  glPopClientAttrib(); 
  //  fprintf(stderr,"]");
  return 0;
}

int gfxRestoreDepthBuffer(void)
{
  //  fprintf(stderr,"w[");
  //  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  //  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  //  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixd(gfx.pmat);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
  glRasterPos2f(0.0,0.0);
  glDrawPixels(gui.win_width,gui.win_height,
	       GL_ALPHA,GL_FLOAT,
	       gfx.save_buf);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
//  glPopClientAttrib(); 
//  fprintf(stderr,"]\n");
  return 0;
}

// under development TODO later

int gfxGenFeedback(void)
{
  int ret,failure;

  failure=0;
  do {
    glFeedbackBuffer(gfx.fbuf_size, GL_3D_COLOR,gfx.fbuf);
    glRenderMode(GL_FEEDBACK);
    comDBRedraw();
    ret=glRenderMode(GL_RENDER);
    if(ret<0) {
      if(gfx.fbuf_size>100000000)
	break;
      failure++;
      gfx.fbuf_size*=2;
      gfx.fbuf=Crecalloc(gfx.fbuf,gfx.fbuf_size,sizeof(GLfloat));
    }
  } while(ret<0 && failure<8);

  if(ret<0) {
    comMessage("\nError during feedback buffer generation: scene reset to !sort");
    gfx.sort=0;
    gfx.fbuf_entries=0;
    gfx.fbuf_size=100000;
    gfx.fbuf=Crecalloc(gfx.fbuf,gfx.fbuf_size,sizeof(GLfloat));
    comRedraw();
    return -1;
  } else {
    gfx.fbuf_entries=ret;
    return 0;
  }
}


int gfxDrawFeedback(void)
{
  int i,indx;

  for(i=0;i<gfx.fbuf_entries;i++) {
    
  }
  return 0;
}

int gfxSetAccProjection(int view, float fpixdx, float fpixdy)
{
  GLdouble eye_offset;
  GLdouble pixdx, pixdy;

  if(view==GFX_LEFT) {
  } else if(view==GFX_RIGHT) {
  } else {
  }

  pixdx=fpixdx;
  pixdy=fpixdy;

  gfxAccPerspective(gfx.fovy,gfx.aspect,
		    gfx.transform.slabn,gfx.transform.slabf,
		    pixdx, pixdy);

  return 0;

}


/* after EXAMPLE 10-2 OpenGL Programming Guide 3rd Edition */

int gfxAccFrustum(GLdouble left, GLdouble right, 
		  GLdouble bottom, GLdouble top,
		  GLdouble zNear, GLdouble zFar,
		  GLdouble pixdx, GLdouble pixdy)
{
  GLdouble xwsize, ywsize, dx, dy;
  GLint viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

  xwsize=right-left;
  ywsize=top-bottom;
  dx=-(pixdx*xwsize/(GLdouble) viewport[2]);
  dy=-(pixdy*ywsize/(GLdouble) viewport[3]);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(left+dx,right+dx,bottom+dy,top+dy, zNear, zFar);

  return 0;
}

int gfxAccPerspective(GLdouble fovy, GLdouble aspect,
		      GLdouble zNear, GLdouble zFar,
		      GLdouble pixdx, GLdouble pixdy)
{
  GLdouble fov2, left, right, bottom, top;

  fov2=((fovy*M_PI)/180.0)/2.0;

  top=zNear / (cos(fov2) / sin(fov2));
  bottom=-top;
  right=top*aspect;
  left=-right;

  gfxAccFrustum(left, right, bottom, top, zNear, zFar, pixdx, pixdy);

  return 0;
}
