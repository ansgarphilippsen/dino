#ifdef LINUX
#include <tiff.h>
#include <tiffio.h>
#endif
#ifdef SGI
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif
#ifdef DEC
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif

#ifdef SUN
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif

#include "com.h"
#include "grid_db.h"
#include "Cmalloc.h"

int tiffRead(int fd, char *fn, dbmGridNode *node)
{
  TIFF *t;
  uint32 w,h;
  uint16 b;
  uint32 *data;
  int i,j,p1,p2;
  char message[256];
  char emsg[1024];
  float scale;
  TIFFRGBAImage ti;

  t=TIFFFdOpen(fd,fn,"r");
  if(t==NULL) {
    sprintf(message,"\nreadGrid: error opening file");
    comMessage(message);
    return -1;
  }
  TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&w);
  TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&h);
  TIFFGetField(t,TIFFTAG_BITSPERSAMPLE,&b);

  if(b==8) {
    scale=256.0;

    /*  } else if(b==16) {
	scale=65536.0;*/
  } else {
    sprintf(message,"\nreadGrid: BITSPERSAMPLE is %d and this is not supported",b);
    comMessage(message);
    TIFFClose(t);
    return -1;
  }

  if((w*h)>(4096*4096)) {
    sprintf(message,"\nsize exceeded or invalid file format");
    comMessage(message);
    TIFFClose(t);
    return -1;
  }

  if((data=Ccalloc(w*h,sizeof(uint32)))==NULL) {
    sprintf(message,"\nMemory allocation error #1");
    comMessage(message);
    return -1;
  }

  //  TIFFReadRGBAImage(t,w,h,data,-1);   
  if(!TIFFRGBAImageOK(t, emsg)) {
    comMessage(emsg);
    TIFFClose(t);
    return -1;
  }
  if(!TIFFRGBAImageBegin(&ti, t, 1, emsg)) {
    comMessage(emsg);
    TIFFClose(t);
    return -1;
  }
  TIFFRGBAImageGet(&ti, data, w, h);
  TIFFRGBAImageEnd(&ti);

  node->field.width=(int)w; 
  node->field.height=(int)h;
  node->field.scale_x=10.0/(float)w;
  node->field.scale_y=10.0/(float)h;
  node->field.scale_z=1.0/scale;
  node->field.offset_x=(float)w/2.0;
  node->field.offset_y=(float)h/2.0;
  node->field.offset_z=0;
  node->field.point_count=node->field.width*node->field.height;

  node->field.point=Ccalloc((w)*(h),sizeof(gridPoint));

  if(node->field.point==NULL) {
    sprintf(message,"\nMemory allocation error #2");
    comMessage(message);
    Cfree(data);
    TIFFClose(t);
    return -1;
  }
  /*
    transfer red value to field data
  */
  for(j=0;j<h;j++) {
    for(i=0;i<w;i++) {
      p1=j*(int)w+i;
      p2=(j+1)*((int)w+2)+(i+1);
      node->field.point[p1].z=(int)TIFFGetR(data[p1]);
    }
  }

  for(i=0;i<node->field.width;i++)
    for(j=0;j<node->field.height;j++) {
      p2=j*(int)w+i;
      node->field.point[p2].x=i;
      node->field.point[p2].y=j;
      node->field.point[p2].n=p2;
      node->field.point[p2].attach_flag=0;
      node->field.point[p2].restrict=0;
      node->field.point[p2].attach_node=NULL;
    }

  Cfree(data);
  
  TIFFClose(t);

  sprintf(message," %ldx%ld pixels",w,h);
  comMessage(message);

  return 0;
}

int tiffReadTex(char *fn, gridTexture *tex)
{
  TIFF *t;
  uint32 w,h;
  uint16 b;
  uint32 *data;
  int i,j,p1,p2;
  char message[256];
  grid_t *dp;

  t=TIFFOpen(fn,"r");

  if(t==NULL) {
    sprintf(message,"\nreadTex: error opening file");
    comMessage(message);
    return -1;
  }
  TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&w);
  TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&h);
  TIFFGetField(t,TIFFTAG_BITSPERSAMPLE,&b);

  if((w*h)>(4096*4096)) {
    sprintf(message,"\nsize exceeded or invalid file format");
    comMessage(message);
    return -1;
  }

  if((data=Ccalloc(w*h,sizeof(uint32)))==NULL) {
    sprintf(message,"\nMemory allocation error #1");
    comMessage(message);
    return -1;
  }

  TIFFReadRGBAImage(t,w,h,data,-1);   

  tex->width=(int)w; 
  tex->height=(int)h;
  tex->data=Ccalloc(w*h*4,sizeof(grid_t));
  dp=tex->data;
  for(j=0;j<h;j++) {
    for(i=0;i<w;i++) {
      p1=j*(int)w+i;
      dp[0]=TIFFGetR(data[p1])>>1;
      dp[1]=TIFFGetG(data[p1])>>1;
      dp[2]=TIFFGetB(data[p1])>>1;
      dp[3]=TIFFGetA(data[p1])>>1;
      dp+=4;
    }
  }

  Cfree(data);

  TIFFClose(t);

  sprintf(message," %ldx%ld pixels",w,h);
  comMessage(message);

  return 0;
}
