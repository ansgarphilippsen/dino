#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include <X11/Xlib.h>

//#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glx.h>

#include "write.h"
#include "gui_x11.h"
#include "Cmalloc.h"

extern struct GUI gui;

static XVisualInfo * writeGetVis(int accum_flag);
static int ximage2img(XImage *image,struct WRITE_IMAGE *wimage);

int guiGetImage(struct WRITE_IMAGE *wimage)
{
  XImage *image;
  Pixmap pixmap;
  GLXPixmap glx_pixmap;
  GLXContext glx_context;
  XWindowAttributes win_info;
  XVisualInfo *visinfo,vis_copy;
  GLuint fl16,fl24;
  int owidth,oheight;
  int i,j;
  char message[256];
  int accum_flag;
  int dump=1;

  if(dump) {
    visinfo=gui.visinfo;

    // convert glxwindow to ximage
    image=XGetImage(gui.dpy,gui.glxwindow,
		    0,0,gui.win_width,gui.win_height,
		    AllPlanes,ZPixmap);
    
    if(image==NULL) {
      fprintf(stderr,"\nError while creating image");
    } else {
      image->red_mask = visinfo->red_mask;
      image->green_mask = visinfo->green_mask;
      image->blue_mask = visinfo->blue_mask;
      
      ximage2img(image,wimage);

      /* Destroy XImage */
      XDestroyImage(image);
    }
  } else {
    
    
  }  
  return 0;
}

static int ximage2img(XImage *image,struct WRITE_IMAGE *wimage)
{
  long val;
  int w,h,k;
  unsigned long rm, bm, gm, rs, bs, gs;
  int rv,gv,bv;
  unsigned char red, green, blue;
  float r1,g1,b1,r2,g2,b2;
  char message[512];
  unsigned char *data;

  data=Ccalloc(image->width*image->height*3,sizeof(unsigned char));

  /* initialize the conversion stuff */
  rm=image->red_mask;
  bm=image->blue_mask;
  gm=image->green_mask;

  rs=0;
  while(!(rm & 1)) { rm>>=1; rs++; }
  bs=0;
  while(!(bm & 1)) { bm>>=1; bs++; }
  gs=0;
  while(!(gm & 1)) { gm>>=1; gs++; }

  r1=255.0/((float)rm);
  g1=255.0/((float)gm);
  b1=255.0/((float)bm);

  rm=image->red_mask;
  bm=image->blue_mask;
  gm=image->green_mask;

  /* get the image pixel by pixel */

  for(h=0;h<image->height;h++) {
    for(w=0;w<image->width;w++) {
      val=XGetPixel(image,w,h);
      rv=(int)((val & rm)>> rs);
      gv=(int)((val & gm)>> gs);
      bv=(int)((val & bm)>> bs);
      r2=r1*(float)rv;
      g2=g1*(float)gv;
      b2=b1*(float)bv;
      red=(unsigned char)r2;
      green=(unsigned char)g2;
      blue=(unsigned char)b2;
      data[(h*image->width+w)*3+0]=red;
      data[(h*image->width+w)*3+1]=green;
      data[(h*image->width+w)*3+2]=blue;
    }
  }  

  wimage->data=data;
  wimage->width=image->width;
  wimage->height=image->height;

  return 0;
}


// old stuff below

#ifdef DUMMY_X11

