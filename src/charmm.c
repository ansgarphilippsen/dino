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
#include "struct_read.h"

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
    comMessage("charmmReadB: memory allocation error\n");
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

  sprintf(message,"%d %d %d\n%g %g %g %g\n%g %g %g %g %g %g\n\n",
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

#define BDTRJ_MAX_TYPES 32

int bdtrjRead(dbmStructNode *node, FILE *f, int swap_flag)
{
  int i;
  int nframe, ntype;
  char atname[BDTRJ_MAX_TYPES][5];
  char dummy[4];
  struct STRUCT_BDTRJ_FRAME *frame;
  struct STRUCT_BDTRJ_ENTRY *entry;
  int entry_count,entry_max;
  long runtime;
  int nmoveion,framec;
  int ctype;
  float cx,cy,cz;
  struct STRUCT_FILE sf;
  struct STRUCT_FILE_ATOM_ENTRY *ae;
  char message[256];

  fread(dummy,1,4,f);
  fread(&nframe,sizeof(int),1,f);
  fread(dummy,1,4,f);
  if(swap_flag)
    swap_4b((unsigned char *)&nframe);

  fread(dummy,1,4,f);
  fread(&ntype,sizeof(int),1,f);
  fread(dummy,1,4,f);
  if(swap_flag)
    swap_4b((unsigned char *)&ntype);

  //  fprintf(stderr,"nframe: %d  ntype: %d\n",nframe,ntype);

  if(nframe>1e6) {
    comMessage("error: huge framecount: invalid fileformat or byte-swapped ?\n");
    return -1;
  }

  if(ntype<=0) {
    comMessage("error: ntype <=0\n");
    return -1;
  }

  if(ntype>=BDTRJ_MAX_TYPES) {
    comMessage("error: ntypes exceeds maximum\n");
    return -1;
  }

  fread(dummy,1,4,f);
  for(i=0;i<ntype;i++) {
    fread(atname[i],sizeof(char),4,f);
    if(atname[i][3]==' ')
      atname[i][3]='\0';
    if(atname[i][2]==' ')
      atname[i][2]='\0';
    if(atname[i][1]==' ')
      atname[i][1]='\0';
    atname[i][4]='\0';
  }
  fread(dummy,1,4,f);

  frame=Ccalloc(nframe,sizeof(struct STRUCT_BDTRJ_FRAME));

  entry_max=ntype*nframe*10;
  entry=Crecalloc(NULL,entry_max,sizeof(struct STRUCT_BDTRJ_ENTRY));
  entry_count=0;

  for(framec=0;framec<nframe;framec++) {
    fread(dummy,1,4,f);
    fread(dummy,1,4,f);
    fread(dummy,1,4,f);
    fread(dummy,1,4,f);

    fread(dummy,1,4,f);
    fread(&nmoveion,sizeof(int),1,f);
    fread(dummy,1,4,f);

    if(entry_count+nmoveion>=entry_max) {
      entry_max+=nmoveion;
      entry=Crecalloc(entry,entry_max,sizeof(struct STRUCT_BDTRJ_ENTRY));
    }

    fread(dummy,1,4,f);
    for(i=0;i<nmoveion;i++) {
      fread(&ctype,sizeof(int),1,f);
      entry[entry_count+i].frame=framec;
      entry[entry_count+i].index=ctype;
    }
    fread(dummy,1,4,f);

    fread(dummy,1,4,f);
    for(i=0;i<nmoveion;i++) {
      fread(&cx,sizeof(float),1,f);
      entry[entry_count+i].x=cx;
    }
    fread(dummy,1,4,f);

    fread(dummy,1,4,f);
    for(i=0;i<nmoveion;i++) {
      fread(&cy,sizeof(float),1,f);
      entry[entry_count+i].y=cy;
    }
    fread(dummy,1,4,f);

    fread(dummy,1,4,f);
    for(i=0;i<nmoveion;i++) {
      fread(&cz,sizeof(float),1,f);
      entry[entry_count+i].z=cz;
    }
    fread(dummy,1,4,f);
    
    /*
    for(i=0;i<nmoveion;i++) {
      fprintf(stderr,"%d %d %f %f %f\n",
	      entry[entry_count+i].frame,
	      entry[entry_count+i].index,
	      entry[entry_count+i].x,
	      entry[entry_count+i].y,
	      entry[entry_count+i].z);
    }
    */

    frame[framec].n=framec;
    frame[framec].start=entry_count;
    frame[framec].end=entry_count+nmoveion-1;

    entry_count+=nmoveion;
  }

  entry=Crecalloc(entry,entry_count,sizeof(struct STRUCT_BDTRJ_ENTRY));


  /*
    create the normal struct entries
  */

  ae=Ccalloc(ntype,sizeof(struct STRUCT_FILE_ATOM_ENTRY));

  for(i=0;i<ntype;i++) {
    ae[i].anum=i+1;
    strcpy(ae[i].aname,atname[i]);
    strcpy(ae[i].element,"NN");
    ae[i].rnum=i+1;
    strcpy(ae[i].rname,"TRJ");
    ae[i].cnum=-1;
    strcpy(ae[i].cname,"");
    ae[i].mnum=-1;
    strcpy(ae[i].mname,"");
    ae[i].x=0.0;
    ae[i].y=0.0;
    ae[i].z=0.0;
    ae[i].c1=0.0;
    ae[i].c2=0.0;
    ae[i].c3=0.0;
  }

  sf.atom_entry=ae;
  sf.atom_count=ntype;
  sf.connect_entry=NULL;
  sf.connect_count=0;

  structFileEntry2DB(&sf,node);

  /*
    set trj params
  */

  node->trj_flag=1;
  node->trj.type=STRUCT_TRJ_BD;
  node->trj.pos=NULL;
  node->trj.atom_count=0;
  node->trj.size=0;
  node->trj.frame=frame;
  node->trj.frame_count=nframe;
  node->trj.entry=entry;
  node->trj.entry_count=entry_count;

  sprintf(message," %d atomtypes, %d frames, %d entries",
	  ntype,nframe,entry_count);
  comMessage(message);

  return 0;
}
