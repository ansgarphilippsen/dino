#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#include <png.h>

#include "write.h"
#include "com.h"
#include "Cmalloc.h"
#include "gui_ext.h"


static int write_png(struct WRITE_IMAGE *img,char *name);
static int get_image(struct WRITE_IMAGE *img);

int writeFile(char *name, struct WRITE_PARAM *p)
{
  struct WRITE_IMAGE img;

  memcpy(&img.param,p,sizeof(struct WRITE_PARAM));

  if(img.param.dump && !img.param.accum) {
    get_image(&img);
  }


  switch(img.param.type) {
  case WRITE_TYPE_PNG:
    write_png(&img,name);
    break;
  }

  return 0;

}

static int get_image(struct WRITE_IMAGE *img)
{
  int w=img->param.width,h=img->param.height;
  img->data=malloc(w*h*3);
  glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,img->data);
  return 0;
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
  png_byte *data,*row;
  

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
  data=Ccalloc(image->param.width*image->param.height*3,sizeof(png_byte));

  if(row_pointers==NULL || data==NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    comMessage("error during memory allocation in pngWrite()\n");
    return -1;
  }

  for(k=0;k<image->param.height;k++)
    row_pointers[k]=&image->data[k*image->param.width*3];

  png_write_image(png_ptr, row_pointers);

  png_write_end(png_ptr, info_ptr);

  Cfree(data);
  Cfree(row_pointers);
 
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
 
  fclose(fp);
     

  return 0;
}
