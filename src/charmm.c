#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

#include "dino.h"
#include "com.h"
#include "struct_db.h"
#include "charmm.h"
#include "Cmalloc.h"

extern int debug_mode;

int charmmReadB(FILE *f, dbmScalNode *sn)
{
  struct CHARMM_HEADER header;
  unsigned char dummy[4];
  char message[256];
  float *data,val=0.0;
  int u,v,w;
  long p;

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  if(sn->field==NULL) {
    comMessage("\ncharmmReadB: memory allocation error");
    return -1;
  }

  fread(&header,sizeof(struct CHARMM_HEADER),1,f);

  if(sn->swap_flag) {
    swap_4b((unsigned char *)&header.nclx);
    swap_4b((unsigned char *)&header.ncly);
    swap_4b((unsigned char *)&header.nclz);
    swap_double(&header.dcel);
    swap_double(&header.xbcen);
    swap_double(&header.ybcen);
    swap_double(&header.zbcen);
    swap_double(&header.epsw);
    swap_double(&header.epsp);
    swap_double(&header.conc);
    swap_double(&header.tmemb);
    swap_double(&header.zmemb);
    swap_double(&header.epsm);
  }

  sprintf(message,"\n%d %d %d\n%g %g %g %g\n%g %g %g %g %g %g\n",
	  header.nclx,header.ncly, header.nclz,
	  header.dcel, header.xbcen, header.ybcen, header.zbcen,
	  header.epsw, header.epsp, header.conc, header.tmemb,
	  header.zmemb, header.epsm);

  debmsg(message);


  sn->field->size=header.nclx*header.ncly*header.nclz;

  data=Ccalloc(sn->field->size,sizeof(float));

  if(data==NULL) {
    sprintf(message,"charmmReadB: memory allocation error for %dx%dx%d scalar field",header.nclx,header.ncly, header.nclz);
    Cfree(sn->field);
    return -1;
  }

  sn->field->data=Ccalloc(sn->field->size,sizeof(float));

  if(sn->field->data==NULL) {
    sprintf(message,"charmmReadB: memory allocation error for %dx%dx%d scalar field",header.nclx,header.ncly, header.nclz);
    Cfree(sn->field);
    return -1;
  }

//  fread(&dummy,sizeof(dummy),1,f);
  fread(data,sizeof(float),sn->field->size,f);
  if(sn->swap_flag)
    swap_4bs((unsigned char *)data,sn->field->size);

  sn->field->u_size=header.nclx;
  sn->field->v_size=header.ncly;
  sn->field->w_size=header.nclz;

  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=header.nclx-1;
  sn->field->v2=header.ncly-1;
  sn->field->w2=header.nclz-1;
  sn->field->wrap=0;
  sn->field->sigma=1.0;
  sn->field->scale=1.0;

  sn->field->offset_x=-header.dcel*(float)(header.nclx-1)/2.0;
  sn->field->offset_y=-header.dcel*(float)(header.ncly-1)/2.0;
  sn->field->offset_z=-header.dcel*(float)(header.nclz-1)/2.0;

  sn->field->offset_x+=header.xbcen;
  sn->field->offset_y+=header.ybcen;
  sn->field->offset_z+=header.zbcen;

  sprintf(message,"offset: %f %f %f",
	  sn->field->offset_x,
	  sn->field->offset_y,
	  sn->field->offset_z);

  debmsg(message);

  for(w=0;w<sn->field->w_size;w++) {
    for(v=0;v<sn->field->v_size;v++) {
      for(u=0;u<sn->field->u_size;u++) {
	p=(long)u*(long)sn->field->w_size*(long)sn->field->v_size+
	  (long)v*(long)sn->field->w_size+
	  (long)w;
	val=data[p];
	
	scalWriteField(sn->field,u,v,w,val);
      }
    }
  }

  scalCELLtoVECT(sn->field,1.0,1.0,1.0,90.0,90.0,90.0,
		 header.dcel,header.dcel,header.dcel);

  Cfree(data);

  return 0;
}

