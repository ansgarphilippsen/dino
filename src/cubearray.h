#ifndef CUBE_ARRAY_H
#define CUBE_ARRAY_H

enum {CA_ADD,CA_WRITE};

typedef void * caPointer;

typedef struct CUBE_ARRAY_ELEMENT {
  caPointer *list;
  int count,max;
}caElement;

typedef struct CUBE_ARRAY {
  int a,b,c;
  float x1,y1,z1,x2,y2,z2;
  float size;
  caElement *element;
  int ecount;
  caPointer *list;
  int lcount,lmax;
}cubeArray;

cubeArray *caInit(float xyz1[3], float xyz2[3], int list_max, float size);
int caOutit(cubeArray *ca);
int caXYZtoABC(cubeArray *ca, float xyz[3], int abc[3]);
int caABCtoXYZ(cubeArray *ca, int abc[3], float xyz[3]);
int caGetLimit(cubeArray *ca, int abc[3], float xyz1[3],float xyz2[3]);
int caAddPointer(cubeArray *ca, int abc[3], caPointer p, int mode);
int caFix(cubeArray *ca);
int caGetList(cubeArray *ca, int abc[3], caPointer **list, int *count);
int caABCtoI(cubeArray *ca, int abc[3]);

int caGetWithinList(cubeArray *ca, float *p, float d, caPointer **l, int *c);

#endif
