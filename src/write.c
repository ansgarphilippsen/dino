#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#ifdef WRITE_FORK
#include <unistd.h>
#endif

#include <png.h>

#ifdef X11_GUI
#include <X11/Xlib.h>
#endif

#include "dino.h"
#include "gui.h"
#include "com.h"
#include "dbm.h"
#include "gfx.h"
#include "glf.h"
#include "scene.h"
#include "struct_db.h"
#include "scal_db.h"
#include "surf_db.h"
#include "write.h"
#include "mat.h"
#include "Cmalloc.h"
#include "cgfx.h"
#include "glw.h"

#ifdef FORMAT_RGB
#include "ximage_util.h"
#endif

#ifdef IMAGICK
#include "imagick.h"
#endif

extern struct DBM dbm;
extern struct GFX gfx;
extern struct GUI gui;
extern struct GLF glf;
extern int debug_mode;

/********************
 THESE ARE OBSOLETE

static int write_buf1[]={GLX_RGBA, 
			 GLX_DEPTH_SIZE,12,
			 GLX_RED_SIZE,8,
			 GLX_GREEN_SIZE,8,
			 GLX_BLUE_SIZE,8,
			 GLX_ALPHA_SIZE,8,
			 GLX_ACCUM_RED_SIZE, 8, 
			 GLX_ACCUM_GREEN_SIZE, 8, 
			 GLX_ACCUM_BLUE_SIZE, 8, 
			 None};

static int write_buf2[]={GLX_RGBA, 
			 GLX_DEPTH_SIZE,6,
			 GLX_RED_SIZE,4,
			 GLX_GREEN_SIZE,4,
			 GLX_BLUE_SIZE,4,
			 GLX_ALPHA_SIZE,4,
			 GLX_ACCUM_RED_SIZE, 1, 
			 GLX_ACCUM_GREEN_SIZE, 1, 
			 GLX_ACCUM_BLUE_SIZE, 1, 
			 None};
static int write_buf3[]={GLX_RGBA, 
			 GLX_ALPHA_SIZE,4,
			 GLX_ACCUM_RED_SIZE, 1, 
			 GLX_ACCUM_GREEN_SIZE, 1, 
			 GLX_ACCUM_BLUE_SIZE, 1, 
			 None};

static int write_buf4[]={GLX_RGBA, 
			 GLX_DEPTH_SIZE,4,
			 GLX_RED_SIZE,8,
			 GLX_GREEN_SIZE,8,
			 GLX_BLUE_SIZE,8,
			 GLX_ALPHA_SIZE,4,
			 GLX_ACCUM_RED_SIZE, 4, 
			 GLX_ACCUM_GREEN_SIZE, 4, 
			 GLX_ACCUM_BLUE_SIZE, 4, 
			 None};

static int write_buf5[]={GLX_RGBA,
			 GLX_RED_SIZE,2,
			 GLX_GREEN_SIZE,2,
			 GLX_BLUE_SIZE,2,
			 None};

static int write_buf6[]={GLX_RGBA, 
			 GLX_DOUBLEBUFFER,
			 GLX_ALPHA_SIZE,4,
			 GLX_ACCUM_RED_SIZE, 1, 
			 GLX_ACCUM_GREEN_SIZE, 1, 
			 GLX_ACCUM_BLUE_SIZE, 1, 
			 None};
*****************/

float jitter2[][2]={{0.25, 0.75}, {0.75, 0.25}}; 
float jitter3[][2]={{0.5033922635, 0.8317967229}, 
		    {0.7806016275, 0.2504380877}, 
		    {0.2261828938, 0.4131553612}}; 
float jitter4[][2]={{0.375, 0.25}, {0.125, 0.75},
		    {0.875, 0.25}, {0.625, 0.75}};
float jitter5[][2]= {{0.5, 0.5}, {0.3, 0.1}, {0.7, 0.9},
		     {0.9, 0.3}, {0.1, 0.7}};
float jitter6[][2]={{0.4646464646, 0.4646464646},
		    {0.1313131313, 0.7979797979},
		    {0.5353535353, 0.8686868686},
		    {0.8686868686, 0.5353535353}, 
		    {0.7979797979, 0.1313131313},
		    {0.2020202020, 0.2020202020}};
float jitter8[][2]={{0.5625, 0.4375}, {0.0625, 0.9375},
		    {0.3125, 0.6875}, {0.6875, 0.8125}, 
		    {0.8125, 0.1875}, {0.9375, 0.5625},
		    {0.4375, 0.0625}, {0.1875, 0.3125}};
float jitter9[][2]={{0.5, 0.5}, {0.1666666666, 0.9444444444},
		    {0.5, 0.1666666666}, {0.5, 0.8333333333},
		    {0.1666666666, 0.2777777777},
		    {0.8333333333, 0.3888888888},
		    {0.1666666666, 0.6111111111}, 
		    {0.8333333333, 0.7222222222},
		    {0.8333333333, 0.0555555555}}; 
