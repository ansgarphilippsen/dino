#ifndef CGFX_H
#define CGFX_H

#define CGFX_INTPOL_COL 0x1

#include <GL/gl.h>

#include "render.h" 

enum {CGFX_BLUNT,
      CGFX_SINGLE_ROUND,
      CGFX_DOUBLE_ROUND,
      CGFX_ROUND_BEGIN,
      CGFX_ROUND_END,
      CGFX_CAP};

enum  {CGFX_SLINE,
       CGFX_TUBE,
       CGFX_COIL,
       CGFX_HELIX,
       CGFX_STRAND,
       CGFX_STRAND1,
       CGFX_STRAND2,
       CGFX_HCYL,
       CGFX_HSC,
       CGFX_NA};

typedef struct CGFX_POINT {
  float v[4],n[4],d[4];
#ifdef HSC_NEWCOL
  float col[3][4];
  int fc; // this parameter is not working!
#else
  float c[4];
#endif
}cgfxPoint;

typedef struct CGFX_D_POINT {
  double v[4],n[4],c[4],d[4];
}cgfxDPoint;

typedef struct CGFX_SPLINEC {
  struct CGFX_POINT *p;
  int pc;
  int id,ext, res_id;
  float v[4],n[4],d[4],*v1,*v2,*v3,*v4,*v5,*v6;
#ifdef HSC_NEWCOL
  float *colp[3];
#else
  float c[4],c2[4],c3[4];
#endif
  float rad;
}cgfxSplinePoint;

typedef struct CGFX_VERTEX_ARRAY_FIELD {
  GLfloat v[4],n[4],c[4];
}cgfxVAField;

typedef struct CGFX_VERTEX_ARRAY {
  struct CGFX_VERTEX_ARRAY_FIELD *p;
  int count;
  int max;
}cgfxVA;

typedef struct CGFX_PROFILE {
  cgfxPoint p[721];
  int pc;
  float f;
}cgfxProfile;

int cgfxSphere(double radius, int detail);
int cgfxCylinder(int type, double len, double radius, int detail);

int cgfxSphereVA(float radius, float p[3], float c[4], cgfxVA *va, int detail);

int cgfxSplineObject(int type, struct CGFX_SPLINEC *p, struct CGFX_VERTEX_ARRAY *va, int step, double, double, int flag);

int cgfxGenSpline(struct CGFX_SPLINEC *s,struct CGFX_SPLINEC *d, int step, int flag);
double cgfxB(double u);

int cgfxGenCylinder(cgfxVA *va, float *beg, float *end, float rad, float sti,float sto,int detail, int type, float *col);

int cgfxGenHSC(cgfxVA *va, cgfxSplinePoint *sp, int pc, Render *render);
int cgfxHSCTransform(cgfxProfile *p1, cgfxPoint *p, cgfxProfile *p2);
int cgfxConnectProfile(cgfxVA *va, cgfxProfile *p1, cgfxProfile *p2, int f);
int cgfxMorphProfile(cgfxProfile *p1, cgfxProfile *p2, cgfxProfile *p3);
int cgfxGenProfile(cgfxProfile *p, int type, Render *r);

int cgfxAppend(cgfxVA *va1, cgfxVA *va2);

int cgfxCopyPVa(cgfxPoint *p1, cgfxVAField *f);

int cgfxGenNA(cgfxVA *va, cgfxSplinePoint *p, Render *render);
int cgfxGenNA2(cgfxVA *va, cgfxSplinePoint *p, Render *render);
int cgfxGenNASugar(cgfxVA *va, cgfxSplinePoint *p, Render *render);
int cgfxGenNAConn(cgfxVA *va, cgfxSplinePoint *p, Render *render);
int cgfxGenNABase(cgfxVA *va, cgfxSplinePoint *p, Render *render);
int cgfxGenNAMat(cgfxSplinePoint *p, double mat[16]);

#endif

