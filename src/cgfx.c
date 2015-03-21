#include <stdio.h>
#include <math.h>
#include <string.h>

#include <OpenGL/gl.h>

#include "dino.h"
#include "cgfx.h"
#include "Cmalloc.h"
#include "mat.h"
#include "bspline.h"

#define CGFX_D2

extern int debug_mode;

/*
  these are the newer, vertex-array based, routines
*/

/********************************************

  cgfxSphereVA
  ------------

  adds a sphere to the given vertex array

**********************************************/

static cgfxVAField cgfx_sphere_field[5000];

int cgfxSphereVA(float radius, float p[3], float c[4], cgfxVA *va, int detail)
{
  double x,y,z;
  double xn,yn,zn;
  int i,j,k,l,inc_size;
  int step1,step2;
  double fact1,fact2;
  int p1,p2,p3,p4;
  cgfxVAField *vs=cgfx_sphere_field;
  int point_count;
  float o[4];

  if(detail<1)
    detail=1;
  if(detail>25)
    detail=25;
  
  if(radius<=0.0)
    return -1;

#ifdef CGFX_D2
  step1=detail*4;
  step2=detail*2;
#else
  step1=detail*4;
  step2=detail*2;
#endif

  fact1=M_PI*2.0/((double)step1);
  fact2=M_PI/((double)step2);
  
  point_count=(step2-1)*step1+2;

  // i=0 j=0
  vs[0].v[0]=0.0;
  vs[0].v[1]=1.0*radius;
  vs[0].v[2]=0.0;
  vs[0].n[0]=0.0;
  vs[0].n[1]=1.0;
  vs[0].n[2]=0.0;
  vs[0].c[0]=c[0];
  vs[0].c[1]=c[1];
  vs[0].c[2]=c[2];
  vs[0].c[3]=c[3];
  vs[point_count-1].v[0]=0.0;
  vs[point_count-1].v[1]=-1.0*radius;
  vs[point_count-1].v[2]=0.0;
  vs[point_count-1].n[0]=0.0;
  vs[point_count-1].n[1]=-1.0;
  vs[point_count-1].n[2]=0.0;
  vs[point_count-1].c[0]=c[0];
  vs[point_count-1].c[1]=c[1];
  vs[point_count-1].c[2]=c[2];
  vs[point_count-1].c[3]=c[3];
  for(i=0;i<step1;i++) {
    for(j=1;j<step2;j++) {
      x=sin(fact2*(double)j)*cos(M_PI+fact1*(double)i);
      y=cos(fact2*(double)j);
      z=sin(fact2*(double)j)*sin(M_PI+fact1*(double)i);
      vs[i*(step2-1)+j].v[0]=radius*(float)x;
      vs[i*(step2-1)+j].v[1]=radius*(float)y;
      vs[i*(step2-1)+j].v[2]=radius*(float)z;
      vs[i*(step2-1)+j].n[0]=(float)x;
      vs[i*(step2-1)+j].n[1]=(float)y;
      vs[i*(step2-1)+j].n[2]=(float)z;
      vs[i*(step2-1)+j].c[0]=c[0];
      vs[i*(step2-1)+j].c[1]=c[1];
      vs[i*(step2-1)+j].c[2]=c[2];
      vs[i*(step2-1)+j].c[3]=c[3];
    }
  }

  for(i=0;i<point_count;i++) {
    o[0]=vs[i].v[0];
    o[1]=vs[i].v[1];
    o[2]=vs[i].v[2];

    vs[i].v[0]=o[1]+p[0];
    vs[i].v[1]=o[2]+p[1];
    vs[i].v[2]=o[0]+p[2];
    
    o[0]=vs[i].n[0];
    o[1]=vs[i].n[1];
    o[2]=vs[i].n[2];
    vs[i].n[0]=o[1];
    vs[i].n[1]=o[2];
    vs[i].n[2]=o[0];
  }

  // MAXCALC IS BASED ON:
  //         top     bottom       middle
  inc_size=step1*3 + step1*3 + step1*(step2-2)*6;
  //  inc_size=((step2-2)*step1*2+2*step1)*3;

  if(va->p==NULL) {
    va->p=Crecalloc(NULL,inc_size,sizeof(cgfxVAField));
    va->count=0;
    va->max=inc_size;
  } else {
    if(va->count+inc_size>=va->max) {
      va->p=Crecalloc(va->p,va->max+inc_size,sizeof(cgfxVAField));
      va->max+=inc_size;
    }
  }

  // top / bottom
  j=1;
  for(i=0;i<step1;i++) {
    k=i+1;
    k%=step1;
    p1=0;
    p2=i*(step2-1)+j;
    p3=k*(step2-1)+j;
    memcpy(&va->p[va->count++],&vs[p1],sizeof(cgfxVAField));
    memcpy(&va->p[va->count++],&vs[p3],sizeof(cgfxVAField));
    memcpy(&va->p[va->count++],&vs[p2],sizeof(cgfxVAField));
  }  
  j=step2-1;
  for(i=0;i<step1;i++) {
    k=i+1;
    k%=step1;
    p1=point_count-1;
    p2=i*(step2-1)+j;
    p3=k*(step2-1)+j;
    memcpy(&va->p[va->count++],&vs[p1],sizeof(cgfxVAField));
    memcpy(&va->p[va->count++],&vs[p2],sizeof(cgfxVAField));
    memcpy(&va->p[va->count++],&vs[p3],sizeof(cgfxVAField));
  }  
  // middle
  for(i=0;i<step1;i++) {
    k=i+1;
    k%=step1;
    for(j=1;j<step2-1;j++) {
      l=j+1;
      l%=(step2);
      // add 2 triangles, counterclockwise
      p1=i*(step2-1)+j;
      p2=i*(step2-1)+l;
      p3=k*(step2-1)+j;
      p4=k*(step2-1)+l;
      // p1 p2 p3
      memcpy(&va->p[va->count++],&vs[p1],sizeof(cgfxVAField));
      memcpy(&va->p[va->count++],&vs[p3],sizeof(cgfxVAField));
      memcpy(&va->p[va->count++],&vs[p2],sizeof(cgfxVAField));
      // p2 p4 p3 
      memcpy(&va->p[va->count++],&vs[p2],sizeof(cgfxVAField));
      memcpy(&va->p[va->count++],&vs[p3],sizeof(cgfxVAField));
      memcpy(&va->p[va->count++],&vs[p4],sizeof(cgfxVAField));
    }
  }

  // fprintf(stderr,"%d %d\n",inc_size,va->count);

  return 0;
}


