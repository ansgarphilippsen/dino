#ifndef SCAL_DB_H
#define SCAL_DB_H

#include "scal_obj.h"
#include "rex.h"
#include "render.h"
#include "xtal.h"
#include "set.h"
#include "transform.h"

enum           {SCAL_TYPE_EMPTY, 
		SCAL_TYPE_CCP4, 
		SCAL_TYPE_XPLOR,
		SCAL_TYPE_UHBD};

enum           {SCAL_XYZ,
		SCAL_XZY,
		SCAL_YXZ,
		SCAL_YZX,
		SCAL_ZXY,
		SCAL_ZYX};

enum METHOD {SCAL_M_MC,SCAL_M_BONO};

enum {SCAL_SEL_WITHIN,
      SCAL_SEL_OBJ,
      SCAL_SEL_V,
      SCAL_SEL_X,
      SCAL_SEL_Y,
      SCAL_SEL_Z};

enum {SCAL_PROP_COLOR,
      SCAL_PROP_CENTER,
      SCAL_PROP_RCEN,
      SCAL_PROP_RTC,
      SCAL_PROP_SIZE,
      SCAL_PROP_LEVEL,
      SCAL_PROP_STEP,
      SCAL_PROP_METHOD,
      SCAL_PROP_RAD,
      SCAL_PROP_TRANS,
      SCAL_PROP_ROT,
      SCAL_PROP_EDGE,
      SCAL_PROP_DIR,
      SCAL_PROP_SCALE,
      SCAL_PROP_LENGTH,
      SCAL_PROP_VM,
      SCAL_PROP_VC,
      SCAL_PROP_START,
      SCAL_PROP_END};

struct SCAL_PROP_MIN_MAX {
  float v1,v2;
};

typedef struct SCAL_FIELD
{
  int type;
  int u_size,v_size,w_size;
  int u1,u2,v1,v2,w1,w2;
  int size;
  float *data;
  float *dmin,*dmax;
  float sigma;
  float vmin,vmax;
  int wrap;
  double scale,vc,vm;
  double m[12],m_1[12];
  double a[3],b[3],c[3];
  double offset_x, offset_y, offset_z;
  int axis;
  float edge;
}scalField;

#ifdef BONO

struct SCAL_OCTREE_GLOBS {
  scalOctEntry *tree;
  float level;
  int u1,u2,v1,v2,w1,w2;
  struct SCAL_FIELD *field;
  scalObj *obj;
};
#endif

typedef struct DBM_SCAL_NODE {
  int type;
  char name[256];
  char path[1024];
  union DBM_NODE *attach;
  struct LEX_STACK restrict;
  scalObj **obj;
  int obj_count;
  int obj_max;
  struct SCAL_FIELD *field;
  struct XTAL *xtal;              /* pointer to crystallographic info */
  int show_cell;
  struct SCAL_PROP_MIN_MAX min_max;
  float def_level;

  double center[3];

  int swap_flag;
  transMat transform,transform_save;
}dbmScalNode;


int scalNewNode(dbmScalNode *node);

int scalCommand(dbmScalNode *node,int wc,char **wl);
int scalComNew(dbmScalNode *node,int wc,char **wl);
int scalComSet(dbmScalNode *node,int wc,char **wl);
int scalComGet(dbmScalNode *node,int wc,char **wl);
int scalComDel(dbmScalNode *node,int wc,char **wl);

int scalNew(dbmScalNode *node, char *name, int type, Set *set, Select *sel);
int scalSet(dbmScalNode *node, Set *set);
int scalGet(dbmScalNode *node, char *prop);

scalObj *scalNewObj(dbmScalNode *node, char *name);
int scalDelObj(dbmScalNode *node,char *name);


int scalWriteField(struct SCAL_FIELD *field, int u, int v, int w, float);
float scalReadField(struct SCAL_FIELD *field, int u, int v, int w);

int scalIsSelected(dbmScalNode *n, int u, int v, int w, Select *stack);
int scalIsXYZSelected(dbmScalNode *, float x, float y, float z, Select *stack);
int scalEvalPOV(dbmScalNode *n, int u, int v, int w, POV *poe);
int scalEvalXYZPOV(dbmScalNode *,float x, float y, float z, POV *poe);

int scalUVWtoXYZ(struct SCAL_FIELD *field,double *uvw,double *xyz);
int scalUVWtoXYZf(struct SCAL_FIELD *field,float *uvw,float *xyz);
int scalXYZtoUVW(struct SCAL_FIELD *field,double *xyz,double *uvw);
int scalXYZtoUVWf(struct SCAL_FIELD *field,float *xyz,float *uvw);
int scalCELLtoVECT(struct SCAL_FIELD *field, double a, double b, double c, double alpha, double beta, double gamma, double fa, double fb, double fc);

int scalSetMinMax(dbmScalNode *node);
int scalCalcOffset(struct SCAL_FIELD *field, int u, int v,int w);
int scalUnCalcOffset(struct SCAL_FIELD *field, int o, int *u, int *v,int *w);

#ifdef BONO
int scalOctGetMinMax(struct SCAL_FIELD *field, int u, int v, int w,
		     float *min, float *max);
float scalReadFieldO(struct SCAL_FIELD *field, int u, int v, int w, int off);

#endif

void swap_floats(float *src, float *dest, int n);
void swap_4b(unsigned char *a);
void swap_4bs(unsigned char *a, int n);
void swap_double(double *d);

int scalGetRangeXYZVal(dbmScalNode *node, const char *prop, float *p, float *r);

int scalDraw(dbmScalNode *node, int f);
int scalDrawObj(scalObj *obj);

int scalSub(dbmScalNode *,char *s);

#endif



