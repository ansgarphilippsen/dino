#ifndef STRUCT_DB_H
#define STRUCT_DB_H

#include "dbm_common.h"
#include "rex.h"
#include "render.h"
#include "cgfx.h"
#include "struct_obj.h"
#include "struct_common.h"
#include "cubearray.h"
#include "set.h"
#include "transform.h"
#include "pick.h"

enum                 {SEL_ATOM,
		      SEL_RESIDUE,
		      SEL_CHAIN,
		      SEL_MODEL};

enum                 {STRUCT_TRJ_NONE,
		      STRUCT_TRJ_CHARMM,
		      STRUCT_TRJ_XPLOR,
		      STRUCT_TRJ_DINO,
		      STRUCT_TRJ_CNS,
		      STRUCT_TRJ_BD,
		      STRUCT_TRJ_BINPOS};

enum {STRUCT_WRITE_PDB,
      STRUCT_WRITE_XPL,
      STRUCT_WRITE_CRD,
      STRUCT_WRITE_XYZR};


enum {STRUCT_SEL_ANUM,
      STRUCT_SEL_ANAME,
      STRUCT_SEL_RNUM,
      STRUCT_SEL_RNAME,
      STRUCT_SEL_RTYPE,
      STRUCT_SEL_CLASS,
      STRUCT_SEL_CHAIN,
      STRUCT_SEL_MODEL,
      STRUCT_SEL_OCC,
      STRUCT_SEL_BFAC,
      STRUCT_SEL_ELE,
      STRUCT_SEL_OBJECT,
      STRUCT_SEL_WITHIN};

enum {STRUCT_PROP_COLOR,
      STRUCT_PROP_COLOR1,
      STRUCT_PROP_COLOR2,
      STRUCT_PROP_COLOR3,
      STRUCT_PROP_DIST,   // distance from $CP
      STRUCT_PROP_BFAC,
      STRUCT_PROP_ROT,
      STRUCT_PROP_TRANS,
      STRUCT_PROP_RTC,    // rot trans cen all in one
      STRUCT_PROP_RCEN,   // center of rotation
      STRUCT_PROP_RTYPE,  // residue type
      STRUCT_PROP_NCI,    // non-covalent-interaction
      STRUCT_PROP_RAD,    // radius
      STRUCT_PROP_SMODE,  // selection mode
      STRUCT_PROP_CELL,   // unit cell
      STRUCT_PROP_SG,     // space group  
      STRUCT_PROP_TFAST,  // fast trj update
      STRUCT_PROP_FRAME   // current frame to display
};

enum {STRUCT_RTYPE_COIL,
      STRUCT_RTYPE_HELIX,
      STRUCT_RTYPE_STRAND};

enum {STRUCT_MISC=0,
      STRUCT_PROTEIN,
      STRUCT_NA,
      STRUCT_CARBO};

enum {STRUCT_PLAY_LOOP=0,
      STRUCT_PLAY_ROCK,
      STRUCT_PLAY_SINGLE};

/*
  the dataset to describe 3d structures based on
  atomic coordinates
*/

/* chemical properties of an atom */
struct STRUCT_ATOM_CHEM {
  int an;                /* atomic number */
  char element[4];       /* element */
  double charge;         /* charge */
  double vdwr;           /* van der Waals radius */
};


/* some bit parameters */

// restrict is not implemented this way
// TODO
#define STRUCT_RESTRICT 0x1

#define STRUCT_HBD 0x2  // HBond donor
#define STRUCT_HBA 0x4  // HBond acceptor
#define STRUCT_SBD 0x8  // SBridge donor
#define STRUCT_SBA 0x10 // SBridge acceptor
#define STRUCT_BUILD_TAG 0x20

#define STRUCT_MAX_BOND_PER_ATOM 8

#define STRUCT_CONN_IMPL 1<<0
#define STRUCT_CONN_EXPL 1<<1
#define STRUCT_CONN_DIST 1<<2

/* bonds */
struct STRUCT_BOND {
  int n; /* internal counter */
  struct STRUCT_ATOM *atom1,*atom2;
  struct STRUCT_ATOM_PROP *prop1,*prop2;
  float axis[3],length,angle;
  float rotmat[16];
  struct STRUCT_OBJ_ATOM *oap1,*oap2; // temp var
};

/* simple element: the atom */
typedef struct STRUCT_ATOM {
  int n;                /* continuos number in db */
  int anum;             /* number from file */
  char name[16];        /* atom name */
  struct STRUCT_APOS *p;  /* position in space, implemented for trj */

  float xx,yy,zz;         /* position in space, ONLY used during read */

  float bfac;          /* crystallographic data: b-factor*/
  float weight;        /* weight, is occupancy for xtallographic data */
  float c1,c2,c3;      /* custom values */
  struct STRUCT_ATOM_CHEM chem; 
                        /* chemical properties */

  int residue_num;                /*               */
  struct STRUCT_RESIDUE *residue; /*               */
  int chain_num;                  /*               */
  struct STRUCT_CHAIN *chain;     /* back pointers */
  int model_num;                  /*               */
  struct STRUCT_MODEL *model;     /*               */

  struct STRUCT_ATOM_PROP def_prop;
  int restrict2;
  int flag;    // used for a number of bit parameters
  
  int bondi[STRUCT_MAX_BOND_PER_ATOM],bondc,bondm;
}structAtom;

