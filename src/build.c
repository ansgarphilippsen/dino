#include <stdio.h>
#include <stdlib.h>

#include "build.h"
#include "dino.h"
#include "com.h"
#include "cl.h"
#include "mat.h"
#include "Cmalloc.h"
#include "conn.h"

static int buildTagByBond(dbmStructNode *node, structAtom *ap, int *count);
static int buildSetTorsion(dbmStructNode *node, struct BUILD_TORSION_SPEC *t, int wc, char **wl);
static int buildCalcTorsion(buildInst *bi, int num);
static int buildApplyTorsion(buildInst *bi, int num);
static int buildTransformPos(transMat *trans, struct STRUCT_APOS *pos);
static int buildCompareAtoms(const void *ptr1, const void *ptr2);

int buildTag(dbmStructNode *node, const char *x1, const char *x2) 
{
  char m1[256],m2[256],c1[256],c2[256],r1[256],r2[256],a1[256],a2[256];
  const char *s1,*s2;
  int n1,n2;
  char message[256];
  int tag_count;

  // split on :
  s1=clStrchr(x1,':')+1;
  s2=clStrchr(x2,':')+1;

  if(structSubMatch(node,s1,m1,c1,r1,a1)!=0)
    return -1;
  if(structSubMatch(node,s2,m2,c2,r2,a2)!=0)
    return -1;

  n1=structSubGetNum(node,m1,c1,r1,a1);
  if(n1<0) {
    sprintf(message,"atom not found: %s\n",s1);
    comMessage(message);
    return -1;
  }
  n2=structSubGetNum(node,m2,c2,r2,a2);
  if(n2<0) {
    sprintf(message,"atom not found: %s\n",s2);
    comMessage(message);
    return -1;
  }

  structClearFlag(node,STRUCT_BUILD_TAG);

  node->atom[n2].flag |= STRUCT_BUILD_TAG;

  tag_count=0;
  buildTagByBond(node, &node->atom[n1], &tag_count);

  fprintf(stderr,"%d atoms tagged\n",tag_count);

  return 0;
}

int buildTagByBond(dbmStructNode *node, structAtom *ap, int *count)
{
  int i;
  if(ap==NULL)
    return 0;

  // return if already tagged
  if(ap->flag & STRUCT_BUILD_TAG)
    return 0;

  ap->flag |= STRUCT_BUILD_TAG;
  (*count)++;
  for(i=0;i<ap->bondc;i++) {
    buildTagByBond(node,node->bond[ap->bondi[i]].atom1,count);
    buildTagByBond(node,node->bond[ap->bondi[i]].atom2,count);
  }
  return 0;
}

static int buildSetTorsion(dbmStructNode *node, struct BUILD_TORSION_SPEC *t, int wc, char **wl)
{
  /*
    1. identify which four atoms make up the torsion angle
      from 1 atom :
      from 2 atoms:
      from 4 atoms: 
  */
  if(wc<=0) {
    return -1;
  } else if(wc==1) {
  } else if(wc==2) {
  } else if(wc==4) {
  } else {
    return -1;
  }

  return 0;
}

int buildTransAll(buildInst *bi)
{
  int i;
  
  // reset all atoms by copying apos_orig to apos
  memcpy(bi->apos,bi->apos_orig,bi->apos_size);

  // apply all torsion angle transformations
  for(i=0;i<bi->tor_count;i++) {
    buildCalcTorsion(bi,i);
    buildApplyTorsion(bi,i);
  }

  // the rigid body transformation applies to all atoms
  // and is implemented in the draw routine !
  return 0;
}

static int buildCalcTorsion(buildInst *bi, int num)
{
  struct BUILD_TORSION_SPEC *ts=&bi->tor[num];
  double ax,ay,az;
  // extract axis from atoms 1 and 2
  /*
  ax=ts->ap[1]->p->x-ts->ap[2]->p->x;
  ay=ts->ap[1]->p->y-ts->ap[2]->p->y;
  az=ts->ap[1]->p->z-ts->ap[2]->p->z;
  */

  matMakeRotMat(ts->delta,ax,ay,az,ts->trans.rot);
  return 0;
}

