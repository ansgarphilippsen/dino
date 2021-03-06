#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "dino.h"
#include "scal_db.h"
#include "struct_db.h"
#include "dbm.h"
#include "com.h"
#include "xplor.h"
#include "mat.h"
#include "Cmalloc.h"

extern struct STRUCT_ATOM_TABLE struct_atom_table[];
extern int struct_atom_table_len;

extern int debug_mode;

int xplorMapARead(FILE *f, dbmScalNode *sn)
{
  char line[1025];
  int titlec,i;
  struct XPLOR_MAP_HEADER header;
  int size;
  int an,bn,cn;
  char message[256];
  char da[13],db[13],dc[13],dalpha[13],dbeta[13],dgamma[13];
  int a,b,c,section,sectionc;
#ifdef XPLOR_OLD_ASCII
  char val[6][13];
#else
  float val[6];
#endif
  int u,v,w;
  float value;
  int vc;
  double fx,fy,fz;


  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));

  if(sn->field==NULL) {
    comMessage("xplorRead: memory allocation error\n");
    return -1;
  }

  strcpy(line,"\n");
  
  while(!strcmp(line,"\n"))
    fgets(line,1024,f);
  
  //if(strstr(line,"!NTITLE")!=NULL) {
  sscanf(line,"%8d",&titlec);
  for(i=0;i<titlec;i++)
    fgets(line,1024,f);
  //}  else {
  //  titlec=0;
  //}
  fgets(line,1024,f);
  sscanf(line,"%8d%8d%8d%8d%8d%8d%8d%8d%8d",
	 &header.na,
	 &header.amin,
	 &header.amax,
	 &header.nb,
	 &header.bmin,
	 &header.bmax,
	 &header.nc,
	 &header.cmin,
	 &header.cmax);
  fgets(line,1024,f);

  sprintf(message,"%d %d %d  %d %d %d  %d %d %d",
	  header.na,header.amin,header.amax,
	  header.nb,header.bmin,header.bmax,
	  header.nc,header.cmin,header.cmax);

  debmsg(message);

  sscanf(line,"%12c%12c%12c%12c%12c%12c",
	 da,
	 db,
	 dc,
	 dalpha,
	 dbeta,
	 dgamma);
  da[12]='\0'; db[12]='\0'; dc[12]='\0';
  dalpha[12]='\0'; dbeta[12]='\0'; dgamma[12]='\0';
  header.a=atof(da);
  header.b=atof(db);
  header.c=atof(dc);
  header.alpha=atof(dalpha);
  header.beta=atof(dbeta);
  header.gamma=atof(dgamma);

  sprintf(message,"%e %e %e  %e %e %e",
	  header.a,header.b,header.c,header.alpha,header.beta,header.gamma);
  debmsg(message);

  fgets(line,1024,f);
  strncpy(header.format,line,3);

  an=header.amax-header.amin+1;
  bn=header.bmax-header.bmin+1;
  cn=header.cmax-header.cmin+1;

  
  size=an*bn*cn;
  section=an*bn;

  sn->field->size=size;

  sn->field->data=Ccalloc(size,sizeof(float));

  sprintf(message,"Allocating for [%d %d %d] = %d",an,bn,cn,size);

  debmsg(message);
  if(sn->field->data==NULL) {
    sprintf(message,"xplorRead: memory allocation error for %dkb\n",size/1024);
    comMessage(message);
    Cfree(sn->field);
    return -1;
  }

  sn->field->u_size=an;
  sn->field->v_size=bn;
  sn->field->w_size=cn;
  sn->field->u1=header.amin;
  sn->field->u2=header.amax;
  sn->field->v1=header.bmin;
  sn->field->v2=header.bmax;
  sn->field->w1=header.cmin;
  sn->field->w2=header.cmax;
  sn->field->wrap=1;
  sn->field->offset_x=0.0;
  sn->field->offset_y=0.0;
  sn->field->offset_z=0.0;
  sn->field->sigma=1.0;

  sn->def_level=1.0;

  for(c=header.cmin;c<header.cmax;c++) {
    fgets(line,1024,f);
    sectionc=0;
    a=header.amin;
    b=header.bmin;
    while(sectionc<section) {
      fgets(line,1024,f);

#ifdef XPLOR_OLD_ASCII
      sscanf(line,"%12c%12c%12c%12c%12c%12c",
	     val[0],val[1],val[2],val[3],val[4],val[5]);
      for(vc=0;vc<6;vc++)
	if(val[vc][0]!='\0') {
	  val[vc][12]='\0';
	  value=(float)atof(val[vc]);
	  u=a;
	  v=b;
	  w=c;
	  
	  /*
	  while(u<0)
	    u+=an;
	  while(v<0)
	    v+=bn;
	  while(w<0)
	    w+=cn;
	  
	  u%=an;
	  v%=bn;
	  w%=cn;
	  */
	  scalWriteField(sn->field,u,v,w,value);
	  
	  sectionc++;
	  a++;
	  if(a>header.amax) {
	    b++;
	    if(b>header.bmax) {
	      break;
	    }
	    a=header.amin;
	  }
	}
#else
      sscanf(line,
	     "%12f%12f%12f%12f%12f%12f",
	     val+0,val+1,val+2,val+3,val+4,val+5);
      for(vc=0;vc<6;vc++) {
	scalWriteField(sn->field,a,b,c,val[vc]);
	sectionc++;
	a++;
	if(a>header.amax) {
	  b++;
	  if(b>header.bmax) {
	    break;
	  }
	  a=header.amin;
	}
      }
#endif
    }
  }
  

  /* transformation vectors */

  fx=1.0/(double)header.na;
  fy=1.0/(double)header.nb;
  fz=1.0/(double)header.nc;

  scalCELLtoVECT(sn->field,header.a, header.b, header.c,
		 header.alpha, header.beta, header.gamma,
		 fx,fy,fz);

  //fprintf(stderr,"%f\n",scalRMSD(sn->field));

  return 0;
}