/* first level of organization: the residue */
struct STRUCT_RESIDUE {
  char name[16];
  int num;
  int type;                    /* type: helix, strand or coil */
  int class;                   /* class: misc, protein, na, ch */
  struct STRUCT_ATOM **atom;   /* pointer to pointer to atoms */
  int *atom_index;             /* atom indexes */
  int atom_count;              /* number of atoms in this residue */
  int atom_max,atom_add;       /* memory management */
  struct STRUCT_CHAIN *chain;  /* back pointer */
  int chain_num;               /*      "       */
  float dir[3], v0[3],v1[3], v2[3], v3[3], v4[3], v5[3], v6[3], v7[3];
  int res_id;
};

/* second level of organization: the chain */
struct STRUCT_CHAIN {
  char name[16];
  int num;
  struct STRUCT_RESIDUE **residue;  /* pointer to pointers to residues */
  int *residue_index;               /* residue indexes */
  int residue_count;                /* number of residues in this chain */
  int residue_max, residue_add;     /* memory management */
  struct STRUCT_ATOM **atom;        /* pointer to pointer to atoms */
  int *atom_index;
  int atom_count;                   /* number of atoms in this chain */
  int atom_max,atom_add;            /* memory management */
  struct STRUCT_MODEL *model;       /* back pointer */
  int model_num;
};

/* third level of organization: the model */
struct STRUCT_MODEL {
  int num;
  struct STRUCT_CHAIN **chain;   /* pointer to pointers to chains */
  int *chain_index;              /* chain indexes */
  int chain_count;               /* number of chains in this model */
  int chain_max,chain_add;       /* memory management */
  struct STRUCT_ATOM **atom;     /* pointer to pointer to atoms */
  int *atom_index;               /* atom indexes */
  int atom_count;                /* number of atoms in this model */
  int atom_max,atom_add;         /* memory management */
};


struct STRUCT_SINGULAR_BOND {
  struct STRUCT_ATOM *atom;
  struct STRUCT_ATOM_PROP *prop;
};

struct STRUCT_NBOND {
  struct STRUCT_ATOM *atom1,*atom2;
  float c[3];
};


struct STRUCT_CONNECTIVITY {
  struct STRUCT_ATOM *ap1,*ap2;
};


struct STRUCT_TRJ_POSITION {
  float x,y,z;
};

struct STRUCT_BDTRJ_FRAME {
  int n;
  int start,end;
};

struct STRUCT_BDTRJ_ENTRY {
  int frame;
  int index;
  float x,y,z;
};

typedef struct STRUCT_TRJ {
  int type;
  struct STRUCT_TRJ_POSITION *pos;
  int atom_count;  /* number of atoms in each trj */
  int size;        /* = atom_count*sizeof(struct STRUCT_TRJ_POSITION) */
  int frame_count;
  // following is for TRJ_BD
  struct STRUCT_BDTRJ_FRAME *frame;
  // use above frame_count;
  struct STRUCT_BDTRJ_ENTRY *entry;
  int entry_count;
}structTrj;

struct TRJ_PLAY {
  int frame,start,end,step;
  int mode;
  int wait,wait_count;
  int delay,delay_count;
};


typedef struct DBM_STRUCT_NODE {
  DBM_NODE_COMMON_HEADER

  Select *restrict2;

  struct STRUCT_OBJ *obj;
  int *obj_flag;
  int obj_count;
  int obj_max;

  struct STRUCT_MODEL *model;     /* pointer to models */
  int model_count;                /* number of models in this db */
  int model_max,model_add;        /* memory management */
  int model_flag;

  struct STRUCT_CHAIN *chain;     /* pointer to chains */
  int chain_count;                /* number of chains in this db */
  int chain_max,chain_add;        /* memory management */
  int chain_flag;

  struct STRUCT_RESIDUE *residue; /* pointer to residue */
  int residue_count;              /* number of residues in this db */
  int residue_max,residue_add;    /* memory management */
  int residue_flag;

  struct STRUCT_ATOM *atom;       /* pointer to atoms */
  int atom_count;                 /* number of atoms in this db */
  int atom_max,atom_add;          /* memory management */

  struct STRUCT_APOS *apos;   /* atom positions array */
  int apos_count;
  int apos_max,apos_add;

  struct STRUCT_BOND *bond;       /* pointer to bonds */
  int bond_count;                 /* number of bonds in this db */
  int bond_max,bond_add;          /* memory management */

#ifdef WITH_NCBONDS
  struct STRUCT_BOND *nbond;      /* pointer to nbonds */
  int nbond_count;                /* number of nbonds in this db */
  int nbond_max,nbond_add;        /* memory management */
#endif

  struct STRUCT_CONNECTIVITY *conn; /* explicit connectivy from file */
  int conn_count;
  int conn_max,conn_add;
  int conn_flag;

  struct XTAL *xtal;              /* pointer to crystallographic info */
  int show_cell;

  struct STRUCT_ATOM_TABLE *atom_table;
  int atom_table_len;
  int smode;            /* selection mode */

  struct STRUCT_DB_MIN_MAX min_max;
  cubeArray *ca;
  int frame;
  int trj_flag;
  structTrj trj;
  int trj_play,trj_fast;
  struct TRJ_PLAY play;

  /*
  float mmat[16];
  */
  /*
  double center[3];
  double rmat[16];
  double trans[3];
  */
}dbmStructNode;

