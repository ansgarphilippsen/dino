#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef COM_H
#define COM_H

#include "struct_db.h"
#include "dbm.h"
#include "cgfx.h"
#include "transform.h"
#include "render.h"
#include "cmi.h"

/****************************************

the com routines are the communication
interface between shell commands, gfx
commands and the databases

******************************************/

enum {GC_ROTX, GC_ROTY, GC_ROTZ,
      GC_TRANSX, GC_TRANSY, GC_TRANSZ,
      GC_SLAB_NEAR, GC_SLAB_FAR, 
      GC_SLAB_LEFT, GC_SLAB_RIGHT,
      GC_SLAB_TOP, GC_SLAB_BOTTOM,
      GC_ZOOM,
      GC_CUSTOM};

typedef int (*comInputFunc)(int, int, double, void *ptr);

struct GLOBAL_COM {
  int play_count;
  dbmNode *play_node[64];
  comInputFunc dial_input;
  void *dial_ptr;

  int joyflag;

  transDeviceList *tlist;
  int tlist_max,tlist_count;

  int benchmark;
  long t,t2;

  unsigned char cube_lookup[144];

  int ms_mask,ms_axis,ms_ivalue;

  float mouse_spin_x, mouse_spin_y;
};

struct COM_PARAMS {
  float mouse_rot_scale,mouse_tra_scale;
  float sb_rot_scale, sb_tra_scale;
  float dials_rot_scale,dials_tra_scale;
  int stereo_flag;
};

enum  {COM_PLAY_ON,
       COM_PLAY_OFF};

int comInit(struct COM_PARAMS* cp);
int comWorkPrompt(int word_count, const char ** word_list);
void comWorkGfxCommand(int word_count, const char ** word_list);
int comWorkObject(char *,int, const char **);
int comSceneCommand(int wc, const char **wl);
void comTimeProc(void);
void comRedraw(void);
void comDBRedraw(void);
void comMessage(const char *s);
void comSetCP(transMat *t,double x, double y, double z);
int comPick(int x, int y, int flag);
int comCustom(double value);
int comGetColor(const char *, float *r, float *g, float *b);
float comGetProperty(dbmNode *src,const char *prop,float *pos);
int comNewDB(const char *name);
int comDelDB(const char *name);
int comNewObj(const char *db, const char *name);
int comDelObj(const char *db, const char *name);
int comHideObj(const char *db, const char *name);
int comShowObj(const char *db, const char *name);
int comObjCommand(const char *db, const char *obj, char *command);
int comRawCommand(const char *c);
int comIsDB(const char *name);
dbmNode * comGetDB(const char *name);
int comWrite(int wc,const char **wl);
int comSave(int wc,const char **wl);
int comGenLists(int n);
int comNewDisplayList(int l);
int comEndDisplayList(void);
int comGetMinMaxSlab(void);
int comWriteCharBuf(char c);
int comPlay(dbmNode *node, int command);
int comGrabInput(int, comInputFunc, void *ptr);
void comReturn(const char *r);
const char *comGetReturn(void);

int comTransform(int device, int mask, int axis, int value);

int comGrab(transMat *tm, transCallback cb, void* cdata, char *name);

int comGetCurrentCenter(double *);

void comWriteModelview(void);

int comSetDefMat(struct RENDER_MATERIAL *mat);

int comTestTex3D(int, int, int);

int comGenCubeLookup();

void comCMICallback(const cmiToken *t);

void comSetInitCommand(const char *s);
void comBench(void);

void comOutit();

const float *comGetCP();

void tunnelvision(structObj *o);

#endif