int xplorMapBRead(FILE *f, dbmScalNode *node)
{
  int ntitle,i;
  char title[2048];
  char dummy[4];
  struct XPLOR_MAP_HEADER header;
  int h1[9];
  double h2[6];
  char h3[4];
  double fx,fy,fz;
  char message[4096];
  int a,b,c,an,bn,cn,size,section;
  double *sdata;

  memset(title,0,sizeof(title));
  memset(&header,0,sizeof(header));

  fread(dummy,sizeof(dummy),1,f);
  fread(&ntitle,sizeof(int),1,f);
  //  fprintf(stderr,"%x\n",ntitle);

  if(ntitle>1024 || ntitle<0) {
    node->swap_flag=1;
    swap_4b((unsigned char *)&ntitle);
    if(ntitle>1024 || ntitle<0) {
      comMessage("error reading header (even after byte-swap)\n");
      return -1;
    } else {
      comMessage("(byte swapping) ");
    }
  }
  fread(title,sizeof(char),ntitle*80,f);
  fread(dummy,sizeof(dummy),1,f);
  fread(dummy,sizeof(dummy),1,f);
  fread(h1,sizeof(int),9,f);
  fread(dummy,sizeof(dummy),1,f);
  fread(dummy,sizeof(dummy),1,f);
  fread(h2,sizeof(double),6,f);
  fread(dummy,sizeof(dummy),1,f);
  fread(dummy,sizeof(dummy),1,f);
  fread(h3,sizeof(char),3,f);
  fread(dummy,sizeof(dummy),1,f);

  if(node->swap_flag) {
    swap_int(h1,9);
    swap_double(h2,6);
  }

  sprintf(message,
	  "%d\n%s\n%d %d %d  %d %d %d  %d %d %d\n%e %e %e  %e %e %e\n%c%c%c\n",
	  ntitle,title,
	  h1[0],h1[1],h1[2],h1[3],h1[4],h1[5],h1[6],h1[7],h1[8],
	  h2[0],h2[1],h2[2],h2[3],h2[4],h2[5],
	  h3[0],h3[1],h3[2]);
  debmsg(message);

  header.na=h1[0]; header.amin=h1[1]; header.amax=h1[2];
  header.nb=h1[3]; header.bmin=h1[4]; header.bmax=h1[5];
  header.nc=h1[6]; header.cmin=h1[7]; header.cmax=h1[8];
  header.a=h2[0];  header.b=h2[1];    header.c=h2[2];
  header.alpha=h2[3]; header.beta=h2[4];    header.gamma=h2[5];
  header.format[0]=h3[0]; header.format[1]=h3[1]; header.format[2]=h3[2];
  header.format[3]='\0';

  if(strncmp(header.format,"ZYX",3)) {
    sprintf(message,"unknown xplor map format %s\n",header.format);
    comMessage(message);
    return -1;
  }

  node->field=Cmalloc(sizeof(struct SCAL_FIELD));
  
  if(node->field==NULL) {
    sprintf(message,"xplorRead: memory allocation error for %ldkb\n",
	    sizeof(struct SCAL_FIELD)/1024);
    comMessage(message);
    return -1;
  }

  /* read in the sections */
  an=header.amax-header.amin+1;
  bn=header.bmax-header.bmin+1;
  cn=header.cmax-header.cmin+1;

  size=an*bn*cn;
  section=an*bn;

  node->field->size=size;


  sprintf(message,"Allocating for [%d %d %d] = %d",an,bn,cn,size);
  debmsg(message);
  node->field->data=Ccalloc(size,sizeof(float));
  
  if(node->field->data==NULL) {
    sprintf(message,"xplorRead: memory allocation error for %ldkb\n",
	    size*sizeof(float)/1024);
    comMessage(message);
    return -1;
  }

  sprintf(message,"Allocating for [%d %d] = %d",an,bn,section);
  debmsg(message);
  sdata=Ccalloc(section,sizeof(double));
  
  if(sdata==NULL) {
    sprintf(message,"xplorRead: memory allocation error for %ldkb\n",
	    section*sizeof(double)/1024);
    comMessage(message);
    return -1;
  }

  node->field->u_size=an;
  node->field->v_size=bn;
  node->field->w_size=cn;
  node->field->u1=header.amin;
  node->field->u2=header.amax;
  node->field->v1=header.bmin;
  node->field->v2=header.bmax;
  node->field->w1=header.cmin;
  node->field->w2=header.cmax;
  node->field->wrap=1;
  node->field->offset_x=0.0;
  node->field->offset_y=0.0;
  node->field->offset_z=0.0;
  node->field->sigma=1.0;

  node->def_level=1.0;

  for(c=header.cmin;c<header.cmax;c++) {
    fread(dummy,sizeof(dummy),1,f);
    fread(&i,sizeof(int),1,f);
    fread(dummy,sizeof(dummy),1,f);

    fread(dummy,sizeof(dummy),1,f);
    fread(sdata,sizeof(double),section,f);
    if(node->swap_flag)
      swap_double(sdata,section);
    fread(dummy,sizeof(dummy),1,f);
    for(b=0;b<bn;b++)
      for(a=0;a<an;a++)
	scalWriteField(node->field,
		       a+header.amin,b+header.bmin,c,
		       sdata[b*an+a]);
  }
  Cfree(sdata);
  /* transformation vectors */
  fx=1.0/(double)header.na;
  fy=1.0/(double)header.nb;
  fz=1.0/(double)header.nc;
  
  scalCELLtoVECT(node->field,header.a, header.b, header.c,
		 header.alpha, header.beta, header.gamma,
		 fx,fy,fz);
  return 0;
}

