#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include <png.h>
#include <tiffio.h>

#include "dino.h"
#include "write.h"
#include "com.h"
#include "Cmalloc.h"
#include "gui_ext.h"
#include "gfx.h"

extern struct GFX gfx;
extern int debug_mode;

static int write_png(struct WRITE_IMAGE *img,char *name);
static int write_tiff(struct WRITE_IMAGE *img,char *name);
static void do_jitter(int accum);
static int get_image(struct WRITE_IMAGE *img);

int writeFile(char *name, struct WRITE_PARAM *p)
{
  int ow,oh,cn;
  struct WRITE_IMAGE img;

  memcpy(&img.param,p,sizeof(struct WRITE_PARAM));

  if(img.param.dump && !img.param.accum) {
    debmsg("retrieving image");
    get_image(&img);
  } else {
    // create offscreen rendering context
    debmsg("creating offscreen rendering context");
    cn=guiCreateOffscreenContext(img.param.width,img.param.height,
				 img.param.accum);

    // initial gl
    debmsg("initializing GL");
    gfxGLInit();

    // draw into offscreen context
    debmsg("drawing into offscreen context");
    ow=gfx.win_width;
    oh=gfx.win_height;
    glDrawBuffer(GL_FRONT);
    gfx.win_width=img.param.width;
    gfx.win_height=img.param.height;
    gfxSetViewport();
    if(img.param.accum) {
      debmsg("doing accumulation series");
      do_jitter(img.param.accum);
    } else {
      gfxSetProjection(gfx.current_view);
      gfxSceneRedraw(1);
      comDBRedraw();
    }
    // get image
    debmsg("retrieving image");
    get_image(&img);

    // destry offscreen context
    debmsg("destroying offscreen context");
    guiDestroyOffscreenContext(cn);
    
    // restore old gfx settings
    debmsg("restoring settings");
    gfx.win_width=ow;
    gfx.win_height=oh;
    gfxSetViewport();
  }


  switch(img.param.type) {
  case WRITE_TYPE_PNG:
    debmsg("writing out image in png format");
    write_png(&img,name);
    break;
  case WRITE_TYPE_TIFF:
    debmsg("writing out image in tiff format");
    write_tiff(&img,name);
    break;
  }

  free(img.data);

  return 0;
}

static int get_image(struct WRITE_IMAGE *img)
{
  int w=img->param.width,h=img->param.height;
  img->data=malloc(w*h*3);
  glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,img->data);
  return 0;
}

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


static void do_jitter(int accum)
{
  int i,flag=0;
  float f, *jitter;
  char message[256];

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
  default: return;
  }
  f=1.0/(float)accum;
  for(i=0;i<accum;i++) {
    sprintf(message,"accumulation pass %d (%.5f %.5f)",
	    i+1,jitter[i*2+0], jitter[i*2+1]);
    debmsg(message);
    gfxSetAccProjection(gfx.current_view, jitter[i*2+0], jitter[i*2+1]);
    //gfxSetFog();
    gfxSceneRedraw(1);
    comDBRedraw();
    if(i==0)
      glAccum(GL_LOAD, f);	   
    else
      glAccum(GL_ACCUM, f);	   
  }
  glAccum(GL_RETURN, 1.0);
}


static int write_png(struct WRITE_IMAGE *image,char *name)
{
  long val;
  int w,h,k;
  unsigned long rm, bm, gm, rs, bs, gs;
  int rv,gv,bv;
  unsigned char red, green, blue;
  float r1,g1,b1,r2,g2,b2;
  char message[512];

  png_bytep *row_pointers;
  png_byte *row;
  

  png_structp png_ptr;
  png_infop info_ptr;
  FILE *fp;
  
  if((fp=fopen(name, "wb"))==NULL) {
    comMessage("error opening file \n");
    comMessage(name);
    return -1;
  }

  /* initialize the png stuff */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				    NULL, NULL, NULL);
  if(png_ptr==NULL) {
    fclose (fp);
    comMessage("error creating PNG write_struct\n");
    return -1;
  }
 
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("error creating PNG info_struct\n");
    return -1;
  }
   
  if (setjmp(png_ptr->jmpbuf)) {
    /* If we get here, we had a problem reading the file */
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("Internal PNG error\n");
    return -1;
  }
  
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, image->param.width, image->param.height, 8, 
	       PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  row_pointers=Ccalloc(image->param.height, sizeof(png_bytep));

  if(row_pointers==NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("error during memory allocation in pngWrite()\n");
    return -1;
  }

  /*
    the data returned by glReadPixels is 
    inverted in respect to the png format
  */
  for(k=0;k<image->param.height;k++)
    row_pointers[image->param.height-k-1]=&image->data[k*image->param.width*3];

  png_write_image(png_ptr, row_pointers);

  png_write_end(png_ptr, info_ptr);

  Cfree(row_pointers);
 
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
 
  fclose(fp);

  return 0;
}

static int write_tiff(struct WRITE_IMAGE *image,char *name)
{
  TIFF *tif;
  tdata_t buf;
  uint32 row;

  tif=TIFFOpen(name,"w");

  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, image->param.width);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, image->param.height);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, 1);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3); 

  for(row=0;row<image->param.height;row++) {
    buf=(tdata_t)&image->data[(image->param.height-row-1)*image->param.width*3];
    TIFFWriteScanline(tif,buf,row,0);
  }

  TIFFClose(tif);
}
