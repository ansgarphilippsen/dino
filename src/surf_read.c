#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbm.h"
#include "com.h"
#include "Cmalloc.h"
#include "surf_read.h"
#include "surf_db.h"
#include "cl.h"
#include "mat.h"

int msmsRead(FILE *f1, FILE *f2, union DBM_NODE *node, int swap_flag)
{
  char line[256];
  int vn,fn,vc,fc,fcc,i,k;
  int dummy1,id;
  float prober,fdummy1;
  struct DBM_SURF_NODE *sn=&node->surfNode;
  char d1[11],d2[11],d3[11],d4[11],d5[11],d6[11];
  
  do {
    fgets(line,256,f1);
  }while(line[0]=='#');
  sscanf(line," %d",&vc);
  
  do {
    fgets(line,256,f2);
  }while(line[0]=='#');
  
  sscanf(line," %d%d%f %f",&fc,&dummy1,&prober,&fdummy1);
  
  sn->v=Ccalloc(vc+1,sizeof(struct SURF_VERTICE));
  sn->vc=vc;
  sn->f=Ccalloc(fc+1,sizeof(struct SURF_FACE));
  sn->fc=fc;
  
  for(vn=0;vn<vc;vn++) {
    fgets(line,256,f1);
    
    /*
      for(i=0;i<9;i++)
      d1[i]=line[i+0];
      d1[i]='\0';

    for(i=0;i<9;i++)
      d2[i]=line[i+10];
    d2[i]='\0';

    for(i=0;i<9;i++)
      d3[i]=line[i+20];
    d3[i]='\0';

    for(i=0;i<9;i++)
      d4[i]=line[i+30];
    d4[i]='\0';

    for(i=0;i<9;i++)
      d5[i]=line[i+40];
    d5[i]='\0';

    for(i=0;i<9;i++)
      d6[i]=line[i+50];
    d6[i]='\0';
    */

    sscanf(line,"%10c%10c%10c%10c%10c%10c %d %d",
	   d1,d2,d3,d4,d5,d6,&dummy1,&id);
    d1[10]='\0';
    d2[10]='\0';
    d3[10]='\0';
    d4[10]='\0';
    d5[10]='\0';
    d6[10]='\0';
    
    sn->v[vn].p[0]=(float)atof(d1);
    sn->v[vn].p[1]=(float)atof(d2);
    sn->v[vn].p[2]=(float)atof(d3);
    sn->v[vn].n[0]=(float)atof(d4);
    sn->v[vn].n[1]=(float)atof(d5);
    sn->v[vn].n[2]=(float)atof(d6);
    if(swap_flag) {
      sn->v[vn].n[0]*=-1.0;
      sn->v[vn].n[1]*=-1.0;
      sn->v[vn].n[2]*=-1.0;
    }
    matfNormalize(sn->v[vn].n,sn->v[vn].n); 
    sn->v[vn].id=id;
    sn->v[vn].num=vn;
    sn->v[vn].restrict=0;

    for(k=0;k<10;k++)
      sn->v[vn].cprop[k]=0.0;
  }

  fcc=0;

  for(fn=0;fn<fc;fn++) {
    fgets(line,256,f2);

    for(i=0;i<6;i++)
      d1[i]=line[i+0];
    d1[i]='\0';

    for(i=0;i<6;i++)
      d2[i]=line[i+7];
    d2[i]='\0';

    for(i=0;i<6;i++)
      d3[i]=line[i+14];
    d3[i]='\0';
    

    if(atoi(d1)<=vc && atoi(d2)<=vc && atoi(d3)<=vc) {
      sn->f[fcc].v[0]=atoi(d1)-1;
      sn->f[fcc].v[1]=atoi(d2)-1;
      sn->f[fcc].v[2]=atoi(d3)-1;
      fcc++;
    }
  }
  return 0;
}

