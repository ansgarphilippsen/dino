#ifndef SYMM_H
#define SYMM_H

#include "sglib.h"

struct SYMM_MATRIX {
  float m[16];
};

struct SYMM_INFO {
  char name[64];
  int num;
  struct SYMM_MATRIX mat[192];
  int mcount;
};

int symGetSgInfo(T_SgInfo *SgInfo, const char *SgName);
int symExtractMatrix(T_SgInfo *SgInfo, struct SYMM_INFO *s);
int symGetMatrixByNumber(struct SYMM_INFO *s);
int symGetMatrixByName(struct SYMM_INFO *s);
int symGetNumberByName(char *n);

#endif
