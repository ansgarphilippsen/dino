#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "dbm.h"
#include "scal_db.h"
#include "mead.h"
#include "com.h"
#include "Cmalloc.h"
#include "dino.h"

extern int debug_mode;

int meadRead(FILE *f, dbmScalNode *sn)
{
  char message[1024];
  char line[1024];
  char prop[256],value[256];
  int lc,i;
  float dim[6],dims[6],*datas;
  char c;
  int dim1=-1,dim2=-1,dim3=-1;
  float min_ext[10],max_ext[10];
  int ret;
  double fx,fy,fz;
  int size;

  /* 
     first line should be 
     # AVS field file
  */
  fgets(line,1024,f);
  line[5]='\0';
  if(strcmp(line,"# AVS")) {
    sprintf(message,"Error: Expected '# AVS' on first line\n");
    comMessage(message);
    return -1;
  }

  while(1) {
    line[0]='\0';
    lc=0;
    do {
      fread(&c,sizeof(char),1,f);
      line[lc++]=c;
    } while(c!='\n' && c!=12);
    line[lc]='\0';
    if(c==12) {
      fread(&c,sizeof(char),1,f); 
      if(c!=12) {
	sprintf(message,"Warning: Expected 2nd FF\n");
	comMessage(message);
      }
      break;
    } else {
      /* check prop=value */
      if(meadSplit(line,prop,value)!=-1) {
	if(!strcmp(prop,"dim1")) {
	  dim1=atoi(value);
	} else if(!strcmp(prop,"dim2")) {
	  dim2=atoi(value);
	} else if(!strcmp(prop,"dim3")) {
	  dim3=atoi(value);
	} else if(!strcmp(prop,"max_ext")) {
	  meadSplitValue(value,max_ext);
	} else if(!strcmp(prop,"min_ext")) {
	  meadSplitValue(value,min_ext);
	}
      }
      
    }
    if(feof(f)) {
      comMessage(" reached unexpected end of file\n");
      return -1;
    }
  }

  size=dim1*dim2*dim3;

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));
  sn->field->data=Ccalloc(size,sizeof(float));
  sn->field->size=size;

  if(sn->swap_flag) {
    datas=Ccalloc(size,sizeof(float));
    ret=fread(sn->field->data,sizeof(float),size,f);
    swap_float(sn->field->data,size);
    Cfree(datas);
  } else {
    ret=fread(sn->field->data,sizeof(float),size,f);
  }
  if(sn->swap_flag) {
    fread((char*)dim,sizeof(float),6,f);
    swap_float(dim,6);
  } else {
    fread((char*)dim,sizeof(float),6,f);
  }

  sn->field->u_size=dim1;
  sn->field->v_size=dim2;
  sn->field->w_size=dim3;
  sn->field->u1=0;
  sn->field->v1=0;
  sn->field->w1=0;
  sn->field->u2=sn->field->u1+sn->field->u_size-1;
  sn->field->v2=sn->field->v1+sn->field->v_size-1;
  sn->field->w2=sn->field->w1+sn->field->w_size-1;
  sn->field->wrap=0;
  sn->field->sigma=1.0;

  sn->field->offset_x=dim[0];
  sn->field->offset_y=dim[2];
  sn->field->offset_z=dim[4];

  fx=(dim[1]-dim[0])/((float)dim1-1.0);
  fy=(dim[3]-dim[2])/((float)dim2-1.0);
  fz=(dim[5]-dim[4])/((float)dim3-1.0);

  sprintf(message,"Header Info\n%d %d %d\n%d %d %d  %d %d %d\n%f %f %f\n%f %f %f\n",
	  sn->field->u_size, sn->field->v_size, sn->field->w_size,
	  sn->field->u1, sn->field->v1, sn->field->w1,
	  sn->field->u2, sn->field->v2, sn->field->w2,
	  sn->field->offset_x, sn->field->offset_y, sn->field->offset_z,
	  fx,fy,fz);

  debmsg(message);

  scalCELLtoVECT(sn->field,
		  1.0,1.0,1.0,
		  90.0,90.0,90.0,
		  fx,fy,fz);


  return 0;
}

int meadSplit(char *line, char *prop, char *value)
{
  char *p;

  p=strchr(line,'=');
  if(p==NULL)
    return -1;
  
  p[0]='\0';
  strcpy(prop,line);
  strcpy(value,p+1);
  p=strchr(value,'#');
  if(p!=NULL)
    p[0]='\0';
  return 0;
}

int meadSplitValue(char *val, float *v)
{
  int vc=0;
  char n[256];
  int nc;
  int pc=0;

  v[0]=0.0;
  v[1]=0.0;
  v[2]=0.0;

  while(vc<strlen(val)) {
    /* strip ws */
    while(isspace(val[vc]) && vc<strlen(val)) vc++;
    nc=0;
    while(!isspace(val[vc]) && vc<strlen(val)) n[nc++]=val[vc++];
    n[nc]='\0';
    v[pc++]=atof(n);
    if(pc>3)
      break;
  }
  
  return 0;
}
