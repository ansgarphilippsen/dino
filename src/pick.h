#ifndef _PICK_H
#define _PICK_H

typedef struct DBM_PICK_ELEMENT {
  char name[64];
  char id[64];
  float p[3];
}dbmPickElement;

typedef struct DBM_PICK_LIST {
  dbmPickElement *ele;
  int count,max;
}dbmPickList;

/*
  routines in dbm.c
*/

#endif
