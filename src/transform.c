#include <stdio.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "transform.h"
#include "com.h"
#include "mat.h"
#include "gfx.h"

extern struct GFX gfx;

int transCommand(transMat *trans, int command, int axis, double value)
{
  int i;
  double rmat[16],nmat[16],mm[16],pm[16];
  int vp[4];
  double ox1,oy1,oz1,ox2,oy2,oz2,wx,wy,wz,fx,fy;
  double *mat,mat2[16];
  double d1[4],d2[4];


  if(trans==&gfx.transform) {
    mat=trans->rot;
  } else {
    matMultMM(trans->rot,gfx.transform.rot,mat2);
    mat=mat2;
  }

  if(command==TRANS_TRAX || command==TRANS_TRAY) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0,0.0,gfx.transform.tra[2]);
    glGetDoublev(GL_MODELVIEW_MATRIX,mm);
    glGetDoublev(GL_PROJECTION_MATRIX,pm);
    glGetIntegerv(GL_VIEWPORT,vp);
    ox1=0.0;
    oy1=0.0;
    oz1=gfx.transform.cen[2];
    gluProject(ox1,oy1,oz1,mm,pm,vp,&wx,&wy,&wz);
    gluUnProject(wx+1.0,wy+1.0,wz,mm,pm,vp,&ox2,&oy2,&oz2);
    
    fx=ox2-ox1;
    fy=oy2-oy1;
  }

  switch(command) {
  case TRANS_ROTX:
    matMakeRotMat(value,
		  mat[0],
		  mat[4],
		  mat[8],
		  rmat);
    matMultMM(rmat,trans->rot,nmat);
    for(i=0;i<16;i++) trans->rot[i]=nmat[i];
    break;
  case TRANS_ROTY:
    matMakeRotMat(value,
		  mat[1],
		  mat[5],
		  mat[9],
		  rmat);
    matMultMM(rmat,trans->rot,nmat);
    for(i=0;i<16;i++) trans->rot[i]=nmat[i];
    break;
  case TRANS_ROTZ:
    matMakeRotMat(value,
		  mat[2],
		  mat[6],
		  mat[10],
		  rmat);
    matMultMM(rmat,trans->rot,nmat);
    for(i=0;i<16;i++) trans->rot[i]=nmat[i];
    break;
  case TRANS_TRAX:
    d1[0]=value*fx;
    d1[1]=0.0;
    d1[2]=0.0;
    d1[3]=1.0;
    break;
  case TRANS_TRAY:
    d1[0]=0.0;
    d1[1]=value*fx;
    d1[2]=0.0;
    d1[3]=1.0;
    break;
  case TRANS_TRAZ:
    d1[0]=0.0;
    d1[1]=0.0;
    d1[2]=value;
    d1[3]=1.0;
#ifndef EXPO
    if(trans==&gfx.transform) {
      if(gfx.fixz) {
	gfx.transform.slabn-=value;
	gfx.transform.slabf-=value;
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
      }
    }
#endif
    break;
  case TRANS_SLABN:
    gfx.transform.slabn+=value;
    gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
    break;
  case TRANS_SLABF:
    gfx.transform.slabf+=value;
    gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
    break;
  }

  if(command==TRANS_TRAX || command==TRANS_TRAY || command==TRANS_TRAZ) {
    if(trans==&gfx.transform) {
#ifdef EXPO
      oz1=trans->tra[2];
      trans->tra[0]+=d1[0];
      trans->tra[1]+=d1[1];
      trans->tra[2]+=d1[2];
      if(gfx.limx1!=0) {
	if(trans->tra[0]>gfx.limx1)
	  trans->tra[0]=gfx.limx1;
	if(trans->tra[0]<gfx.limx2)
	  trans->tra[0]=gfx.limx2;
      }
      if(gfx.limy1!=0) {
	if(trans->tra[1]>gfx.limy1)
	  trans->tra[1]=gfx.limy1;
	if(trans->tra[1]<gfx.limy2)
	  trans->tra[1]=gfx.limy2;
      }
      if(gfx.limz1!=0) {
	if(trans->tra[2]>gfx.limz1)
	  trans->tra[2]=gfx.limz1;
	if(trans->tra[2]<gfx.limz2)
	  trans->tra[2]=gfx.limz2;
      }
      if(gfx.fixz) {
	value=trans->tra[2]-oz1;
	gfx.transform.slabn-=value;
	gfx.transform.slabf-=value;
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
      }
#else
      trans->tra[0]+=d1[0];
      trans->tra[1]+=d1[1];
      if(gfx.mode==GFX_PERSP) {
	trans->tra[2]+=d1[2];
      } else {
	gfx.scale*=(1.0+d1[2]/100.0);
      }
#endif
    } else {
      matMultMV(gfx.transform.rot,d1,d2);
      trans->tra[0]+=d2[0];
      trans->tra[1]+=d2[1];
      trans->tra[2]+=d2[2];
    }
  }
  return 0;
}

