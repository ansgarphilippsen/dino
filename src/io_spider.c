#include <string.h>

#include "dino.h"
#include "ccp4.h"
#include "dbm.h"
#include "mat.h"
#include "xtal.h"
#include "Cmalloc.h"
#include "com.h"

#include "io_spider.h"

extern int debug_mode;

static void dump_header(struct SPIDER_MAP_HEADER *h)
{
  fprintf(stderr,"[DEBUG]SPIDER HEADER DUMP\n");
  fprintf(stderr,"nslice: %f\n",h->fNslice);
  fprintf(stderr,"nrow: %f\n",h->fNrow);
  fprintf(stderr,"ncol: %f\n",h->fNcol);
  fprintf(stderr,"labrec: %f\n",h->fLabrec);
  fprintf(stderr,"iform: %f\n",h->fIform);
  fprintf(stderr,"xoff: %f\n",h->fXoff);
  fprintf(stderr,"yoff: %f\n",h->fYoff);
  fprintf(stderr,"zoff: %f\n",h->fZoff);
  fprintf(stderr,"scale: %f\n",h->fScale);
}

int spiderRead(FILE *f, dbmScalNode *sn)
{
  struct SPIDER_MAP_HEADER header;
  int size,ret,hlen;
  int u,v,w,pointer,size2;
  void *raw_data;
  float *raw_data_float;
  int *raw_data_byte;
  char message[256];
  int mode;

  // check if swap is required
  fread(&header,sizeof(struct SPIDER_MAP_HEADER),1,f);

  if(header.fIform<-10.0 || header.fIform>10.0 || (int)header.fIform==0) {
    // try swapping
    sn->swap_flag=1;
    swap_4bs((unsigned char*)&header, sizeof(header)/4.0);
    if(header.fIform<-10 || header.fIform>10.0  || (int)header.fIform==0) {
      comMessage("spiderRead: failed (even after byte swap)\n");
      return -1;
    } else {
      comMessage("(byte-swapping)\n");
    }
  }

  if(debug_mode) dump_header(&header);

  if(header.fIform==3.0) {
    mode=1;
  } else if(header.fIform==10.0) {
    mode=2;
  } else {
    comMessage("spiderRead: can only read modes 3 (3D float) or 10 (3D int)\n");
    return -1;
  }

  if(header.fScale<=0.0) {
    header.fScale=1.0;
    comMessage("spiderRead: invalid scale parameter set to 1.0\n");
  }

  // size to skip forward to
  size = (int)(header.fNcol * header.fLabrec * 4); //Size of whole header
  size -= sizeof(struct SPIDER_MAP_HEADER);       //Decrease the real header  

  fseek(f,size,SEEK_CUR);

  size=(int)header.fNslice*(int)header.fNrow*(int)header.fNcol;
  size2=(int)header.fNrow*(int)header.fNcol;

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  sn->field->size=size;

  if(sn->field==NULL) {
    comMessage("spiderRead: memory allocation error\n");
    return -1;
  }

  //allocate memory for actual data
  sn->field->data=Ccalloc(size,sizeof(float));

  // allocate temp memory
  if(mode==1) {
    raw_data=Ccalloc(size2,sizeof(float));
  } else {
    raw_data=Ccalloc(size2,sizeof(int));
  }

  if(sn->field==NULL || raw_data==NULL) {
    comMessage("spiderRead: memory allocation error\n");
    Cfree(sn->field);
    Cfree(raw_data);
    return -1;
  }

  sn->field->u_size=(int)(header.fNcol);
  sn->field->v_size=(int)(header.fNrow);
  sn->field->w_size=(int)(header.fNslice);
  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=sn->field->u1+sn->field->u_size-1;
  sn->field->v2=sn->field->v1+sn->field->v_size-1;
  sn->field->w2=sn->field->w1+sn->field->w_size-1;
  sn->field->wrap=0;
  sn->field->offset_x=(double)header.fXoff;
  sn->field->offset_y=(double)header.fYoff;
  sn->field->offset_z=(double)header.fZoff;
  sn->field->sigma=1.0;
  sn->field->scale=1.0;
  //sn->field->scale=(double)header.fScale;
  //if(sn->field->scale<=0.0) sn->field->scale=1.0;

  memset(sn->field->data,0,size);

  for(w=0;w<(int)header.fNslice;w++) {

    // read into raw data slice per slice
    if(mode==1) {
      ret=fread(raw_data,sizeof(float),size2,f);
      raw_data_float=(float *)raw_data;
    } else {
      ret=fread(raw_data,sizeof(int),size2,f);
      raw_data_byte=(int *)raw_data;
    }  
    
    if(sn->swap_flag) {
      swap_4bs(raw_data,size2);
    }

    for(v=0;v<(int)header.fNrow;v++) {
      for(u=0;u<(int)header.fNcol;u++) {
	pointer=(int)header.fNcol*v+u;
	if(mode==1)
	  scalWriteField(sn->field,u,v,w,(float)raw_data_float[pointer]);
	else
	  scalWriteField(sn->field,u,v,w,(float)raw_data_byte[pointer]);
      }
    }
  }
  Cfree(raw_data);

  sprintf(message,
	  "u_size, v_size, w_size: %d,%d,%d\n\n",
	  sn->field->u_size,sn->field->v_size, sn->field->w_size);
  debmsg(message);


  scalCELLtoVECT(sn->field,
		 1.0,1.0,1.0,
		 90.0, 90.0, 90.0,
		 header.fScale, header.fScale, header.fScale);

  return 0;
}
