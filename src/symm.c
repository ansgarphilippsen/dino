#include <stdio.h>
#include <string.h>

#include "symm.h"
#include "sglib.h"

/*
  this code taken from sgquick.c
  RW Grosse-Kunstleve
*/


int symGetSgInfo(T_SgInfo *SgInfo, const char *SgName)
{
  int vol='A';
  const T_TabSgName *tsgn;
  T_RTMx       listSeitzMx[192];
  T_RotMxInfo  listRotMxInfo[192];

  tsgn = NULL;

  tsgn = FindTabSgNameEntry(SgName, vol);
  if (tsgn == NULL) return -1; /* no matching table entry */
  SgName = tsgn->HallSymbol;

  SgInfo->MaxList = 192; /* absolute maximum number of symops */

  SgInfo->ListSeitzMx = listSeitzMx;

  SgInfo->ListRotMxInfo = listRotMxInfo;

  InitSgInfo(SgInfo);
  SgInfo->TabSgName = tsgn; /* in case we know the table entry */

  ParseHallSymbol(SgName, SgInfo);
  if (SgError != NULL) return -1;

  return CompleteSgInfo(SgInfo);
}

int symExtractMatrix(T_SgInfo *sginfo, struct SYMM_INFO *s)
{
  int i;

  /*
    ( xs )     ( R[0]  R[1]  R[2]  T[0]/STBF )  ( x )
    ( ys )  =  ( R[3]  R[4]  R[5]  T[1]/STBF )  ( y )
    ( zs )     ( R[6]  R[7]  R[8]  T[2]/STBF )  ( z )
    ( 1  )     (  0     0     0     1        )  ( 1 )
  */

  /* THESE ARE FRACTIONAL COORDINATES ! */

  for(i=0;i<sginfo->nList;i++) {
    s->mat[i].m[0]=(float)sginfo->ListSeitzMx[i].s.R[0];
    s->mat[i].m[1]=(float)sginfo->ListSeitzMx[i].s.R[1];
    s->mat[i].m[2]=(float)sginfo->ListSeitzMx[i].s.R[2];
    s->mat[i].m[3]=(float)(sginfo->ListSeitzMx[i].s.T[0])/(float)STBF;
    s->mat[i].m[4]=(float)sginfo->ListSeitzMx[i].s.R[3];
    s->mat[i].m[5]=(float)sginfo->ListSeitzMx[i].s.R[4];
    s->mat[i].m[6]=(float)sginfo->ListSeitzMx[i].s.R[5];
    s->mat[i].m[7]=(float)(sginfo->ListSeitzMx[i].s.T[1])/(float)STBF;
    s->mat[i].m[8]=(float)sginfo->ListSeitzMx[i].s.R[6];
    s->mat[i].m[9]=(float)sginfo->ListSeitzMx[i].s.R[7];
    s->mat[i].m[10]=(float)sginfo->ListSeitzMx[i].s.R[8];
    s->mat[i].m[11]=(float)(sginfo->ListSeitzMx[i].s.T[2])/(float)STBF;
    s->mat[i].m[12]=0.0;
    s->mat[i].m[13]=0.0;
    s->mat[i].m[14]=0.0;
    s->mat[i].m[15]=1.0;
  }
  s->mcount=i;
  return 0;
}

int symGetMatrixByNumber(struct SYMM_INFO *s)
{
  char name[256];
  T_SgInfo sginfo;

  sprintf(name,"%d",s->num);

  if(symGetSgInfo(&sginfo,(const char *)name)==-1)
    return -1;
  if(symExtractMatrix(&sginfo,s)==-1)
    return -1;
  return 0;
}


int symGetMatrixByName(struct SYMM_INFO *s)
{
  char name[256];
  T_SgInfo sginfo;

  strcpy(name,s->name);

  if(symGetSgInfo(&sginfo,(const char *)name)==-1)
    return -1;
  if(symExtractMatrix(&sginfo,s)==-1)
    return -1;
  return 0;
}
