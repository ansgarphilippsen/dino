#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dino.h"
#include "dbm.h"
#include "com.h"
#include "scal_db.h"
#include "scal_read.h"
#include "Cmalloc.h"

int scalRead(dbmScalNode *node, int type, FILE *f, int swap_flag)
{
  int ret;
  char message[256];
  node->swap_flag=swap_flag;

  switch(type) {
  case SCAL_READ_DINO:
    ret=scalReadDino(f,node);
    break;
  case SCAL_READ_CNS_ASCII:
  case SCAL_READ_XPLOR_ASCII:
    ret=xplorMapARead(f,node);
    break;
  case SCAL_READ_XPLOR_BINARY:
  case SCAL_READ_CNS_BINARY:
    ret=xplorMapBRead(f,node);
    break;
  case SCAL_READ_UHBD_ASCII:
    break;
  case SCAL_READ_UHBD_BINARY:
    ret=uhbdRead(f,node);
    break;
  case SCAL_READ_CHARMM_BINARY:
    ret=charmmReadB(f,node);
    break;
  case SCAL_READ_CCP4_BINARY:
    ret=ccp4Read(f,node);
    break;
  case SCAL_READ_MEAD:
    ret=meadRead(f,node);
    break;
  case SCAL_READ_DELPHI:
    ret=delphiRead(f,node);
    break;
  case SCAL_READ_DELPHIG:
    ret=delphi2Read(f,node);
    break;
  default:
    ret=-1;
  }


  if(ret==0) {

    sprintf(message," %dx%dx%d",
	    node->field->u_size,
	    node->field->v_size,
	    node->field->w_size);
    comMessage(message);

    node->field->edge=0.0;
    node->field->scale=1.0;

    scalSetMinMax(node);
  }


  return ret;
}



int scalReadDino(FILE *f, dbmScalNode *sn)
{
  int u,v,w;
  int sf=0;
  struct DINO_GRID_HEADER header;
  void *data;
  int i,size;
  int val;
  int *ip;
  float *fp;
  double fx,fy,fz;

  fread(&header,sizeof(header),1,f);

  if(header.magic!=31415) {
    // attempt to swap header
    swap_4bs((unsigned char *)&header,sizeof(header)/4);
    if(header.magic!=31415) {
      comMessage("\nfile is not in dino grid format");
      return -1;
    }
    sf=1;
  }

  if(header.type!=1 && header.type!=2) {
    comMessage("\ninvalid type in header");
    return -1;
  }

  size=header.nu*header.nv*header.nw;
  data=Cmalloc(size*4);
  if(data==NULL) {
    comMessage("\nmemory allocation error");
    return -1;
  }

  fread(data,size,4,f);

  if(sf)
    swap_4bs(data,size);

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  sn->field->size=size;

  sn->field->u_size=header.nu;
  sn->field->v_size=header.nv;
  sn->field->w_size=header.nw;
  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=header.nu-1;
  sn->field->v2=header.nv-1;
  sn->field->w2=header.nw-1;
  sn->field->wrap=0;
  sn->field->offset_x=header.offsetx;
  sn->field->offset_y=header.offsety;
  sn->field->offset_z=header.offsetz;
  sn->field->sigma=1.0;

  ip=(int *)data;
  fp=(float *)data;

  if(header.type==1) {
    for(i=0;i<size;i++)
      fp[i]=(float)ip[i];
  }
    
  sn->field->data=(float *)data;

  fx=1.0/(double)(header.nu);
  fy=1.0/(double)(header.nv);
  fz=1.0/(double)(header.nw);

  scalCELLtoVECT(sn->field,
		 header.deltax, header.deltay, header.deltaz,
		 header.alpha, header.beta, header.gamma,
		 1.0,1.0,1.0);

  return 0;
}




