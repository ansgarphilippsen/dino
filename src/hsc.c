#include <string.h>

#include "hsc.h"
#include "bspline.h"

int hscGenerate(HSC *hsc)
{
  cgfxSplinePoint *opoint;
  int opoint_count;

  if(hsc==NULL)
    return -1;

  if(hsc->spoint_count<2) {
    comReturn("\nspline must have at least 2 points");
  }
  
  opoint=hsc->spoint;
  opoint_count=hsc->spoint_count;
  
  hsc->spoint=Ccalloc(opoint_count+2,sizeof(cgfxSplinePoint));
  hsc->spoint_count+=2;
  memcpy(hsc->spoint+1,opoint,opoint_count*sizeof(cgfxSplinePoint));
  hscGenEndPoints(hsc);
  
  if(hsc->spoint_count<3) {
    comReturn("\n2 point splines not implemented");
    return -1;
  } else {
    if(hscGenSpline(hsc)<0) {
      Cfree(hsc->spoint);
      hsc->spoint=opoint;
      return -1;
    }
  }
  if(hscBuild(hsc)<0) {
    Cfree(hsc->spoint);
    hsc->spoint=opoint;
    return -1;
  }
  
  return 0;
}

int hscGenEndPoints(HSC *hsc)
{
  float *v1,*v2,*v3;
  int n=hsc->spoint_count-1;

  memcpy(hsc->spoint+0,hsc->spoint+1,sizeof(cgfxSplinePoint));
  memcpy(hsc->spoint+n,hsc->spoint+n-1,sizeof(cgfxSplinePoint));
  v1=hsc->spoint[0].v;
  v2=hsc->spoint[1].v;
  v3=hsc->spoint[2].v;
  v1[0]=v2[0]-(v3[0]-v2[0]);
  v1[1]=v2[1]-(v3[1]-v2[1]);
  v1[2]=v2[2]-(v3[2]-v2[2]);
  v1=hsc->spoint[n].v;
  v2=hsc->spoint[n-1].v;
  v3=hsc->spoint[n-2].v;
  v1[0]=v2[0]-(v3[0]-v2[0]);
  v1[1]=v2[1]-(v3[1]-v2[1]);
  v1[2]=v2[2]-(v3[2]-v2[2]);
  return -1;
}

int hscGenSpline(HSC *hsc)
{
  int i,j,k,n,detail,pc;
  float p1[5][3],p2[5][3];
  float x,y,z;
  float b,f;

  /*
    hsc already contains the two phantom endpoints,
    at least 5 points !
  */
  n=hsc->spoint_count;
  detail=hsc->render->detail;

  hsc->point_count=(hsc->spoint_count-2)*detail;
  hsc->point=Ccalloc(hsc->point_count,sizeof(cgfxPoint));

  f=1.0/((float)detail);

  pc=0;
 
  for(i=0;i<n-4;i++) {

    for(j=0;j<5;j++) {
      p1[j][0]=hsc->spoint[i+j].v[0];
      p1[j][1]=hsc->spoint[i+j].v[1];
      p1[j][2]=hsc->spoint[i+j].v[2];
    }
    bspline_c2p(p1,p2);

    x=0.0; y=0.0; z=0.0;
    for(j=0;j<detail;j++) {
      for(k=0;k<5;k++) {
	b=cgfxB(f*(float)j+float(k)-2.0);
	x+=p2[k][0]*b;
	y+=p2[k][1]*b;
	z+=p2[k][2]*b;
      }
      hsc->point[pc].v[0]=x;
      hsc->point[pc].v[1]=y;
      hsc->point[pc].v[2]=z;
      pc++;
    }
  }

  return 0;
}

int hscBuild(HSC *hsc)
{
  return 0;
}
