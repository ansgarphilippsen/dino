#ifndef MAT_H
#define MAT_H

#define MAT_MAX_WORD_COUNT 100
#define MAT_MAX_STRING_LENGTH 255

typedef struct MAT_WORD_LIST
{
  int wc;
  char word[MAT_MAX_WORD_COUNT][MAT_MAX_STRING_LENGTH];
}matWordList;

int matMultMV(double *m,double *v,double *r);
int matMultVM(double *v,double *m, double *r);
int matMultMVf(float *m,float *v,float *r);
int matMultVMf(float *v,float *m, float *r);
int matMultMM(double *m1, double *m2, double *mr);
int matAddV(double *v1,double *v2); 
int matCopyVdf(double *v1, float *v2);
int matCopyMM(double *m1, double *m2);
int matMakeRotMat(double angle, double a1, double a2, double a3, double *rm);
int matMakeSkew(double a, double b, double c, int flag, double *mat);
int matMakeScale(double a, double b, double c,double *rm);
int matMakeTrans(double a, double b, double c, double *rm);
int matMakeT2O(double a, double b, double c, double alpha, double beta, double gamma, double *rm);
int matMakeO2T(double a, double b, double c, double alpha, double beta, double gamma, double *rm);
int matInverse(double *m1, double *m2);
int matLUdcmp(double *aa, int *indx, double *dd,double *r);
int matLUbksb(double *aa,int *indx,double *b);
int matGetDim(const char *s,int *d1, int *d2);
int matExtract1D(const char *string, int dim, double *res);
int matExtract1Df(const char *string, int dim, float *res);
int matExtract2D(const char *string, int dim1, int dim2, double *res);
int matExtract2Df(const char *string, int dim1, int dim2, float *res);
int matAssemble1D(double *m, int dim, char *string);
int matAssemble2D(double *m, int dim1, int dim2, char *string);
int matSplitString(const char *string, matWordList *list);
int matCalcCross(double *v1, double *v2, double *res);
double matCalcDot(double *v1, double *v2);
int matNormalize(double *v1,double *v2);
int matfCalcCross(float *v1, float *v2, float *res);
float matfCalcDot(float *v1, float *v2);
float matfCalcNDot(float *v1, float *v2);
int matfNormalize(float *v1, float *v2);
int matCalcCircleM(float p1[3], float p2[3],float p3[3],float m[3]);
int matfCopyVV(float *v1,float *v2);
double matCalcLen(double *);
float matfCalcLen(float *);
int matfCalcDiff(float *, float *, float *);
float matCalcTriArea(float *p1, float *p2, float *p3);

int matCalcPolar(double *v1, double *v2, double *o, double *p, double *k);
int matPolarToMat(double o, double p, double k, double *mat);

int matVectToRot(double *v1[3], double *v2[3], double *mat);

int matTransformAtoB(double a[3],double b[3], double mat[16]);
int matTransformXYtoNM(double n[3],double m[3], double mat[16]);
int matfTransformXYtoNM(float n[3],float m[3], double mat[16]);

int matInverse2(double m[16],double r[16]);
int matTranspose(double m[16], double r[16]);

int matFitHelix(double p1[3],double p2[3], double p3[3], double ret[3]);

double matCalcDistance(double *,double *);
float matfCalcDistance(float *, float *);
double matCalcAngle(double *, double *,double *, double *);
float matfCalcAngle(float *, float *,float *, float *);
double matCalcTorsion(double *, double *,double *, double *);

double matCalcDistancePointToLine(double *l0, double *l1, double *p);

double matCalcGrad(double xy1[2],double xy2[2], double xy3[2]);
#endif











