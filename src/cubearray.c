#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "dino.h"
#include "cubearray.h"
#include "Cmalloc.h"
#include "com.h"

extern int debug_mode;

cubeArray *caInit(float xyz1[3], float xyz2[3], int list_max, float size)
{
  float fa,fb,fc,size2;
  int a,b,c,i;
  cubeArray *ca;
  float xyz[3];
  int flag;
  char message[256];

  if(size<=0.0) {
    fprintf(stderr,"internal error #2 in caInit()\n");
    return NULL;
  }
  if((ca=Cmalloc(sizeof(cubeArray)))==NULL) {
    fprintf(stderr," memory allocation error in caInit()\n");
    return NULL;
  }

  xyz[0]=xyz2[0]-xyz1[0];
  xyz[1]=xyz2[1]-xyz1[1];
  xyz[2]=xyz2[2]-xyz1[2];

  sprintf(message,"cubearray: %f %f %f",xyz[0],xyz[1],xyz[2]);
  debmsg(message);

  flag=0;
  size2=size;
  do {
    fa=xyz[0]/size;
    fb=xyz[1]/size;
    fc=xyz[2]/size;

    a=1+(int)fa;
    b=1+(int)fb;
    c=1+(int)fc;

    if(a*b*c>500000) {
      size+=size2;
      sprintf(message,"caInit: adjusting cubearray size to %.2f\n",size);
      comMessage(message);
    } else {
      flag=1;
    }
  } while(!flag);


  ca->x1=xyz1[0];
  ca->y1=xyz1[1];
  ca->z1=xyz1[2];
  ca->x2=xyz2[0];
  ca->y2=xyz2[1];
  ca->z2=xyz2[2];

  ca->a=a;
  ca->b=b;
  ca->c=c;

  ca->size=size;

  ca->ecount=a*b*c;

  sprintf(message,"cubearray: using %dx%dx%d grid, allocating %d bytes",a,b,c,ca->ecount*sizeof(caElement));
  debmsg(message);

  ca->element=Ccalloc(ca->ecount,sizeof(caElement));
  if(ca->element==NULL) {
    fprintf(stderr,"memory allocation error in caInit()\n");
    return NULL;
  }
  for(i=0;i<ca->ecount;i++)
    ca->element[i].count=0;
  

  if(list_max<ca->ecount)
    list_max=ca->ecount;

  ca->lmax=list_max;
  ca->lcount=0;

  ca->list=Ccalloc(ca->lmax,sizeof(caPointer));
  if(ca->list==NULL) {
    fprintf(stderr,"memory allocation error in caInit()\n");
    return NULL;
  }

  return ca;
}

int caOutit(cubeArray *ca)
{
  Cfree(ca->list);
  Cfree(ca->element);
  Cfree(ca);
  return 0;
}

int caXYZtoABC(cubeArray *ca, float xyz[3], int abc[3])
{
  float fa,fb,fc;
  fa=(xyz[0]-ca->x1)/ca->size;
  fb=(xyz[1]-ca->y1)/ca->size;
  fc=(xyz[2]-ca->z1)/ca->size;

  abc[0]=(int)fa;
  abc[1]=(int)fb;
  abc[2]=(int)fc;
  
  return 0;
}

int caABCtoXYZ(cubeArray *ca, int abc[3], float xyz[3])
{
  xyz[0]=ca->x1+ca->size*(float)abc[0]+ca->size/2.0;
  xyz[1]=ca->y1+ca->size*(float)abc[1]+ca->size/2.0;
  xyz[2]=ca->z1+ca->size*(float)abc[2]+ca->size/2.0;

  return 0;
}

int caGetLimit(cubeArray *ca, int abc[3], float xyz1[3],float xyz2[3])
{
  xyz1[0]=ca->x1+ca->size*(float)abc[0];
  xyz1[1]=ca->y1+ca->size*(float)abc[1];
  xyz1[2]=ca->z1+ca->size*(float)abc[2];

  xyz2[0]=ca->x1+ca->size*(float)abc[0]+ca->size;
  xyz2[1]=ca->y1+ca->size*(float)abc[1]+ca->size;
  xyz2[2]=ca->z1+ca->size*(float)abc[2]+ca->size;

  return 0;
}


int caAddPointer(cubeArray *ca, int abc[3], caPointer p, int mode)
{
  int i;
  if(mode==CA_ADD) {
    ca->element[caABCtoI(ca,abc)].count++;
  } else if(mode==CA_WRITE) {
    i=caABCtoI(ca,abc);
    ca->element[i].list[ca->element[i].count++]=p;
  }

  return 0;
}

int caFix(cubeArray *ca)
{
  int i,lcount;
  caElement *el;

  lcount=0;

  for(i=0;i<ca->ecount;i++) {
    el=&ca->element[i];

    el->max=el->count;
    el->count=0;

    lcount+=el->max;
  }

  Cfree(ca->list);
  ca->lmax=lcount;
  ca->list=Ccalloc(ca->lmax,sizeof(caPointer));
  ca->lcount=lcount;

  if(ca->list==NULL) {
    fprintf(stderr,"memory allocation error in caFix()\n");
    return -1;
  }
  lcount=0;
  for(i=0;i<ca->ecount;i++) {
    el=&ca->element[i];
    el->list=&ca->list[lcount];
    lcount+=el->max;
  }


  return 0;
}


int caGetList(cubeArray *ca, int abc[3], caPointer **list, int *count)
{
  int i;

  i=caABCtoI(ca,abc);
  if(i==-1) {
    (*list)=NULL;
    (*count)=0;
  } else {
    (*list)=ca->element[i].list;
    (*count)=ca->element[i].count;
  }
  return i;
}

int caABCtoI(cubeArray *ca, int abc[3])
{
  if(abc[0]<0 || abc[0]>=ca->a)
    return -1;
  if(abc[1]<0 || abc[1]>=ca->b)
    return -1;
  if(abc[2]<0 || abc[2]>=ca->c)
    return -1;
  return ca->a*ca->b*abc[2]+ca->a*abc[1]+abc[0];
}

// assuming l is already allocated
int caGetWithinList(cubeArray *ca, float *p, float d, caPointer **l, int *c)
{
  float p1[3],p2[3];
  int i[3],i1[3],i2[3];
  caPointer *dp;
  int lc,lc2;

  p1[0]=p[0]-d;
  p1[1]=p[1]-d;
  p1[2]=p[2]-d;
  p2[0]=p[0]+d;
  p2[1]=p[1]+d;
  p2[2]=p[2]+d;

  caXYZtoABC(ca,p1,i1);
  caXYZtoABC(ca,p2,i2);

  /* just count the list size */
  /********
	   lc=0;
	   for(i[0]=i1[0]; i[0]<=i2[0]; i[0]++)
	   for(i[1]=i1[1]; i[1]<=i2[1]; i[1]++)
	   for(i[2]=i1[2]; i[2]<=i2[2]; i[2]++) {
	   caGetList(ca,i,&dp,&lc2);
	   lc+=lc2;
	   }
	   
	   (*l)=Ccalloc(lc+10,sizeof(caPointer));
  ***********/

  (*c)=0;
  for(i[0]=i1[0]; i[0]<=i2[0]; i[0]++)
    for(i[1]=i1[1]; i[1]<=i2[1]; i[1]++)
      for(i[2]=i1[2]; i[2]<=i2[2]; i[2]++) {
	caGetList(ca,i,&dp,&lc2);
	for(lc=0;lc<lc2;lc++)
	  (*l)[(*c)++]=dp[lc];
      }

  return 0;
}
