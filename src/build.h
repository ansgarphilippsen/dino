#ifndef BUILD_H
#define BUILD_H

#include "struct_db.h"
#include "transform.h"

typedef struct BUILD_INSTANCE_ATOM {
  int n;
  // pointer to actual atom
  struct STRUCT_ATOM *ap;
  // list of pointers to actual bonds
  struct STRUCT_BOND *bond[STRUCT_MAX_BOND_PER_ATOM];
  int bond_count;
}biAtom;

typedef struct BUILD_INSTANCE_HP {
  int offset, count;
}biHP;

struct BUILD_ATOM_GROUP {
  struct STRUCT_ATOM **ap;
  int count, max;
};

struct BUILD_TORSION_SPEC {
  //  struct STRUCT_ATOM *ap[4];
  int atomi[4];
  double axis[3];
  double angle,delta;
  transMat trans;
  struct BUILD_ATOM_GROUP agroup;
};

typedef struct BUILD_INSTANCE {
  // temporary copy of data
  struct STRUCT_ATOM **ap_save;  // orig atom pointers
  struct STRUCT_ATOM *atom;      // atoms
  struct STRUCT_BOND *bond;      // bonds
  struct STRUCT_APOS *apos;      // position
  struct STRUCT_APOS *apos_orig; // original position
  int atom_count,atom_max;
  int apos_size;
  int bond_count,bond_max;

  // hierarchy
  biAtom *batom;
  biHP *hr;
  int hrc;
  biHP *hc;
  int hcc;
  biHP *hm;
  int hmc;

  struct BUILD_TORSION_SPEC *tor;
  int tor_count;

  transMat trans,trans_frag;
}buildInst;

// test routine
int buildTag(struct DBM_STRUCT_NODE *n, const char *s1, const char *s2);

int buildGenHierarchy(buildInst *bi);

int buldGenTor(buildInst *bi);
int buildTransAll(buildInst *bi);

#endif
