#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#include "mat.h"
#include "cl.h"
#include "Cmalloc.h"

#ifdef SUN
#define sqrtf(x) ((float)(sqrt((double)(x))))
#endif

#ifdef OSX
#define sqrtf(x) ((float)(sqrt((double)(x))))
#endif

#define MAT_NEW_EXTRACTION

/**************************************************

  the linear array double[16] defines a matrix:

  | m0  m1  m2  m3  |
  | m4  m5  m6  m7  |
  | m8  m9  m10 m11 |
  | m12 m13 m14 m15 |

  the linear array double[4] defines a vector:

  | v0 |
  | v1 |
  | v2 |
  | v3 |

***************************************************/

int matMultMV(double *m,double *v,double *r)
{
  r[0]=m[0]*v[0]+m[1]*v[1]+m[2]*v[2]+m[3]*v[3];
  r[1]=m[4]*v[0]+m[5]*v[1]+m[6]*v[2]+m[7]*v[3];
  r[2]=m[8]*v[0]+m[9]*v[1]+m[10]*v[2]+m[11]*v[3];
  r[3]=m[12]*v[0]+m[13]*v[1]+m[14]*v[2]+m[15]*v[3];

  return 0;
}

int matMultMVf(float *m,float *v,float *r)
{
  r[0]=m[0]*v[0]+m[1]*v[1]+m[2]*v[2]+m[3]*v[3];
  r[1]=m[4]*v[0]+m[5]*v[1]+m[6]*v[2]+m[7]*v[3];
  r[2]=m[8]*v[0]+m[9]*v[1]+m[10]*v[2]+m[11]*v[3];
  r[3]=m[12]*v[0]+m[13]*v[1]+m[14]*v[2]+m[15]*v[3];

  return 0;
}

int matMultVM(double *v,double *m, double *r)
{
  r[0]=v[0]*m[0]+v[1]*m[4]+v[2]*m[8]+v[3]*m[12];
  r[1]=v[0]*m[1]+v[1]*m[5]+v[2]*m[9]+v[3]*m[13];
  r[2]=v[0]*m[2]+v[1]*m[6]+v[2]*m[10]+v[3]*m[14];
  r[3]=v[0]*m[3]+v[1]*m[7]+v[2]*m[11]+v[3]*m[15];

  return 0;
}

int matMultVMf(float *v,float *m, float *r)
{
  r[0]=v[0]*m[0]+v[1]*m[4]+v[2]*m[8]+v[3]*m[12];
  r[1]=v[0]*m[1]+v[1]*m[5]+v[2]*m[9]+v[3]*m[13];
  r[2]=v[0]*m[2]+v[1]*m[6]+v[2]*m[10]+v[3]*m[14];
  r[3]=v[0]*m[3]+v[1]*m[7]+v[2]*m[11]+v[3]*m[15];

  return 0;
}


int matMultMM(double *m1, double *m2, double *mr)
{
  int i,j,k;
  double tmp;

  for(j=0;j<4;j++)
    for(i=0;i<4;i++) {
      tmp=0.0;
      for(k=0;k<4;k++)
	tmp+=m2[k*4+i]*m1[j*4+k];
      mr[j*4+i]=tmp;
    }
  return 0;
}

int matAddV(double *v1,double *v2)
{
  v1[0]+=v2[0];
  v1[1]+=v2[1];
  v1[2]+=v2[2];
  return 0;
}

int matCopyVdf(double *v1, float *v2)
{
  v2[0]=v1[0];
  v2[1]=v1[1];
  v2[2]=v1[2];
  return 0;
}


int matMakeRotMat(double angle, double a1, double a2, double a3, double *rm)
{
  double sa,ca;
  double x,y,z,xx,xy,xz,yy,yz,zz;
  double v,vv;
  int i;

  sa=sin(angle*M_PI/180.0);
  ca=cos(angle*M_PI/180.0);

  v=sqrt(a1*a1+a2*a2+a3*a3);
  if(v==0.0) {
    for(i=0;i<15;i++)
      rm[i]=0.0;
    rm[0]=1.0;
    rm[5]=1.0;
    rm[10]=1.0;
    rm[15]=1.0;
    return -1;
  }
  vv=1.0/v;
  x=a1*vv;
  y=a2*vv;
  z=a3*vv;

  xx=x*x;
  xy=x*y;
  xz=x*z;
  yy=y*y;
  yz=y*z;
  zz=z*z;

  rm[0]=xx+ca-xx*ca;
  rm[4]=xy-ca*xy-sa*z;
  rm[8]=xz-ca*xz+sa*y;
  rm[12]=0.0;

  rm[1]=xy-ca*xy+sa*z;
  rm[5]=yy+ca-ca*yy;
  rm[9]=yz-ca*yz-sa*x;
  rm[13]=0.0;

  rm[2]=xz-ca*xz-sa*y;
  rm[6]=yz-ca*yz+sa*x;
  rm[10]=zz+ca-ca*zz;
  rm[14]=0.0;

  rm[3]=0.0;
  rm[7]=0.0;
  rm[11]=0.0;
  rm[15]=1.0;
  return 0;
}

int matMkeSkew(double alpha, double beta, double gamma, int flag, double *rm)
{
  int i;
  double conv=M_PI/180.0;
  double a,b,g;
  double sa,sb,sg,ca,cb,cg;
  double sas,sbs,sgs,cas,cbs,cgs;
  double mat[16];

  a=alpha*conv;
  b=beta*conv;
  g=gamma*conv;
  sa=sin(a);
  sb=sin(b);
  sg=sin(g);
  ca=cos(a);
  cb=cos(b);
  cg=cos(g);
  cas=(cg*cb-ca)/(sb*sg);
  sas=sqrt(1.0-cas*cas);
  cbs=(ca*cg-cb)/(sa*sg);
  sbs=sqrt(1.0-cbs*cbs);
  cgs=(ca*cb-cg)/(sa*sb);
  sgs=sqrt(1.0-cgs*cgs);
  
  switch(flag) {
  case 1:
    mat[0]=1.0;   mat[1]=cg;   mat[2]=cb;      mat[3]=0.0;
    mat[4]=0.0;   mat[5]=sg;   mat[6]=-sb*cas; mat[7]=0.0;
    mat[8]=0.0;   mat[9]=0.0;  mat[10]=sb*sas; mat[11]=0.0;
    mat[12]=0.0;  mat[13]=0.0; mat[14]=0.0;    mat[15]=1.0;
    break;
  default:
    for(i=0;i<15;i++)
      mat[i]=0.0;
  }
  matInverse(mat,rm);
  return 0;
}