int transReset(transMat *trans)
{
  int i;
  double imat[]={1.0,0.0,0.0,0.0,
		 0.0,1.0,0.0,0.0,
		 0.0,0.0,1.0,0.0,
		 0.0,0.0,0.0,1.0};
  for(i=0;i<16;i++) trans->rot[i]=imat[i];
  trans->cen[0]=0.0; trans->cen[1]=0.0; trans->cen[2]=0.0; trans->cen[3]=0.0;
  trans->tra[0]=0.0; trans->tra[1]=0.0; trans->tra[2]=0.0; trans->tra[3]=0.0;

  trans->slabn=1.0;
  trans->slabf=1000;
  return 0;
}

int transApply(transMat *trans, double *v)
{
  double a[4],b[4];

  a[0]=v[0]-trans->cen[0];
  a[1]=v[1]-trans->cen[1];
  a[2]=v[2]-trans->cen[2];
  a[3]=1.0;

  matMultVM(a,trans->rot,b);
  b[0]+=trans->tra[0];
  b[1]+=trans->tra[1];
  b[2]+=trans->tra[2];

  v[0]=b[0]+trans->cen[0];
  v[1]=b[1]+trans->cen[1];
  v[2]=b[2]+trans->cen[2];

  return 0;
}

int transApplyf(transMat *trans, float *v)
{
  double v2[4];

  v2[0]=v[0];
  v2[1]=v[1];
  v2[2]=v[2];
  v2[3]=v[3];
  transApply(trans,v2);
  v[0]=v2[0];
  v[1]=v2[1];
  v[2]=v2[2];
  v[3]=v2[3];
  return 0;
}

int transSetRot(transMat *t, char *s)
{
  double v[9];
  if(matExtract2D(s,3,3,v)!=0) {
    comMessage("error in expected 3x3 matrix: \n");
    comMessage(s);
    return -1;
  }

  t->rot[0]=v[0]; t->rot[1]=v[1]; t->rot[2]=v[2]; t->rot[3]=0.0;
  t->rot[4]=v[3]; t->rot[5]=v[4]; t->rot[6]=v[5]; t->rot[7]=0.0;
  t->rot[8]=v[6]; t->rot[9]=v[7]; t->rot[10]=v[8]; t->rot[11]=0.0;
  t->rot[12]=0.0; t->rot[13]=0.0; t->rot[14]=0.0; t->rot[15]=1.0;

  return 0;
}

int transSetTra(transMat *t, char *s)
{
  double v[3];
  if(matExtract1D(s,3,v)!=0) {
    comMessage("error in expected vector: \n");
    comMessage(s);
    return -1;
  }
  t->tra[0]=v[0]; t->tra[1]=v[1]; t->tra[2]=v[2]; t->tra[3]=1.0;
  return 0;
}

int transSetCen(transMat *t, char *s)
{
  double v[3];
  if(matExtract1D(s,3,v)!=0) {
    comMessage("error in expected vector: \n");
    comMessage(s);
    return -1;
  }
  t->cen[0]=v[0]; t->cen[1]=v[1]; t->cen[2]=v[2]; t->cen[3]=1.0;
  return 0;
}

int transSetAll(transMat *t, char *s)
{
  double v[16];
  if(matExtract2D(s,4,4,v)!=0) {
    comMessage("error in expected 4x4 matrix: \n");
    comMessage(s);
    return -1;
  }

  t->rot[0]=v[0]; t->rot[1]=v[1]; t->rot[2]=v[2]; t->rot[3]=0.0;
  t->rot[4]=v[4]; t->rot[5]=v[5]; t->rot[6]=v[6]; t->rot[7]=0.0;
  t->rot[8]=v[8]; t->rot[9]=v[9]; t->rot[10]=v[10]; t->rot[11]=0.0;
  t->rot[12]=0.0; t->rot[13]=0.0; t->rot[14]=0.0; t->rot[15]=1.0;

  t->tra[0]=v[3];
  t->tra[1]=v[7];
  t->tra[2]=v[11];
  t->tra[3]=1.0;

  t->cen[0]=v[12];
  t->cen[1]=v[13];
  t->cen[2]=v[14];
  t->cen[3]=1.0;

  return 0;
}


