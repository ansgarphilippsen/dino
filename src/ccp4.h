#include <stdio.h>

#include "dbm.h"

#ifndef CCP4_H
#define CCP4_H

struct CCP4_MAP_HEADER
{
  long nc,nr,ns;
  long mode;
  long ncstart,nrstart,nsstart;
  long nx,ny,nz;
  float x,y,z;
  float alpha,beta,gamma;
  long mapc,mapr,maps;
  long amin,amax,amean;
  long ispg;
  long nsymbt;
  long lskflag;
  long skwmat[9];
  long skwtrn[3];
  long dummy[15];
  long map;
  long machst;
  float arms;
  long nlabl;
  long label[200];
};

int ccp4Read(FILE *f, dbmScalNode *node);

#endif