int matMakeScale(double a, double b, double c, double *rm)
{
  int i;
  
  for(i=0;i<16;i++)
    rm[i]=0.0;

  rm[0]=a;
  rm[5]=b;
  rm[10]=c;
  rm[15]=1.0;
  return 0;
}

int matMakeTrans(double a, double b, double c, double *rm)
{
  int i;
  
  for(i=0;i<16;i++)
    rm[i]=0.0;

  rm[0]=1.0;
  rm[5]=1.0;
  rm[10]=1.0;
  rm[15]=1.0;

  rm[12]=a;
  rm[13]=b;
  rm[14]=c;
  return 0;
}

int matMakeT2O(double a, double b, double c, double alpha, double beta, double gamma, double *rm)
{
  double ax,bx,cx;
  double sa,sb,sc,ca,cb,cc;
  double sax,sbx,scx,cax,cbx,ccx;
  double v,v_1;

  alpha*=M_PI/180.0;
  beta*=M_PI/180.0;
  gamma*=M_PI/180.0;
  
  sa=sin(alpha);
  sb=sin(beta);
  sc=sin(gamma);
  ca=cos(alpha);
  cb=cos(beta);
  cc=cos(gamma);

  v=a*b*c*sqrt(1-ca*ca-cb*cb-cc*cc+2*ca*cb*cc);
  v_1=1.0/v;

  ax=b*c*sa*v_1;
  bx=a*c*sb*v_1;
  cx=a*b*sc*v_1;

  sax=v/(a*b*c*sb*sc);
  sbx=v/(a*b*c*sa*sc);
  scx=v/(a*b*c*sa*sb);
  cax=(cb*cc-ca)/(sb*sc);
  cbx=(ca*cc-cb)/(sa*sc);
  ccx=(ca*cb-cc)/(sa*sb);

  rm[0]=1.0/a;
  rm[1]=0.0;
  rm[2]=0.0;
  rm[3]=0.0;
  rm[4]=-cc/(a*sc);
  rm[5]=1.0/(b*sc);
  rm[6]=0.0;
  rm[7]=0.0;
  rm[8]=ax*cbx;
  rm[9]=bx*cax;
  rm[10]=cx;
  rm[11]=0.0;
  rm[12]=0.0;
  rm[13]=0.0;
  rm[14]=0.0;
  rm[15]=1.0;

  /*
  p[0]=a;
  p[1]=0.0;
  p[2]=0.0;
  q[0]=b*cc;
  q[1]=b*sc;
  q[2]=0.0;
  r[0]=c*cb;
  r[1]=(b*c*ca-b*c*cc*cb)/(b*sc);
  r[2]=sqrt(c*c-r[0]*r[0]-r[1]*r[1]);
  */
  /*
  fprintf(stdout,"%.5f %.5f %.5f %.5f\n%.5f %.5f %.5f %.5f\n%.5f %.5f %.5f %.5f\n%.5f %.5f %.5f %.5f\n",
	  rm[0],rm[1],rm[2],rm[3],
	  rm[4],rm[5],rm[6],rm[7],
	  rm[8],rm[9],rm[10],rm[11],
	  rm[12],rm[13],rm[14],rm[15]);
  */	  

  return 0;
}

int matMakeO2T(double a, double b, double c, double alpha, double beta, double gamma, double *rm)
{
  double ax,bx,cx;
  double sa,sb,sc,ca,cb,cc;
  double sax,sbx,scx,cax,cbx,ccx;
  double v,v_1;
  double p[3],q[3],r[3];

  alpha*=M_PI/180.0;
  beta*=M_PI/180.0;
  gamma*=M_PI/180.0;
  
  sa=sin(alpha);
  sb=sin(beta);
  sc=sin(gamma);
  ca=cos(alpha);
  cb=cos(beta);
  cc=cos(gamma);

  v=a*b*c*sqrt(1-ca*ca-cb*cb-cc*cc+2*ca*cb*cc);
  v_1=1.0/v;

  ax=b*c*sa*v_1;
  bx=a*c*sb*v_1;
  cx=a*b*sc*v_1;

  sax=v/(a*b*c*sb*sc);
  sbx=v/(a*b*c*sa*sc);
  scx=v/(a*b*c*sa*sb);
  cax=(cb*cc-ca)/(sb*sc);
  cbx=(ca*cc-cb)/(sa*sc);
  ccx=(ca*cb-cc)/(sa*sb);

  rm[0]=a;
  rm[1]=0.0;
  rm[2]=0.0;
  rm[3]=0.0;
  rm[4]=b*cc;
  rm[5]=b*sc;
  rm[6]=0.0;
  rm[7]=0.0;
  rm[8]=c*cb;
  rm[9]=-c*sb*cax;
  rm[10]=1.0/cx;
  rm[11]=0.0;
  rm[12]=0.0;
  rm[13]=0.0;
  rm[14]=0.0;
  rm[15]=1.0;


  p[0]=a;
  p[1]=0.0;
  p[2]=0.0;
  q[0]=b*cc;
  q[1]=b*sc;
  q[2]=0.0;
  r[0]=c*cb;
  r[1]=(b*c*ca-b*c*cc*cb)/(b*sc);
  r[2]=sqrt(c*c-r[0]*r[0]-r[1]*r[1]);

  /*
  fprintf(stdout,"%.3f %.3f %.3f\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n",
	  p[0],p[1],p[2],
	  q[0],q[1],q[2],
	  r[0],r[1],r[2]);
	  */

  return 0;
}

/*
  THIS DOES NOT WORK !!!!!!!!!!
  USE matInverse2
*/

int matInverse(double *m1, double *m2)
{
  int i,j;
  int indx[4];
  double d,tmp[16];
  double col[4];
  double y[][4]={
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}};


  matLUdcmp(m1,indx,&d,tmp);

  for(j=0;j<4;j++) {
    for(i=0;i<4;i++) col[i]=0.0;
    col[j]=1.0;
    matLUbksb(tmp,indx,col);
    for(i=0;i<4;i++) m2[i*4+j]=col[i];
  }

  /*
  for(j=0;j<4;j++)
    matLUbksb(tmp,indx,y[j]);


  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      m2[i*4+j]=y[j][i];
  */

  return 0;
}

