#ifndef XTAL_H
#define XTAL_H

// helper class
struct XTAL_SYMMOP {
  double mat[16];
};

// crystallographic symmetry
struct XTAL {
  double a,b,c,alpha,beta,gamma;
  double va[3],vb[3],vc[3];
  double cc[8][3];
  int space_group;
  char space_group_name[64];
  struct XTAL_SYMMOP symmop[32];
  int symmop_count;
};

// helical symmetry
struct HELICAL {
  double angle; // rotation angle per step
  double dist;  // offset along axis per step 
  double axr; // axial ratio
};

#endif