struct STRUCT_ATOM_TABLE {
  int z;
  char e[4];
  double r,g,b;
  double vdwr;
};

int structNewNode(struct DBM_STRUCT_NODE *node);

int structPrep(dbmStructNode *node);
int structPrepRes(dbmStructNode *node);

int structCommand(dbmStructNode *,int ,char **);
int structComNew(dbmStructNode *, int , char **);
int structComSet(dbmStructNode *, int , char **);
int structComGet(dbmStructNode *, int , char **);
int structComRestrict(dbmStructNode *, int , char **);
int structComDel(dbmStructNode *, int, char **);
int structComLoad(dbmStructNode *, int, char **);
int structComPlay(dbmStructNode *, int, char **);
int structComConnect(dbmStructNode *, char *, char *);

int structNew(dbmStructNode *, char *name, int type, Set *set, Select *sel);
int structSet(dbmStructNode *, Set *set);
int structGet(dbmStructNode *, char *prop);

structObj *structNewObj(dbmStructNode *node, char *name);
int structDelObj(dbmStructNode *node,char *name);

int structSetDefault(structObj *obj);

int structBuildCA(struct DBM_STRUCT_NODE *n);

int structReconnect(struct DBM_STRUCT_NODE *n);
int structReconnectNC(struct DBM_STRUCT_NODE *n);
int structCheckNCB(struct DBM_STRUCT_NODE *node, struct STRUCT_ATOM *a1, struct STRUCT_ATOM *a2, float co);

int structConnectAtoms(struct DBM_STRUCT_NODE *n,struct STRUCT_BOND **,int *bc, int *bm, struct STRUCT_ATOM *a1, struct STRUCT_ATOM *a2);

int structSubCommand(struct DBM_STRUCT_NODE *n,char *sub, int wc, char **wl);
int structSubComGet(struct DBM_STRUCT_NODE *node,struct STRUCT_ATOM *ap,int wc, char **wl);
int structSubMatch(struct DBM_STRUCT_NODE *n, char *sub, char *m, char *c, char *r, char *a);
int structSubGetNum(struct DBM_STRUCT_NODE *n, char *m, char *c, char *r, char *a);

int structIsConnected(struct STRUCT_ATOM *a1, struct STRUCT_ATOM *a2);

#ifdef PICK_NEW
int structPick(dbmStructNode *n, double *p1, double *p2, double d, dbmPickList *pl);
#else
struct STRUCT_ATOM ** structPick(struct DBM_STRUCT_NODE *node, double *p1, double *p2);
#endif

int structIsAtomSelected(struct DBM_STRUCT_NODE *node, struct STRUCT_ATOM *atom, Select *sel);
int structEvalAtomPOV(struct DBM_STRUCT_NODE *node, struct STRUCT_ATOM *atom,POV *pov);

float structGetAtomProperty(struct STRUCT_ATOM *ap,const char *prop);
int structSetMinMax(struct DBM_STRUCT_NODE *node);
int structGetMinMax(struct DBM_STRUCT_NODE *, const char *, float *, float *);

int structLoad(struct DBM_STRUCT_NODE *node, int wc, char **wl);

int structPlay(struct DBM_STRUCT_NODE *node);
int structSetFrame(struct DBM_STRUCT_NODE *node, int frame);

int structWrite(struct DBM_STRUCT_NODE *node, structObj *obj, int wc, char **wl);

int structFix(struct DBM_STRUCT_NODE *node);

int structGetRangeVal(dbmStructNode *node, struct STRUCT_ATOM *atom, const char *prop, float *rval);

int structGetRangeXYZVal(dbmStructNode *node, const char *prop, float *p, float *r);

int structDraw(dbmStructNode *node, int f);
int structDrawObj(structObj *obj);
int structDrawBDObj(dbmStructNode *node, structObj *obj);

int structRecalcBonds(dbmStructNode *node);
int structRecalcBondList(struct STRUCT_BOND *bond, int bond_count);

int structGetVectProtein(struct STRUCT_RESIDUE *r);
int structGetVectNA(struct STRUCT_RESIDUE *r);

int structRecenter(dbmStructNode *node);

void structSetFlag(dbmStructNode *node,int mask);
void structClearFlag(dbmStructNode *node,int mask);

#endif