int matLUdcmp(double *aa, int *indx, double *dd,double *r)
{
  double a[4][4];
  int i,imax,j,k;
  double big,dum,sum,temp;
  float vv[4];

  (*dd)=1.0;

  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      a[i][j]=aa[i*4+j];

  for(i=0;i<4;i++) {
    big=0.0;
    for(j=0;j<4;j++)
      if((temp=fabs(a[i][j])) > big)
	big=temp;
    if(big==0.0)
      return -1;
    vv[i]=1.0/big;
  }
  for(j=0;j<4;j++) {
    for(i=0;i<j;i++) {
      sum=a[i][j];
      for(k=0;k<i;k++)
	sum-=a[i][k]*a[k][j];
      a[i][j]=sum;
    }
    big=0.0;
    for(i=j;i<4;i++) {
      sum=a[i][j];
      for(k=0;k<j;k++)
	sum-=a[i][k]*a[k][j];
      a[i][j]=sum;
      if((dum=vv[i]*fabs(sum))>=big) {
	big=dum;
	imax=i;
      }
    }
    if(j!=imax) {
      for(k=0;k<4;k++) {
	dum=a[imax][k];
	a[imax][k]=a[j][k];
	a[j][k]=dum;
      }
      (*dd)=-(*dd);
      vv[imax]=vv[j];
    }
    indx[j]=imax;
    if(a[j][j]==0.0)
      a[j][j]=1e-20;
    if(j<3) {
      dum=1.0/a[j][j];
      for(i=j+1;i<4;i++)
	a[i][j]*=dum;
    }
  }
  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      r[i*4+j]=a[i][j];
  return 0;
}

int matLUbksb(double *aa,int *indx,double *b)
{
  double a[4][4];
  int i,j;
  int ii=0;
  int ll;
  double sum;

  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      a[i][j]=aa[i*4+j];

  for(i=0;i<4;i++) {
    ll=indx[i];
    sum=b[ll];
    b[ll]=b[i];
    if(ii!=0) {
      for(j=ii-1;j<i-1;j++)
	sum-=a[i][j]*b[j];
    } else if(sum!=0.0) {
      ii=i;
    }
    b[i]=sum;
  }
  for(i=3;i>=0;i--){
    sum=b[i];
    for(j=i+1;j<4;j++)
      sum-=a[i][j]*b[j];
    b[i]=sum/a[i][i];
  }

  return 0;
}


/*
  float versions for extraction routines
*/

int matExtract1Df(const char *string, int dim, float *res)
{
  int i;
  double v1[16];

  if(matExtract1D(string,dim,v1)==-1)
    return -1;
  else
    for(i=0;i<dim;i++)
      res[i]=(float)v1[i];

  return 0;
}

int matExtract2Df(const char *string, int dim1, int dim2, float *res)
{
  int i;
  double r[128];

  if(matExtract2D(string,dim1,dim2,r)==-1)
    return -1;

  for(i=0;i<dim1*dim2;i++)
    res[i]=(float)r[i];

  return 0;
}

/*
  matrix extraction routines
*/

#ifdef MAT_NEW_EXTRACTION

int matExtractMatrix(const char *s, int *d1, int *d2, double *res)
{
  int ret=-1;
  if(matExtract1D(s,3,res)==0) {
    (*d1)=3;
    (*d2)=1;
    ret=0;
  } 
  if(matExtract1D(s,4,res)==0) {
    (*d1)=4;
    (*d2)=1;
    ret=0;
  } 
  if(matExtract2D(s,3,3,res)==0) {
    (*d1)=3;
    (*d2)=3;
    ret=0;
  }
  if(matExtract2D(s,4,4,res)==0) {
    (*d1)=4;
    (*d2)=4;
    ret=0;
  }
  return ret;
}

int matGetDim(const char *s,int *d1, int *d2)
{
  double res[16];
  return matExtractMatrix(s,d1,d2,res);
}

static regmatch_t mat_pmatch_buf[20];
static char mat_regex_string[1024];
static char mat_regex_expr[1024];

int matExtract1D(const char *string2, int dim, double *res)
{
  int i,j,ret;
  regex_t preg;
  regmatch_t *pmatch;
  static char message[256];
  static char *sub="([-0-9+.eE]*)";
  char *expr=mat_regex_expr;
  char *string=mat_regex_string;

  pmatch=mat_pmatch_buf;

  j=0;
  for(i=0;i<clStrlen(string2);i++) {
    if(!isspace(string2[i]))
      string[j++]=string2[i];
  }
  string[j]='\0';

  clStrcpy(expr,"[\\{]?");
  for(i=0;i<dim;i++) {
    clStrcat(expr,sub);
    if(i+1<dim)
      clStrcat(expr,",");
  }
  clStrcat(expr,"[\\}]?(.*)");

//  fprintf(stderr,"%s\n%s\n",string,expr);
 
  ret=regcomp(&preg,expr,REG_EXTENDED);
  if(ret>0) {
    regerror(ret,&preg,message,256);
    fprintf(stderr,"regcomp: %s\n",message);
    return -1;
  } else {
    ret=regexec(&preg,string,dim+3,pmatch,0);
    if(ret==REG_NOMATCH) {
      regfree(&preg);
      return -1;
    } else {
      for(i=0;i<dim;i++) {
	res[i]=atof(clSubstr(string,pmatch[i+1].rm_so,pmatch[i+1].rm_eo-1));
      }
      regfree(&preg);
      if(pmatch[i+1].rm_so!=pmatch[i+1].rm_eo) {
	return -1;
      }
    }
  }
  return 0;
}

int matExtract2D(const char *string2, int dim2, int dim1, double *res)
{
  int i,j,k,ret;
  regex_t preg;
  regmatch_t *pmatch;
  static char message[256];
  static char *sub="([-0-9+.eE]*)";
  char *expr=mat_regex_expr;
  char *string=mat_regex_string;

  pmatch=mat_pmatch_buf;
  j=0;
  for(i=0;i<clStrlen(string2);i++) {
    if(!isspace(string2[i]))
      string[j++]=string2[i];
  }
  string[j]='\0';

  clStrcpy(expr,"[\\{]?");
  for(k=0;k<dim2;k++) {
    clStrcat(expr,"\\{");
    for(i=0;i<dim1;i++) {
      clStrcat(expr,sub);
      if(i+1<dim1)
	clStrcat(expr,",");
    }
    clStrcat(expr,"\\}");
    if(k+1<dim2)
      clStrcat(expr,",");
  }
  clStrcat(expr,"[\\}]?(.*)");

//  fprintf(stderr,"%s\n%s\n",string,expr);
 
  ret=regcomp(&preg,expr,REG_EXTENDED);
  if(ret>0) {
    regerror(ret,&preg,message,256);
    fprintf(stderr,"regcomp: %s\n",message);
    return -1;
  } else {
    ret=regexec(&preg,string,dim1*dim2+3,pmatch,0);
    if(ret==REG_NOMATCH) {
      regfree(&preg);
      return -1;
    } else {
      for(i=0;i<dim1*dim2;i++) {
	res[i]=atof(clSubstr(string,pmatch[i+1].rm_so,pmatch[i+1].rm_eo-1));
      }
      regfree(&preg);
      if(pmatch[i+1].rm_so!=pmatch[i+1].rm_eo) {
	return -1;
      }
    }
  }
  return 0;
}

