#include <stdio.h>
#include <string.h>

#include "dino.h"
#include "uhbd.h"
#include "dbm.h"
#include "scal_db.h"
#include "com.h"
#include "Cmalloc.h"

extern int debug_mode;

int uhbdRead(FILE *f, struct DBM_SCAL_NODE *sn, int cflag)
{
  int size;
  char dummy[4];
  struct UHBD_GRID_HEADER header;
  int uv_size;
  float *uv;
  float uv_dummy[3];
  int i,j,k;
  char dmesg[256];
  float sfactor=1.0;

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  if(sn->field==NULL) {
    comMessage("uhbdRead: memory allocation error\n");
    return -1;
  }

  memset(&header,0,sizeof(struct UHBD_GRID_HEADER));

  fread(dummy, sizeof(dummy),1,f);
  fread(&header, sizeof(struct UHBD_GRID_HEADER), 1, f);

  if(abs(header.im)>1e6 || 
     abs(header.jm)>1e6 ||
     abs(header.km)>1e6) {
    sn->swap_flag=1;
    swap_4bs((unsigned char*)&header, sizeof(header)/4.0);
    if(abs(header.im)>1e6 || 
       abs(header.jm)>1e6 ||
       abs(header.km)>1e6) {
      
      comMessage("error: uhbdRead: header not recognized (even after byte-swap)\n");
      return -1;
    } else {
      comMessage(" (byte-swapping) ");
    }
  }


  fread(dummy, sizeof(dummy),1,f);

  debmsg("uhbdRead:");
  if(debug_mode) {
    fprintf(stderr,"title: %s\nscale: %g\nh: %g\no: %g %g %g\ns: %d %d %d\n\n",
	    header.title,header.scale,header.h,
	    header.ox,header.oy,header.oz,
	    header.im, header.jm, header.km);
  }


  if(cflag) {
    if(header.scale==0.0) {
      comMessage("warning: scale value in header ist 0.0, unit conversion not done\n");
      sfactor=1.0;
    } else {
      debmsg("uhbdRead: applying header scale factor");
      sfactor=1.0/header.scale;
    }
  } else {
    sfactor=1.0;
  }
  
  size=header.im*header.jm*header.km;


  sn->field->size=size;

  sprintf(dmesg,"uhbdRead: allocating %d kb for data",size/1024);
  debmsg(dmesg);
  sn->field->data=Ccalloc(size, sizeof(float));
  if(sn->field->data==NULL) {
    comMessage("error: uhbdRead: memory allocation error\n");
    return -1;
  }

  uv_size=header.jm*header.im;
  sprintf(dmesg,"uhbdRead: allocating %d kb tmp",uv_size/1024);
  debmsg(dmesg);
  uv=Ccalloc(uv_size, sizeof(float));
  if(uv==NULL) {
    comMessage("error: uhbdRead: memory allocation error\n");
    return -1;
  }

  
  debmsg("uhbdRead: filling struct");
  sn->field->u_size=(int)header.im;
  sn->field->v_size=(int)header.jm;
  sn->field->w_size=(int)header.km;
  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=sn->field->u1+sn->field->u_size-1;
  sn->field->v2=sn->field->v1+sn->field->v_size-1;
  sn->field->w2=sn->field->w1+sn->field->w_size-1;
  sn->field->wrap=0;
  sn->field->offset_x=(double)header.ox+header.h;
  sn->field->offset_y=(double)header.oy+header.h;
  sn->field->offset_z=(double)header.oz+header.h;
  sn->field->sigma=1.0;
  sn->field->scale=1.0;

  sprintf(dmesg,"uhbdRead: center: %g %g %g",
	  sn->field->offset_x,
	  sn->field->offset_y,
	  sn->field->offset_z);
  debmsg(dmesg);

  for(k=0;k<header.km;k++) {
    sprintf(dmesg,"uhbdRead: reading section %d",k);
    debmsg(dmesg);
    fread(dummy, sizeof(dummy),1,f);
    fread(uv_dummy, sizeof(uv_dummy), 1,f);
    fread(dummy, sizeof(dummy),1,f);
    fread(dummy, sizeof(dummy),1,f);
    fread(uv,uv_size*sizeof(float),1,f);
    if(sn->swap_flag) {
      swap_float(uv,uv_size);
    }
    fread(dummy, sizeof(dummy),1,f);

    for(i=0;i<header.im;i++) 
      for(j=0;j<header.jm;j++)
	scalWriteField(sn->field,i,j,k,uv[j*header.im+i]*sfactor);
    
  }
  Cfree(uv);

  debmsg("uhbdRead: calculating transformation matrix");
  scalCELLtoVECT(sn->field,
		 1.0,1.0,1.0,
		 90.0, 90.0, 90.0,
		 header.h, header.h, header.h);
  
  return 0;
}