int mspRead(FILE *f, union DBM_NODE *node)
{
  char line[256];
  int vn,ec,fn,vc,fc,i,k;
  struct DBM_SURF_NODE *sn=&node->surfNode;
  char d1[13],d2[13],d3[13],d4[13],d5[13],d6[13];

  vc=0; ec=0; fc=0;

  fgets(line,256,f);
  sscanf(line,"%d %d %d",&vc,&ec,&fc);
  
  sn->v=Ccalloc(vc+1,sizeof(struct SURF_VERTICE));
  sn->vc=vc;
  sn->f=Ccalloc(fc+1,sizeof(struct SURF_FACE));
  sn->fc=fc;

  for(vn=0;vn<vc;vn++) {
    fgets(line,256,f);

    sscanf(line,"%13c%13c%13c%8c%8c%8c",
	   d1,d2,d3,d4,d5,d6);
    d1[12]='\0';
    d2[12]='\0';
    d3[12]='\0';
    d4[7]='\0';
    d5[7]='\0';
    d6[7]='\0';

    sn->v[vn].p[0]=(float)atof(d1);
    sn->v[vn].p[1]=(float)atof(d2);
    sn->v[vn].p[2]=(float)atof(d3);
    sn->v[vn].n[0]=(float)atof(d4);
    sn->v[vn].n[1]=(float)atof(d5);
    sn->v[vn].n[2]=(float)atof(d6);
    sn->v[vn].num=vn;
    sn->v[vn].restrict=0;
    for(k=0;k<10;k++)
      sn->v[vn].cprop[k]=0.0;
  }

  /* ignore the edges */
  for(i=0;i<ec;i++)
    fgets(line,256,f);

  /* faces */
  for(fn=0;fn<fc;fn++) {
    fgets(line,256,f);

    sscanf(line,"%8c%8c%8c%7c%7c%7c",
	   d1,d2,d3,d4,d5,d6);
    d1[7]='\0';
    d2[7]='\0';
    d3[7]='\0';
    d4[6]='\0';
    d5[6]='\0';
    d6[6]='\0';


    sn->f[fn].v[0]=atoi(d4)-1;
    sn->f[fn].v[1]=atoi(d5)-1;
    sn->f[fn].v[2]=atoi(d6)-1;
  }
  return 0;
}