float jitter12[][2]={{0.4166666666, 0.625}, {0.9166666666, 0.875}, 
		     {0.25, 0.375}, {0.4166666666, 0.125},
		     {0.75, 0.125}, {0.0833333333, 0.125}, 
		     {0.75, 0.625}, {0.25, 0.875}, 
		     {0.5833333333, 0.375}, {0.9166666666, 0.375}, 
		     {0.0833333333, 0.625}, {0.583333333, 0.875}};
float jitter16[][2]={ {0.375, 0.4375}, {0.625, 0.0625}, 
		      {0.875, 0.1875}, {0.125, 0.0625}, 
		      {0.375, 0.6875}, {0.875, 0.4375},
		      {0.625, 0.5625}, {0.375, 0.9375}, 
		      {0.625, 0.3125}, {0.125, 0.5625}, 
		      {0.125, 0.8125}, {0.375, 0.1875}, 
		      {0.875, 0.9375}, {0.875, 0.6875}, 
		      {0.125, 0.3125}, {0.625, 0.8125}};

#ifdef X11_GUI
XVisualInfo * writeGetVis(int accum_flag)
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

#ifdef FORMAT_RGB
static int writeImage2RGB(XImage *image,char *name)
{
  /* branch to ximage_util code */
  xu_writeimage(image,name);
  return 0;
}
#endif

static int writeRedraw(int mode, int accum, int flag)
{
  float *jitter,f;
  int i;

  glDrawBuffer(GL_FRONT);
  glReadBuffer(GL_FRONT);

  if(accum==1) {
    if(mode==WRITE_RIGHT_VIEW) {
      gfxSetProjection(GFX_RIGHT);
      gfxSetFog();
      gfxSceneRedraw(flag);
    } else if(mode==WRITE_LEFT_VIEW) {
      gfxSetProjection(GFX_LEFT);
      gfxSetFog();
      gfxSceneRedraw(flag);
    } else {
      gfxSetProjection(GFX_CENTER);
      gfxSetFog();
      gfxSceneRedraw(flag);
    }
    comDBRedraw();
  } else {

    //    glClear(GL_ACCUM_BUFFER_BIT);

    switch(accum) {
    case 2: jitter=(float *)jitter2; break;
    case 3: jitter=(float *)jitter3; break;
    case 4: jitter=(float *)jitter4; break;
    case 5: jitter=(float *)jitter5; break;
    case 6: jitter=(float *)jitter6; break;
    case 8: jitter=(float *)jitter8; break;
    case 9: jitter=(float *)jitter9; break;
    case 12: jitter=(float *)jitter12; break;
    case 16: jitter=(float *)jitter16; break;
    default: comMessage("\nInternal Error"); return -1;
    }
    f=1.0/(float)accum;
    for(i=0;i<accum;i++) {
      gfxSetAccProjection(gfx.stereo_view, jitter[i*2+0], jitter[i*2+1]);
      gfxSetFog();
      gfxSceneRedraw(flag);
      comDBRedraw();
      if(i==0)
	glAccum(GL_LOAD, f);	   
      else
	glAccum(GL_ACCUM, f);	   
    }
    glAccum(GL_RETURN, 1.0);
  }
  
  glFinish();
  
  return 0;
}