#else

int matExtractMatrix(const char *string, int *d1, int *d2, double *res)
{
  return -1;
}

int matGetDim(const char *s,int *dd1, int *dd2)
{
  int sp=0;
  int bc=0;
  int d0=0,d1=0,d2=0,d2s=-1;

  while(sp<strlen(s)) {
    if(s[sp]=='{') {
      bc++;
      if(bc==1)
	d1++;
      else if(bc==2)
	d2++;
      else
	return -1;
    } else if(s[sp]=='}') {
      bc--;
      if(bc<0) {
	return -1;
      } else if(bc==0) {
	if(sp+1<strlen(s))
	  return -1;
      } else if(bc==1) {
	if(d2s==-1)
	  d2s=d2;
	if(d2s!=d2)
	  return -1;
	d2=0;
      } else {
	return -1;
      }
    } else if(s[sp]==',') {
      if(bc==1)
	d1++;
      else if(bc==2)
	d2++;
      else
	return -1;
    } else {
    }
    sp++;
  }
  if(d2s==-1) {
    (*dd1)=d1;
    (*dd2)=1;
  } else {
    (*dd1)=d2s;
    (*dd2)=d1;
  }

  return 0;
}


int matExtract1D(const char *string, int dim, double *res)
{
  int i;
  matWordList wl;
  double val;
  int count;
  int bcount;

  if(strlen(string)==0)
    return -1;
  matSplitString(string, &wl);

  count=0;
  bcount=0;
  for(i=0;i<wl.wc;i++) {
    if(!strcmp(wl.word[i],"{")) {
      bcount++;
      if(bcount>1)
	return -1;
    } else if(!strcmp(wl.word[i],"}")) {
      bcount--;
    } else if(!strcmp(wl.word[i],",")) {
    } else {
      if(bcount!=1)
	return -1;

      val=atof(wl.word[i]);
      res[count++]=val;
      if(count>dim)
	break;
    }
  }
  for(i=count;i<dim;i++)
    res[i]=0.0;
  
  return 0;
}


int matExtract2D(const char *string, int dim1, int dim2, double *res)
{
  int i,j;
  matWordList wl;
  int count1;
  int count2;
  int bcount;
  double *v;
  char *sub;

  v=Ccalloc(dim2, sizeof(double));
  sub=Cmalloc(strlen(string));

  matSplitString(string, &wl);

  count1=0;
  count2=0;
  bcount=0;
  i=0;
  while(i<wl.wc) {
    if(!strcmp(wl.word[i],"{")) {
      bcount++;
      if(bcount>2) {
	Cfree(sub);
	Cfree(v);
	return -1;
      }
    } else if(!strcmp(wl.word[i],"}")) {
      bcount--;
      if(bcount<0) {
	Cfree(sub);
	Cfree(v);
	return -1;
      }
    } else if(!strcmp(wl.word[i],",")) {
      if(bcount==1) {
	count1++;
	count2=0;
      } else if(bcount==2) {
	count2++;
      } else {
	Cfree(sub);
	Cfree(v);
	return -1;
      }
      if(count1>dim1)
	break;
    } else {
      if(bcount!=2) {
	Cfree(sub);
	Cfree(v);
	return -1;
      }

      if(count2<dim2)
	res[count1*dim2+count2]=atof(wl.word[i]);

    }
    i++;
  }
  for(i=count1+1;i<dim1;i++)
    for(j=0;j<dim2;j++)
      res[i*dim2+j]=0.0;
  
  Cfree(sub);
  Cfree(v);

  return 0;
}

#endif

int matAssemble1D(double *m, int dim, char *string)
{
  int i;
  strcpy(string,"{");
  for(i=0;i<dim;i++)
    sprintf(string,"%s%g,",string,m[i]);
  string[strlen(string)-1]='\0';
  strcat(string,"}");
  return 0;
}

int matAssemble2D(double *m, int d1, int d2, char *string)
{
  int i,j;

  strcpy(string,"{{");
  for(i=0;i<d1;i++) {
    for(j=0;j<d2;j++) {
      sprintf(string,"%s%g,",string,m[i*d2+j]);
    }
    string[strlen(string)-1]='\0';
    strcat(string,"},{");
  }
  string[strlen(string)-2]='\0';
  strcat(string,"}");
  return 0;

}

static char mat_static_buffer[1024];

int matSplitString(const char *string, matWordList *list)
{
  char *cword=mat_static_buffer;
  int cpos;
  char cchar;
  int c=0;
  int whiteflag=1;
  int nextflag=0;

  list->wc=0;
  strcpy(cword,"");
  cpos=0;

  while(c<strlen(string)) {
    cchar=string[c++];
    if(isspace(cchar)){
      if(whiteflag==0){
        cword[cpos++]='\0';
        strcpy(list->word[list->wc++],cword);
        strcpy(cword,"");
        cpos=0;
      }
      whiteflag=1;
      nextflag=0;
    } else if(cchar=='{'){
      nextflag=1;
    } else if(cchar=='}'){
      nextflag=1;
    } else if(cchar==','){
      nextflag=1;
    } else {
      cword[cpos++]=cchar;
      nextflag=0;
      whiteflag=0;
    }
    if(nextflag==1) {
      if(whiteflag==0) {
        cword[cpos++]='\0';
        strcpy(list->word[list->wc++],cword);
        strcpy(cword,"");
        cpos=0;
      }
      cword[cpos++]=cchar;
      cword[cpos++]='\0';
      strcpy(list->word[list->wc++],cword);
      strcpy(cword,"");
      cpos=0;
      
      nextflag=0;
      whiteflag=1;
    }
    
  }
  if(cpos>0) {
    cword[cpos++]='\0';
    strcpy(list->word[list->wc++],cword);
  }
  return 0;
}



