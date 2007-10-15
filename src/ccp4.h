#include <stdio.h>

#include "dbm.h"

#ifndef CCP4_H
#define CCP4_H

struct CCP4_MAP_HEADER
{
  int nc,nr,ns;
  int mode;
  int ncstart,nrstart,nsstart;
  int nx,ny,nz;
  float x,y,z;
  float alpha,beta,gamma;
  int mapc,mapr,maps;
  int amin,amax,amean;
  int ispg;
  int nsymbt;
  int lskflag;
  float skwmat[9];
  float skwtrn[3];
  int dummy[15];
  int map;
  int machst;
  float arms;
  int nlabl;
  int label[200];
};

int ccp4Read(FILE *f, dbmScalNode *node);

#endif