#ifdef FORMAT_TIFF
static int writeImage2Tiff(XImage *ximage,char *name)
{
  TIFF *t;
  tdata_t buf;
  short *r,*g,*b;
  int x,y;
  char message[256];
  uint8 *pp;
  
  int index;
  int xsize, ysize;
  int rshift = 0, gshift = 0, bshift = 0;
  unsigned long rmask, gmask, bmask;
  float rf,gf,bf;
  
  xsize = ximage->width;
  ysize = ximage->height;

  sprintf(message," %dx%d",xsize,ysize);
  comMessage(message);

  if((t=TIFFOpen(name,"w"))==NULL) {
    sprintf(message,"\nerror opening %s",name);
    return -1;
  }

  TIFFSetField(t, TIFFTAG_IMAGEWIDTH, (uint32) xsize);
  TIFFSetField(t, TIFFTAG_IMAGELENGTH, (uint32) ysize);
  TIFFSetField(t, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(t, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(t, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(t, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(t, TIFFTAG_ORIENTATION, ORIENTATION_BOTLEFT);
  TIFFSetField(t, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(t, TIFFTAG_ROWSPERSTRIP,
	       TIFFDefaultStripSize(t, (uint32) -1));
  buf=_TIFFmalloc(TIFFScanlineSize(t));
  if(buf==NULL) {
    comMessage("\nmemory allocation error");
    return -1;
  }  

  r=(short *)_TIFFmalloc((long)(3*xsize*sizeof(short)));
  g=r+xsize;
  b=g+xsize;
  if(r==NULL) {
    comMessage("\nmemory allocation error");
    return -1;
  }  


  if ((rmask = ximage->red_mask)){
    while (!(rmask & 1)) {
      rmask >>= 1;
      rshift++;
    }
  }
  
  if ((gmask = ximage->green_mask)){
    while (!(gmask & 1)) {
      gmask >>= 1;
      gshift++;
    }
  }
  
  if ((bmask = ximage->blue_mask)){
    while (!(bmask & 1)) {
      bmask >>= 1;
      bshift++;
    }
  }
  
  rf=(float)0xff/(float)rmask;
  gf=(float)0xff/(float)gmask;
  bf=(float)0xff/(float)bmask;
  
  for(y=ysize-1; y>=0; y--) {
    pp=(uint8*) buf;
    for(x=0; x<xsize; x++) {
      index = XGetPixel(ximage,x,ysize-y-1);
      
      r[x] = (short)(rf * (float)((index>>rshift)&rmask));
      g[x] = (short)(gf * (float)((index>>gshift)&gmask));
      b[x] = (short)(bf * (float)((index>>bshift)&bmask));

      pp[0]=r[x];
      pp[1]=g[x];
      pp[2]=b[x];
      pp+=3;
    }
    if(TIFFWriteScanline(t,buf,ysize-y-1,0)<0) {
      break;
    }
      
  }
  _TIFFfree(r);
  _TIFFfree(buf);

  TIFFClose(t);
  return 0;
}
#endif

static int writeImage2PNG(XImage *image,char *name)
{
  long val;
  int w,h,k;
  unsigned long rm, bm, gm, rs, bs, gs;
  int rv,gv,bv;
  unsigned char red, green, blue;
  float r1,g1,b1,r2,g2,b2;
  char message[512];

  png_bytep *row_pointers;
  png_byte *data,*row;
  

  png_structp png_ptr;
  png_infop info_ptr;
  FILE *fp;
  
  if((fp=fopen(name, "wb"))==NULL) {
    comMessage("\nerror opening file ");
    comMessage(name);
    return -1;
  }

  /* initialize the png stuff */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				    NULL, NULL, NULL);
  if(png_ptr==NULL) {
    fclose (fp);
    comMessage("\nerror creating PNG write_struct");
    return -1;
  }
 
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("\nerror creating PNG info_struct");
    return -1;
  }
   
  if (setjmp(png_ptr->jmpbuf)) {
    /* If we get here, we had a problem reading the file */
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("\nInternal PNG error");
    return -1;
  }
  
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, image->width, image->height, 8, 
	       PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  row_pointers=Ccalloc(image->height, sizeof(png_bytep));
  data=Ccalloc(image->width*image->height*3,sizeof(png_byte));

  if(row_pointers==NULL || data==NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("\nerror during memory allocation in pngWrite()");
    return -1;
  }

  for(k=0;k<image->height;k++)
    row_pointers[k]=data+k*3*image->width;

  /*
  image->red_mask=0xff0000;
  image->green_mask=0xff00;
  image->blue_mask=0xff;
  */

  /* initialize the conversion stuff */
  rm=image->red_mask;
  bm=image->blue_mask;
  gm=image->green_mask;

//  fprintf(stderr,"\n%08lx %08lx %08lx",rm,gm,bm);

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

  sprintf(message," %dx%d",image->width, image->height);
  comMessage(message);


  for(h=0;h<image->height;h++) {
    row=row_pointers[h];
  //  fprintf(stderr,"\n");
    for(w=0;w<image->width;w++) {
      val=XGetPixel(image,w,h);
      rv=(int)((val & rm)>> rs);
      gv=(int)((val & gm)>> gs);
      bv=(int)((val & bm)>> bs);
    //  fprintf(stderr,"%06lx %02lx%02lx%02lx ",val,rv,gv,bv);
      r2=r1*(float)rv;
      g2=g1*(float)gv;
      b2=b1*(float)bv;
      red=(unsigned char)r2;
      green=(unsigned char)g2;
      blue=(unsigned char)b2;
      (*row++)=red;
      (*row++)=green;
      (*row++)=blue;
    }
  }  

  png_write_image(png_ptr, row_pointers);

  png_write_end(png_ptr, info_ptr);

  Cfree(data);
  Cfree(row_pointers);
 
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
 
  fclose(fp);
     

  return 0;
}
// X11_GUI
#endif


/*
  externally visible functions below
*/

int writeFile(char *name, int type, int accum, float scale, int dump)
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
#ifdef WRITE_FORK
  pid_t child_id;
#endif
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
	    cgfxSphere(1.0,dbm.node[i].structNode.obj[j].render.detail);
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