int matCalcCross(double *v1, double *v2, double *vr)
{
        vr[0]=v1[1]*v2[2]-v2[1]*v1[2];
        vr[1]=v1[2]*v2[0]-v2[2]*v1[0];
        vr[2]=v1[0]*v2[1]-v2[0]*v1[1];
	return 0;
}

double matCalcDot(double *v1, double *v2)
{
  return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}

int matNormalize(double *v1,double *v2)
{
  double d;

  d=sqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]);
  if(d==0.0)
    return -1;

  v2[0]=v1[0]/d;
  v2[1]=v1[1]/d;
  v2[2]=v1[2]/d;
  return 0;
}


int matfCalcCross(float *v1, float *v2, float *vr)
{
        vr[0]=v1[1]*v2[2]-v2[1]*v1[2];
        vr[1]=v1[2]*v2[0]-v2[2]*v1[0];
        vr[2]=v1[0]*v2[1]-v2[0]*v1[1];
	return 0;
}

float matfCalcDot(float *v1, float *v2)
{
  return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];
}

float matfCalcNDot(float *v1, float *v2)
{
  float l1,l2;
  l1=sqrtf(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]);
  l2=sqrtf(v2[0]*v2[0]+v2[1]*v2[1]+v2[2]*v2[2]);
  if(l1==0.0 || l2==0.0)
    return 0.0;
  else
    return (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])/(l1*l2);
}


int matfNormalize(float *v1,float *v2)
{
  float d;
  double x,y,z;

  x=(double)v1[0]; y=(double)v1[1]; z=(double)v1[2];

  d=(float)(sqrt(x*x+y*y+z*z));

  //  d=sqrtf(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]);

  if(d==0.0)
    d=1.0;

  v2[0]=v1[0]/d;
  v2[1]=v1[1]/d;
  v2[2]=v1[2]/d;

  return 0;
}

int matCalcCircleM(float p1[3], float p2[3],float p3[3],float m[3])
{
  double ma,mb,mc,md,me,mf,mg,mh,mi;
  double b1,b2,b3;
  double det,det1,det2,det3;

  /*
  fprintf(stderr,"%.3f %.3f %.3f  %.3f %.3f %.3f  %.3f %.3f %.3f\n",
	  p1[0],p1[1],p1[2],p2[0],p2[1],p2[2],p3[0],p3[1],p3[2]);
  */

  ma=2.0*(p1[0]-p2[0]);
  mb=2.0*(p1[1]-p2[1]);
  mc=2.0*(p1[2]-p2[2]);
  md=2.0*(p1[0]-p3[0]);
  me=2.0*(p1[1]-p3[1]);
  mf=2.0*(p1[2]-p3[2]);
  
  mg=p1[1]*p3[2]-p3[1]*p1[2]+p2[1]*p1[2]-p1[1]*p2[2]+p3[1]*p2[2]-p2[1]*p3[2];
  mh=p2[0]*p3[2]-p2[0]*p1[2]+p3[0]*p1[2]-p3[0]*p2[2]+p1[0]*p2[2]-p1[0]*p3[2];
  mi=p2[0]*p1[1]-p2[0]*p3[1]+p3[0]*p2[1]-p3[0]*p1[1]+p1[0]*p3[1]-p1[0]*p2[1];

  /*
  fprintf(stderr,"%.3f %.3f %.3f\n%.3f %.3f %.3f\n%.3f %.3f %.3f\n\n",
	  ma,mb,mc,md,me,mf,mg,mh,mi);
  */

  b1=(p1[0]*p1[0]+p1[1]*p1[1]+p1[2]*p1[2])-(p2[0]*p2[0]+p2[1]*p2[1]+p2[2]*p2[2]);
  b2=(p1[0]*p1[0]+p1[1]*p1[1]+p1[2]*p1[2])-(p3[0]*p3[0]+p3[1]*p3[1]+p3[2]*p3[2]);
  b3=p2[0]*p3[1]*p1[2]+p3[0]*p1[1]*p2[2]+p1[0]*p2[1]*p3[2]-p2[0]*p1[1]*p3[2]-p3[0]*p2[1]*p1[2]-p1[0]*p3[1]*p2[2];
  b3*=-1.0;

  det=ma*(me*mi-mh*mf)-mb*(md*mi-mg*mf)+mc*(md*mh-mg*me);

  det1=b1*(me*mi-mh*mf)-mb*(b2*mi-b3*mf)+mc*(b2*mh-b3*me);
  det2=ma*(b2*mi-b3*mf)-b1*(md*mi-mg*mf)+mc*(md*b3-mg*b2);
  det3=ma*(me*b3-mh*b2)-mb*(md*b3-mg*b2)+b1*(md*mh-mg*me);

  if(det==0.0) {
    /*
    fprintf(stderr,"det=0\n");
    */
    return -1;
  }

  /*
  fprintf(stderr,"%.3f %.3f %.3f  %.3f\n",det1,det2,det3,det);
  */

	  
  m[0]=(float)(det1/det);
  m[1]=(float)(det2/det);
  m[2]=(float)(det3/det);

  /*
  fprintf(stderr,"%.4f %.4f %.4f\n",
	  ((m[0]-p1[0])*(m[0]-p1[0]))+((m[1]-p1[1])*(m[1]-p1[1]))+((m[2]-p1[2])*(m[2]-p1[2])),
	  ((m[0]-p2[0])*(m[0]-p2[0]))+((m[1]-p2[1])*(m[1]-p2[1]))+((m[2]-p2[2])*(m[2]-p2[2])),
	  ((m[0]-p3[0])*(m[0]-p3[0]))+((m[1]-p3[1])*(m[1]-p3[1]))+((m[2]-p3[2])*(m[2]-p3[2])));
	  */
  return 0;
}


int matCopyMM(double *m1, double *m2)
{
  int i;
  for(i=0;i<16;i++)
    m2[i]=m1[i];

  return 0;
}

int matfCopyVV(float *v1,float *v2)
{
  v2[0]=v1[0];
  v2[1]=v1[1];
  v2[2]=v1[2];
  return 0;
}

int matCopyVV(double *v1,double *v2)
{
  v2[0]=v1[0];
  v2[1]=v1[1];
  v2[2]=v1[2];
  v2[3]=v1[3];
  return 0;
}

double matCalcLen(double *v)
{
  double r;
  r=v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
  return sqrt(r);
}

float matfCalcLen(float *v)
{
  float r;
  r=v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
  return sqrtf(r);
}

