#include <stdio.h>

#ifndef XPLOR_H
#define XPLOR_H

#include "dbm.h"

struct XPLOR_MAP_HEADER
{
  int na,amin,amax,nb,bmin,bmax,nc,cmin,cmax;
  double a,b,c,alpha,beta,gamma; /* 48 byte */
  char format[4];
};

struct XPLOR_ATOM_ENTRY {
  char record[6+1];
  int serial;
  char name[4+1];
  char ele[2+1];
  char altLoc;
  char resName[3+1];
  char chainID[4];
  int resSeq;
  char iCode;
  float x,y,z,occupancy,tempFactor;
};

struct XPLOR_PDB {
  struct XPLOR_ATOM_ENTRY *atom_entry;
  int atom_count;
  struct XPLOR_ATOM_ENTRY *hetatm_entry;
  int hetatm_count;
};


/*******
CHARACTER*4 HDR
INTEGER IO ISTART NATOM NSAVC NFREAT
DOUBLE PRECISION DELTA


      WRITE(UNIT) HDR,I0,ISTART,NSAVC,I0,I0,
     &              I0,I0,I0,NATOM-NFREAT,DELTA,
     &              I0,I0,I0,I0,I0,I0,I0,I0,I0

     WRTITL(UNIT,-1): CHARACTER*80
     WRITE(UNIT) NATOM
     IF (NFREAT.NE.NATOM) WRITE(UNIT) (FREEAT(I),I=1,NFREAT)

     if QFIRST
     single precision
     WRITE (UNIT) (REAL(X(I)),I=1,NATOM)
     WRITE (UNIT) (REAL(Y(I)),I=1,NATOM)
     WRITE (UNIT) (REAL(Z(I)),I=1,NATOM)

     for each step
     single precision
     WRITE (UNIT) (REAL(X(FREEAT(I))),I=1,NFREAT)
     WRITE (UNIT) (REAL(Y(FREEAT(I))),I=1,NFREAT)
     WRITE (UNIT) (REAL(Z(FREEAT(I))),I=1,NFREAT)


******/

// align on 8 byte !!

struct CNS_TRJ_HEADER {
  char hdr[4];
  int dummy1;
  int istart,nsavc;
  int dummy2[5];
  int diff;
  double delta;
  int dummy3[8]; // should really be 9 !
};

struct CNS_TRJ_ENTRY {
  float x,y,z;
};

int xplorMapARead(FILE *f, dbmScalNode *node);
int xplorMapBRead(FILE *f, dbmScalNode *node);

int cnsTrjRead(FILE *f, dbmStructNode *node, int flag);

#endif
