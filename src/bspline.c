#include "bspline.h"
#include "Cmalloc.h"
#include "cgfx.h"

#include "mat.h"

int bsplineGenerate(cgfxSplinePoint *sp, cgfxPoint **pp, int n, int detail, int cgfx_flag)
{
  int i,d,k;
  float *px,*py,*pz,*x,*y,*y2;
  float u,v;
  float v1[3],v2[3];
  int col;
  cgfxPoint *p;

  p=Ccalloc(detail*n+detail,sizeof(cgfxPoint));
  (*pp)=p;

  px=Ccalloc(n*6,sizeof(float));
  py=&px[n];
  pz=&py[n];
  x=&pz[n];
  y=&x[n];
  y2=&y[n];

  // interpolate vertices
  for(k=0;k<3;k++) {
    for(i=0;i<n;i++) {
      x[i]=(float)i;
      y[i]=sp[i].v[k];
    }
    bsplineGen(x-1,y-1,n,1e30,1e30,y2-1);
    for(i=0;i<n;i++) {
      sp[i].p=&p[i*detail];
      sp[i].pc=detail;
      for(d=0;d<detail;d++) {
	u=((float)(i*detail+d))/((float)detail);
	bsplineGet(x-1,y-1,y2-1,n,u,&v);
	p[i*detail+d].v[k]=v;
      }
    }
  }

  // same thing for normals
  for(k=0;k<3;k++) {
    for(i=0;i<n;i++) {
      x[i]=(float)i;
      y[i]=sp[i].n[k];
    }
    bsplineGen(x-1,y-1,n,1e30,1e30,y2-1);
    for(i=0;i<n;i++) {
      for(d=0;d<detail;d++) {
	u=((float)(i*detail+d))/((float)detail);
	bsplineGet(x-1,y-1,y2-1,n,u,&v);
	p[i*detail+d].n[k]=v;
      }
    }
  }

  // and colors
  for(col=0;col<3;col++) {
    if(cgfx_flag & CGFX_INTPOL_COL) {
      for(k=0;k<3;k++) {
	for(i=0;i<n;i++) {
	  x[i]=(float)i;
	  y[i]=sp[i].colp[col][k];
	}
	bsplineGen(x-1,y-1,n,1e30,1e30,y2-1);
	for(i=0;i<n;i++) {
	  for(d=0;d<detail;d++) {
	    u=((float)(i*detail+d))/((float)detail);
	    bsplineGet(x-1,y-1,y2-1,n,u,&v);
	    if(v<0.0)
	      v=0.0;
	    if(v>1.0)
	      v=1.0;
	    p[i*detail+d].col[col][k]=v;
	  }
	}
      }
    } else {
      for(k=0;k<3;k++) {
	for(i=0;i<n-1;i++) {
	  for(d=0;d<detail/2;d++) {
	    v=sp[i].colp[col][k];
	    p[i*detail+d].col[col][k]=v;
	  }
	  for(d=detail/2;d<detail;d++) {
	    v=sp[i+1].colp[col][k];
	    p[i*detail+d].col[col][k]=v;
	  }
	}    
      }
    }
    
    // set transparency
    for(i=0;i<n;i++)
      for(d=0;d<detail;d++)
	p[i*detail+d].col[col][3]=sp[i].colp[col][3];
    
  }

  Cfree(px);

  // calculate dir vectors
  for(i=1;i<n*detail-1;i++) {
    p[i].d[0]=(p[i+1].v[0]-p[i].v[0]);
    p[i].d[1]=(p[i+1].v[1]-p[i].v[1]);
    p[i].d[2]=(p[i+1].v[2]-p[i].v[2]);
    p[i].d[0]+=(p[i].v[0]-p[i-1].v[0]);
    p[i].d[1]+=(p[i].v[1]-p[i-1].v[1]);
    p[i].d[2]+=(p[i].v[2]-p[i-1].v[2]);
    p[i].d[0]*=0.5;
    p[i].d[1]*=0.5;
    p[i].d[2]*=0.5;
    matfNormalize(p[i].d,p[i].d);
  }
  matfNormalize(p[1].d,p[0].d);
  matfNormalize(p[i-1].d,p[i].d);

  
  return 0;
}