int matfCalcDiff(float *v1, float *v2, float *v3)
{
  v3[0]=v1[0]-v2[0];
  v3[1]=v1[1]-v2[1];
  v3[2]=v1[2]-v2[2];
  return 0;
}

float matCalcTriArea(float *p1, float *p2, float *p3)
{
  float n[3],v1[3],v2[3],v3[3],r[3];
  float a;
  
  v1[0]=p3[0]-p1[0];
  v1[1]=p3[1]-p1[1];
  v1[2]=p3[2]-p1[2];
  v2[0]=p3[0]-p2[0];
  v2[1]=p3[1]-p2[1];
  v2[2]=p3[2]-p2[2];

  matfCalcCross(v1,v2,n);

  matfCalcCross(p1,p2,v1);
  matfCalcCross(p2,p3,v2);
  matfCalcCross(p3,p1,v3);

  r[0]=v1[0]+v2[0]+v3[0];
  r[1]=v1[1]+v2[1]+v3[1];
  r[2]=v1[2]+v2[2]+v3[2];

  a=matfCalcDot(r,n);

  return fabs(a*0.5);
}

/*
  calculate polar angles from two (perpendicular) vectors
  it is not clear wether this is ambiguous and needs a third
  vector. on the assumption that v1 and v2 are x and y from a 
  right-handed, orthogonal coordinate system v3 is given
  by v3=v1 x v2

  the transformation around axis v1 to yield v2, if v2 is (0,1,0)
  to begin with, is (with p=v2x, q=v2y, r=v2z, ca=cos kappa, sa=sin kappa)

  xy-ca*xy+sa*z=p
  yy+ca-ca*yy=q
  yz-ca*yz-sa*x=r

  just using (2) is ambiguous, so (2) is combined with (1) to yield
  kappa 
  
*/

int matCalcPolar(double *v1, double *v2, double *om, double *ph, double *ka)
{
  double x,y,z,xx,yy,zz,xy,xz,yz;
  double n1[3],n2[3];
  double q,p,r;

  /* 
     calculation uses normalized vectors from v1 and v2
  */
  n1[0]=v1[0]; n1[1]=v1[1]; n1[2]=v1[2];
  n2[0]=v2[0]; n2[1]=v2[1]; n2[2]=v2[2];

  matNormalize(n1,n1);
  matNormalize(n2,n2);

  x=n1[0];
  y=n1[1];
  z=n1[2];

  xx=x*x; xy=x*y; xz=x*z;
  yy=y*y; yz=y*z;
  zz=z*z;

  q=n2[0];
  p=n2[1];
  r=n2[2];

  /* 
     omega and phi describe the polar angles
     to get from (1,0,0) to v1
  */
  (*om)=acos(x);
  (*ph)=acos(y/sin((*om)));

  /*
    v1 is used as the rotation axis around
    which (0,1,0) is rotated kappa degrees
    to yield v2
  */

  (*ka)=asin((p-xy+(xy*(q-yy)/(1-yy)))/z);

  return 0;
}

/*
  generate a rotation matrix (4x4) from
  the given polar angles omega, phi and kappa

  the rotation matrix is assembled:

  rotate (1,0,0) around z-axis omega degrees
  rotate resulting vector around x-axis phi degrees
  rotate around resulting vector kappa degrees

*/

int matPolarToMat(double omega, double phi, double kappa, double *mat)
{
  double ax,ay,az;
  double mat1[16],mat2[16],mat3[16], mat4[16];

  matMakeRotMat(omega, 0.0, 0.0, 1.0, mat1);

  matMakeRotMat(phi, 1.0, 0.0, 0.0, mat2);

  ax=cos(omega);
  ay=sin(omega)*cos(phi);
  az=sin(omega)*sin(phi);

  matMakeRotMat(kappa, ax, ay, az, mat3);

  matMultMM(mat1,mat2,mat4);

  matMultMM(mat4,mat3,mat);


  matMultMM(mat1,mat2,mat);

  return 0;
}

/* 
   calculate a transformation matrix to get v1 to v2
   
   | a b c |
   | d e f | = M 
   | g h i |

   M*v1_1=v2_1
   M*v1_2=v2_2
   M*v1_3=v2_3
   
   v1 and v2 are each three orthogonal vectors, the resulting
   matrix is a rotation matrix with determinant 1

*/

int matVectToRot(double *v1[3], double *v2[3], double *mat)
{
  double px,py,pz, qx,qy,qz, rx,ry,rz;
  double ux,uy,uz, vx,vy,vz, wx,wy,wz;
  double a,b,c, d,e,f, g,h,i;
  double detm;
  double detx1, detx2, detx3;
  double dety1, dety2, dety3;
  double detz1, detz2, detz3;

  px=v2[0][0]; py=v2[0][1]; pz=v2[0][2];
  qx=v2[1][0]; qy=v2[1][1]; qz=v2[1][2];
  rx=v2[2][0]; ry=v2[2][1]; rz=v2[2][2];

  ux=v1[0][0]; uy=v1[0][1]; uz=v1[0][2];
  vx=v1[1][0]; vy=v1[1][1]; vz=v1[1][2];
  wx=v1[2][0]; wy=v1[2][1]; wz=v1[2][2];

  //  fprintf(stderr,"%f %f %f  %f %f %f  %f %f %f\n",
  //	  ux,uy,uz,vx,vy,vz,wx,wy,wz);

  detm=px*(qy*rz-qz*ry)-py*(qx*rz-qz*rx)+pz*(qx*ry-qy*rx);

  if(detm==0.0) {
    return -1;
  }

  detx1=ux*(qy*rz-qz*ry)-py*(vx*rz-qz*wx)+pz*(vx*ry-qy*wx);
  detx2=px*(vx*rz-qz*wx)-ux*(qx*rz-qz*rx)+pz*(qx*wx-vx*rx);
  detx3=px*(qy*wx-vx*ry)-py*(qx*wx-vx*rx)+ux*(qx*ry-qy*rx);

  dety1=uy*(qy*rz-qz*ry)-py*(vy*rz-qz*wy)+pz*(vy*ry-qy*wy);
  dety2=px*(vy*rz-qz*wy)-uy*(qx*rz-qz*rx)+pz*(qx*wy-vy*rx);
  dety3=px*(qy*wy-vy*ry)-py*(qx*wy-vy*rx)+uy*(qx*ry-qy*rx);

  detz1=uz*(qy*rz-qz*ry)-py*(vz*rz-qz*wz)+pz*(vz*ry-qy*wz);
  detz2=px*(vz*rz-qz*wz)-uz*(qx*rz-qz*rx)+pz*(qx*wz-vz*rx);
  detz3=px*(qy*wz-vz*ry)-py*(qx*wz-vz*rx)+uz*(qx*ry-qy*rx);

  a=detx1/detm;
  b=detx2/detm;
  c=detx3/detm;
 
  d=dety1/detm;
  e=dety2/detm;
  f=dety3/detm;
 
  g=detz1/detm;
  h=detz2/detm;
  i=detz3/detm;
 
  mat[0]=a; mat[1]=b; mat[2]=c;  mat[3]=0.0;
  mat[4]=d; mat[5]=e; mat[6]=f;  mat[7]=0.0;
  mat[8]=g; mat[9]=h; mat[10]=i;  mat[11]=0.0;

  mat[12]=0.0; mat[13]=0.0; mat[14]=0.0; mat[15]=1.0;

  return 0;
}

