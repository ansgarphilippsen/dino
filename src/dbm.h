#ifndef DBM_H
#define DBM_H

#include "dbm_common.h"
#include "struct_db.h"
#include "scal_db.h"
#include "vect_db.h"
#include "surf_db.h"
#include "grid_db.h"
#include "geom_db.h"
#include "xtal.h"
#include "transform.h"

#define DBM_FLAG_SWAP 0x1
#define DBM_FLAG_CONV 0x2

enum {DBM_NODE_EMPTY=0,
      DBM_NODE_STRUCT=1,
      DBM_NODE_SCAL,
      DBM_NODE_VECT,
      DBM_NODE_SURF,
      DBM_NODE_GRID,
      DBM_NODE_GEOM};

enum {OBJ_UNKNOWN=0,
      STRUCT_CONNECT=10,
      STRUCT_TRACE,
      STRUCT_NBOND,
      SCAL_CONTOUR=20,
      SCAL_GRID,
      SCAL_GRAD,
      SCAL_SLAB,
#ifdef VR
      SCAL_VR,
#endif
#ifdef BONO
      SCAL_BONO,
#endif
      SURF_NORMAL=30,
      GRID_SURFACE=40,
      GRID_CONTOUR,
      GEOM_NORMAL=50};

enum         {DBM_OP_EQ, /* equal */
	      DBM_OP_LT, /* less */
	      DBM_OP_LE, /* less or equal */
	      DBM_OP_GT, /* greater */
	      DBM_OP_GE, /* greater or equal */
	      DBM_OP_NE, /* not equal */
	      DBM_OP_WI, /* within */
	      DBM_OP_PE, /* += */
	      DBM_OP_ME, /* -= */
	      DBM_OP_SE, /* *= */
	      DBM_OP_DE, /* /= */
	      DBM_OP_NN};

enum            {STRUCT_EMPTY=1000,
		 STRUCT_COLOR,
		 SCAL_EMPTY=2000,
		 SCAL_CENTER,
		 SCAL_COLOR,
		 SCAL_LEVEL,
		 SCAL_LEVELS,
		 SCAL_SIZE,
		 SCAL_STEP,
		 SCAL_POINTS,
		 SCAL_METHOD,
		 SCAL_T,
		 SURF_EMPTY=3000,
		 SURF_COLOR,
		 GRID_EMPTY=4000,
		 GEOM_EMPTY=5000,
		 GEOM_POSITION,
		 GEOM_TRANSPARENCY,
		 GEOM_LINEWIDTH,
		 GEOM_POINTSIZE,
		 GEOM_RADIUS,
		 GEOM_COLOR,
		 GEOM_DIRECTION,
		 GEOM_FILL};

enum           {DBM_PROP_NONE=0,
		STRUCT_ANUM,
		STRUCT_BFAC,
		STRUCT_CHAIN,
		STRUCT_RNUM,
		STRUCT_WEIGHT,
		SCAL_V};


typedef union DBM_OBJECT {
  int type;
  struct DBM_OBJECT_COMMON {
    int type;
    char name[64];
  }common;
  struct STRUCT_OBJ struc;
  struct SCAL_OBJ scal;
  struct SURF_OBJ surf;
  struct GRID_OBJ grid;
}dbmObject;


typedef struct DBM_RANGE {
  char expr[2048];
  union DBM_NODE *src;
  char prop[64];
  int prop_id;
  float v1,v2;
}dbmRange;

		       
typedef struct DBM_SET {
  int ec;
  struct DBM_SET_ELEMENT {
    int id;
    int op;
    union DBM_SET_VALUE {
      int i[2];
      float f[2];
      double d[2];
      float v[2][3];
      float m[2][16];
      double vd[2][3];
      char s[2][256];
    }value;
  }e[64];
  int selection_flag;
  struct LEX_STACK selection;
  char sel_string[2048];
  int range_flag;
  struct DBM_RANGE range;
}dbmSet;



typedef struct DBM_VECT_NODE {
  int type;
  char name[256];
  char path[1024];
  union DBM_NODE *attach;
}dbmVectNode;


typedef union DBM_NODE {
  struct DBM_NODE_COMMON {
    DBM_NODE_COMMON_HEADER
  }common;
  dbmStructNode structNode;
  dbmScalNode scalNode;
  dbmVectNode vectNode;
  dbmSurfNode surfNode;
  dbmGridNode gridNode;
  dbmGeomNode geomNode;
}dbmNode;

struct DBM {
  dbmNode *node;
  int nodec_max;
  int node_count;
};

#include "pick.h"

int dbmInit(void);
int dbmNew(int wc, const char **wl);
int dbmLoad(int, const char **);
dbmNode *dbmNewNode(int type, const char *name);
int dbmDeleteNode(const char *name);
int dbmGetNodeType(const char *);

int dbmStructCheckDist(structObj *, double *v, double d);
float dbmStructGetProperty(dbmStructNode *, float *pos, const char *prop);

int dbmScalCheckDist(scalObj *, double *v, double d);
float dbmScalGetProperty(dbmScalNode *, float *pos, const char *prop);

int dbmSurfCheckDist(surfObj *, double *v, double d);
float dbmSurfGetProperty(dbmSurfNode *node, float *pos, const char *prop);

int dbmGridCheckDist(gridObj *, double *v, double d);

int dbmCalcXtal(struct XTAL *xtal);

int dbmSplit(char *exp, char c, int *wc, char ***wl);
int dbmSplitPOV(const char *expr, char *prop, char *op, char *val);
int dbmGetColorHash(const char *expr, double *r, double *g, double *b);

float dbmGetProperty(float *pos, struct DBM_RANGE *r);
int dbmGetMinMax(dbmNode *node, const char *prop, float *min, float *max);
float dbmGetMin(dbmNode *node, char *prop);
float dbmGetMax(dbmNode *node, char *prop);

int dbmScalCommand(struct DBM_SCAL_NODE *n,int wc,const char **wl);
int dbmScalCommandNew(struct DBM_SCAL_NODE *node,int wc,const char **wl);

int dbmSetExtract(dbmNode *node, struct DBM_SET *set, char *expr, int type);
int dbmRangeExtract(dbmNode *node, struct DBM_RANGE *range);

int dbmIsWithin(float *p, float d2, const char *ds, const char *obj);
int dbmIsElementInObj(const char *db, const char *obj, int ele_num);
int dbmGetRangeVal(Range *range, float *p, float *r);

int dbmPickAdd(dbmPickList *pl, float x, float y, float z, char *n, char *id);

#endif
