#ifndef _SET_H
#define _SET_H

enum POV_OP {POV_OP_EQ, /* equal */
	     POV_OP_LT, /* less */
	     POV_OP_LE, /* less or equal */
	     POV_OP_GT, /* greater */
	     POV_OP_GE, /* greater or equal */
	     POV_OP_NE, /* not equal */
	     POV_OP_WI, /* within */
	     POV_OP_PE, /* += */
	     POV_OP_ME, /* -= */
	     POV_OP_SE, /* *= */
	     POV_OP_DE, /* /= */
	     POV_OP_OBJ,
	     POV_OP_NOT,
	     POV_OP_NN};

struct POV_VALUE {
  int range_flag;
  char *val1,*val2;
  int wi_flag;
  float vect[4];
};

typedef struct _POV {
  const char *prop;
  int op;
  struct POV_VALUE *val;
  int val_offset, val_count;
  int id;
}POV;

typedef struct RANGE {
  char *buf;
  int buf_count,buf_max;
  union DBM_NODE *src;
  const char *prop;
  const char *val1,*val2;
  int clamp;
}Range;

typedef struct SELECT {
  POV *pov;
  int pov_count,pov_max;

  struct POV_VALUE* vlist;

  char *rbuf;
  int rbuf_len,rbuf_max;
  char **rwl;
  int rwc,rwl_max;

  char *buf;
  int buf_len,buf_max;
  char **wl;
  int wc,wl_max;

  int *stack;
  int stack_count, stack_max;

  int *eval;
  int eval_count, eval_max;

  int *result;
  int result_count;
}Select;

typedef struct SET {
  int valid;
  POV *pov;
  int pov_count,pov_max;

  struct POV_VALUE *pov_value;
  int pov_value_count, pov_value_max;

  char *buf;
  int buf_len,buf_max;
  char **wl;
  int wc,wl_max;

  int select_flag;
  Select select;

  int range_flag;
  Range range;

  int blend;

  struct POV_VALUE *vlist;
}Set;



int selectNew(Select *s, int argc, char **argv);
int selectDelete(Select *s);
int selectSplit(Select *s);
int selectBuildStack(Select *s, int argc, char **argv);
int selectPush(Select *s, int v);
int selectPop(Select *s);
int selectPeek(Select *s);
int selectGetPOVCount(Select *s);
POV *selectGetPOV(Select *s, int n);
int selectSetResult(Select *s, int n, int v);
int selectResult(Select *s);
int selectDump(Select *s);

int setNew(Set *s, int argc, char **argv);
int setDelete(Set *s);
int setSplit(Set *s);

int povNew(POV *pov, char *s, int *, int *, struct POV_VALUE **);
int povSplitVal(POV *pov, char *val, int *, int *, struct POV_VALUE **vl);
struct POV_VALUE *povGetVal(POV *pov, int n);

int rangeNew(Range *r, int argc, char **argv);
int rangeDelete(Range *r);

#endif