/*
  transform a to b with a rotation around the axis perpendicular
  to both, returns the transformation matrix
*/

int matTransformAtoB(double oa[3],double ob[3], double mat[16])
{
  double a[4],b[4];
  double axis[3], angle, dot, la, lb;

  a[0]=oa[0]; a[1]=oa[1]; a[2]=oa[2]; a[3]=1.0;
  b[0]=ob[0]; b[1]=ob[1]; b[2]=ob[2]; b[3]=1.0;

  la=sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
  lb=sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2]);

  if(la*lb==0.0) {
    fprintf(stderr,"error in matTransform\n");
    return -1;
  }
  matCalcCross(a,b,axis);

  dot=matCalcDot(a,b);

  angle=(180.0/M_PI)*(acos(dot/(la*lb)));

  matMakeRotMat(angle,axis[0],axis[1],axis[2],mat);
  matTranspose(mat,mat);

  /* verified that Ma = b ! */

  return 0;
}


/*

  calculate the transformation matrix that will transform x to n
  and y to m, n and m are perpendicular

*/

int matfTransformXYtoNM(float on[3],float om[3], double mat[16])
{
  double n[3],m[3];

  n[0]=on[0];
  n[1]=on[1];
  n[2]=on[2];

  m[0]=om[0];
  m[1]=om[1];
  m[2]=om[2];

  return matTransformXYtoNM(n,m,mat);
}

int matTransformXYtoNM(double on[3],double om[3], double mat[16])
{
  double x[]={1.0,0.0,0.0,1.0},y[]={0.0,0.0,1.0,1.0};
  double n[4],m[4],y2[4],m2[4],c[4];
  double dot,axis[4],angle;
  double mat1[16],mat2[16];

  n[0]=on[0]; n[1]=on[1]; n[2]=on[2]; n[3]=1.0;
  m[0]=om[0]; m[1]=om[1]; m[2]=om[2]; m[3]=1.0;

  matNormalize(n,n);
  matNormalize(m,m);

  if(matTransformAtoB(x,n,mat1)<0) {
    fprintf(stderr,"error in matTransform\n");
    return -1;
  }

  matMultMV(mat1,y,y2);
  matMultMV(mat1,m,m2);

  /*
    now calculate the transformation of y2 to m2
    while leaving n intact -> rotation around n
  */

  /* TODO */
  

  /*
  if(matTransformAtoB(y2,m2,mat2)<0) {
    fprintf(stderr,"error in matTransform\n");
    return -1;
  }

  */

  /* verified that Mx = n */
  /* verified that My2 = m2 */

  matMultMV(mat2,n,c);
  /*
  fprintf(stderr,"  %f %f %f\n  %f %f %f\n",
	  n[0],n[1],n[2],c[0],c[1],c[2]);
	  */
  matMultMM(mat2,mat1,mat);

  return 0;
}


#define MATSWAP(a,b) {temp=(a); (a)=(b); (b)=temp;}

int matInverse2(double m[16],double r[16])
{
  double a[4][4];
  int indxc[4], indxr[4],ipiv[4];
  int i, icol, irow, j,k,l,ll;
  double big, dum, pivinv,temp;
  double b[][1]={{0.0},{0.0},{0.0},{0.0}};

  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      a[i][j]=m[i*4+j];
  
  for(j=0;j<4;j++) ipiv[j]=0;
  for(i=0;i<4;i++) {
    big=0.0;
    for(j=0;j<4;j++)
      if(ipiv[j]!=1)
	for(k=0;k<4;k++) {
	  if(ipiv[k]==0) {
	    if(fabs(a[j][k]) >=big) {
	      big=fabs(a[j][k]);
	      irow=j;
	      icol=k;
	    }
	  } else if(ipiv[k] >1) {
//	    fprintf(stderr,"error in matInverse2\n");
	    return -1;
	  }
	}
    ++(ipiv[icol]);
    if(irow!=icol) {
      for(l=0;l<4;l++) MATSWAP(a[irow][l], a[icol][l]);
      for(l=0;l<1;l++) MATSWAP(b[irow][l], b[icol][l]);
    }
    indxr[i]=irow;
    indxc[i]=icol;
    if(a[icol][icol]==0.0) {
//      fprintf(stderr,"error in matInverse2\n");
      return -1;
    }
    pivinv=1.0/a[icol][icol];
    a[icol][icol]=1.0;
    for(l=0;l<4;l++) a[icol][l]*=pivinv;
    for(l=0;l<1;l++) b[icol][l]*=pivinv;

    for(ll=0;ll<4;ll++)
      if(ll!=icol) {
	dum=a[ll][icol];
	a[ll][icol]=0.0;
	for(l=0;l<4;l++) a[ll][l] -=a[icol][l]*dum;
	for(l=0;l<1;l++) b[ll][l] -=b[icol][l]*dum;
      }
  }
  for(l=3;l>=0;l--) {
    if(indxr[l]!=indxc[l])
      for(k=0;k<4;k++)
	MATSWAP(a[k][indxr[l]],a[k][indxc[l]]);
  }

  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      r[i*4+j]=a[i][j];

  return 0;
}

int matTranspose(double m[16], double r[16])
{
  int i,j;
  double tmp[16];

  for(i=0;i<16;i++)
    tmp[i]=m[i];

  for(i=0;i<4;i++)
    for(j=0;j<4;j++)
      r[i*4+j]=tmp[j*4+i];

  return 0;
}

/******

  fit three points on a perfect 3.6 alpha helix to a spiral with middle
  axis d, first normal n1 and second normal n2, starting at v0
  
******/