/*
  from numerical recipes in C
*/

/*
  NOTE THAT ARRAYS START WITH 1
*/

int bsplineGen(float *x,float *y, int n, float yp1, float ypn, float *y2)
{
  int i,k;
  float p,qn,sig,un,*u;

  u=Ccalloc(n+1,sizeof(float));

  if(yp1>0.99e30) {
    y2[1]=u[1]=0.0;
  } else {
    y2[1]=-0.5;
    u[1]=(3.0/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
  }
  for(i=2;i<=n-1;i++) {
    sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
    p=sig*y2[i-1]+2.0;
    y2[i]=(sig-1.0)/p;
    u[i]=(y[i+1]-y[i])/(x[i+1]-x[i])-(y[i]-y[i-1])/(x[i]-x[i-1]);
    u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
  }
  if(ypn>0.99e30) {
    qn=un=0.0;
  } else {
    qn=0.5;
    un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
  }
  y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
  for(k=n-1;k>=1;k--)
    y2[k]=y2[k]*y2[k+1]+u[k];
  Cfree(u);
  return 0;
}

int bsplineGet(float *xa, float *ya, float *y2a, int n, float x, float *y)
{
  int klo,khi,k;
  float h,b,a;

  klo=1;
  khi=n;
  while(khi-klo>1) {
    k=(khi+klo)>>1;
    if(xa[k]>x) 
      khi=k;
    else
      klo=k;
  }
  h=xa[khi]-xa[klo];
  if(h==0.0)
    return -1;
  a=(xa[khi]-x)/h;
  b=(x-xa[klo])/h;
  (*y)=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
  return 0;
}


/**************************************************/

/*

  -13/18 -1/2   2  -1/2   5/90

  7/36    7/4  -1   1/4  -1/36

  -5/90  -1/2   2  -1/2   5/90

  1/36    1/4  -1   7/4  -7/36

  -5/90  -1/2   2  -1/2   13/18

*/


/*****************

   bspline_c2p

   input 5 control points
   output 5 phantom points

*****************/

int bspline_c2p(float *v[3],float *p[3])
{
  float aa,bb,cc,dd,ee,ff,gg,hh,ii,kk,ll,mm,nn;
  float oo,pp,qq,rr,ss,tt,uu,vv,ww,xx,yy,zz;
  int i;

  aa=-13.0/18.0; bb=-0.5; cc=2.0; dd=-0.5; ee=5.0/90.0;
  ff=7.0/36.0; gg=7.0/4.0; hh=-1.0; ii=0.25; kk=-1.0/36.0;
  ll=-5.0/90.0; mm =-0.5; nn=2.0; oo=-0.5; pp=5.0/90.0;
  qq=1.0/36.0; rr=0.25; ss=-1.0; tt=7.0/4.0; uu=-7.0/36.0;
  vv=-5.0/90.0; ww=-0.5; xx=2.0; yy=-0.5; zz=13.0/18.0;

  for(i=0;i<3;i++) {
    p[0][i]=aa*v[0][i]+bb*v[1][i]+cc*v[2][i]+dd*v[3][i]+ee*v[4][i];
    p[1][i]=ff*v[0][i]+gg*v[1][i]+hh*v[2][i]+ii*v[3][i]+kk*v[4][i];
    p[2][i]=ll*v[0][i]+mm*v[1][i]+nn*v[2][i]+oo*v[3][i]+pp*v[4][i];
    p[3][i]=qq*v[0][i]+rr*v[1][i]+ss*v[2][i]+tt*v[3][i]+uu*v[4][i];
    p[4][i]=vv*v[0][i]+ww*v[1][i]+xx*v[2][i]+yy*v[3][i]+zz*v[4][i];
  }  
  return 0;
}