int adsRead(FILE *f, union DBM_NODE *node)
{
  struct DBM_SURF_NODE *sn=&node->surfNode;
  struct ADS_ENTRY *entry,ce;
  int usize=0,vsize=0;
  char line[256],*p,*b;
  char *d=" \t\n";
  char message[256];
  int i,off_u,off_v,u,v,k;
  int p1,p2,p3,p4,p5,p6;
  struct SURF_VERTICE *vert;
  struct SURF_FACE *face;
  int vc,vm,fc,fm;
  float middlex,middley,middlez;
  int middlec;
  float n1[3],n2[3],diff1[3],diff2[3];
  int ncount;
  PropTable propt;
  Prop prop;
  char propn[16];
  int propc,propi;
  float cprop[10];

  propReset(&propt);
  propc=0;

  for(k=0;k<10;k++)
    prop.v[k]=0.0;

  while(!feof(f)) {
    fgets(line,255,f);
    b=line;
    // remove CR
    b[clStrlen(b)-1]='\0';
    p=strtok(b,d);
    if(!strcmp(p,"REMARK")) {
      /* ignore */
    } else if(!strcmp(p,"DIMENSION")) {
      usize=atoi(strtok(NULL,d));
      vsize=atoi(strtok(NULL,d));
    } else if(!strcmp(p,"PROPERTY")) {
      propc++;
      sprintf(propn,"c%d",propc);
      propAddName(&propt,propn);
      sprintf(message,"\ncp%d: %s",propc-1,b+9);
      comMessage(message);
    } else if(!strcmp(p,"DATA")) {
      break;
    } else {
      sprintf(message,"\nadsRead: error: unknown keyword %s",p);
      comMessage(message);
      return -1;
    }
  }

  if(usize==0 || vsize==0) {
    comMessage("\nadsRead: error: missing or zero dimension");
    return -1;
  }

  entry=Ccalloc(usize*vsize+128,sizeof(struct ADS_ENTRY));

  if(entry==NULL) {
    comMessage("\nadsRead: memory allocation error");
    return -1;
  }

  for(i=0;i<usize*vsize;i++)
    entry[i].flag=0;

  fgets(line,255,f);
  ce.u=atoi(strtok(line,d));
  ce.v=atoi(strtok(NULL,d));
  ce.x=atof(strtok(NULL,d));
  ce.y=atof(strtok(NULL,d));
  ce.z=atof(strtok(NULL,d));

  if(propc>10)
    propc=10;

  for(propi=0;propi<propc;propi++) {
    p=strtok(NULL,d);
    if(p!=NULL) {
      prop.v[propi]=atof(p);
    } else {
      prop.v[propi]=0.0;
    }
  }

  off_u=-ce.u;
  off_v=-ce.v;
  i=(ce.v+off_v)*usize+(ce.u+off_u);
  entry[i].flag=1;
  entry[i].u=ce.u; entry[i].v=ce.v;
  entry[i].x=ce.x; entry[i].y=ce.y; entry[i].z=ce.z;
  for(k=0;k<10;k++)
    entry[i].c[k]=prop.v[k];
  
  vm=0;
  while(!feof(f)) {
    if(fgets(line,255,f)==NULL) break;
    //sscanf(line,"%d %d %f %f %f",&ce.u,&ce.v,&ce.x,&ce.y,&ce.z);
    ce.u=atoi(strtok(line,d));
    ce.v=atoi(strtok(NULL,d));
    ce.x=atof(strtok(NULL,d));
    ce.y=atof(strtok(NULL,d));
    ce.z=atof(strtok(NULL,d));
    for(propi=0;propi<propc;propi++) {
      p=strtok(NULL,d);
      if(p!=NULL) {
	prop.v[propi]=atof(p);
      } else {
	prop.v[propi]=0.0;
      }
    }

    i=(ce.v+off_v)*usize+(ce.u+off_u);
    if(i<0 && i>=usize*vsize) {
      sprintf(message,"\nadsRead: error: out of bounds with %d %d",ce.u,ce.v);
      comMessage(message);
      Cfree(entry);
      return -1;
    }
    entry[i].flag=1;
    entry[i].u=ce.u; entry[i].v=ce.v;
    entry[i].x=ce.x; entry[i].y=ce.y; entry[i].z=ce.z;
    for(k=0;k<10;k++)
      entry[i].c[k]=prop.v[k];
    vm++;
  }
  vm+=10;
  vm*=2;

  vert=Crecalloc(NULL,vm,sizeof(struct SURF_VERTICE));

  vc=0;

  for(u=0;u<usize;u++) {
    for(v=0;v<vsize;v++) {
      i=(v+0)*usize+(u+0);
      p2=(v+0)*usize+(u+1);
      p3=(v+1)*usize+(u+0);
      p4=(v+1)*usize+(u+1);
      p5=(v)*usize+(u-1);
      p6=(v-1)*usize+(u);
      if(entry[i].flag) {
	vert[vc].num=vc;
	vert[vc].p[0]=entry[i].x;
	vert[vc].p[1]=entry[i].y;
	vert[vc].p[2]=entry[i].z;

	// calculate normal from surrounding points
	ncount=0;
	n2[0]=0.0;
	n2[1]=0.0;
	n2[2]=0.0;
	if(entry[p2].flag && entry[p3].flag) {
	  diff1[0]=entry[i].x-entry[p2].x;
	  diff1[1]=entry[i].y-entry[p2].y;
	  diff1[2]=entry[i].z-entry[p2].z;
	  diff2[0]=entry[i].x-entry[p3].x;
	  diff2[1]=entry[i].y-entry[p3].y;
	  diff2[2]=entry[i].z-entry[p3].z;
	  matfCalcCross(diff1,diff2,n1);
	  n2[0]+=n1[0];
	  n2[1]+=n1[1];
	  n2[2]+=n1[2];
	  ncount++;
	}
	if(p5>=0) {
	  if(entry[p3].flag && entry[p5].flag) {
	    diff1[0]=entry[i].x-entry[p3].x;
	    diff1[1]=entry[i].y-entry[p3].y;
	    diff1[2]=entry[i].z-entry[p3].z;
	    diff2[0]=entry[i].x-entry[p5].x;
	    diff2[1]=entry[i].y-entry[p5].y;
	    diff2[2]=entry[i].z-entry[p5].z;
	    matfCalcCross(diff1,diff2,n1);
	    n2[0]+=n1[0];
	    n2[1]+=n1[1];
	    n2[2]+=n1[2];
	    ncount++;
	  }
	}
	if(p5>=0 && p6>=0) {
	  if(entry[p5].flag && entry[p6].flag) {
	    diff1[0]=entry[i].x-entry[p5].x;
	    diff1[1]=entry[i].y-entry[p5].y;
	    diff1[2]=entry[i].z-entry[p5].z;
	    diff2[0]=entry[i].x-entry[p6].x;
	    diff2[1]=entry[i].y-entry[p6].y;
	    diff2[2]=entry[i].z-entry[p6].z;
	    matfCalcCross(diff1,diff2,n1);
	    n2[0]+=n1[0];
	    n2[1]+=n1[1];
	    n2[2]+=n1[2];
	    ncount++;
	  }
	}
	if(p6>=0) {
	  if(entry[p6].flag && entry[p2].flag) {
	    diff1[0]=entry[i].x-entry[p6].x;
	    diff1[1]=entry[i].y-entry[p6].y;
	    diff1[2]=entry[i].z-entry[p6].z;
	    diff2[0]=entry[i].x-entry[p2].x;
	    diff2[1]=entry[i].y-entry[p2].y;
	    diff2[2]=entry[i].z-entry[p2].z;
	    matfCalcCross(diff1,diff2,n1);
	    n2[0]+=n1[0];
	    n2[1]+=n1[1];
	    n2[2]+=n1[2];
	    ncount++;
	  }
	}
	if(ncount>0) {
	  vert[vc].n[0]=n2[0]/((float)ncount);
	  vert[vc].n[1]=n2[1]/((float)ncount);
	  vert[vc].n[2]=n2[2]/((float)ncount);
	  matfNormalize(vert[vc].n,vert[vc].n);
	} else {
	  vert[vc].n[0]=0.0;
	  vert[vc].n[1]=0.0;
	  vert[vc].n[2]=0.0;
	}

	vert[vc].attach_node=NULL;
	vert[vc].attach_element=0;
	vert[vc].restrict=0;
	entry[i].vert_p=vc;
	for(k=0;k<10;k++)
	  vert[vc].cprop[k]=entry[i].c[k];
	vc++;

	if(u<usize-1 && v<vsize-1) {
	  middlec=1;
	  middlex=entry[i].x;
	  middley=entry[i].y;
	  middlez=entry[i].z;
	  for(k=0;k<10;k++)
	    cprop[k]=entry[i].c[k];
	  if(entry[p2].flag) {
	    middlec++;
	    middlex+=entry[p2].x;
	    middley+=entry[p2].y;
	    middlez+=entry[p2].z;
	    for(k=0;k<10;k++)
	      cprop[k]+=entry[p2].c[k];
	  }
	  if(entry[p3].flag) {
	    middlec++;
	    middlex+=entry[p3].x;
	    middley+=entry[p3].y;
	    middlez+=entry[p3].z;
	    for(k=0;k<10;k++)
	      cprop[k]+=entry[p3].c[k];
	  }
	  if(entry[p4].flag) {
	    middlec++;
	    middlex+=entry[p4].x;
	    middley+=entry[p4].y;
	    middlez+=entry[p4].z;
	    for(k=0;k<10;k++)
	      cprop[k]+=entry[p4].c[k];
	  }
	  // only if 3 or 4 corners defined
	  if(middlec>2) {
	    middlex/=(float)middlec;
	    middley/=(float)middlec;
	    middlez/=(float)middlec;
	    for(k=0;k<10;k++)
	      cprop[k]/=(float)middlec;
	    vert[vc].p[0]=middlex;
	    vert[vc].p[1]=middley;
	    vert[vc].p[2]=middlez;

	    // calculate normals for middle point
	    diff1[0]=middlex-entry[i].x;
	    diff1[1]=middley-entry[i].y;
	    diff1[2]=middlez-entry[i].z;
	    ncount=0;
	    n2[0]=0.0; n2[1]=0.0; n2[2]=0.0;
	    if(entry[p2].flag) {
	      diff2[0]=middlex-entry[p2].x;
	      diff2[1]=middley-entry[p2].y;
	      diff2[2]=middlez-entry[p2].z;
	      ncount++;
	      matfCalcCross(diff1,diff2,n1);
	      n2[0]+=n1[0];
	      n2[1]+=n1[1];
	      n2[2]+=n1[2];
	    }
	    if(entry[p3].flag) {
	      diff2[0]=middlex-entry[p3].x;
	      diff2[1]=middley-entry[p3].y;
	      diff2[2]=middlez-entry[p3].z;
	      ncount++;
	      matfCalcCross(diff2,diff1,n1);
	      n2[0]+=n1[0];
	      n2[1]+=n1[1];
	      n2[2]+=n1[2];
	    }
	    if(entry[p4].flag && entry[p2].flag) {
	      diff1[0]=middlex-entry[p2].x;
	      diff1[1]=middley-entry[p2].y;
	      diff1[2]=middlez-entry[p2].z;
	      diff2[0]=middlex-entry[p4].x;
	      diff2[1]=middley-entry[p4].y;
	      diff2[2]=middlez-entry[p4].z;
	      ncount++;
	      matfCalcCross(diff1,diff2,n1);
	      n2[0]+=n1[0];
	      n2[1]+=n1[1];
	      n2[2]+=n1[2];
	    } else  if(entry[p4].flag && entry[p3].flag) {
	      diff1[0]=middlex-entry[p3].x;
	      diff1[1]=middley-entry[p3].y;
	      diff1[2]=middlez-entry[p3].z;
	      diff2[0]=middlex-entry[p4].x;
	      diff2[1]=middley-entry[p4].y;
	      diff2[2]=middlez-entry[p4].z;
	      ncount++;
	      matfCalcCross(diff2,diff1,n1);
	      n2[0]+=n1[0];
	      n2[1]+=n1[1];
	      n2[2]+=n1[2];
	    }

	    if(ncount>0) {
	      vert[vc].n[0]=n2[0]/((float)ncount);
	      vert[vc].n[1]=n2[1]/((float)ncount);
	      vert[vc].n[2]=n2[2]/((float)ncount);
	      matfNormalize(vert[vc].n,vert[vc].n);
	    } else {
	      vert[vc].n[0]=0.0;
	      vert[vc].n[1]=0.0;
	      vert[vc].n[2]=0.0;
	    }

	    vert[vc].num=vc;
	    vert[vc].attach_node=NULL;
	    vert[vc].attach_element=0;
	    vert[vc].restrict=0;
	    entry[i].vertm_p=vc;
	    for(k=0;k<10;k++)
	      vert[vc].cprop[k]=cprop[k];
	    vc++;
	  } else {
	    entry[i].vertm_p=-1;
	  }
	}
      }
    }
  }

  fc=0;
  fm=10000;
  face=Crecalloc(NULL,fm,sizeof(struct SURF_FACE));

  for(u=0;u<usize-1;u++) {
    for(v=0;v<vsize-1;v++) {
      p1=(v+0)*usize+(u+0);
      p2=(v+0)*usize+(u+1);
      p3=(v+1)*usize+(u+1);
      p4=(v+1)*usize+(u+0);

      if(entry[p1].flag && entry[p2].flag && entry[p1].vertm_p>-1) {
	face[fc].v[0]=entry[p1].vert_p;
	face[fc].v[1]=entry[p2].vert_p;
	face[fc].v[2]=entry[p1].vertm_p;
	fc++;
      }
      if(entry[p1].flag && entry[p2].flag && entry[p3].flag && entry[p1].vertm_p>-1) {
	face[fc].v[0]=entry[p2].vert_p;
	face[fc].v[1]=entry[p3].vert_p;
	face[fc].v[2]=entry[p1].vertm_p;
	fc++;
      }
      if(entry[p1].flag && entry[p3].flag && entry[p4].flag && entry[p1].vertm_p>-1) {
	face[fc].v[0]=entry[p3].vert_p;
	face[fc].v[1]=entry[p4].vert_p;
	face[fc].v[2]=entry[p1].vertm_p;
	fc++;
      }
      if(entry[p4].flag && entry[p1].flag && entry[p1].vertm_p>-1) {
	face[fc].v[0]=entry[p4].vert_p;
	face[fc].v[1]=entry[p1].vert_p;
	face[fc].v[2]=entry[p1].vertm_p;
	fc++;
      }

      if(fc+5>=fm) {
	Crecalloc(face,fm+10000,sizeof(struct SURF_FACE));
	fm+=10000;
      }
      
    }
  }

  sn->v=Ccalloc(vc,sizeof(struct SURF_VERTICE));
  sn->vc=vc;
  memcpy(sn->v,vert,vc*sizeof(struct SURF_VERTICE));

  sn->f=Ccalloc(fc,sizeof(struct SURF_FACE));
  sn->fc=fc;
  memcpy(sn->f,face,fc*sizeof(struct SURF_FACE));

  comMessage("\n");

  Cfree(entry);

  return 0;
}

