#ifndef SCALE_READ_H
#define SCALE_READ_H

#include <stdio.h>

#include "dbm.h"
#include "ccp4.h"
#include "xplor.h"
#include "uhbd.h"
#include "mead.h"
#include "delphi.h"
#include "charmm.h"

enum                {SCAL_READ_DINO,
		     SCAL_READ_XPLOR_ASCII,
		     SCAL_READ_XPLOR_BINARY,
		     SCAL_READ_CNS_ASCII,
		     SCAL_READ_CNS_BINARY,
		     SCAL_READ_UHBD_ASCII,
		     SCAL_READ_UHBD_BINARY,
		     SCAL_READ_CCP4_BINARY,
		     SCAL_READ_CHARMM_BINARY,
		     SCAL_READ_MEAD,
		     SCAL_READ_DELPHI,
		     SCAL_READ_DELPHIG};

struct DINO_GRID_HEADER
{
  int magic; /* must be 31415 */
  int type; /* 1 is integers 2 is floats */
  int nu,nv,nw;
  float deltax,deltay,deltaz;
  float alpha,beta,gamma;
  float offsetx,offsety,offsetz;
};


int scalRead(dbmScalNode *node, int type, FILE *f, int swap_flag);
int scalReadDino(FILE *f, dbmScalNode *node);


#endif
