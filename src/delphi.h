#include <stdio.h>

#ifndef _DELPHI_H
#define _DELPHI_H

#include "dbm.h"

struct DELPHI_HEADER {
  int ivary, nbyte, intdat;
  float ext[3];
  float xang,yang,zang;
  float xstart,xend,ystart,yend,zstart,zend;
  int intx,inty,intz;
};

int delphiRead(FILE *f, dbmScalNode *node);
int delphi2Read(FILE *f, dbmScalNode *node);

#endif