int graspRead(FILE *f, union DBM_NODE *node, int swap_flag)
{
  struct DBM_SURF_NODE *sn=&node->surfNode;
  struct GRASP_HEADER header;
  char message[255];
  char dummy[4],tmp[80],*p;

  struct SURF_VERTICE *vert;
  struct SURF_FACE *face;

  float *data;
  int *datai;

  char *ws=" \t";
  int vertc,facec,grid,latt,i,k,propc;

  

  fread(&header, sizeof(header),1,f);

  /*
  fprintf(stderr,"\n%s\n%s\n%s\n%s\n%s",
	  header.format,
	  header.content1,
	  header.content2,
	  header.count,
	  header.midpoint);
  */

  if(!clStrncmp(header.format,"format=2",8)) {
    comMessage("\nerror: format not recognized");
    return -1;
  }

  if(header.d1!=0x50) {
    swap_4b((unsigned char *)&header.d1);
    if(header.d1==0x50) {
      swap_flag=1;
      comMessage(" (auto-swap) ");
    } else {
      comMessage("\nerror: format not recognized");
      return -1;
    }
  }

  strncpy(tmp,header.count,80);

  p=strtok(tmp,ws);
  vertc=atoi(p);
  p=strtok(NULL,ws);
  facec=atoi(p);
  p=strtok(NULL,ws);
  grid=atoi(p);
  p=strtok(NULL,ws);
  latt=atoi(p);

  //  fprintf(stderr,"\n%d %d",vertc,facec);
  
  propc=0;
  p=strtok(header.content2,",");
  while(p!=NULL && propc<10) {
    sprintf(message,"\ncp%d: %s",propc,p);
    comMessage(message);
    propc++;
    p=strtok(NULL,",");
  }

  comMessage("\n");

  vert=Ccalloc(vertc,sizeof(struct SURF_VERTICE));
  face=Ccalloc(facec,sizeof(struct SURF_FACE));

  data=Ccalloc(3*vertc,sizeof(float));
  datai=Ccalloc(3*facec,sizeof(int));

  // read vertice data
  fread(dummy,4,1,f);
  fread(data,3*vertc,sizeof(float),f);
  fread(dummy,4,1,f);

  if(swap_flag)
    swap_floats(data,data,3*vertc);

  for(i=0;i<vertc;i++) {
    vert[i].p[0]=data[i*3+0];
    vert[i].p[1]=data[i*3+1];
    vert[i].p[2]=data[i*3+2];
    /*
    fprintf(stderr,"\n%f %f %f",
	    vert[i].p[0],
	    vert[i].p[1],
	    vert[i].p[2]);
    */
    vert[i].num=i;
    vert[i].restrict=0;
    for(k=0;k<10;k++)
      vert[i].cprop[k]=0.0;
  }

  // read accessible data (ignored)
  fread(dummy,4,1,f);
  fread(data,3*vertc,sizeof(float),f);
  fread(dummy,4,1,f);

  if(swap_flag)
    swap_4bs((unsigned char *)data,3*vertc);

  // read normal data
  fread(dummy,4,1,f);
  fread(data,3*vertc,sizeof(float),f);
  fread(dummy,4,1,f);

  if(swap_flag)
    swap_4bs((unsigned char *)data,3*vertc);

  for(i=0;i<vertc;i++) {
    vert[i].n[0]=data[i*3+0];
    vert[i].n[1]=data[i*3+1];
    vert[i].n[2]=data[i*3+2];
    matfNormalize(vert[i].n,vert[i].n);
  }

  // triangle indices (starting at 1!)
  fread(dummy,4,1,f);
  fread(datai,3*facec,sizeof(int),f);
  fread(dummy,4,1,f);

  if(swap_flag)
    swap_4bs((unsigned char *)datai,3*facec);
  
  for(i=0;i<facec;i++) {
    face[i].v[0]=datai[i*3+0]-1;
    face[i].v[1]=datai[i*3+1]-1;
    face[i].v[2]=datai[i*3+2]-1;
  }

  for(k=0;k<propc;k++) {
    // read property data
    fread(dummy,4,1,f);
    fread(data,vertc,sizeof(float),f);
    fread(dummy,4,1,f);
    
    if(swap_flag)
      swap_4bs((unsigned char *)data,vertc);
    
    for(i=0;i<vertc;i++)
      vert[i].cprop[k]=data[i];

  }  

  Cfree(data);
  Cfree(datai);

  sn->v=vert;
  sn->vc=vertc;
  sn->f=face;
  sn->fc=facec;

  return 0;
}
