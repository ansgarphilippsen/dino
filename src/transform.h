#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include <stdio.h>
#include "symm.h"

enum TRANSFORM_DEVICE {TRANS_NONE=0,
		       TRANS_MOUSE,
		       TRANS_DIALS,
		       TRANS_SPACEBALL,
		       TRANS_PAD};
			 

enum TRANSFORM_COMMAND {TRANS_NN=0,
			TRANS_ROTX,
			TRANS_ROTY,
			TRANS_ROTZ,
			TRANS_TRAX,
			TRANS_TRAY,
			TRANS_TRAZ,
			TRANS_SLABN,
			TRANS_SLABF,
			TRANS_CUSTOM};

#define TRANSFORM_MAX_LIST 10
#define TRANSFORM_MAX_COMMAND_LIST 18
#define TRANSFORM_MAX_CUSTOM 8

typedef void (*transCustomFunc)(double val, void *ptr);

typedef struct TRANSFORM_CUSTOM {
  int flag;
  transCustomFunc cb;
  void *ptr;
  double factor;
}transCustom;

typedef struct TRANSFORM_MATRIX {
  double rot[16],tra[4],cen[4];
  double slabn,slabf;
  double slabn2,slabf2;
}transMat;

typedef struct TRANSFORM_LIST {
  transMat *trans;
  int count,max;
} transList;


struct TRANSFORM_DEVICE_LIST_COMMAND { // list of commands for each axis/mask
  int axis,mask;    // axis/mask that defines the command
  int command;      // the command id
  double factor;     // multiplication factor
};

typedef struct TRANSFORM_DEVICE_LIST {
  int device,mask;    // device/name combination of this entry
  char name[64];      // name for this device/mask combination
  struct TRANSFORM_DEVICE_LIST_COMMAND command[TRANSFORM_MAX_COMMAND_LIST];
  transCustom custom[TRANSFORM_MAX_CUSTOM];
  double factor;      // generic factor to regulate ALL commands
  transMat *transform;
}transDeviceList;

int transCommand(transMat *trans, int command, int axis, double value);

void transResetRot(transMat *trans);
void transResetTra(transMat *trans);
void transResetCen(transMat *trans);
void transResetSlab(transMat *trans);
void transReset(transMat *trans);

int transApply(transMat *trans, double *v);
int transApplyf(transMat *trans, float *v);

int transApplyI(transMat *trans, double *v);
int transApplyIf(transMat *trans, float *v);

int transSetRot(transMat *trans, char *s);
int transSetTra(transMat *trans, char *s);
int transSetCen(transMat *trans, char *s);
int transSetAll(transMat *trans, char *s);

const char *transGetRot(transMat *trans);
const char *transGetTra(transMat *trans);
const char *transGetCen(transMat *trans);
const char *transGetCen2(transMat *trans);
const char *transGetAll(transMat *trans);
const char *transGetMM(transMat *trans);

int transMultM(transMat *trans, double *m);
int transMultMf(transMat *trans, float *m);

int transApplyRot(transMat *trans, double *v);
int transApplyRotf(transMat *trans, float *v);

int transCopy(transMat *src, transMat *dest);

void transFromSymm(transMat *tm, struct SYMM_MATRIX *sm);

void transListInit(transList* list, int max);
void transListDelete(transList* list);
int transListGetEntryCount(transList *list);
transMat* transListGetEntry(transList* list, int n);
void transListAddEntry(transList* list,transMat* t);
void transListCopy(transList* src, transList *dest);

#endif
