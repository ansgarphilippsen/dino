#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "dino.h"
#include "ccp4.h"
#include "dbm.h"
#include "mat.h"
#include "xtal.h"
#include "Cmalloc.h"
#include "com.h"

extern int debug_mode;

int ccp4Read(FILE *f, dbmScalNode *sn)
{
  struct CCP4_MAP_HEADER header;
  int size,ret;
   double fx,fy,fz;
  int c1,r1,s1,c2,r2,s2,uvw[3],pc,pr,ps;
  int c,r,s,u,v,w,pointer,pointer_s,pointer_r,size2;
  void *raw_data;
  float *raw_data_float;
  unsigned char *raw_data_byte;
  char message[256];

  // check if swap is required
  fread(&header,sizeof(struct CCP4_MAP_HEADER),1,f);

  if(header.mode!=2 && header.mode!=0) {
    // try swapping
    sn->swap_flag=1;
    swap_4bs((unsigned char*)&header, sizeof(header)/4.0);
    if(header.mode!=2 && header.mode!=0) {
      comMessage("\nccp4Read: error: map not in mode 0 or 2 (even after byte-swap");
      return -1;
    } else {
      comMessage(" (byte-swapping) ");
    }
  }

  fseek(f,header.nsymbt,SEEK_CUR);

  /*
    PATCH for EM based CCP4 maps
    26 jan 99 
  */
  if(header.arms==0.0)
    header.arms=1.0;
  if(header.alpha==0.0)
    header.alpha=90.0;
  if(header.beta==0.0)
    header.beta=90.0;
  if(header.gamma==0.0)
    header.gamma=90.0;
  if(header.x==0.0)
    header.x=(float)header.nc;
  if(header.y==0.0)
    header.y=(float)header.nr;
  if(header.z==0.0)
    header.z=(float)header.ns;
  if(header.nx==0.0)
    header.nx=header.nc;
  if(header.ny==0.0)
    header.ny=header.nr;
  if(header.nz==0.0)
    header.nz=header.ns;


  /*
    nx,ny and nz are only needed for the 
    scaling of the map to cartesian space
    and not for the size of the map
  */
  /*
  size=header.nx*header.ny*header.nz;
  */
  size2=header.nc*header.nr*header.ns;
  size=size2;

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  sn->field->size=size;

  if(sn->field==NULL) {
    comMessage("\nccp4Read: memory allocation error");
    return -1;
  }

  sn->field->data=Ccalloc(size,sizeof(float));

  if(header.mode==2) {
    raw_data=Ccalloc(size2,sizeof(float));
  } else {
    raw_data=Ccalloc(size2,sizeof(int));
  }

  if(sn->field==NULL || raw_data==NULL) {
    comMessage("\nccp4Read: memory allocation error");
    Cfree(sn->field);
    Cfree(raw_data);
    return -1;
  }


  if(header.mode==2) {
    ret=fread(raw_data,sizeof(float),size2,f);
    raw_data_float=(float *)raw_data;
  } else {
    ret=fread(raw_data,sizeof(unsigned char),size2,f);
    raw_data_byte=(unsigned char *)raw_data;
  }  

  if(sn->swap_flag) {
    swap_4bs(raw_data,size2);
  }

  c1=header.ncstart;
  c2=header.ncstart+header.nc;
  r1=header.nrstart;
  r2=header.nrstart+header.nr;
  s1=header.nsstart;
  s2=header.nsstart+header.ns;

  pc=header.mapc-1;
  pr=header.mapr-1;
  ps=header.maps-1;


  memset(sn->field->data,0,size);

  uvw[pc]=header.nc;
  uvw[pr]=header.nr;
  uvw[ps]=header.ns;
  sn->field->u_size=uvw[0];
  sn->field->v_size=uvw[1];
  sn->field->w_size=uvw[2];
  uvw[pc]=header.ncstart;
  uvw[pr]=header.nrstart;
  uvw[ps]=header.nsstart;
  sn->field->u1=uvw[0];
  sn->field->v1=uvw[1];
  sn->field->w1=uvw[2];
  sn->field->u2=sn->field->u1+sn->field->u_size-1;
  sn->field->v2=sn->field->v1+sn->field->v_size-1;
  sn->field->w2=sn->field->w1+sn->field->w_size-1;
  /* WRAP MODE */
  sn->field->wrap=0;
  sn->field->offset_x=0.0;
  sn->field->offset_y=0.0;
  sn->field->offset_z=0.0;
  sn->field->sigma=header.arms;
  sn->field->scale=1.0;

  sn->def_level=sn->field->sigma;

  for(s=s1;s<s2;s++) {
    pointer_s=header.nc*header.nr*(s-s1);
    for(r=r1;r<r2;r++) {
      pointer_r=pointer_s+header.nc*(r-r1);
      for(c=c1;c<c2;c++) {
	
	pointer=pointer_r+(c-c1);

	uvw[pc]=c;
	uvw[pr]=r;
	uvw[ps]=s;
	
	u=uvw[0];
	v=uvw[1];
	w=uvw[2];

	if(header.mode==2)
	  scalWriteField(sn->field,u,v,w,(float)raw_data_float[pointer]);
	else
	  scalWriteField(sn->field,u,v,w,(float)raw_data_byte[pointer]);
      }
    }
  }
  Cfree(raw_data);


  sprintf(message,
	  "\nheader:\nnc,nr,ns: %ld,%ld,%ld\nncstart,nrstart,nsstart: %ld,%ld,%ld\nnx,ny,nz: %ld,%ld,%ld\nx,y,z: %.3f,%.3f,%.3f\nmapc,mapr,maps: %ld,%ld,%ld",
	  header.nc, header.nr, header.ns,
	  header.ncstart, header.nrstart, header.nsstart,
	  header.nx, header.ny, header.nz, 
	  header.x, header.y, header.z,
	  header.mapc, header.mapr, header.maps);
  debmsg(message);
  sprintf(message,
	  "\nu_size, v_size, w_size: %d,%d,%d\n",
	  sn->field->u_size,sn->field->v_size, sn->field->w_size);
  debmsg(message);

  fx=1.0/(double)(header.nx);
  fy=1.0/(double)(header.ny);
  fz=1.0/(double)(header.nz);

  scalCELLtoVECT(sn->field,
		 header.x, header.y, header.z,
		 header.alpha, header.beta, header.gamma,
		 fx,fy,fz);

  sn->xtal=Cmalloc(sizeof(struct XTAL));
  sn->xtal->a=header.x;
  sn->xtal->b=header.y;
  sn->xtal->c=header.z;
  sn->xtal->alpha=header.alpha;
  sn->xtal->beta=header.beta;
  sn->xtal->gamma=header.gamma;

  sn->xtal->space_group=(int)header.ispg;
  dbmCalcXtal(sn->xtal);
  sn->show_cell=0;


  return 0;
}