char trans_buffer[256];

const char *transGetRot(transMat *t)
{
  sprintf(trans_buffer,"{{%.5f,%.5f,%.5f},{%.5f,%.5f,%.5f},{%.5f,%.5f,%.5f}}",
	  t->rot[0], t->rot[1], t->rot[2],
	  t->rot[4], t->rot[5], t->rot[6],
	  t->rot[8], t->rot[9], t->rot[10]);
  return trans_buffer;
}
const char *transGetTra(transMat *t)
{
  sprintf(trans_buffer,"{%.5f,%.5f,%.5f}",t->tra[0],t->tra[1],t->tra[2]);
  return trans_buffer;
}

const char *transGetCen(transMat *t)
{
  sprintf(trans_buffer,"{%.5f,%.5f,%.5f}",t->cen[0],t->cen[1],t->cen[2]);
  return trans_buffer;
}

const char *transGetAll(transMat *t)
{
  sprintf(trans_buffer,"{{%5.5g,%5.5g,%5.5g,%5.5g},{%5.5g,%5.5g,%5.5g,%5.5g},{%5.5g,%5.5g,%5.5g,%5.5g},{%5.5g,%5.5g,%5.5g,%5.5g}}",
	  t->rot[0], t->rot[1], t->rot[2],t->tra[0],
	  t->rot[4], t->rot[5], t->rot[6],t->tra[1],
	  t->rot[8], t->rot[9], t->rot[10],t->tra[2],
	  t->cen[0], t->cen[1], t->cen[2], 1.0);
  return trans_buffer;
}

int transMultM(transMat *trans, double *m)
{
  double m1[16],m2[16];
  int i;
  m1[0]=m[0]; m1[1]=m[1]; m1[2]=m[2]; m1[3]=0.0;
  m1[4]=m[3]; m1[5]=m[4]; m1[6]=m[5]; m1[7]=0.0;
  m1[8]=m[6]; m1[9]=m[7]; m1[10]=m[8]; m1[11]=0.0;
  m1[12]=0.0; m1[13]=0.0; m1[14]=0.0; m1[15]=1.0;
  matMultMM(trans->rot,m1,m2);
  for(i=0;i<16;i++)
    trans->rot[i]=m2[i];
  return 0;
}

int transMultMf(transMat *trans, float *m)
{
  double m2[9];
  int i;
  for(i=0;i<9;i++)
    m2[i]=m[i];
  return transMultM(trans,m2);
}

int transApplyRotf(transMat *trans, float *v)
{
  double v2[4];
  int r;

  v2[0]=v[0];
  v2[1]=v[1];
  v2[2]=v[2];
  v2[3]=v[3];
  r=transApplyRot(trans,v2);
  v[0]=v2[0];
  v[1]=v2[1];
  v[2]=v2[2];
  v[3]=v2[3];
  return r;
}

int transApplyRot(transMat *trans, double *v)
{
  double a[4],b[4];

  a[0]=v[0]; a[1]=v[1]; a[2]=v[2]; a[3]=1.0;

  matMultVM(a,trans->rot,b);

  v[0]=b[0]; v[1]=b[1]; v[2]=b[2];

  return 0;
}

int transApplyI(transMat *trans, double *v)
{
  double a[4],b[4],t[16];

  a[0]=v[0]-trans->cen[0];
  a[1]=v[1]-trans->cen[1];
  a[2]=v[2]-trans->cen[2];
  a[3]=1.0;

  a[0]-=trans->tra[0];
  a[1]-=trans->tra[1];
  a[2]-=trans->tra[2];
  matTranspose(trans->rot,t);
  matMultVM(a,t,b);

  v[0]=b[0]+trans->cen[0];
  v[1]=b[1]+trans->cen[1];
  v[2]=b[2]+trans->cen[2];

  return 0;
}

int transApplyIf(transMat *trans, float *v)
{
  double v2[4];

  v2[0]=v[0];
  v2[1]=v[1];
  v2[2]=v[2];
  v2[3]=v[3];
  transApplyI(trans,v2);
  v[0]=v2[0];
  v[1]=v2[1];
  v[2]=v2[2];
  v[3]=v2[3];
  return 0;
}
