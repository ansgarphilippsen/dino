#include "dino.h"
#include "delphi.h"
#include "Cmalloc.h"
#include "com.h"

extern int debug_mode;

int delphiRead(FILE *f, dbmScalNode *sn)
{
  char message[256];
  char title[132];
  unsigned char dummy[4];
  float x1,x2,y1,y2,z1,z2,dx,dy,dz;
  int u,v,w;
  float *data;

  struct DELPHI_HEADER header;

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  if(sn->field==NULL) {
    comMessage("delphiRead: memory allocation error\n");
    return -1;
  }

  fread(dummy, sizeof(dummy),1,f);
  fread(title, sizeof(title), 1, f);
  fread(dummy, sizeof(dummy),1,f);
  fread(dummy, sizeof(dummy),1,f);
  fread(&header,sizeof(struct DELPHI_HEADER),1,f);
  fread(dummy, sizeof(dummy),1,f);

  if(sn->swap_flag) {
  }

  if(header.nbyte!=4 || header.ivary!=0 || header.intdat!=0 ||
     header.xang!=90.0 || header.yang!=90.0 || header.zang!=90.0) {
    sn->swap_flag=1;
    swap_4bs((unsigned char *)&header,18);
    if(header.nbyte!=4 || header.ivary!=0 || header.intdat!=0 ||
       header.xang!=90.0 || header.yang!=90.0 || header.zang!=90.0) {
      comMessage("error: invalid DELPHI header (even after byte-swap)\n");
      return -1;
    } else {
      comMessage(" byte-swapping ");
    }
  }
    
  sprintf(message,"DELPHI HEADER:");
  debmsg(message);
  title[131]='\0';
  sprintf(message,"%s\n",title);
  //  comMessage(message);
  sprintf(message,"ivary:%d nbyte:%d intdat:%d ext:%f %f %f",
	  header.ivary, header.nbyte, header.intdat, 
	  header.ext[0],header.ext[1],header.ext[2]);
  debmsg(message);
  sprintf(message,"ang:%f %f %f",header.xang,header.yang, header.zang);
  debmsg(message);
  sprintf(message,"x:%f %f  y:%f %f  z:%f %f",
	  header.xstart,header.xend,
	  header.ystart,header.yend,
	  header.zstart,header.zend);
  debmsg(message);
  sprintf(message,"int:%d %d %d",header.intx,header.inty, header.intz);
  debmsg(message);

  x1=header.xstart*header.ext[0];
  x2=header.xend*header.ext[0];
  y1=header.ystart*header.ext[1];
  y2=header.yend*header.ext[1];
  z1=header.zstart*header.ext[2];
  z2=header.zend*header.ext[2];

  dx=(x2-x1)/(float)(header.intx);
  dy=(y2-y1)/(float)(header.inty);
  dz=(z2-z1)/(float)(header.intz);

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));
  sn->field->size=(header.intx+1)*(header.inty+1)*(header.intz+1);

  sn->field->data=Ccalloc(sn->field->size,sizeof(float));

  data=Ccalloc(header.intx+1,sizeof(float));

  sn->field->u_size=header.intx+1;
  sn->field->v_size=header.inty+1;
  sn->field->w_size=header.intz+1;
  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=header.intx;
  sn->field->v2=header.inty;
  sn->field->w2=header.intz;
  sn->field->wrap=0;
  sn->field->sigma=1.0;
  sn->field->scale=1.0;

  for(w=0;w<header.intz+1;w++) {
    for(v=0;v<header.inty+1;v++) {
      fread(dummy, sizeof(dummy),1,f);
      fread(data, sizeof(float), header.intx+1,f);
      fread(dummy, sizeof(dummy),1,f);
      if(sn->swap_flag)
	swap_4bs((unsigned char *)data,header.intx+1);

      for(u=0;u<header.intx+1;u++) {
	scalWriteField(sn->field,u,v,w,data[u]);
      }
    }
  }

  
  sn->field->offset_x=x1;
  sn->field->offset_y=y1;
  sn->field->offset_z=z1;

  scalCELLtoVECT(sn->field,1.0,1.0,1.0,90.0,90.0,90.0,dx,dy,dz);

  Cfree(data);

  return 0;
}

int delphi2Read(FILE *f, dbmScalNode *sn)
{
  char uplbl[21],nxtlbl[71],botlbl[17];
  float scale, mid[3],fact;
  unsigned char dummy[4];
  char message[1024];
  
  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  if(sn->field==NULL) {
    comMessage("delphi2Read: memory allocation error\n");
    return -1;
  }

  fread(dummy, sizeof(dummy),1,f);
  fread(uplbl,sizeof(uplbl)-1,1,f);
  uplbl[20]='\0';
  fread(dummy, sizeof(dummy),1,f);
  fread(dummy, sizeof(dummy),1,f);
  fread(nxtlbl,sizeof(nxtlbl)-1,1,f);
  nxtlbl[70]='\0';
  fread(dummy, sizeof(dummy),1,f);
  fread(dummy, sizeof(dummy),1,f);

  sn->field->u_size=65;
  sn->field->v_size=65;
  sn->field->w_size=65;
  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=64;
  sn->field->v2=64;
  sn->field->w2=64;
  sn->field->wrap=0;
  sn->field->sigma=1.0;
  sn->field->scale=1.0;

  sn->field->size=65*65*65;
  sn->field->data=Ccalloc(sn->field->size,sizeof(float));

  fread(sn->field->data,sizeof(float),sn->field->size,f);

  if(sn->swap_flag) {
    swap_4bs((unsigned char *)sn->field->data,sn->field->size);
  }

  fread(dummy, sizeof(dummy),1,f);
  fread(dummy, sizeof(dummy),1,f);
  fread(botlbl,sizeof(botlbl)-1,1,f);
  botlbl[16]='\0';
  fread(dummy, sizeof(dummy),1,f);
  fread(dummy, sizeof(dummy),1,f);
  fread(&scale,sizeof(float),1,f);
  fread(mid,sizeof(float),3,f);

  if(sn->swap_flag) {
    swap_4b((unsigned char *)&scale);
    swap_4bs((unsigned char *)mid,3);
  }

  sn->field->offset_x=-32.0/scale+mid[0];
  sn->field->offset_y=-32.0/scale+mid[1];
  sn->field->offset_z=-32.0/scale+mid[2];

  if(scale==0.0) {
    comMessage("scale 0.0, set to 1.0\n");
    scale=1.0;
  }
  fact=1.0/scale;
  scalCELLtoVECT(sn->field,1.0,1.0,1.0,90.0,90.0,90.0,fact,fact,fact);

  sprintf(message," DELPHIG HEADER:\n %s\n %s\n %s\n", uplbl,nxtlbl,botlbl);
  debmsg(message);

  return 0;
  
}