int cnsTrjRead(FILE *f, dbmStructNode *node, int sf)
{
  int i,j;
  char dummy[4],dummy2[2];
  struct CNS_TRJ_HEADER header;
  float *x,*y,*z;
  struct STRUCT_TRJ_POSITION *pos;
  int ntitle,natom;
  char title[80];
  int tcount,tmax,tsize;
  char message[256];

  fread(dummy,sizeof(char),4,f);
  fread(&header,sizeof(header),1,f);
  fread(dummy,sizeof(char),4,f);
  fread(dummy,sizeof(char),4,f); // extra fread due to alignement

  fread(dummy,sizeof(char),4,f);
  fread(&ntitle,sizeof(int),1,f);

  if(ntitle<0 || ntitle>500) {
    // try swapping
    swap_int(&ntitle,1);

    if(ntitle<0 || ntitle>500) {
      // still not ok, probably invalid file
      comMessage("Error reading header (even tried byte swapping)\n");
      return -1;
    } else {
      comMessage("(byte-swapping)");
      sf=1;
    }
    
  }

  if(sf) {
    swap_int(&header.istart,1);
    swap_int(&header.nsavc,1);
    swap_int(&header.diff,1);
    swap_double(&header.delta,1);
  }

  sprintf(message,"hdr: %4s  istart:%d  nsavc: %d  diff:%d  delta:%f", 
	  header.hdr,header.istart,header.nsavc, header.diff, header.delta);
  debmsg(message);

  if(header.diff!=0) {
    comMessage("error: all atoms must be free atoms\n");
    return -1;
  }


  for(i=0;i<ntitle;i++) {
    fread(title,sizeof(title),1,f);
    title[79]='\0';
    sprintf(message,"titel %d: %s",i+1,title);
    debmsg(message);
  }

  fread(dummy,sizeof(dummy),1,f);

  fread(dummy,sizeof(dummy),1,f);
  fread(&natom,sizeof(natom),1,f);
  fread(dummy,sizeof(dummy),1,f);

  if(sf)
    swap_int(&natom,1);

  sprintf(message,"natom:%d\n",natom);
  debmsg(message);

  /*
    we don't know number of trajectories . . .
    memory is allocated for 100, and increased as more are read
    or is it nsavc from the header ???
  */
  tcount=0;
  tmax=100;

  if(node->trj_flag) {
    /* there is already a trajectory associated */
    Cfree(node->trj.pos);
  }

  node->trj_flag=1;
  node->trj.type=STRUCT_TRJ_CNS;
  node->trj.atom_count=natom;
  tsize=node->trj.size=node->trj.atom_count*sizeof(struct STRUCT_TRJ_POSITION);
  // node->frame_count not known yet

  x=Ccalloc(natom,sizeof(float));
  y=Ccalloc(natom,sizeof(float));
  z=Ccalloc(natom,sizeof(float));
  pos=Crecalloc(NULL,tmax,node->trj.size);

  while(!feof(f)) {
    fread(dummy,sizeof(dummy),1,f);
    fread(x,sizeof(float), natom,f );
    fread(dummy,sizeof(dummy),1,f);
    fread(dummy,sizeof(dummy),1,f);
    fread(y,sizeof(float), natom,f );
    fread(dummy,sizeof(dummy),1,f);
    fread(dummy,sizeof(dummy),1,f);
    fread(z,sizeof(float), natom,f );
    fread(dummy,sizeof(dummy),1,f);

    if(sf) {
      swap_float(x,natom);
      swap_float(y,natom);
      swap_float(z,natom);
    }
    
    for(i=0;i<natom;i++) {
      pos[tcount*natom+i].x=x[i];
      pos[tcount*natom+i].y=y[i];
      pos[tcount*natom+i].z=z[i];
    }

    tcount++;
    if(tcount>=tmax) {
      tmax+=100;
      pos=Crecalloc(pos,tmax,node->trj.size);
    }
  }

  // shrink memory to fit size
  pos=Crecalloc(pos,tcount,node->trj.size);

  sprintf(message,"frames read: %d\n",tcount-1);
  debmsg(message);

  node->trj.frame_count=tcount-1;
  node->trj.pos=pos;

  Cfree(x);
  Cfree(y);
  Cfree(z);

  return -1;
}