int cgfxSphereVA2(float radius, float p[3], float c[4], cgfxVA *va, int detail)
{
  double x,y,z,v,w,step,step2;
  double xn,yn,zn;
  int inc_size,j,i;
  float v1[3],v2[3],v3[3],v4[3];
  float n1[3],n2[3],n3[3],n4[3];
  cgfxVAField *ov;

  if(detail<1)
    detail=1;
  if(detail>90)
    detail=90;
  
  if(radius<=0.0)
    return -1;

#ifdef CGFX_D2
  detail*=4;
#else
  detail*=2;
#endif

  step=M_PI*2.0/(double)(detail);
  step2=M_PI*2.0/(double)(detail);

  // MAXCALC is determined by dry-run
  inc_size=0;
  for(w=0.0;w<M_PI*2+step;w+=step) {
    inc_size+=6;
    for(v=0.0;v<M_PI_2;v+=step2)
      inc_size+=12;
  }
  inc_size+=256;

  if(va->p==NULL) {
    va->p=Crecalloc(NULL,inc_size,sizeof(cgfxVAField));
    va->count=0;
    va->max=inc_size;
  } else {
    va->p=Crecalloc(va->p,va->count+inc_size,sizeof(cgfxVAField));
    va->max=va->count+inc_size;
  }
  
  for(w=0.0;w<M_PI*2+step;w+=step) {
    for(j=0,v=0.0;v<M_PI_2;v+=step2,j++) {
      n3[0]=sin(w+step)*cos(v);
      n3[1]=cos(w+step)*cos(v);
      n3[2]=sin(v);
      v3[0]=n3[0]*radius+p[0];
      v3[1]=n3[1]*radius+p[1];
      v3[2]=n3[2]*radius+p[2];

      n4[0]=sin(w)*cos(v);
      n4[1]=cos(w)*cos(v);
      n4[2]=sin(v);
      v4[0]=n4[0]*radius+p[0];
      v4[1]=n4[1]*radius+p[1];
      v4[2]=n4[2]*radius+p[2];

      if(j>0) {
	/* combine v1,v2,v3 */
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n1[i];
	  va->p[va->count].v[i]=v1[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n2[i];
	  va->p[va->count].v[i]=v2[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n3[i];
	  va->p[va->count].v[i]=v3[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	/* combine v3,v2,v4 */
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n3[i];
	  va->p[va->count].v[i]=v3[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n2[i];
	  va->p[va->count].v[i]=v2[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n4[i];
	  va->p[va->count].v[i]=v4[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;

      }
      for(i=0;i<3;i++) {
	n1[i]=n3[i];
	n2[i]=n4[i];
	v1[i]=v3[i];
	v2[i]=v4[i];
      }

    }
    n3[0]=0.0;
    n3[1]=0.0;
    n3[2]=1.0;
    v3[0]=p[0];
    v3[1]=p[1];
    v3[2]=p[2]+radius;
    for(i=0;i<3;i++) {
      va->p[va->count].n[i]=n1[i];
      va->p[va->count].v[i]=v1[i];
      va->p[va->count].c[i]=c[i];
    }
    va->p[va->count].c[3]=c[3];
    va->count++;
    for(i=0;i<3;i++) {
      va->p[va->count].n[i]=n2[i];
      va->p[va->count].v[i]=v2[i];
      va->p[va->count].c[i]=c[i];
    }
    va->p[va->count].c[3]=c[3];
    va->count++;
    for(i=0;i<3;i++) {
      va->p[va->count].n[i]=n3[i];
      va->p[va->count].v[i]=v3[i];
      va->p[va->count].c[i]=c[i];
    }
    va->p[va->count].c[3]=c[3];
    va->count++;


    for(v=0.0;v<M_PI_2;v+=step2) {
      glNormal3d(xn,yn,zn);
      glVertex3d(x,y,z);
      n3[0]=sin(w)*cos(v);
      n3[1]=cos(w)*cos(v);
      n3[2]=-sin(v);
      v3[0]=n3[0]*radius+p[0];
      v3[1]=n3[1]*radius+p[1];
      v3[2]=n3[2]*radius+p[2];

      n4[0]=sin(w+step)*cos(v);
      n4[1]=cos(w+step)*cos(v);
      n4[2]=-sin(v);
      v4[0]=n4[0]*radius+p[0];
      v4[1]=n4[1]*radius+p[1];
      v4[2]=n4[2]*radius+p[2];

      if(j>0) {
	/* combine v1,v2,v3 */
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n1[i];
	  va->p[va->count].v[i]=v1[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n2[i];
	  va->p[va->count].v[i]=v2[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n3[i];
	  va->p[va->count].v[i]=v3[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	/* combine v3,v2,v4 */
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n3[i];
	  va->p[va->count].v[i]=v3[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n2[i];
	  va->p[va->count].v[i]=v2[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;
	for(i=0;i<3;i++) {
	  va->p[va->count].n[i]=n4[i];
	  va->p[va->count].v[i]=v4[i];
	  va->p[va->count].c[i]=c[i];
	}
	va->p[va->count].c[3]=c[3];
	va->count++;

      }
      for(i=0;i<3;i++) {
	n1[i]=n3[i];
	n2[i]=n4[i];
	v1[i]=v3[i];
	v2[i]=v4[i];
      }
    }
    n3[0]=0.0;
    n3[1]=0.0;
    n3[2]=-1.0;
    v3[0]=p[0];
    v3[1]=p[1];
    v3[2]=p[2]-radius;
    for(i=0;i<3;i++) {
      va->p[va->count].n[i]=n1[i];
      va->p[va->count].v[i]=v1[i];
      va->p[va->count].c[i]=c[i];
    }
    va->p[va->count].c[3]=c[3];
    va->count++;
    for(i=0;i<3;i++) {
      va->p[va->count].n[i]=n2[i];
      va->p[va->count].v[i]=v2[i];
      va->p[va->count].c[i]=c[i];
    }
    va->p[va->count].c[3]=c[3];
    va->count++;
    for(i=0;i<3;i++) {
      va->p[va->count].n[i]=n3[i];
      va->p[va->count].v[i]=v3[i];
      va->p[va->count].c[i]=c[i];
    }
    va->p[va->count].c[3]=c[3];
    va->count++;
  }
  
  return 0;
}

/******************
 
   construct a cylinder from beg to end of radius rad
   type can be blunt, round or cap

*******************/

int cgfxGenCylinder(cgfxVA *va, float *fbeg, float *fend, float frad, float fsti, float fsto, int detail, int type,float *fcol)
{
  cgfxVAField *of;
  int num,add,num2;
  double vdiff[3],length,axis[3],mat[16],dotp,vcyl[]={0.0,0.0,1.0},angle;
  double vn[360][2],vinit[360][2];
  double v1[4],v2[4],v3[4],v4[4],v5[4],v6[4],v7[4],v8[4];
  double n1[4],n2[4],n3[4],n4[4],n5[4],n6[4],n7[4],n8[4];
  double arcf,arcc;
  int arci;
  double beg[3],end[3],rad,st0,st1,stt,sts,col[4];
  int stc,sti;
  int roundi;
  double rx,ry,rz,sv1,sv2;

  beg[0]=fbeg[0];
  beg[1]=fbeg[1];
  beg[2]=fbeg[2];
  end[0]=fend[0];
  end[1]=fend[1];
  end[2]=fend[2];
  rad=frad;
  col[0]=fcol[0];
  col[1]=fcol[1];
  col[2]=fcol[2];
  col[3]=fcol[3];

  st0=fsto;
  st1=fsti;

  // difference vector
  vdiff[0]=end[0]-beg[0];
  vdiff[1]=end[1]-beg[1];
  vdiff[2]=end[2]-beg[2];

  // length of cylinder
  length=sqrt(vdiff[0]*vdiff[0]+vdiff[1]*vdiff[1]+vdiff[2]*vdiff[2]);
  
  // how many gaps ?
  if(st0==0) {
    st0=0.0;
    st1=length;
    stt=length;
    stc=1;
  } else {
    st0=fsto; st1=fsti; stt=st0+st1;
    stc=1+(int)floor(length/stt);
  }
  
  arcf=M_PI_2/(double)detail;

  // precalculate the points on the circle
  for(arci=0, arcc=0.0; arci<=detail; arci++, arcc+=arcf) {
    vn[arci+detail*0][0]=cos(arcc);
    vn[arci+detail*0][1]=sin(arcc);
    vn[arci+detail*1][0]=cos(arcc+M_PI_2);
    vn[arci+detail*1][1]=sin(arcc+M_PI_2);
    vn[arci+detail*2][0]=cos(arcc+M_PI);
    vn[arci+detail*2][1]=sin(arcc+M_PI);
    vn[arci+detail*3][0]=cos(arcc+M_PI+M_PI_2);
    vn[arci+detail*3][1]=sin(arcc+M_PI+M_PI_2);
    vinit[arci+detail*0][0]=rad*vn[arci+detail*0][0];
    vinit[arci+detail*0][1]=rad*vn[arci+detail*0][1];
    vinit[arci+detail*1][0]=rad*vn[arci+detail*1][0];
    vinit[arci+detail*1][1]=rad*vn[arci+detail*1][1];
    vinit[arci+detail*2][0]=rad*vn[arci+detail*2][0];
    vinit[arci+detail*2][1]=rad*vn[arci+detail*2][1];
    vinit[arci+detail*3][0]=rad*vn[arci+detail*3][0];
    vinit[arci+detail*3][1]=rad*vn[arci+detail*3][1];
  }

  // the number of the new triangles for one cylinder
  num=(detail*4)*2;
  if(type==CGFX_CAP)
    num+=(detail*4+1)*2;
  else {
    if(type==CGFX_ROUND_BEGIN)
      num+=(detail)*(detail*4)*2; 
    if(type==CGFX_ROUND_END)
      num+=(detail)*(detail*4)*2; 
  }
  // multiply with the number of cylinders to be drawn
  num*=stc;

  // tri -> points
  num*=3;

  // ?
  num*=2;

  // check limits of vertex array
  if(va->count+num>=va->max) {
    if(num<256)
      add=256;
    else
      add=num;
    va->p=Crecalloc(va->p,va->max+add,sizeof(cgfxVAField));
    va->max+=add;
  }

  // calculate matrix to transform cylinder from z-axis to real orientation
    
  matCalcCross(vcyl,vdiff,axis);
    
  if(axis[0]==0.0 && axis[1]==0.0)
    if(vdiff[2]>=0)
      angle=0.0;
    else
      angle=180.0;
  else {
    dotp=matCalcDot(vcyl,vdiff);
    angle=180.0*acos(dotp/length)/M_PI;
  }

  matMakeRotMat(angle, axis[0], axis[1], axis[2], mat);

  // generate cylinder along positive z, apply matrix to each point
  for(sti=0,sts=0.0;sti<stc;sti++,sts+=stt) {
    for(arci=0; arci<detail*4; arci++) {
      n1[0]=vn[arci][0];
      n1[1]=vn[arci][1];
      n1[2]=0.0;
      n1[3]=1.0;
      v1[0]=vinit[arci][0];
      v1[1]=vinit[arci][1];
      v1[2]=sts;
      v1[3]=1.0;
      n2[0]=vn[arci+1][0];
      n2[1]=vn[arci+1][1];
      n2[2]=0.0;
      n2[3]=1.0;
      v2[0]=vinit[arci+1][0];
      v2[1]=vinit[arci+1][1];
      v2[2]=sts;
      v2[3]=1.0;
      n3[0]=vn[arci][0];
      n3[1]=vn[arci][1];
      n3[2]=0.0;
      n3[3]=1.0;
      v3[0]=vinit[arci][0];
      v3[1]=vinit[arci][1];
      v3[2]=sts+st1;
      if(v3[2]<-length)
	v3[2]=-length;
      v3[3]=1.0;
      n4[0]=vn[arci+1][0];
      n4[1]=vn[arci+1][1];
      n4[2]=0.0;
      n4[3]=1.0;
      v4[0]=vinit[arci+1][0];
      v4[1]=vinit[arci+1][1];
      v4[2]=sts+st1;
      if(v4[2]<-length)
	v4[2]=-length;
      v4[3]=1.0;

      matMultVM(n1,mat,n5);
      matMultVM(v1,mat,v5);
      matAddV(v5,beg);
      matMultVM(n2,mat,n6);
      matMultVM(v2,mat,v6);
      matAddV(v6,beg);
      matMultVM(n3,mat,n7);
      matMultVM(v3,mat,v7);
      matAddV(v7,beg);
      matMultVM(n4,mat,n8);
      matMultVM(v4,mat,v8);
      matAddV(v8,beg);

      // 1-3-2  2-3-4
      matCopyVdf(col,va->p[va->count].c);
      va->p[va->count].c[3]=col[3];
      matCopyVdf(n5,va->p[va->count].n);
      matCopyVdf(v5,va->p[va->count].v);
      va->count++;
      matCopyVdf(col,va->p[va->count].c);
      va->p[va->count].c[3]=col[3];
      matCopyVdf(n6,va->p[va->count].n);
      matCopyVdf(v6,va->p[va->count].v);
      va->count++;
      matCopyVdf(col,va->p[va->count].c);
      va->p[va->count].c[3]=col[3];
      matCopyVdf(n7,va->p[va->count].n);
      matCopyVdf(v7,va->p[va->count].v);
      va->count++;

      matCopyVdf(col,va->p[va->count].c);
      va->p[va->count].c[3]=col[3];
      matCopyVdf(n6,va->p[va->count].n);
      matCopyVdf(v6,va->p[va->count].v);
      va->count++;
      matCopyVdf(col,va->p[va->count].c);
      va->p[va->count].c[3]=col[3];
      matCopyVdf(n8,va->p[va->count].n);
      matCopyVdf(v8,va->p[va->count].v);
      va->count++;
      matCopyVdf(col,va->p[va->count].c);
      va->p[va->count].c[3]=col[3];
      matCopyVdf(n7,va->p[va->count].n);
      matCopyVdf(v7,va->p[va->count].v);
      va->count++;

      if(type==CGFX_CAP) {
	// cap 1 2 middle  4 3 middle
	n4[0]=0.0;
	n4[1]=0.0;
	n4[2]=-1.0;
	matMultVM(n4,mat,n1);
	n5[0]=0.0;
	n5[1]=0.0;
	n5[2]=1.0;
	matMultVM(n5,mat,n2);
	n6[0]=0.0;
	n6[1]=0.0;
	n6[2]=sts; 
	matMultVM(n6,mat,n3);
	matAddV(n3,beg);
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n1,va->p[va->count].n);
	matCopyVdf(v6,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n1,va->p[va->count].n);
	matCopyVdf(v5,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n1,va->p[va->count].n);
	matCopyVdf(n3,va->p[va->count].v);
	va->count++;
	n6[2]=sts+st1;
	if(n6[2]>length)
	  n6[2]=length;
	matMultVM(n6,mat,n3);
	matAddV(n3,beg);
	
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n2,va->p[va->count].n);
	matCopyVdf(v7,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n2,va->p[va->count].n);
	matCopyVdf(v8,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n2,va->p[va->count].n);
	matCopyVdf(n3,va->p[va->count].v);
	va->count++;
      } else if(type==CGFX_ROUND_END) {
	/*
	  use detail steps
	*/
	v1[0]=v3[0]; v1[1]=v3[1]; v1[2]=v3[2];
	n1[0]=n3[0]; n1[1]=n3[1]; n1[2]=n3[2];
	v2[0]=v4[0]; v2[1]=v4[1]; v2[2]=v4[2];
	n2[0]=n4[0]; n2[1]=n4[1]; n2[2]=n4[2];
	sv1=v3[2];
	sv2=v4[2];
	for(roundi=1;roundi<detail;roundi++) {
	  angle=M_PI_2*((double)roundi)/((double)detail);
	  rx=cos(angle);
	  ry=cos(angle);
	  rz=sin(angle);

	  v3[0]=v1[0]*rx; v3[1]=v1[1]*ry; v3[2]=sv1+rz*rad;
	  n3[0]=n1[0]*rx; n3[1]=n1[1]*ry; n3[2]=1.0*rz;
	  v4[0]=v2[0]*rx; v4[1]=v2[1]*ry; v4[2]=sv2+rz*rad;
	  n4[0]=n2[0]*rx; n4[1]=n2[1]*ry; n4[2]=1.0*rz;
	  
	  matMultVM(n1,mat,n5);
	  matMultVM(v1,mat,v5);
	  matAddV(v5,beg);
	  matMultVM(n2,mat,n6);
	  matMultVM(v2,mat,v6);
	  matAddV(v6,beg);
	  matMultVM(n3,mat,n7);
	  matMultVM(v3,mat,v7);
	  matAddV(v7,beg);
	  matMultVM(n4,mat,n8);
	  matMultVM(v4,mat,v8);
	  matAddV(v8,beg);

	  v1[0]=v3[0]; v1[1]=v3[1]; v1[2]=v3[2];
	  n1[0]=n3[0]; n1[1]=n3[1]; n1[2]=n3[2];
	  v2[0]=v4[0]; v2[1]=v4[1]; v2[2]=v4[2];
	  n2[0]=n4[0]; n2[1]=n4[1]; n2[2]=n4[2];
	  
	  // 1-3-2  2-3-4
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n5,va->p[va->count].n);
	  matCopyVdf(v5,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n6,va->p[va->count].n);
	  matCopyVdf(v6,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n7,va->p[va->count].n);
	  matCopyVdf(v7,va->p[va->count].v);
	  va->count++;
	  
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n6,va->p[va->count].n);
	  matCopyVdf(v6,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n8,va->p[va->count].n);
	  matCopyVdf(v8,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n7,va->p[va->count].n);
	  matCopyVdf(v7,va->p[va->count].v);
	  va->count++;
	}
	
	v3[0]=0.0; v3[1]=0.0; v3[2]=sv1+rad;
	n3[0]=0.0; n3[1]=0.0; n3[2]=1.0;

	matMultVM(n1,mat,n5);
	matMultVM(v1,mat,v5);
	matAddV(v5,beg);
	matMultVM(n2,mat,n6);
	matMultVM(v2,mat,v6);
	matAddV(v6,beg);
	matMultVM(n3,mat,n7);
	matMultVM(v3,mat,v7);
	matAddV(v7,beg);
	matMultVM(n4,mat,n8);
	matMultVM(v4,mat,v8);
	matAddV(v8,beg);
	
	// 1-3-2  2-3-4
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n5,va->p[va->count].n);
	matCopyVdf(v5,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n6,va->p[va->count].n);
	matCopyVdf(v6,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n7,va->p[va->count].n);
	matCopyVdf(v7,va->p[va->count].v);
	va->count++;
      } else if(type==CGFX_ROUND_BEGIN) {
	/*
	  use detail steps
	*/
	v3[0]=v1[0]; v3[1]=v1[1]; v3[2]=v1[2];
	n3[0]=n1[0]; n3[1]=n1[1]; n3[2]=n1[2];
	v4[0]=v2[0]; v4[1]=v2[1]; v4[2]=v2[2];
	n4[0]=n2[0]; n4[1]=n2[1]; n4[2]=n2[2];
	sv1=v1[2];
	sv2=v2[2];
	for(roundi=1;roundi<detail;roundi++) {
	  angle=M_PI_2*((double)roundi)/((double)detail);
	  rx=cos(angle);
	  ry=cos(angle);
	  rz=sin(angle);

	  v1[0]=v3[0]*rx; v1[1]=v3[1]*ry; v1[2]=sv1-rz*rad;
	  n1[0]=n3[0]*rx; n1[1]=n3[1]*ry; n1[2]=-1.0*rz;
	  v2[0]=v4[0]*rx; v2[1]=v4[1]*ry; v2[2]=sv2-rz*rad;
	  n2[0]=n4[0]*rx; n2[1]=n4[1]*ry; n2[2]=-1.0*rz;
	  
	  matMultVM(n1,mat,n5);
	  matMultVM(v1,mat,v5);
	  matAddV(v5,beg);
	  matMultVM(n2,mat,n6);
	  matMultVM(v2,mat,v6);
	  matAddV(v6,beg);
	  matMultVM(n3,mat,n7);
	  matMultVM(v3,mat,v7);
	  matAddV(v7,beg);
	  matMultVM(n4,mat,n8);
	  matMultVM(v4,mat,v8);
	  matAddV(v8,beg);

	  v3[0]=v1[0]; v3[1]=v1[1]; v3[2]=v1[2];
	  n3[0]=n1[0]; n3[1]=n1[1]; n3[2]=n1[2];
	  v4[0]=v2[0]; v4[1]=v2[1]; v4[2]=v2[2];
	  n4[0]=n2[0]; n4[1]=n2[1]; n4[2]=n2[2];
	  
	  // 1-3-2  2-3-4
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n5,va->p[va->count].n);
	  matCopyVdf(v5,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n6,va->p[va->count].n);
	  matCopyVdf(v6,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n7,va->p[va->count].n);
	  matCopyVdf(v7,va->p[va->count].v);
	  va->count++;
	  
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n6,va->p[va->count].n);
	  matCopyVdf(v6,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n8,va->p[va->count].n);
	  matCopyVdf(v8,va->p[va->count].v);
	  va->count++;
	  matCopyVdf(col,va->p[va->count].c);
	  va->p[va->count].c[3]=col[3];
	  matCopyVdf(n7,va->p[va->count].n);
	  matCopyVdf(v7,va->p[va->count].v);
	  va->count++;
	}

	v1[0]=0.0; v1[1]=0.0; v1[2]=sv1-rad;
	n1[0]=0.0; n1[1]=0.0; n1[2]=-1.0;

	matMultVM(n1,mat,n5);
	matMultVM(v1,mat,v5);
	matAddV(v5,beg);
	matMultVM(n2,mat,n6);
	matMultVM(v2,mat,v6);
	matAddV(v6,beg);
	matMultVM(n3,mat,n7);
	matMultVM(v3,mat,v7);
	matAddV(v7,beg);
	matMultVM(n4,mat,n8);
	matMultVM(v4,mat,v8);
	matAddV(v8,beg);
	
	// 1-3-2
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n5,va->p[va->count].n);
	matCopyVdf(v5,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n6,va->p[va->count].n);
	matCopyVdf(v6,va->p[va->count].v);
	va->count++;
	matCopyVdf(col,va->p[va->count].c);
	va->p[va->count].c[3]=col[3];
	matCopyVdf(n7,va->p[va->count].n);
	matCopyVdf(v7,va->p[va->count].v);
	va->count++;
      }
    }
  }

  return 0;
}
static  cgfxProfile pro1, pro2, pro3, pro4, pro5, pro6, pro_last;
static Render tmp_render1, tmp_render2;

int cgfxGenHSC(cgfxVA *va, cgfxSplinePoint *sp, int pc, Render *render)
{
  int i,k,ks,maxp;
  cgfxPoint *cp1,*cp2;
  float cylp1[3],cylp2[3],cyld[3];
  Render *render1=&tmp_render1,*render2=&tmp_render2;  

  int morph,sf;
  float frac,bw_save;

  maxp=0;
  for(i=0;i<pc;i++)
    maxp+=sp[i].pc;

  if(render->mode==RENDER_HSC || render->mode==RENDER_TUBE) {

    // MAXCALC is based on dry run
    va->max=0;
    cgfxGenProfile(&pro1, CGFX_TUBE, render);
    for(i=0;i<pc-1;i++)
      for(k=0;k<=sp[i].pc;k++) {
	va->max+=pro1.pc*6;
      }
    va->max+=pro1.pc*20;
    
    va->count=0;
    va->p=Crecalloc(NULL,va->max, sizeof(cgfxVAField));

    memcpy(render2,render,sizeof(Render));
    if(render->cgfx_flag & CGFX_USE_RAD) {
      render2->helix_width*=sp[0].rad;
      render2->strand_width*=sp[0].rad;
      render2->helix_thickness*=sp[0].rad;
      render2->strand_thickness*=sp[0].rad;
      render2->tube_width*=sp[0].rad;
    }
    bw_save=render2->tube_width;
    
    //    for(i=0;i<pc-1;i++) {
    debmsg("GenHSC: spline extrusion");
    for(i=0;i<pc-1;i++) {
      memcpy(render1,render2,sizeof(Render));
      memcpy(render2,render,sizeof(Render));
      if(render->cgfx_flag & CGFX_USE_RAD) {
	render2->helix_width*=sp[i+1].rad;
	render2->strand_width*=sp[i+1].rad;
	render2->helix_thickness*=sp[i+1].rad;
	render2->strand_thickness*=sp[i+1].rad;
	render2->tube_width*=sp[i+1].rad;
      }
      if(render->mode==RENDER_HSC) {

	/* CYLINDRICAL HELIX */
	if(sp[i+1].id==CGFX_HELIX && render->helix_method==1) {
	  cgfxGenProfile(&pro1, sp[i].id, render);
	  for(k=0;k<=sp[i].pc;k++) {
	    cgfxHSCTransform(&pro1, &sp[i].p[k], &pro5);
	    cgfxConnectProfile(va,&pro_last,&pro5,0);
	    memcpy(&pro_last,&pro5,sizeof(cgfxProfile));
	  }

	  ks=i;
	  while(sp[++i].id==CGFX_HELIX && i<pc-1) {}
	  if(i<pc) {
	    //	    cgfxGenProfile(&pro1, sp[ks].id, render);
	    cgfxHSCTransform(&pro1, &sp[i-1].p[0], &pro_last);
	    for(k=1;k<=sp[i-1].pc;k++) {
	      cgfxHSCTransform(&pro1, &sp[i-1].p[k], &pro5);
	      cgfxConnectProfile(va,&pro_last,&pro5,0);
	      memcpy(&pro_last,&pro5,sizeof(cgfxProfile));
	    }
	  }

	  k=ks;
	  cylp1[0]=(sp[k+1].v[0]+sp[k+3].v[0])*0.5;
	  cylp1[1]=(sp[k+1].v[1]+sp[k+3].v[1])*0.5;
	  cylp1[2]=(sp[k+1].v[2]+sp[k+3].v[2])*0.5;
	  cylp2[0]=(sp[i-1].v[0]+sp[i-3].v[0])*0.5;
	  cylp2[1]=(sp[i-1].v[1]+sp[i-3].v[1])*0.5;
	  cylp2[2]=(sp[i-1].v[2]+sp[i-3].v[2])*0.5;
	  cyld[0]=cylp2[0]-cylp1[0];
	  cyld[1]=cylp2[1]-cylp1[1];
	  cyld[2]=cylp2[2]-cylp1[2];
	  matfNormalize(cyld,cyld);
	  cylp1[0]-=2.0*cyld[0];
	  cylp1[1]-=2.0*cyld[1];
	  cylp1[2]-=2.0*cyld[2];
	  cylp2[0]+=2.0*cyld[0];
	  cylp2[1]+=2.0*cyld[1];
	  cylp2[2]+=2.0*cyld[2];


	  cgfxGenCylinder(va,cylp1,cylp2,render->helix_width*2.0,
			  1,0,
			  render->detail1,CGFX_CAP,sp[k].colp[0]);

	  if(i>=pc)
	    break;

	} // CYLINDRICAL HELIX END

	/* determine i and i+1 profile */
	if(sp[i].id==sp[i+1].id) {
	  morph=1;
	  cgfxGenProfile(&pro2, sp[i].id, render1);
	  cgfxGenProfile(&pro1, sp[i].id, render2);
	  //morph=0;
	  //cgfxGenProfile(&pro1, sp[i].id, render);
	} else {
	  morph=1;
	  if(sp[i+1].id==CGFX_STRAND2) {
	    cgfxGenProfile(&pro2, CGFX_STRAND1, render1);
	    cgfxGenProfile(&pro1, CGFX_STRAND2, render2);
	  } else if(sp[i].id !=CGFX_STRAND && sp[i+1].id==CGFX_STRAND) {
	    cgfxGenProfile(&pro2, sp[i].id, render1);
	    cgfxGenProfile(&pro1, sp[i].id, render2);
	    //morph=0;
	    //cgfxGenProfile(&pro1, sp[i].id, render);
	  } else {
	    cgfxGenProfile(&pro2, sp[i].id, render1);
	    cgfxGenProfile(&pro1, sp[i+1].id, render2);
	  }
	}
      } else {
	morph=1;
	cgfxGenProfile(&pro2, CGFX_TUBE, render1);
	cgfxGenProfile(&pro1, CGFX_TUBE, render2);
	/*
	morph=1;
	bw_save=render->bond_width;
	render->bond_width=sp[i].rad;
	cgfxGenProfile(&pro2, CGFX_TUBE, render);
	render->bond_width=sp[i+1].rad;
	cgfxGenProfile(&pro1, CGFX_TUBE, render);
	render->bond_width=bw_save;
	*/
      }

      // go through interpolated points
      if(i==0) { // very first round
	ks=1;
	if(morph) {
	  pro3.f=0.0;
	  pro4.f=1.0;
	  cgfxHSCTransform(&pro1, &sp[i].p[0], &pro3);
	  cgfxHSCTransform(&pro2, &sp[i].p[0], &pro4);
	  cgfxMorphProfile(&pro3,&pro4, &pro_last);
	} else {
	  cgfxHSCTransform(&pro1, &sp[i].p[0], &pro_last);
	}
      } else {
	ks=0;
      }
      for(k=ks;k<=sp[i].pc;k++) {
	
	if(morph) {
	  frac=(float)k/(float)(sp[i].pc);
	  // set factors
	  pro3.f=frac;
	  pro4.f=1.0-pro3.f;
	  cgfxHSCTransform(&pro1, &sp[i].p[k], &pro3);
	  cgfxHSCTransform(&pro2, &sp[i].p[k], &pro4);
	  cgfxMorphProfile(&pro3,&pro4, &pro5);
	} else {
	  cgfxHSCTransform(&pro1, &sp[i].p[k], &pro5);
	}

	if(cgfxConnectProfile(va,&pro_last,&pro5,0)!=0) {
	}
	memcpy(&pro_last,&pro5,sizeof(cgfxProfile));
      }
      
    }

    if(render->cgfx_flag & CGFX_HSC_CAP) {
      debmsg("GenHSC: adding endpoints");
      cgfxSphereVA(bw_save,sp[0].v,sp[0].colp[0],va,render->detail1);
      cgfxSphereVA(render2->tube_width,sp[pc-1].v,sp[pc-1].colp[0],va,render->detail1);
    }

    debmsg("GenHSC: adding additional elements");
    if(render->mode==RENDER_HSC) {
      for(i=0;i<pc;i++) {
	if(sp[i].id==CGFX_NA) {
	  if(render->na_method==0) {
	    cgfxGenNA(va, &sp[i], render);
	  } else {
	    cgfxGenNA2(va,&sp[i], render);
	  }
	}
      }    
    }

  } else if(render->mode==RENDER_SLINE) {
    // MAXCALC is based on
    // point_count*2
    va->max=maxp*2;
    va->count=0;
    
    va->p=Crecalloc(NULL,va->max, sizeof(cgfxVAField));
    
    for(i=0;i<pc-1;i++) {
      for(k=0;k<sp[i].pc;k++) {
	if(k+1<sp[i].pc) {
	  cp1=sp[i].p+k;
	  cp2=sp[i].p+k+1;
	} else {
	  if(i+1<pc) {
	    cp1=sp[i].p+k;
	    cp2=sp[i+1].p+0;
	  } else {
	    break;
	  }
	}
	
	va->p[va->count].v[0]=cp1->v[0];
	va->p[va->count].v[1]=cp1->v[1];
	va->p[va->count].v[2]=cp1->v[2];
	va->p[va->count].c[0]=cp1->col[0][0];
	va->p[va->count].c[1]=cp1->col[0][1];
	va->p[va->count].c[2]=cp1->col[0][2];
	va->p[va->count].c[3]=cp1->col[0][3];
	va->count++;
	va->p[va->count].v[0]=cp2->v[0];
	va->p[va->count].v[1]=cp2->v[1];
	va->p[va->count].v[2]=cp2->v[2];
	va->p[va->count].c[0]=cp2->col[0][0];
	va->p[va->count].c[1]=cp2->col[0][1];
	va->p[va->count].c[2]=cp2->col[0][2];
	va->p[va->count].c[3]=cp2->col[0][3];
	va->count++;
      }
    }

  }
  return 0;
}

int cgfxConnectProfile(cgfxVA *va, cgfxProfile *p1, cgfxProfile *p2, int f)
{
  int i,j,k,l1,l2,o,o2;
  float d[3],d1,d2;
  float v1[4],v2[4];
  
  o=0;

  if(p1->p[0].v[0]==p2->p[0].v[0] &&
     p1->p[0].v[1]==p2->p[0].v[1] &&
     p1->p[0].v[2]==p2->p[0].v[2]) {
    return -1;
  }
  
  v1[0]=p1->p[0].n[0];
  v1[1]=p1->p[0].n[1];
  v1[2]=p1->p[0].n[2];
  v1[3]=1.0;
  d1=0;
  for(i=0;i<p1->pc;i++) {
    v2[0]=p2->p[i].n[0];
    v2[1]=p2->p[i].n[1];
    v2[2]=p2->p[i].n[2];
    v2[3]=1.0;
    if((d2=matfCalcDot(v1,v2))>d1) {
      d1=d2;
      o=i;
    }
  }

  //  fprintf(stderr,"%f\n",matfCalcDot(p1->p[0].d,p2->p[0].d));

  o=0;  // still does not work correctly, i.e. gives jagged edges for strands

  o=p1->pc-2;  // EMPIRICAL ??? why does this work ???

  // memory check
  if((va->count+(p1->pc-1)*6)>=va->max) {
    va->max+=1024;
    va->p=Crecalloc(va->p,va->max,sizeof(cgfxVAField));
  }

  //fprintf(stderr,"[%d %d",va->count,va->max);

  for(i=0;i<p1->pc-1;i++) {
    k=i+1;
    l1=i+o;
    l2=k+o;
    l1%=p1->pc-1;
    l2%=p1->pc-1;
    cgfxCopyPVa(&p1->p[i],&va->p[va->count]);
    va->count++;
    cgfxCopyPVa(&p2->p[l2],&va->p[va->count]);
    va->count++;
    cgfxCopyPVa(&p2->p[l1],&va->p[va->count]);
    va->count++;
    cgfxCopyPVa(&p1->p[i],&va->p[va->count]);
    va->count++;
    cgfxCopyPVa(&p1->p[k],&va->p[va->count]);
    va->count++;
    cgfxCopyPVa(&p2->p[l2],&va->p[va->count]);
    va->count++;
  }

  //fprintf(stderr,"]\n");

  return 0;
}

/*
  utility routines
 */


int cgfxCopyPVa(cgfxPoint *p, cgfxVAField *f)
{
  f->v[0]=p->v[0];
  f->v[1]=p->v[1];
  f->v[2]=p->v[2];
  f->n[0]=p->n[0];
  f->n[1]=p->n[1];
  f->n[2]=p->n[2];
  p->fc=0;
  f->c[0]=p->col[p->fc][0];
  f->c[1]=p->col[p->fc][1];
  f->c[2]=p->col[p->fc][2];
  f->c[3]=p->col[p->fc][3];
  return 0;
}


int cgfxHSCTransform(cgfxProfile *profile, cgfxPoint *point, cgfxProfile *p2)
{
  int i;
  double omega, phi, kappa;
  double v1[4],v2[4],v3[4],mat[16],*vp1[3],*vp2[3],mati[16],mati2[16];
  double r1[]={0,1,0}, r2[]={0,0,1}, r3[]={1,0,0};
  double id[]={1.0,0.0,0.0,0.0,
	       0.0,1.0,0.0,0.0,
	       0.0,0.0,1.0,0.0,
	       0.0,0.0,0.0,1.0};
  int col;

  p2->pc=profile->pc;

  // generate the rotation matrix
  v1[0]=point->n[0];
  v1[1]=point->n[1];
  v1[2]=point->n[2];
  v2[0]=point->d[0];
  v2[1]=point->d[1];
  v2[2]=point->d[2];

  matCalcCross(v1,v2,v3);

  matNormalize(v1,v1);
  matNormalize(v2,v2);
  matNormalize(v3,v3);

  vp1[0]=v1;
  vp1[1]=v2;
  vp1[2]=v3;

  vp2[0]=r1;
  vp2[1]=r2;
  vp2[2]=r3;

  matVectToRot(vp1,vp2,mat);

  if(matInverse2(mat,mati)!=0) {
    fprintf(stderr,"error in matInverse2\n");
    return -1;
  }


  matTranspose(mati,mati);
  //  matTranspose(mati,mati);
  
  /*
  fprintf(stderr,"%f %f %f\n%f %f %f\n%f %f %f\n\n\n",
  	  mat[0],mat[1],mat[2],mat[4],mat[5],mat[6],mat[8],mat[9],mat[10]);
  */

  for(i=0;i<profile->pc; i++) {
    v1[0]=profile->p[i].v[0];
    v1[1]=profile->p[i].v[1];
    v1[2]=profile->p[i].v[2];
    v1[3]=1.0;
    matMultMV(mat,v1,v2);
    p2->p[i].v[0]=(float)v2[0]+point->v[0];
    p2->p[i].v[1]=(float)v2[1]+point->v[1];
    p2->p[i].v[2]=(float)v2[2]+point->v[2];

    v1[0]=(double)profile->p[i].n[0];
    v1[1]=(double)profile->p[i].n[1];
    v1[2]=(double)profile->p[i].n[2];
    v1[3]=1.0;
    matMultMV(mati,v1,v2);
    p2->p[i].n[0]=(float)v2[0];
    p2->p[i].n[1]=(float)v2[1];
    p2->p[i].n[2]=(float)v2[2];

    v1[0]=(double)profile->p[i].d[0];
    v1[1]=(double)profile->p[i].d[1];
    v1[2]=(double)profile->p[i].d[2];
    v1[3]=1.0;
    matMultMV(mati,v1,v2);
    p2->p[i].d[0]=(float)v2[0];
    p2->p[i].d[1]=(float)v2[1];
    p2->p[i].d[2]=(float)v2[2];

    for(col=0;col<3;col++) {
      p2->p[i].col[col][0]=point->col[col][0];
      p2->p[i].col[col][1]=point->col[col][1];
      p2->p[i].col[col][2]=point->col[col][2];
      p2->p[i].col[col][3]=point->col[col][3];
    }
    p2->p[i].fc=profile->p[i].fc;
  }

  return 0;
}

int cgfxMorphProfile(cgfxProfile *p1, cgfxProfile *p2, cgfxProfile *p3)
{
  int i;
  float d[3];
  int col;

  for(i=0;i<p1->pc;i++) {
    d[0]=p2->p[i].v[0]-p1->p[i].v[0];
    d[1]=p2->p[i].v[1]-p1->p[i].v[1];
    d[2]=p2->p[i].v[2]-p1->p[i].v[2];

    p3->p[i].v[0]=p1->p[i].v[0]+d[0]*p2->f;
    p3->p[i].v[1]=p1->p[i].v[1]+d[1]*p2->f;
    p3->p[i].v[2]=p1->p[i].v[2]+d[2]*p2->f;

    p3->p[i].n[0]=(p1->f*p1->p[i].n[0]+p2->f*p2->p[i].n[0]);
    p3->p[i].n[1]=(p1->f*p1->p[i].n[1]+p2->f*p2->p[i].n[1]);
    p3->p[i].n[2]=(p1->f*p1->p[i].n[2]+p2->f*p2->p[i].n[2]);
    matfNormalize(p3->p[i].n, p3->p[i].n);
    p1->p[i].fc=0;
    p2->p[i].fc=0;
    p3->p[i].fc=0;
    p3->p[i].col[0][0]=(p1->f*p1->p[i].col[p1->p[i].fc][0]+
			p2->f*p2->p[i].col[p2->p[i].fc][0]);
    p3->p[i].col[0][1]=(p1->f*p1->p[i].col[p1->p[i].fc][1]+
			p2->f*p2->p[i].col[p2->p[i].fc][1]);
    p3->p[i].col[0][2]=(p1->f*p1->p[i].col[p1->p[i].fc][2]+
			p2->f*p2->p[i].col[p2->p[i].fc][2]);
    p3->p[i].col[0][3]=(p1->f*p1->p[i].col[p1->p[i].fc][3]+
			p2->f*p2->p[i].col[p2->p[i].fc][3]);

  }
  p3->pc=p1->pc;
  return 0;
}

int cgfxGenProfile(cgfxProfile *pro, int type, Render *render)
{
  int i,a,b,c;
  float x,y,z,d1[3],d2[3];
  double angle,frac,r1,r2,rad1,rad2;
  int detail;

  detail=render->detail1;

  switch(type) {
  case CGFX_HELIX:
    rad2=(double)render->helix_width;
    rad1=(double)render->helix_thickness;
    pro->pc=detail*4+1;
    frac=M_PI*2.0/(double)(detail*4);
    z=0.0;
    for(i=0;i<detail*4;i++) {

      pro->p[i].fc=0;

      angle=frac*(double)i;
      r1=rad1;
      r2=rad2;
      x=(float)(r1*cos(angle+frac*0.5));
      y=(float)(r2*sin(angle+frac*0.5));
      pro->p[i].v[0]=x;
      pro->p[i].v[1]=y;
      pro->p[i].v[2]=z;
      //      x=(float)(r1*sin(angle+frac*0.5));
      //      y=(float)(r2*cos(angle+frac*0.5));
      //      pro->p[i].n[0]=y;
      //      pro->p[i].n[1]=x;
      y=(float)(r1*sin(angle+frac*0.5));
      x=(float)(r2*cos(angle+frac*0.5));
      pro->p[i].n[0]=x;
      pro->p[i].n[1]=y;
      pro->p[i].n[2]=z;
      matfNormalize(pro->p[i].n,pro->p[i].n);
      pro->p[i].d[0]=0.0;
      pro->p[i].d[1]=0.0;
      pro->p[i].d[2]=1.0;
      pro->p[i].d[3]=1.0;
    }
    pro->p[i].v[0]=pro->p[0].v[0];
    pro->p[i].v[1]=pro->p[0].v[1];
    pro->p[i].v[2]=pro->p[0].v[2];
    pro->p[i].n[0]=pro->p[0].n[0];
    pro->p[i].n[1]=pro->p[0].n[1];
    pro->p[i].n[2]=pro->p[0].n[2];
    pro->p[i].d[0]=0.0;
    pro->p[i].d[1]=0.0;
    pro->p[i].d[2]=1.0;
    pro->p[i].d[3]=1.0;


    break;
  case CGFX_STRAND:
  case CGFX_STRAND1:
  case CGFX_STRAND2:
    if(type==CGFX_STRAND) {
      rad2=render->strand_width;
      rad1=render->strand_thickness;
    } else if(type==CGFX_STRAND1) {
      rad2=render->strand_width*(1.0+render->arrow_thickness);
      rad1=render->strand_thickness;
    } else {
      rad2=render->bond_width;
      rad1=render->bond_width;
    }
    pro->pc=detail*4+1;
    frac=M_PI*2.0/(double)(detail*4);
    z=0.0;
    for(i=0;i<detail*4;i++) {
      pro->p[i].fc=0;
      angle=frac*(double)i;
      r1=rad1;
      r2=rad2;
      x=(float)(r1*cos(angle+frac*0.5));
      y=(float)(r2*sin(angle+frac*0.5));
      pro->p[i].v[0]=x;
      pro->p[i].v[1]=y;
      pro->p[i].v[2]=z;
      //      x=(float)(r1*sin(angle+frac*0.5));
      //      y=(float)(r2*cos(angle+frac*0.5));
      //      pro->p[i].n[0]=y;
      //      pro->p[i].n[1]=x;
      x=(float)(r2*cos(angle+frac*0.5));
      y=(float)(r1*sin(angle+frac*0.5));
      pro->p[i].n[0]=x;
      pro->p[i].n[1]=y;
      pro->p[i].n[2]=z;
      matfNormalize(pro->p[i].n,pro->p[i].n);
      pro->p[i].d[0]=0.0;
      pro->p[i].d[1]=0.0;
      pro->p[i].d[2]=1.0;
      pro->p[i].d[3]=1.0;
    }
    pro->p[i].v[0]=pro->p[0].v[0];
    pro->p[i].v[1]=pro->p[0].v[1];
    pro->p[i].v[2]=pro->p[0].v[2];
    pro->p[i].n[0]=pro->p[0].n[0];
    pro->p[i].n[1]=pro->p[0].n[1];
    pro->p[i].n[2]=pro->p[0].n[2];
    pro->p[i].d[0]=0.0;
    pro->p[i].d[1]=0.0;
    pro->p[i].d[2]=1.0;
    pro->p[i].d[3]=1.0;


    break;
  case CGFX_TUBE:
  case CGFX_COIL:
  case CGFX_NA:
  default:
    if(render->tube_ratio<=1.0) {
      rad1=(double)render->tube_width;
      rad2=(double)rad1*render->tube_ratio;
    } else {
      rad2=(double)render->tube_width;
      rad1=(double)rad2/render->tube_ratio;
    }
    pro->pc=detail*4+1;
    frac=M_PI*2.0/(double)(detail*4);
    for(i=0;i<detail*4;i++) {
      pro->p[i].fc=0;
      angle=frac*(double)i;
      z=0.0;
      x=(float)(rad1*cos(angle+frac*0.5));
      y=(float)(rad2*sin(angle+frac*0.5));
      pro->p[i].v[0]=x;
      pro->p[i].v[1]=y;
      pro->p[i].v[2]=z;
      y=(float)(rad1*sin(angle+frac*0.5));
      x=(float)(rad2*cos(angle+frac*0.5));
      pro->p[i].n[0]=x;
      pro->p[i].n[1]=y;
      pro->p[i].n[2]=z;
      matfNormalize(pro->p[i].n,pro->p[i].n);
      pro->p[i].d[0]=0.0;
      pro->p[i].d[1]=0.0;
      pro->p[i].d[2]=1.0;
      pro->p[i].d[3]=1.0;
    }
    pro->p[i].v[0]=pro->p[0].v[0];
    pro->p[i].v[1]=pro->p[0].v[1];
    pro->p[i].v[2]=pro->p[0].v[2];
    pro->p[i].n[0]=pro->p[0].n[0];
    pro->p[i].n[1]=pro->p[0].n[1];
    pro->p[i].n[2]=pro->p[0].n[2];
    pro->p[i].d[0]=0.0;
    pro->p[i].d[1]=0.0;
    pro->p[i].d[2]=1.0;
    pro->p[i].d[3]=1.0;

    break;
  }
  return 0;
}


int cgfxAppend(cgfxVA *va1, cgfxVA *va2)
{
  cgfxVA vnew;

  vnew.count=va1->count+va2->count;
  vnew.max=vnew.count;
  vnew.p=Ccalloc(vnew.max,sizeof(cgfxVAField));
  if(va1->p!=NULL)
    memcpy(vnew.p+0,va1->p,sizeof(cgfxVAField)*va1->count);
  memcpy(vnew.p+va1->count,va2->p,sizeof(cgfxVAField)*va2->count);
  if(va1->p!=NULL)
    Cfree(va1->p);
  va1->max=vnew.max;
  va1->count=vnew.count;
  va1->p=vnew.p;

  return 0;
}

int cgfxGenNA(cgfxVA *va, cgfxSplinePoint *p, Render *render)
{
  cgfxGenNASugar(va,p,render);
  cgfxGenNAConn(va,p,render);
  cgfxGenNABase(va,p,render);
  return 0;
}

int cgfxGenNASugar(cgfxVA *va, cgfxSplinePoint *p, Render *render)
{
  int i,n;
  double mat[16],trans[3],v1[4],v2[4],v3[4],l1,l2;
  double id[][3]={{1.0,0.0,0.0},{0.0,0.0,1.0},{0.0,1.0,0.0}};
  double *src[3],*dest[3];
  double *rmat=mat;
  float coord[][3]={{0.0,0.0,0.0},
		    {1.21353,-0.881678,0.0},
		    {2.42705,0.0,0.0},
		    {1.96353,1.42658,0.0},
		    {0.46353,1.42658,0.0}};
  // under the assumption that 0-4 is front, 5-9 is back
  int conn[][3]= {{0,1,2},
		  {0,2,3},
		  {0,3,4},
		  {5,7,6},
		  {5,8,7},
		  {5,9,8},
		  {0,6,1},
		  {0,5,6},
		  {1,6,2},
		  {2,6,7},
		  {2,8,3},
		  {2,7,8},
		  {3,8,4},
		  {4,8,9},
		  {4,5,0},
		  {4,9,5}};
  float norm[][3]={{0.0,0.0,1.0},
		   {0.0,0.0,1.0},
		   {0.0,0.0,1.0},
		   {0.0,0.0,-1.0},
		   {0.0,0.0,-1.0},
		   {0.0,0.0,-1.0},
		   {-0.58779, 0.80902, 0.0},
		   {-0.58779, 0.80902, 0.0},
		   {0.58779, 0.80902, 0.0},
		   {0.58779, 0.80902, 0.0},
		   {0.95105, -0.30931, 0.0},
		   {0.95105, -0.30931, 0.0},
		   {0.0, 1.0, 0.0},
		   {0.0, 1.0, 0.0},
		   {-0.95105, 0.30902, 0.0},
		   {-0.95105, 0.30902, 0.0}};

  float c[10][3],w;
  float nc[16][3];
  double d1[4],d2[4];

  v1[0]=p->v1[0];
  v1[1]=p->v1[1];
  v1[2]=p->v1[2];
  v2[0]=p->v2[0];
  v2[1]=p->v2[1];
  v2[2]=p->v2[2];
  matNormalize(v1,v1);
  matNormalize(v2,v2);
  matCalcCross(v2,v1,v3);
  matNormalize(v3,v3);
  src[0]=id[0];
  src[1]=id[1];
  src[2]=id[2];
  dest[0]=v1;
  dest[1]=v2;
  dest[2]=v3;
  matVectToRot(src,dest,mat);
  matTranspose(mat,mat);
  
  trans[0]=p->v[0];
  trans[1]=p->v[1];
  trans[2]=p->v[2];
  
  v3[0]=1.0; v3[1]=0.0; v3[2]=0.0; v3[3]=1.0;
  matMultMV(mat,v3,v1);
  v3[0]=0.0; v3[1]=0.0; v3[2]=1.0; v3[3]=1.0;
  matMultMV(mat,v3,v2);
  matNormalize(v1,v1);
  matNormalize(v2,v2);
  l1=sqrt(p->v1[0]*p->v1[0]+p->v1[1]*p->v1[1]+p->v1[2]*p->v1[2]);
  l2=sqrt(p->v2[0]*p->v2[0]+p->v2[1]*p->v2[1]+p->v2[2]*p->v2[2]);

  rmat=mat;
  w=render->sugar_thickness;

  for(i=0;i<5;i++) {
    c[i][0]=coord[i][0];
    c[i][1]=coord[i][1];
    c[i][2]=w*0.5;
    c[i+5][0]=coord[i][0];
    c[i+5][1]=coord[i][1];
    c[i+5][2]=-w*0.5;
  }
  for(i=0;i<16;i++) {
    d1[0]=norm[i][0];
    d1[1]=norm[i][1];
    d1[2]=norm[i][2];
    d1[3]=1.0;
    matMultMV(rmat,d1,d2);
    nc[i][0]=d2[0];
    nc[i][1]=d2[1];
    nc[i][2]=d2[2];
  }

  for(i=0;i<10;i++) {
    d1[0]=c[i][0];
    d1[1]=c[i][1];
    d1[2]=c[i][2];
    d1[3]=1.0;
    matMultMV(rmat,d1,d2);
    c[i][0]=d2[0]+trans[0];
    c[i][1]=d2[1]+trans[1];
    c[i][2]=d2[2]+trans[2];
  }

  if(va->count+48>=va->max) {
    va->p=Crecalloc(va->p,va->max+48,sizeof(cgfxVAField));
    va->max+=48;
  }

  for(i=0;i<16;i++) {
    va->p[va->count+0].c[0]=p->colp[1][0];
    va->p[va->count+0].c[1]=p->colp[1][1];
    va->p[va->count+0].c[2]=p->colp[1][2];
    va->p[va->count+0].c[3]=p->colp[1][3];
    va->p[va->count+1].c[0]=p->colp[1][0];
    va->p[va->count+1].c[1]=p->colp[1][1];
    va->p[va->count+1].c[2]=p->colp[1][2];
    va->p[va->count+1].c[3]=p->colp[1][3];
    va->p[va->count+2].c[0]=p->colp[1][0];
    va->p[va->count+2].c[1]=p->colp[1][1];
    va->p[va->count+2].c[2]=p->colp[1][2];
    va->p[va->count+2].c[3]=p->colp[1][3];
    va->p[va->count+0].v[0]=c[conn[i][0]][0];
    va->p[va->count+0].v[1]=c[conn[i][0]][1];
    va->p[va->count+0].v[2]=c[conn[i][0]][2];
    va->p[va->count+1].v[0]=c[conn[i][1]][0];
    va->p[va->count+1].v[1]=c[conn[i][1]][1];
    va->p[va->count+1].v[2]=c[conn[i][1]][2];
    va->p[va->count+2].v[0]=c[conn[i][2]][0];
    va->p[va->count+2].v[1]=c[conn[i][2]][1];
    va->p[va->count+2].v[2]=c[conn[i][2]][2];
    va->p[va->count+0].n[0]=nc[i][0];
    va->p[va->count+0].n[1]=nc[i][1];
    va->p[va->count+0].n[2]=nc[i][2];
    va->p[va->count+1].n[0]=nc[i][0];
    va->p[va->count+1].n[1]=nc[i][1];
    va->p[va->count+1].n[2]=nc[i][2];
    va->p[va->count+2].n[0]=nc[i][0];
    va->p[va->count+2].n[1]=nc[i][1];
    va->p[va->count+2].n[2]=nc[i][2];


    v1[0]=c[conn[i][1]][0]-c[conn[i][0]][0];
    v1[1]=c[conn[i][1]][1]-c[conn[i][0]][1];
    v1[2]=c[conn[i][1]][2]-c[conn[i][0]][2];
    v2[0]=c[conn[i][2]][0]-c[conn[i][0]][0];
    v2[1]=c[conn[i][2]][1]-c[conn[i][0]][1];
    v2[2]=c[conn[i][2]][2]-c[conn[i][0]][2];

    matCalcCross(v1,v2,v3);
    matNormalize(v3,v3);

    va->p[va->count+0].n[0]=v3[0];
    va->p[va->count+0].n[1]=v3[1];
    va->p[va->count+0].n[2]=v3[2];
    va->p[va->count+1].n[0]=v3[0];
    va->p[va->count+1].n[1]=v3[1];
    va->p[va->count+1].n[2]=v3[2];
    va->p[va->count+2].n[0]=v3[0];
    va->p[va->count+2].n[1]=v3[1];
    va->p[va->count+2].n[2]=v3[2];

    va->count+=3;
  }

  return 0;
}

int cgfxGenNAConn(cgfxVA *va, cgfxSplinePoint *p, Render *render)
{
  float v1[3],v2[3],v3[3],w=render->sugar_thickness;
  /* 
     add a cylinder from v2 to v3
  */
  v1[0]=p->v[0]+p->v1[0];
  v1[1]=p->v[1]+p->v1[1];
  v1[2]=p->v[2]+p->v1[2];
  v3[0]=v1[0]+p->v3[0];
  v3[1]=v1[1]+p->v3[1];
  v3[2]=v1[2]+p->v3[2];
  v2[0]=(v1[0]+v3[0])*0.5;
  v2[1]=(v1[1]+v3[1])*0.5;
  v2[2]=(v1[2]+v3[2])*0.5;
  cgfxGenCylinder(va,v1,v2,0.5*w, 1.0, 0.0, render->detail2, CGFX_CAP, p->colp[1]);
  cgfxGenCylinder(va,v2,v3,0.5*w, 1.0, 0.0, render->detail2, CGFX_CAP, p->colp[2]);
  return 0;
}

/* 
   hexagon, scale 1
*/

static float cgfx_na6r_v[][3]={
  {0.0,0.0,0.0},
  {0.86603, -0.5, 0.0},
  {1.73206, 0.0, 0.0},
  {1.73206, 1.0, 0.0},
  {0.86603, 1.5, 0.0},
  {0.0, 1.0, 0.0},
};

static int cgfx_na6r_c[][3]={
  {0,1,2},{0,2,3},{0,3,5},{3,4,5},
  {6,8,7},{6,9,8},{6,11,9},{9,11,10},
  {0,5,6},{6,5,11},{0,7,1},{0,6,7},
  {2,7,8},{2,1,7},{2,8,9},{2,9,3},
  {3,9,10},{3,10,4},{11,5,4},{11,4,10}
};

static float cgfx_na6r_n[][3]={
  {0.0,0.0,1.0}, {0.0,0.0,1.0}, {0.0,0.0,1.0}, {0.0,0.0,1.0},
  {0.0,0.0,-1.0}, {0.0,0.0,-1.0}, {0.0,0.0,-1.0}, {0.0,0.0,-1.0},
  {-1.0,0.0,0.0},{-1.0,0.0,0.0},
  {-0.5,-0.866023,0.0}, {-0.5,-0.866023,0.0},
  {0.5,-0.866023,0.0}, {0.5,-0.866023,0.0},
  {1.0,0.0,0.0},{1.0,0.0,0.0},
  {0.5,0.866023,0.0}, {0.5,0.866023,0.0},
  {-0.5,0.866023,0.0}, {-0.5,0.866023,0.0}
};

static float cgfx_na5r_v[][3]={
  {0.0,0.0,0.0},
  {0.0, 1.0, 0.0},
  {-0.96106, 1.30902, 0.0},
  {-1.58335, 0.5, 0.0},
  {-0.95106, -0.30902, 0.0}
};

static int cgfx_na5r_c[][3]={
  {0,1,2}, {0,2,3}, {0,3,4}, {5,7,6}, {5,8,7}, {5,9,8},
  {0,6,1}, {0,5,6}, {1,6,2}, {2,6,7}, {2,8,3}, {2,7,8},
  {3,8,4}, {4,8,9}, {4,5,0}, {4,9,5} 
};

static float cgfx_na5r_n[][3]={
  {0.0,0.0,1.0}, {0.0,0.0,1.0}, {0.0,0.0,1.0}, {0.0,0.0,-1.0},
  {0.0,0.0,-1.0}, {0.0,0.0,-1.0}, {-0.58779, 0.80902, 0.0},
  {-0.58779, 0.80902, 0.0}, {0.58779, 0.80902, 0.0},
  {0.58779, 0.80902, 0.0},{0.95105, -0.30931, 0.0},
  {0.95105, -0.30931, 0.0}, {0.0, 1.0, 0.0}, {0.0, 1.0, 0.0},
  {-0.95105, 0.30902, 0.0}, {-0.95105, 0.30902, 0.0}
};

int cgfxGenNABase(cgfxVA *va, cgfxSplinePoint *p, Render *render)
{
  int i,n1,n2;
  double mat[16],trans[3],v1[4],v2[4],v3[4],l1,l2;
  double id[][3]={{1.0,0.0,0.0},{0.0,0.0,1.0},{0.0,1.0,0.0}};
  double *src[3],*dest[3];
  double *rmat=mat;
  float w=render->base_thickness, s=1.4;
  float c[12][3];
  float nc[20][3];
  double d1[4],d2[4];

  n1=6;
  n2=20;

  v1[0]=p->v5[0];
  v1[1]=p->v5[1];
  v1[2]=p->v5[2];
  v2[0]=p->v6[0];
  v2[1]=p->v6[1];
  v2[2]=p->v6[2];

  matNormalize(v1,v1);
  matNormalize(v2,v2);
  matCalcCross(v2,v1,v3);
  matNormalize(v3,v3);
  src[0]=id[0];
  src[1]=id[1];
  src[2]=id[2];
  dest[0]=v1;
  dest[1]=v2;
  dest[2]=v3;
  matVectToRot(src,dest,mat);
  matTranspose(mat,mat);
  
  trans[0]=p->v[0]+p->v1[0]+p->v3[0]+p->v4[0];
  trans[1]=p->v[1]+p->v1[1]+p->v3[1]+p->v4[1];
  trans[2]=p->v[2]+p->v1[2]+p->v3[2]+p->v4[2];
  
  v3[0]=1.0; v3[1]=0.0; v3[2]=0.0; v3[3]=1.0;
  matMultMV(mat,v3,v1);
  v3[0]=0.0; v3[1]=0.0; v3[2]=1.0; v3[3]=1.0;
  matMultMV(mat,v3,v2);
  matNormalize(v1,v1);
  matNormalize(v2,v2);
  l1=sqrt(p->v1[0]*p->v1[0]+p->v1[1]*p->v1[1]+p->v1[2]*p->v1[2]);
  l2=sqrt(p->v2[0]*p->v2[0]+p->v2[1]*p->v2[1]+p->v2[2]*p->v2[2]);
  rmat=mat;

  for(i=0;i<n1;i++) {
    c[i][0]=cgfx_na6r_v[i][0]*s;
    c[i][1]=cgfx_na6r_v[i][1]*s;
    c[i][2]=w*0.5;
    c[i+n1][0]=c[i][0];
    c[i+n1][1]=c[i][1];
    c[i+n1][2]=-w*0.5;
  }

  for(i=0;i<n2;i++) {
    d1[0]=cgfx_na6r_n[i][0];
    d1[1]=cgfx_na6r_n[i][1];
    d1[2]=cgfx_na6r_n[i][2];
    d1[3]=1.0;
    matMultMV(mat,d1,d2);
    nc[i][0]=d2[0];
    nc[i][1]=d2[1];
    nc[i][2]=d2[2];
  }

  for(i=0;i<n1*2;i++) {
    d1[0]=c[i][0];
    d1[1]=c[i][1];
    d1[2]=c[i][2];
    d1[3]=1.0;
    matMultMV(mat,d1,d2);
    c[i][0]=d2[0]+trans[0];
    c[i][1]=d2[1]+trans[1];
    c[i][2]=d2[2]+trans[2];
  }

  if(va->count+n2*3>=va->max) {
    va->p=Crecalloc(va->p,va->max+n2*3,sizeof(cgfxVAField));
    va->max+=n2*3;
  }

  for(i=0;i<n2;i++) {
    va->p[va->count+0].c[0]=p->colp[2][0];
    va->p[va->count+0].c[1]=p->colp[2][1];
    va->p[va->count+0].c[2]=p->colp[2][2];
    va->p[va->count+0].c[3]=p->colp[2][3];
    va->p[va->count+1].c[0]=p->colp[2][0];
    va->p[va->count+1].c[1]=p->colp[2][1];
    va->p[va->count+1].c[2]=p->colp[2][2];
    va->p[va->count+1].c[3]=p->colp[2][3];
    va->p[va->count+2].c[0]=p->colp[2][0];
    va->p[va->count+2].c[1]=p->colp[2][1];
    va->p[va->count+2].c[2]=p->colp[2][2];
    va->p[va->count+2].c[3]=p->colp[2][3];
    va->p[va->count+0].v[0]=c[cgfx_na6r_c[i][0]][0];
    va->p[va->count+0].v[1]=c[cgfx_na6r_c[i][0]][1];
    va->p[va->count+0].v[2]=c[cgfx_na6r_c[i][0]][2];
    va->p[va->count+1].v[0]=c[cgfx_na6r_c[i][1]][0];
    va->p[va->count+1].v[1]=c[cgfx_na6r_c[i][1]][1];
    va->p[va->count+1].v[2]=c[cgfx_na6r_c[i][1]][2];
    va->p[va->count+2].v[0]=c[cgfx_na6r_c[i][2]][0];
    va->p[va->count+2].v[1]=c[cgfx_na6r_c[i][2]][1];
    va->p[va->count+2].v[2]=c[cgfx_na6r_c[i][2]][2];
    va->p[va->count+0].n[0]=nc[i][0];
    va->p[va->count+0].n[1]=nc[i][1];
    va->p[va->count+0].n[2]=nc[i][2];
    va->p[va->count+1].n[0]=nc[i][0];
    va->p[va->count+1].n[1]=nc[i][1];
    va->p[va->count+1].n[2]=nc[i][2];
    va->p[va->count+2].n[0]=nc[i][0];
    va->p[va->count+2].n[1]=nc[i][1];
    va->p[va->count+2].n[2]=nc[i][2];


    v1[0]=c[cgfx_na6r_c[i][1]][0]-c[cgfx_na6r_c[i][0]][0];
    v1[1]=c[cgfx_na6r_c[i][1]][1]-c[cgfx_na6r_c[i][0]][1];
    v1[2]=c[cgfx_na6r_c[i][1]][2]-c[cgfx_na6r_c[i][0]][2];
    v2[0]=c[cgfx_na6r_c[i][2]][0]-c[cgfx_na6r_c[i][0]][0];
    v2[1]=c[cgfx_na6r_c[i][2]][1]-c[cgfx_na6r_c[i][0]][1];
    v2[2]=c[cgfx_na6r_c[i][2]][2]-c[cgfx_na6r_c[i][0]][2];

    matCalcCross(v1,v2,v3);
    matNormalize(v3,v3);

    va->p[va->count+0].n[0]=v3[0];
    va->p[va->count+0].n[1]=v3[1];
    va->p[va->count+0].n[2]=v3[2];
    va->p[va->count+1].n[0]=v3[0];
    va->p[va->count+1].n[1]=v3[1];
    va->p[va->count+1].n[2]=v3[2];
    va->p[va->count+2].n[0]=v3[0];
    va->p[va->count+2].n[1]=v3[1];
    va->p[va->count+2].n[2]=v3[2];

    va->count+=3;
  }

  if(p->res_id==2) {
    /* purin */

    n1=5;
    n2=16;
    
    for(i=0;i<n1;i++) {
      c[i][0]=cgfx_na5r_v[i][0]*s;
      c[i][1]=cgfx_na5r_v[i][1]*s;
      c[i][2]=w*0.5;
      c[i+n1][0]=c[i][0];
      c[i+n1][1]=c[i][1];
      c[i+n1][2]=-w*0.5;
    }
    
    for(i=0;i<n2;i++) {
      d1[0]=cgfx_na5r_n[i][0];
      d1[1]=cgfx_na5r_n[i][1];
      d1[2]=cgfx_na5r_n[i][2];
      d1[3]=1.0;
      matMultMV(mat,d1,d2);
      nc[i][0]=d2[0];
      nc[i][1]=d2[1];
      nc[i][2]=d2[2];
    }
    
    for(i=0;i<n1*2;i++) {
      d1[0]=c[i][0];
      d1[1]=c[i][1];
      d1[2]=c[i][2];
      d1[3]=1.0;
      matMultMV(mat,d1,d2);
      c[i][0]=d2[0]+trans[0];
      c[i][1]=d2[1]+trans[1];
      c[i][2]=d2[2]+trans[2];
    }
    
    if(va->count+n2*3>=va->max) {
      va->p=Crecalloc(va->p,va->max+n2*3,sizeof(cgfxVAField));
      va->max+=n2*3;
    }
    
    for(i=0;i<n2;i++) {
      va->p[va->count+0].c[0]=p->colp[2][0];
      va->p[va->count+0].c[1]=p->colp[2][1];
      va->p[va->count+0].c[2]=p->colp[2][2];
      va->p[va->count+0].c[3]=p->colp[2][3];
      va->p[va->count+1].c[0]=p->colp[2][0];
      va->p[va->count+1].c[1]=p->colp[2][1];
      va->p[va->count+1].c[2]=p->colp[2][2];
      va->p[va->count+1].c[3]=p->colp[2][3];
      va->p[va->count+2].c[0]=p->colp[2][0];
      va->p[va->count+2].c[1]=p->colp[2][1];
      va->p[va->count+2].c[2]=p->colp[2][2];
      va->p[va->count+2].c[3]=p->colp[2][3];
      va->p[va->count+0].v[0]=c[cgfx_na5r_c[i][0]][0];
      va->p[va->count+0].v[1]=c[cgfx_na5r_c[i][0]][1];
      va->p[va->count+0].v[2]=c[cgfx_na5r_c[i][0]][2];
      va->p[va->count+1].v[0]=c[cgfx_na5r_c[i][1]][0];
      va->p[va->count+1].v[1]=c[cgfx_na5r_c[i][1]][1];
      va->p[va->count+1].v[2]=c[cgfx_na5r_c[i][1]][2];
      va->p[va->count+2].v[0]=c[cgfx_na5r_c[i][2]][0];
      va->p[va->count+2].v[1]=c[cgfx_na5r_c[i][2]][1];
      va->p[va->count+2].v[2]=c[cgfx_na5r_c[i][2]][2];
      va->p[va->count+0].n[0]=nc[i][0];
      va->p[va->count+0].n[1]=nc[i][1];
      va->p[va->count+0].n[2]=nc[i][2];
      va->p[va->count+1].n[0]=nc[i][0];
      va->p[va->count+1].n[1]=nc[i][1];
      va->p[va->count+1].n[2]=nc[i][2];
      va->p[va->count+2].n[0]=nc[i][0];
      va->p[va->count+2].n[1]=nc[i][1];
      va->p[va->count+2].n[2]=nc[i][2];
      
      
      v1[0]=c[cgfx_na5r_c[i][1]][0]-c[cgfx_na5r_c[i][0]][0];
      v1[1]=c[cgfx_na5r_c[i][1]][1]-c[cgfx_na5r_c[i][0]][1];
      v1[2]=c[cgfx_na5r_c[i][1]][2]-c[cgfx_na5r_c[i][0]][2];
      v2[0]=c[cgfx_na5r_c[i][2]][0]-c[cgfx_na5r_c[i][0]][0];
      v2[1]=c[cgfx_na5r_c[i][2]][1]-c[cgfx_na5r_c[i][0]][1];
      v2[2]=c[cgfx_na5r_c[i][2]][2]-c[cgfx_na5r_c[i][0]][2];
      
      matCalcCross(v1,v2,v3);
      matNormalize(v3,v3);
      
      va->p[va->count+0].n[0]=v3[0];
      va->p[va->count+0].n[1]=v3[1];
      va->p[va->count+0].n[2]=v3[2];
      va->p[va->count+1].n[0]=v3[0];
      va->p[va->count+1].n[1]=v3[1];
      va->p[va->count+1].n[2]=v3[2];
      va->p[va->count+2].n[0]=v3[0];
      va->p[va->count+2].n[1]=v3[1];
      va->p[va->count+2].n[2]=v3[2];
      
      va->count+=3;
    }
    
    
  }
  
  
  return 0;
}

int cgfxGenNA2(cgfxVA *va, cgfxSplinePoint *p, Render *render)
{
  float v1[3],v2[3],v3[3],w=render->bond_width;
  /* 
     add a cylinder from v2 to v3
  */
  v1[0]=p->v[0];
  v1[1]=p->v[1];
  v1[2]=p->v[2];
  v2[0]=v1[0]+p->v6[0];
  v2[1]=v1[1]+p->v6[1];
  v2[2]=v1[2]+p->v6[2];

  cgfxGenCylinder(va,v1,v2,w,1.0,0.0,render->detail2, CGFX_CAP, p->colp[1]);
  return 0;
}


/*
  these are the older, non-vertex-array routines
*/

int cgfxSphere(double radius, int detail)
{
  double x,y,z,v,w,step,step2;
  double xn,yn,zn;
  
  if(detail<1 || detail>90)
    return -1;
  
  if(radius<=0.0)
    return -1;
  
#ifdef XGFX_D2
  detail*=2;
#else
  detail*=4;
#endif  

  step=M_PI*2.0/(double)(detail);
  step2=M_PI*2.0/(double)(detail);
    
  for(w=0.0;w<M_PI*2+step;w+=step) {
    glBegin(GL_TRIANGLE_STRIP);
    for(v=0.0;v<M_PI_2;v+=step2) {
      xn=sin(w+step)*cos(v);
      yn=cos(w+step)*cos(v);
      zn=sin(v);
      x=xn*radius;
      y=yn*radius;
      z=zn*radius;
      glNormal3d(xn,yn,zn);
      glVertex3d(x,y,z);
      
      xn=sin(w)*cos(v);
      yn=cos(w)*cos(v);
      zn=sin(v);
      x=xn*radius;
      y=yn*radius;
      z=zn*radius;
      glNormal3d(xn,yn,zn);
      glVertex3d(x,y,z);
    }
    glNormal3d(0,0,1.0);
    glVertex3d(0,0,radius);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for(v=0.0;v<M_PI_2;v+=step2) {
      xn=sin(w)*cos(v);
      yn=cos(w)*cos(v);
      zn=-sin(v);
      x=xn*radius;
      y=yn*radius;
      z=zn*radius;
      glNormal3d(xn,yn,zn);
      glVertex3d(x,y,z);
      
      xn=sin(w+step)*cos(v);
      yn=cos(w+step)*cos(v);
      zn=-sin(v);
      x=xn*radius;
      y=yn*radius;
      z=zn*radius;
      glNormal3d(xn,yn,zn);
      glVertex3d(x,y,z);
    }
    glNormal3d(0,0,-1.0);
    glVertex3d(0,0,-radius);
    glEnd();
  }
  

    
  return 0;
}

/****************************************

  cylinder
  --------

  takes type, length, radius and detail,
  generates such a cylinder along
  positive z, negative len will be
  correctly treated

  types are blunt, rounded at one end or
  rounded at both ends

*****************************************/

int cgfxCylinder(int type, double len, double radius, int detail)
{
  double x,y,z,v,w,step,step4;
  double xn,yn,zn,fact;
  double sw1,cw1,sw2,cw2,radius2;

  if(detail<1 || detail>720)
    return -1;

  if(radius<=0.0)
    return -1;

  if(len==0.0)
    return -1;

#ifdef CGFX_D2
  detail*=2;
#else
  detail*=4;
#endif

  step=M_PI*2.0/(double)detail;
  step4=step;

  fact=len/fabs(len);

  glBegin(GL_TRIANGLE_STRIP);

  for(w=0.0;w<M_PI*2.0+step;w+=step) {
    xn=sin(w*fact);
    yn=cos(w*fact);
    x=xn*radius;
    y=yn*radius;
    z=0.0;
    glNormal3d(xn,yn,0.0);
    glVertex3d(x,y,z);
    z=len;
    glNormal3d(xn,yn,0.0);
    glVertex3d(x,y,z);
  }
  glEnd();

  if(type==CGFX_CAP) {
    glBegin(GL_TRIANGLE_FAN);

    glNormal3d(0.0,0.0,-1.0);
    glVertex3d(0.0,0.0,0.0);

    for(w=0.0;w<M_PI*2.0+step;w+=step) {
      xn=sin(w*fact);
      yn=cos(w*fact);
      x=xn*radius;
      y=yn*radius;
      glVertex3d(x,y,0.0);
    }

    glEnd();

    glBegin(GL_TRIANGLE_FAN);

    glNormal3d(0.0,0.0,1.0);
    glVertex3d(0.0,0.0,len);

    for(w=0.0;w<M_PI*2.0+step;w+=step) {
      xn=sin(w*fact);
      yn=cos(w*fact);
      x=xn*radius;
      y=yn*radius;
      glVertex3d(-x,y,len);
    }

    glEnd();

  } else if(type==CGFX_SINGLE_ROUND) {

    radius2=radius*fact;

    for(w=0.0;w<M_PI*2+step;w+=step) {
      glBegin(GL_TRIANGLE_STRIP);
      sw1=sin((w)*fact);
      cw1=cos((w)*fact);
      sw2=sin((w+step)*fact);
      cw2=cos((w+step)*fact);
      for(v=0.0;v<M_PI_2;v+=step4) {
	xn=sw2*cos(v);
	yn=cw2*cos(v);
	zn=sin(v);
	x=xn*radius;
	y=yn*radius;
	z=len+zn*radius2;
	glNormal3d(xn,yn,zn*fact);
	glVertex3d(x,y,z);

	xn=sw1*cos(v);
	yn=cw1*cos(v);
	zn=sin(v);
	x=xn*radius;
	y=yn*radius;
	z=len+zn*radius2;
	glNormal3d(xn,yn,zn*fact);
	glVertex3d(x,y,z);
      }
      glNormal3d(0,0,fact);
      glVertex3d(0,0,len+radius2);
      glEnd();
    }
  }

  return 0;
}