int matFitHelix(double p1[3],double p2[3], double p3[3], double ret[3])
{
  double A,B,C,D;
  double P,Q;
  double U,V,W;

  A=2.3*cos(2.0*M_PI/3.6);
  B=2.3*sin(2.0*M_PI/3.6);
  C=2.3*cos(4.0*M_PI/3.6);
  D=2.3*sin(4.0*M_PI/3.6);

  P=2.0*A-2.3-C;
  Q=2.0*B-D;

  U=2*p2[0]-p1[0]-p3[0];
  V=2*p2[1]-p1[1]-p3[1];
  W=2*p2[2]-p1[2]-p3[2];

  

  return 0;

}

double matCalcDistance(double *v1,double *v2)
{
  double d[3];
  d[0]=v1[0]-v2[0];
  d[1]=v1[1]-v2[1];
  d[2]=v1[2]-v2[2];
  return matCalcLen(d);
}

float matfCalcDistance(float *v1, float *v2)
{
  double d[3];
  d[0]=v1[0]-v2[0];
  d[1]=v1[1]-v2[1];
  d[2]=v1[2]-v2[2];
  return (float)matCalcLen(d);
}

float matfCalcAngle(float *v1, float *v2,float *v3, float *v4)
{
  double d1[3],d2[3],l1,l2,r;

  d1[0]=(double)(v1[0]-v2[0]);
  d1[1]=(double)(v1[1]-v2[1]);
  d1[2]=(double)(v1[2]-v2[2]);

  d2[0]=(double)(v3[0]-v4[0]);
  d2[1]=(double)(v3[1]-v4[1]);
  d2[2]=(double)(v3[2]-v4[2]);

  l1=matCalcLen(d1);
  l2=matCalcLen(d2);

  if(l1==0.0 || l2==0.0) {
    return 0.0;
  }

  r=acos(matCalcDot(d1,d2)/(l1*l2));

  return (float)(r*180.0/M_PI);
}

double matCalcAngle(double *v1, double *v2,double *v3, double *v4)
{
  double d1[3],d2[3],l1,l2,r;

  d1[0]=v1[0]-v2[0];
  d1[1]=v1[1]-v2[1];
  d1[2]=v1[2]-v2[2];

  d2[0]=v3[0]-v4[0];
  d2[1]=v3[1]-v4[1];
  d2[2]=v3[2]-v4[2];

  l1=matCalcLen(d1);
  l2=matCalcLen(d2);

  if(l1==0.0 || l2==0.0) {
    return 0.0;
  }

  r=acos(matCalcDot(d1,d2)/(l1*l2));

  return r*180.0/M_PI;
}

double matCalcTorsion(double *v1, double *v2,double *v3, double *v4)
{
  double d1[3],d2[3],d3[3],d4[3],c1[3],c2[3],l1,l2,r;

  d1[0]=v1[0]-v2[0];
  d1[1]=v1[1]-v2[1];
  d1[2]=v1[2]-v2[2];

  d2[0]=v3[0]-v2[0];
  d2[1]=v3[1]-v2[1];
  d2[2]=v3[2]-v2[2];

  d3[0]=v2[0]-v3[0];
  d3[1]=v2[1]-v3[1];
  d3[2]=v2[2]-v3[2];

  d4[0]=v4[0]-v3[0];
  d4[1]=v4[1]-v3[1];
  d4[2]=v4[2]-v3[2];

  matCalcCross(d1,d2,c1);
  matCalcCross(d3,d4,c2);

  l1=matCalcLen(c1);
  l2=matCalcLen(c2);

  if(l1==0.0 || l2==0.0) {
    return 0.0;
  }

  r=acos(matCalcDot(c1,c2)/(l1*l2));

  return r*180.0/M_PI;
}

double matCalcDistancePointToLine(double *l0, double *l1, double *p)
{
  int i;
  double diff1[3],diff2[3],cross[3];

  for(i=0;i<3;i++) {
    diff1[i]=l1[i]-l0[i];
    diff2[i]=p[i]-l0[i];
  }

  matCalcCross(diff1,diff2,cross);

  return matCalcLen(cross)/matCalcLen(diff1);
}

/*
  extract vector in the form

  <v1,v2, ... ,vn>
  or
  <v1 v2 ... vn>

  and return count values result

  return 0 on success or neg value on failure

*/

#define mat_max_vector_count 16
static float mat_vector_result[mat_max_vector_count];

int matExtractVector(const char *s, int *count, float **result)
{
  char buf[512],*bp;
  int c,d,len;

  clStrncpy(buf,s,512);
  len=clStrlen(buf);

  c=0;

  (*count)=0;
  (*result)=mat_vector_result;
  
  if(buf[c]!='<') {
    return -1;
  } else {
    c++;
  }

  for(d=0;d<len;d++)
    if(buf[d]==',')
      buf[d]=' ';

  bp=buf+c;

  while(buf[c]!='>' && c<len) {
    
    while(isspace(buf[c]) && c<len)
      c++;
    bp=buf+c;
    while(!isspace(buf[c]) && c<len)
      c++;

    if(c<len)
      buf[c]='\0';
    (*result)[(*count)++]=atof(bp);
    if((*count)>mat_max_vector_count)
      break;
  }

  if(c>=len || buf[c]!='>') {
    return -1;
  }

  return 0;

}

int matV3toV4(double *v3,double *v4)
{
  v4[0]=v3[0]; v4[1]=v3[1]; v4[2]=v3[2]; v4[3]=1.0;
  return 0;
}

int matV4toV3(double *v4,double *v3)
{
  // normalize by 1.0/v4[3] ???
  v3[0]=v4[0]; v3[1]=v4[1]; v3[2]=v4[2];
  return 0;
}

int matM3toM4(double *m3,double *m4)
{
  m4[0]=m3[0]; m4[1]=m3[1]; m4[2]=m3[2]; m4[3]=0.0;
  m4[4]=m3[3]; m4[5]=m3[4]; m4[6]=m3[5]; m4[7]=0.0;
  m4[8]=m3[6]; m4[9]=m3[7]; m4[10]=m3[6]; m4[11]=0.0;
  m4[12]=0.0; m4[13]=0.0; m4[14]=0.0; m4[15]=1.0;
  return 0;
}

int matM4toM3(double *m4,double *m3)
{
  m3[0]=m4[0]; m3[1]=m4[1]; m3[2]=m4[2];
  m3[3]=m4[4]; m3[4]=m4[5]; m3[5]=m4[6];
  m3[6]=m4[8]; m3[7]=m4[8]; m3[8]=m4[10];
  return 0;
}

