#ifndef STRUCT_READ_H
#define STRUCT_READ_H

#include <stdio.h>

#include "dbm.h"
#include "struct_db.h"

#define STRUCT_FILE_FORMAT_PDB 1
#define STRUCT_FILE_FORMAT_XPL 2
#define STRUCT_FILE_FORMAT_PQR 3
#define STRUCT_FILE_FORMAT_CRD 4


struct STRUCT_FILE_ATOM_ENTRY
{
  int anum;        /* unique atom number in file */
  char aname[8];   /* IUPAC name of atom */
  char element[3]; /* element symbol of atom */
  int rnum;        /* residue number */
  char rname[8];   /* residue name */
  int cnum;        /* chain number */
  char cname[8];    /* chain name */
  int mnum;        /* model number */
  char mname[8];    /* model name */
  double x,y,z;    /* cartesian coordinates of atom */
  double c1,c2,c3;  /* custom values */
};

struct STRUCT_FILE_CONNECT_ENTRY
{
  int a1,a2;
};

struct STRUCT_FILE_SECS_ENTRY
{
  int type;
  char chain[4];
  int start, end;
};

struct STRUCT_FILE {
  struct STRUCT_FILE_ATOM_ENTRY *atom_entry;
  int atom_count, atom_max;
  struct STRUCT_FILE_CONNECT_ENTRY *connect_entry;
  int connect_count, connect_max;
  struct STRUCT_FILE_SECS_ENTRY *secs_entry;
  int secs_count, secs_max;
};

struct PDB_CRYST_ENTRY {
  double a,b,c,alpha,beta,gamma;
  char spcgrp[16];
  int z;
};

struct CHARMM_TRJ_HEADER {
  char hdrr[4];
  int icntrl[20];
  int ntitle;
  char title[1024];
  int num, istep, freq,nstep;
  int t_atom_count,f_atom_count, atom_count;
};


int structFileEntry2DB(struct STRUCT_FILE *,dbmStructNode *);

int pdbRead(FILE *f,dbmNode *node);
int pdbLine2AtomEntry(char *,struct STRUCT_FILE_ATOM_ENTRY *);
int pdbLine2ConnectEntry(char *,struct STRUCT_FILE_CONNECT_ENTRY *);

int xplorPDBRead(FILE *f,dbmNode *node);
int xplorPDBLine2AtomEntry(char *,struct STRUCT_FILE_ATOM_ENTRY *);

int charmmRead(FILE *f,dbmNode *node);
int charmmLine2AtomEntry(char *,struct STRUCT_FILE_ATOM_ENTRY *);
int charmmTrjRead(FILE *f, dbmStructNode *node, int sf);

int pqrRead(FILE *f,dbmNode *node);
int pqrLine2AtomEntry(char *,struct STRUCT_FILE_ATOM_ENTRY *);

int dinoTrjRead(FILE *f, dbmStructNode *node, int sf);
int binposTrjRead(FILE *f, dbmStructNode *node, int sf);

int bonesRead(FILE *f, dbmNode *node);

#endif