int writeFile2(char *name, int type, int accum, float scale, int dump)
{
#ifdef X11_GUI
  XImage *image;
  Pixmap pixmap;
  GLXPixmap glx_pixmap;
  GLXContext glx_context;
  XWindowAttributes win_info;
  XVisualInfo *visinfo,vis_copy;
  GLuint fl16,fl24;
  int owidth,oheight;
  int i,j;
  char message[256];
  int accum_flag;

  /* get a new visinfo */

  if(dump) {
    visinfo=gui.visinfo;

    debmsg("writeFile: converting window to XImage");
    image=XGetImage(gui.dpy,gui.glxwindow,
		    0,0,gui.win_width,gui.win_height,
		    AllPlanes,ZPixmap);
    
    if(image==NULL) {
      fprintf(stderr,"\nError while creating image");
    } else {
      image->red_mask = visinfo->red_mask;
      image->green_mask = visinfo->green_mask;
      image->blue_mask = visinfo->blue_mask;
      
      switch(type) {
      case WRITE_TYPE_TIFF:
	writeImage2Tiff(image,name);
	break;
      case WRITE_TYPE_PNG:
	writeImage2PNG(image,name);
	break;
      }
      /* Destroy XImage */
      debmsg("writeFile: Destroying XImage");
      XDestroyImage(image);
    }
  } else {
    owidth=gui.win_width;
    oheight=gui.win_height;
    gui.win_width=(int)(scale*(float)gui.win_width);
    gui.win_height=(int)(scale*(float)gui.win_height);
    gfxResizeEvent();

    if(accum>1)
      accum_flag=1;
    else
      accum_flag=0;

    visinfo=writeGetVis(accum_flag);
    if(visinfo==NULL) {
      comMessage("\nfailed to find a correct visual for offline rendering");
      if(accum_flag) {
	comMessage("\nplease try again without accumulation (-a)");
      }
      return -1;
    }
    
    
    /* Create second GLX context */
    debmsg("writeFile: creating second rendering context");
    if((glx_context=glXCreateContext(gui.dpy, visinfo,0, False))==NULL) {
      comMessage("\nerror while creating offline rendering context");
      return -1;
    }
    
    /* Create X pixmap */
    sprintf(message,"writeFile: creating %dx%dx%d Pixmap",
	    gui.win_width, gui.win_height,visinfo->depth);
    debmsg(message);
    
    pixmap=XCreatePixmap(gui.dpy,gui.glxwindow,
			 gui.win_width,gui.win_height,
			 visinfo->depth);
    
    /* create GLXPixmap */
    debmsg("writeFile: creating GLX Pixmap");
    glx_pixmap=glXCreateGLXPixmap(gui.dpy, visinfo, pixmap);
    
    /* attach to second (indirect) rendering context */
    debmsg("writeFile: attaching GLX Pixmap to second rendering context");
    glXMakeCurrent(gui.dpy,glx_pixmap,glx_context);
    
    /* Initialize the rendering context */
    gfxSetViewport();
    gfxGLInit();
    
    /*
      update the display lists
    */
    fl16=glf.font_list_16;
    fl24=glf.font_list_24;
    glf.font_list_16=glGenLists(128);
    glf.font_list_24=glGenLists(128);
    glfGenFont();
    glDeleteLists(glf.font_list_24,128);
    glDeleteLists(glf.font_list_16,128);
    glf.font_list_16=fl16;
    glf.font_list_24=fl24;
    
    /* check fog */
    if(gfx.fog)
      glEnable(GL_FOG);
    else
      glDisable(GL_FOG);
    
#ifdef CPK_NEW
    for(i=0;i<dbm.nodec_max;i++)
      if(dbm.node[i].common.type==DBM_NODE_STRUCT)
	for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	  if(dbm.node[i].structNode.obj_flag[j]!=0) {
	    comNewDisplayList(dbm.node[i].structNode.obj[j].sphere_list);
	    cgfxSphere(1.0,dbm.node[i].structNode.obj[j].render.detail1);
	    comEndDisplayList();
	  }
#endif
    
    if(gui.stereo_mode==GUI_STEREO_NORMAL) {
      gfx.aspect=1.0*(double)gui.win_width/(double)gui.win_height;
      writeRedraw(WRITE_CENTER_VIEW, accum,1);
      glXWaitGL();
    } else if(gui.stereo_mode==GUI_STEREO_SPLIT) {
      gfx.aspect=0.5*(double)gui.win_width/(double)gui.win_height;
      glViewport(0,0,gui.win_width/2, gui.win_height);
      writeRedraw(WRITE_RIGHT_VIEW, accum,1);
      glViewport(gui.win_width/2+1,0,gui.win_width/2, gui.win_height);
      writeRedraw(WRITE_LEFT_VIEW, accum,0);
      gfx.aspect=1.0*(double)gui.win_width/(double)gui.win_height;
      glViewport(gui.win_width,0,gui.win_width, gui.win_height);
    } else {
      gfx.aspect=1.0*(double)gui.win_width/(double)gui.win_height;
      if(gfx.stereo_view==GFX_LEFT)
	writeRedraw(WRITE_LEFT_VIEW, accum,1);
      else if(gfx.stereo_view==GFX_RIGHT)
	writeRedraw(WRITE_RIGHT_VIEW, accum,1);
      else
	writeRedraw(WRITE_CENTER_VIEW, accum,1);
      glXWaitGL();
    }
    
    /* convert pixmap to XImage */
    debmsg("writeFile: converting Pixmap to XImage");
    image=XGetImage(gui.dpy,pixmap,
		    0,0,gui.win_width,gui.win_height,
		    AllPlanes,ZPixmap);
    
    if(image==NULL) {
      fprintf(stderr,"\nError while creating image");
    } else {
      image->red_mask = visinfo->red_mask;
      image->green_mask = visinfo->green_mask;
      image->blue_mask = visinfo->blue_mask;
      
#ifdef IMAGICK
      
#else
      switch(type) {
      case WRITE_TYPE_TIFF:
	writeImage2Tiff(image,name);
	break;
      case WRITE_TYPE_PNG:
	writeImage2PNG(image,name);
	break;
      }
#endif
      /* Destroy XImage */
      debmsg("writeFile: Destroying XImage");
      XDestroyImage(image);
    }
    
    /* re-attach to direct rendering context */
    debmsg("writeFile: re-atttaching to to direct rendering context");
    glXMakeCurrent(gui.dpy,gui.glxwindow,gui.glxcontext);
    
    /* destroy GLXPixmap */
    debmsg("writeFile: destroying GLX Pixmap");
    glXDestroyGLXPixmap(gui.dpy,glx_pixmap);
    
    /* Destroy X pixmap */
    debmsg("writeFile: destroying Pixmap");
    XFreePixmap(gui.dpy,pixmap);
    
    gui.win_width=owidth;
    gui.win_height=oheight;
    gfxSetViewport();
    gfxResizeEvent();

  }
  return 0;
#else
  return -1;
#endif
}