static int buildApplyTorsion(buildInst *bi, int num)
{
  struct BUILD_TORSION_SPEC *ts = &bi->tor[num];
  struct BUILD_ATOM_GROUP *agroup = &ts->agroup;
  int ac;

  for(ac=0;ac<agroup->count;ac++) {
    buildTransformPos(&ts->trans,agroup->ap[ac]->p);
  }
  return 0;
}

static int buildTransformPos(transMat *trans, struct STRUCT_APOS *pos)
{
  double a[4],b[4];

  a[0]=pos->x-trans->cen[0];
  a[1]=pos->y-trans->cen[1];
  a[2]=pos->z-trans->cen[2];
  a[3]=1.0;

  matMultVM(a,trans->rot,b);
  b[0]+=trans->tra[0];
  b[1]+=trans->tra[1];
  b[2]+=trans->tra[2];

  pos->x=b[0]+trans->cen[0];
  pos->y=b[1]+trans->cen[1];
  pos->z=b[2]+trans->cen[2];

  return 0;
}

int buildGenTor(buildInst *bi)
{
  int count,max;
  int mcount;
  int ccount,coff,cnum;
  int rcount,roff,rnum;
  int acount,aoff,anum;
  int prev_atom,next_atom;

  struct CONN_ENTRY *prev,*current,*next;

  count=0;
  max=bi->atom_count/4;

  /*
    for each residue:
     - get previous (-2) and next (-3) atom
     - for each atom in the torsion entry matching the residue name:
       - try to find match
       - if all four match generate torsion spec

  */

  for(mcount=0;mcount<bi->hmc;mcount++) {
    coff=bi->hm[mcount].offset;
    cnum=bi->hm[mcount].count;
    for(ccount=0;ccount<cnum;ccount++) {
      roff=bi->hc[coff+ccount].offset;
      rnum=bi->hc[coff+ccount].count;
      for(rcount=0;rcount<rnum;rcount++) {
	aoff=bi->hr[roff+rcount].offset;
	anum=bi->hr[roff+rcount].count;
	//	conGetEntry(bi->atom[bi->batom[aoff]].residue->name)
	for(acount=0;acount<anum;acount++) {
	  bi->batom[aoff+acount];
	}
      }
    }
  }
  


  return 0;
}

/*
  (re)generate hierarchy information
  based on the current atom list
*/

