#ifndef CHARMM_H
#define CHARMM_H

#include "dbm.h"

struct CHARMM_ATOM_ENTRY {
  int atomno,resno;
  char res[4+1];
  char type[4+1];
  float x,y,z;
  char segid[4+1];
  char resid[4+1];
  double w;
};


struct CHARMM_HEADER {
  unsigned char dummy1[4];
  int nclx, ncly, nclz;
  double dcel, xbcen,ybcen,zbcen;
  unsigned char dummy2[8];
  double epsw,epsp,conc,tmemb,zmemb,epsm;
  unsigned char dummy3[4];
};

struct CHARMM_HEADER_HACK {
  int nclx, ncly, nclz;
  double dcel, xbcen,ybcen,zbcen;
  double epsw,epsp,conc,tmemb,zmemb,epsm;
};

struct CHARMM {
  struct CHARMM_ATOM_ENTRY *atom_entry;
  int atom_count;
};

int charmmRead(FILE *f, dbmNode *node);
int charmmLine2ATOM_ENTRY(char *,struct CHARMM_ATOM_ENTRY *);
int charmm2structDB(struct CHARMM *, struct DBM_STRUCT_NODE *node);

int charmmReadB(FILE *f, dbmScalNode *node, int hack_flag);

int bdtrjRead(dbmStructNode *node, FILE *f, int swap_flag);

#endif