static XVisualInfo * writeGetVis(int accum_flag)
{
  int buf[64];
  int bufc=0,i,j,k;
  int depthi,rgbai,accumi;
  int d[]={12,8,4,1};
  int c[]={12,8,6,4,2,1};
  int a[]={8,4,2,1};
  char message[256];
  XVisualInfo *vis;

  buf[bufc++]=GLX_RGBA;
  buf[bufc++]=GLX_DOUBLEBUFFER;
  buf[bufc++]=GLX_DEPTH_SIZE;
  depthi=bufc++;
  buf[bufc++]=GLX_RED_SIZE;
  rgbai=bufc++;
  buf[bufc++]=GLX_BLUE_SIZE;
  bufc++;
  buf[bufc++]=GLX_GREEN_SIZE;
  bufc++;
  /*
  buf[bufc++]=GLX_ALPHA_SIZE;
  bufc++;
  */
  if(accum_flag) {
    buf[bufc++]=GLX_ACCUM_RED_SIZE;
    accumi=bufc++;
    buf[bufc++]=GLX_ACCUM_BLUE_SIZE;
    bufc++;
    buf[bufc++]=GLX_ACCUM_GREEN_SIZE;
    bufc++;
  }
  buf[bufc++]=None;

  if(accum_flag) {
    for(k=0;k<sizeof(a);k++) {
      for(i=0;i<sizeof(d);i++) {
	for(j=0;j<sizeof(c);j++) {
	  buf[depthi]=d[i];
	  buf[rgbai+0]=c[j];
	  buf[rgbai+2]=c[j];
	  buf[rgbai+4]=c[j];
	  buf[rgbai+6]=c[j];
	  buf[accumi+0]=a[k];
	  buf[accumi+2]=a[k];
	  buf[accumi+4]=a[k];
	  
	  vis=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),buf);
	  if(vis!=NULL) {
	    sprintf(message,"found visual with rgba %d, depth %d and accum %d",
		    buf[rgbai],buf[depthi],buf[accumi]);
	    debmsg(message);
	    return vis;
	  }
	}
      }
    }
  } else {
    for(i=0;i<sizeof(d);i++) {
      for(j=0;j<sizeof(c);j++) {
	buf[depthi]=d[i];
	buf[rgbai+0]=c[j];
	buf[rgbai+2]=c[j];
	buf[rgbai+4]=c[j];
	buf[rgbai+6]=c[j];
	
	vis=glXChooseVisual(gui.dpy,DefaultScreen(gui.dpy),buf);
	if(vis!=NULL) {
	  sprintf(message,"found visual with rgba %d and depth %d",
		  buf[rgbai],buf[depthi]);
	  debmsg(message);
	  return vis;
	}
      }
    }
  }
  return NULL;
}

#endif