int buildGenHierarchy(buildInst *bi)
{
  int i;
  biAtom *batom;
  int acount,rcount,ccount,mcount;
  int rnum,cnum,mnum;
  int roff,coff,moff;
  biHP *hptr;
  char message[256];

  batom = Ccalloc(bi->atom_count,sizeof(biAtom));
  acount=bi->atom_count;

  for(i=0;i<bi->atom_count;i++) {
    batom[i].n=i;
    batom[i].ap=&bi->atom[i];
  }

  qsort(batom,bi->atom_count,sizeof(biAtom),buildCompareAtoms);

  // now batom is sorted according to model, chain and residue

  /*
    all stuff below is so complicated as to
    save as much memory as possible and to
    avoid fragmentation
  */

  // first round: count only
  rcount=1;
  ccount=1;
  mcount=1;
  rnum=batom[0].ap->residue->num;
  cnum=batom[0].ap->chain->num;
  mnum=batom[0].ap->model->num;

  for(i=1;i<bi->atom_count;i++) {
    if(mnum!=batom[i].ap->model->num) {
      mnum=batom[i].ap->model->num;
      mcount++;
      cnum=batom[i].ap->chain->num;
      ccount++;
      rnum=batom[i].ap->residue->num;
      rcount++;
    } else if(cnum!=batom[i].ap->chain->num) {
      cnum=batom[i].ap->chain->num;
      ccount++;
      rnum=batom[i].ap->residue->num;
      rcount++;
    } else if(rnum!=batom[i].ap->residue->num) {
      rnum=batom[i].ap->residue->num;
      rcount++;
    }
  }

  // allocate array and split it
  hptr=Ccalloc(rcount+ccount+mcount,sizeof(biHP));
  bi->hr=&hptr[0];
  bi->hrc=rcount;
  bi->hc=&hptr[rcount];
  bi->hcc=ccount;
  bi->hm=&hptr[ccount+rcount];
  bi->hmc=mcount;

  // second round: assign actual offset and count

  rcount=0;
  ccount=0;
  mcount=0;

  rnum=batom[0].ap->residue->num;
  cnum=batom[0].ap->chain->num;
  mnum=batom[0].ap->model->num;

  bi->hr[rcount].offset=0; bi->hr[rcount].count=1;
  bi->hc[ccount].offset=0; bi->hc[ccount].count=1;
  bi->hm[mcount].offset=0; bi->hm[mcount].count=1;

  for(i=1;i<bi->atom_count;i++) {
    if(mnum!=batom[i].ap->model->num) {
      // start new model entry
      mnum=batom[i].ap->model->num;
      bi->hm[mcount+1].offset=bi->hm[mcount].offset+bi->hm[mcount].count; 
      bi->hm[mcount+1].count=1;
      mcount++;
      // start new chain entry
      cnum=batom[i].ap->chain->num;
      bi->hc[ccount+1].offset=bi->hc[ccount].offset+bi->hc[ccount].count; 
      bi->hc[ccount+1].count=1;
      ccount++;
      // start new residue entry
      rnum=batom[i].ap->residue->num;
      bi->hr[rcount+1].offset=bi->hr[rcount].offset+bi->hr[rcount].count; 
      bi->hr[rcount+1].count=1;
      rcount++;
    } else if(cnum!=batom[i].ap->chain->num) {
      // increase chain count in model entry
      bi->hm[mcount].count++;
      // start new chain entry
      cnum=batom[i].ap->chain->num;
      bi->hc[ccount+1].offset=bi->hc[ccount].offset+bi->hc[ccount].count; 
      bi->hc[ccount+1].count=1;
      ccount++;
      // start new residue entry
      rnum=batom[i].ap->residue->num;
      bi->hr[rcount+1].offset=bi->hr[rcount].offset+bi->hr[rcount].count;
      bi->hr[rcount+1].count=1;
      rcount++;
    } else if(rnum!=batom[i].ap->residue->num) {
      // increase residue count in chain entry
      bi->hc[ccount].count++;
      // start new residue entry
      rnum=batom[i].ap->residue->num;
      bi->hr[rcount+1].offset=bi->hr[rcount].offset+bi->hr[rcount].count; 
      bi->hr[rcount+1].count=1;
      rcount++;
    } else {
      // increase atom count in residue entry
      bi->hr[rcount].count++;
    }
  }

  // test output - useage skeleton
  /*
  for(mcount=0;mcount<bi->hmc;mcount++) {
    coff=bi->hm[mcount].offset;
    cnum=bi->hm[mcount].count;
    fprintf(stderr,"model %d: coff %d cnum %d\n",mcount,coff,cnum);
    for(ccount=0;ccount<cnum;ccount++) {
      roff=bi->hc[coff+ccount].offset;
      rnum=bi->hc[coff+ccount].count;
      fprintf(stderr," chain %d: roff %d rnum %d\n",
	      coff+ccount, roff, rnum);
      for(rcount=0;rcount<rnum;rcount++) {
	fprintf(stderr,"  residue %d: off %d count %d\n",
		roff+rcount,
		bi->hr[roff+rcount].offset,
		bi->hr[roff+rcount].count);
      }
      
    }
  }
  */

  sprintf(message,"%d models, %d chains, %d residues and %d atoms\n",
	  bi->hmc,bi->hcc, bi->hrc, bi->atom_count);

  bi->batom=batom;

  return 0;
}

static int buildCompareAtoms(const void *ptr1, const void *ptr2)
{
  int ret;
  biAtom *a1=(biAtom *)ptr1;
  biAtom *a2=(biAtom *)ptr2;

  ret=a1->ap->model->num-a2->ap->model->num;
  if(ret!=0) {
    return ret;
  } else {
    ret=a1->ap->chain->num-a2->ap->chain->num;
    if(ret!=0) {
      return ret;
    } else {
      ret=a1->ap->residue->num-a2->ap->residue->num;
      return ret;
    }
  }

  return 0;
}
