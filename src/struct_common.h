#ifndef STRUCT_COMMON_H
#define STRUCT_COMMON_H

struct STRUCT_APOS {
  float x,y,z;
};

struct STRUCT_ATOM_PROP {
  float r,g,b;
  float radius;
  float c[3][4];
};

struct STRUCT_DB_MIN_MAX {
  int anum1,anum2;
  int rnum1,rnum2;
  int model1,model2;
  float bfac1,bfac2;
  float weight1,weight2;
};

#endif
