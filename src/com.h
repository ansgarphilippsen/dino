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

  transList *tlist;
  int tlist_max,tlist_count;

  int benchmark;
  long t,t2;

  unsigned char cube_lookup[144];
};

enum  {COM_PLAY_ON,
       COM_PLAY_OFF};

int comInit(void);
int comWorkPrompt(int word_count, const char ** word_list);
void comWorkGfxCommand(int word_count, const char ** word_list);
int comWorkObject(char *,int, const char **);
int comSceneCommand(int wc, const char **wl);
void comTimeProc(void);
void comRedraw(void);
void comDBRedraw(void);
void comMessage(const char *s);
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
int comGrab(transMat *tm, char *name);
int comGetCurrentCenter(double *);

int comWriteModelview(FILE *f);

int comSetDefMat(struct RENDER_MATERIAL *mat);

int comTestTex3D(int, int, int);

int comGenCubeLookup();

void comCMICallback(const cmiToken *t);

void comSetInitCommand(const char *s);

void tunnelvision(structObj *o);

#endif


