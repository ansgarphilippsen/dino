#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "dino.h"
#include "struct_read.h"
#include "struct_db.h"
#include "dbm.h"
#include "com.h"
#include "Cmalloc.h"
#include "cl.h"

extern int debug_mode;

static void line_to_atom_entry(struct STRUCT_FILE_ATOM_ENTRY *ce, char *line, int format);
static void line_to_connect_entry(struct STRUCT_FILE_CONNECT_ENTRY *ce, char *line);
static void line_to_secs_entry(struct STRUCT_FILE_SECS_ENTRY *ce, char *line);
static void prep_struct_file(struct STRUCT_FILE *sf, int am, int cm, int sm);
static void free_struct_file(struct STRUCT_FILE *sf);
static void add_atom_entry(struct STRUCT_FILE *sf, struct STRUCT_FILE_ATOM_ENTRY *ae);
static void add_connect_entry(struct STRUCT_FILE *sf, struct STRUCT_FILE_CONNECT_ENTRY *ae);
static void add_secs_entry(struct STRUCT_FILE *sf, struct STRUCT_FILE_SECS_ENTRY *ae);

static struct STRUCT_ATOM_TABLE struct_atom_table[]={
  /* z     e      r    g    b  vdw */
  {  0, "def", 1.0, 1.0, 1.0, 1.5},
  {  1,   "H", 1.0, 1.0, 1.0, 1.17},
  {  6,   "C", 0.7, 0.9, 0.9, 1.75},
  {  7,   "N", 0.0, 0.0, 1.0, 1.55},
  {  8,   "O", 1.0, 0.0, 0.0, 1.40},
  { 15,   "P", 0.0, 1.0, 0.0, 1.7},
  { 16,   "S", 1.0, 1.0, 0.0, 1.8},
  {  0,  "nn", 1.0, 1.0, 1.0, 1.5}
};

static int struct_atom_table_len=sizeof(struct_atom_table)/sizeof(struct STRUCT_ATOM_TABLE);



/******************************************

               PDB Section

*******************************************/

int pdbRead(FILE *f,dbmNode *node)
{
  int i;
  char record[7];
  char line[256];
  struct STRUCT_FILE pdb;
  struct PDB_CRYST_ENTRY pdb_cryst;
  int pdb_cryst_flag=0;
  int model_count;
  //int atom_count;
  //int atom_max;
  //int connect_count;
  //int connect_max;
  //int secs_count;
  //int secs_max;
  char num[16];
  //  struct STRUCT_FILE_ATOM_ENTRY *ao;
  //struct STRUCT_FILE_CONNECT_ENTRY *co;
  struct STRUCT_FILE_ATOM_ENTRY tmpae;
  struct STRUCT_FILE_CONNECT_ENTRY tmpce;
  struct STRUCT_FILE_SECS_ENTRY tmpse;
  int d1,d2[5];
  char c1[6],c2[6],c3[6],c4[6],c5[6];
  
  model_count=-1;
  //atom_count=0;
  //connect_count=0;
  //atom_max=5000;
  //connect_max=5000;
  //pdb.atom_entry=Ccalloc(atom_max,sizeof(struct STRUCT_FILE_ATOM_ENTRY));
  //pdb.connect_entry=Ccalloc(connect_max,sizeof(struct STRUCT_FILE_CONNECT_ENTRY));
  prep_struct_file(&pdb,5000,5000,100);

  
  while(!feof(f)) {
		memset(line,0,256);
    fgets(line,sizeof(line),f);
    for(i=0;i<6;i++) {
      if(isspace(line[i]))
	record[i]='\0';
      else
	record[i]=line[i];
    }
    record[6]='\0';
    if(!strcmp(record,"MODEL")) {
      if(model_count==-1)
	model_count=0;
      model_count++;
    } else if(!strcmp(record,"ATOM") ||
	      !strcmp(record,"HETATM")) {
      tmpae.mnum=model_count;
      clStrcpy(tmpae.mname,"");
      line_to_atom_entry(&tmpae,line, STRUCT_FILE_FORMAT_PDB);
      add_atom_entry(&pdb,&tmpae);
      /*
      pdb.atom_entry[atom_count].mnum=model_count;
      strcpy(pdb.atom_entry[atom_count].mname,"");
      pdbLine2AtomEntry(line,&pdb.atom_entry[atom_count]);
      atom_count++;
      if(atom_count>=atom_max) {
	ao=pdb.atom_entry;
	pdb.atom_entry=Ccalloc(atom_max+5000,
			      sizeof(struct STRUCT_FILE_ATOM_ENTRY));
	memcpy(pdb.atom_entry,ao,
	       atom_max*sizeof(struct STRUCT_FILE_ATOM_ENTRY));
	atom_max+=5000;
	Cfree(ao);
      }
      */
    } else if(!strcmp(record,"CONECT")) {
      d1=0;
      sscanf(line,"%6c%5c%5c%5c%5c%5c",
	     record,c1,c2,c3,c4,c5);
      c1[5]='\0';
      c2[5]='\0';
      c3[5]='\0';
      c4[5]='\0';
      c5[5]='\0';
      d1=atoi(c1);
      d2[0]=atoi(c2);
      d2[1]=atoi(c3);
      d2[2]=atoi(c4);
      d2[3]=atoi(c5);
      if(d1>0) {
	for(i=0;i<4;i++) {
	  if(d2[i]>0) {
	    tmpce.a1=d1;
	    tmpce.a2=d2[i];
	    add_connect_entry(&pdb,&tmpce);
	  }
	  /**************
	  pdb.connect_entry[connect_count].a1=d1;
	  pdb.connect_entry[connect_count].a2=d2[i++];
	  connect_count++;
	  if(connect_count>=connect_max) {
	    co=pdb.connect_entry;
	    pdb.connect_entry=Ccalloc(connect_max+5000,
				     sizeof(struct STRUCT_FILE_CONNECT_ENTRY));
	    memcpy(pdb.connect_entry,co,
		   connect_max*sizeof(struct STRUCT_FILE_CONNECT_ENTRY));
	    connect_max+=5000;
	    Cfree(co);
	  }
	  **************/
	}
      }
    } else if(!strcmp(record,"CRYST1")) {
      pdb_cryst_flag=1;
      for(i=0;i<9;i++)
	num[i]=line[7+i];
      num[i]='\0';
      pdb_cryst.a=atof(num);
      for(i=0;i<9;i++)
	num[i]=line[16+i];
      num[i]='\0';
      pdb_cryst.b=atof(num);
      for(i=0;i<9;i++)
	num[i]=line[25+i];
      num[i]='\0';
      pdb_cryst.c=atof(num);

      for(i=0;i<7;i++)
	num[i]=line[34+i];
      num[i]='\0';
      pdb_cryst.alpha=atof(num);
      for(i=0;i<7;i++)
	num[i]=line[41+i];
      num[i]='\0';
      pdb_cryst.beta=atof(num);
      for(i=0;i<7;i++)
	num[i]=line[48+i];
      num[i]='\0';
      pdb_cryst.gamma=atof(num);

      for(i=0;i<11;i++)
	num[i]=line[54+i];
      num[i]='\0';
      strcpy(pdb_cryst.spcgrp,num);

      for(i=0;i<4;i++)
	num[i]=line[67+i];
      num[i]='\0';
      pdb_cryst.z=atoi(num);
    } else if(!strcmp(record,"ENDMDL")) {
    } else if(clStrcmp(record,"HELIX") ||
	      clStrcmp(record,"SHEET")) {
      line_to_secs_entry(&tmpse,line);
      add_secs_entry(&pdb,&tmpse);
    }
  }

  debmsg("structRead: file parsing done");

  //pdb.atom_count=atom_count;
  //pdb.connect_count=connect_count;

  structFileEntry2DB(&pdb,&node->structNode);
  //node->structNode.conn_flag=1;

  if(pdb_cryst_flag) {
    node->structNode.xtal=Cmalloc(sizeof(struct XTAL));
    node->structNode.xtal->a=pdb_cryst.a;
    node->structNode.xtal->b=pdb_cryst.b;
    node->structNode.xtal->c=pdb_cryst.c;
    node->structNode.xtal->alpha=pdb_cryst.alpha;
    node->structNode.xtal->beta=pdb_cryst.beta;
    node->structNode.xtal->gamma=pdb_cryst.gamma;
    strcpy(node->structNode.xtal->space_group_name,pdb_cryst.spcgrp);
    dbmCalcXtal(node->structNode.xtal);
  } else {
    node->structNode.xtal=NULL;
  }

  
  Cfree(pdb.atom_entry);
  Cfree(pdb.connect_entry);
  
  return 0;
}

/*
  a problem is clearly the altLoc entry,
  at ATOM/HETATM field 17, which gives
  the possibility to give to alternate
  conformations of the same atom. It is
  unclear how this should be handled, right
  now it is prepended to the residue name
*/
  

int pdbLine2AtomEntry(char *line,struct STRUCT_FILE_ATOM_ENTRY *ae)
{
  int i,j;
  char field[10];
  char altLoc;
  
  memset(field,'\0',10);
  for(i=0;i<5;i++)
    field[i]=line[i+6];
  field[5]='\0';
  ae->anum=atoi(field);

  for(j=0,i=0;i<4;i++) {
    if(isalnum(line[i+12]) || line[i+12]=='*'  || line[i+12]=='\'')
      if(line[i+12]=='*')
	ae->aname[j++]='\'';
      else
	ae->aname[j++]=line[i+12];
  }
  ae->aname[j]='\0';
  
  for(j=0,i=0;i<2;i++) {
    if(isalpha(line[i+12]))
      ae->element[j++]=line[i+12];
  }
  ae->element[j]='\0';

  /*
  fprintf(stderr,"ATOM %d: aname: %s  ele: %s\n",ae->anum,ae->aname,ae->element);
  */

  altLoc=line[16];
  /*
  fprintf(stderr,"ATOM %d: aname: %s altLoc: %c\n",ae->anum,ae->aname,altLoc);
  */
  if(!isprint(altLoc) || isspace(altLoc)) {
    for(i=0,j=0;i<3;i++)
      if(isprint(line[i+17]) && !isspace(line[i+17]))
	 ae->rname[j++]=line[i+17];
    ae->rname[j]='\0';
  } else {
    ae->rname[0]=altLoc;
    for(i=0,j=1;i<3;i++)
      if(isprint(line[i+17]) && !isspace(line[i+17]))
	 ae->rname[j++]=line[i+17];
    ae->rname[j+1]='\0';
  }
  ae->cname[0]=line[21];

  if(!isprint(ae->cname[0]) || isspace(ae->cname[0])) {
    ae->cnum=-1;
    ae->cname[0]='0';
    ae->cname[1]='\0';
  } else {
    ae->cnum=0;
    ae->cname[1]='\0';
  }

  memset(field,'\0',10);
  for(i=0;i<4;i++)
    field[i]=line[i+22];
  field[4]='\0';
  ae->rnum=atoi(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+30];
  field[8]='\0';
  ae->x=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+38];
  field[8]='\0';
  ae->y=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+46];
  field[8]='\0';
  ae->z=atof(field);

  memset(field,'\0',10);
  for(i=0;i<6;i++)
    field[i]=line[i+54];
  field[6]='\0';
  ae->c2=atof(field);

  memset(field,'\0',10);
  for(i=0;i<6;i++)
    field[i]=line[i+60];
  field[6]='\0';
  ae->c1=atof(field);

  ae->c3=0.0;

  return 0;
}

int pdbLine2ConnectEntry(char *line,struct STRUCT_FILE_CONNECT_ENTRY *ce)
{
  return 0;
}


/******************************************

             XPLOR-PDB Section

*******************************************/

int xplorPDBRead(FILE *f,dbmNode *node)
{
  int i;
  char record[7];
  char line[256];
  struct STRUCT_FILE pdb;
  int model_count;
  int atom_count;
  int atom_max;
  struct STRUCT_FILE_ATOM_ENTRY *ao;
  
  model_count=-1;
  atom_count=0;
  atom_max=5000;
  pdb.atom_entry=Ccalloc(atom_max,sizeof(struct STRUCT_FILE_ATOM_ENTRY));
  
  while(!feof(f)) {
    memset(line,0,sizeof(line));
    fgets(line,sizeof(line),f);
    for(i=0;i<6;i++) {
      if(isspace(line[i]))
	record[i]='\0';
      else
	record[i]=line[i];
    }
    record[6]='\0';
    if(!strcmp(record,"MODEL")) {
      if(model_count==-1)
	model_count=0;
      model_count++;
    } else if(!strcmp(record,"ATOM") ||
	      !strcmp(record,"HETATM")) {
      pdb.atom_entry[atom_count].mnum=model_count;
      strcpy(pdb.atom_entry[atom_count].mname,"");
      xplorPDBLine2AtomEntry(line,&pdb.atom_entry[atom_count]);
      atom_count++;
      if(atom_count>=atom_max) {
	ao=pdb.atom_entry;
	pdb.atom_entry=Ccalloc(atom_max+5000,
			      sizeof(struct STRUCT_FILE_ATOM_ENTRY));
	memcpy(pdb.atom_entry,ao,
	       atom_max*sizeof(struct STRUCT_FILE_ATOM_ENTRY));
	atom_max+=5000;
	Cfree(ao);
      }
    } else if(!strcmp(record,"ENDMDL")) {
    }
  }
  pdb.atom_count=atom_count;
  pdb.connect_count=0;
  pdb.secs_count=0;

  structFileEntry2DB(&pdb,&node->structNode);

  //node->structNode.conn_flag=1;
  node->structNode.xtal=NULL;
  
  Cfree(pdb.atom_entry);
  
  return 0;
}


int xplorPDBLine2AtomEntry(char *line,struct STRUCT_FILE_ATOM_ENTRY *ae)
{
  int i,j;
  char field[10];
  
  memset(field,'\0',10);
  for(i=0;i<5;i++)
    field[i]=line[i+6];
  field[5]='\0';
  ae->anum=atoi(field);

  for(j=0,i=0;i<4;i++) {
    if(isalnum(line[i+12]) || line[i+12]=='*' || line[i+12]=='\'')
      if(line[i+12]=='*')
	ae->aname[j++]='\'';
      else
	ae->aname[j++]=line[i+12];
  }
  ae->aname[j]='\0';

  for(j=0,i=0;i<2;i++) {
    if(isalpha(line[i+12]))
      ae->element[j++]=line[i+12];
  }
  ae->element[j]='\0';


  /****
  for(j=0,i=0;i<4;i++) {
    if(isalnum(line[i+12]))
      ae->aname[j++]=line[i+12];
  }
  ae->aname[j]='\0';

  for(j=0,i=0;i<2;i++) {
      ae->element[j++]=line[i+13];
  }
  ae->element[2]='\0';
  ********/
  
  for(i=0;i<3;i++)
    ae->rname[i]=line[i+17];
  ae->rname[3]='\0';

  j=0;
  for(i=0;i<4;i++)
    if(isalnum(line[i+71]) || ispunct(line[i+71]))
      ae->cname[j++]=line[i+71];

  ae->cname[j]='\0';

  memset(field,'\0',10);
  for(i=0;i<5;i++)
    field[i]=line[i+22];
  field[i]='\0';
  ae->rnum=atoi(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+30];
  field[8]='\0';
  ae->x=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+38];
  field[8]='\0';
  ae->y=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+46];
  field[8]='\0';
  ae->z=atof(field);

  memset(field,'\0',10);
  for(i=0;i<6;i++)
    field[i]=line[i+54];
  field[6]='\0';
  ae->c2=atof(field);

  memset(field,'\0',10);
  for(i=0;i<6;i++)
    field[i]=line[i+60];
  field[6]='\0';
  ae->c1=atof(field);

  ae->c3=0.0;

  return 0;
}


/******************************************

          CHARMM cardfile Section

*******************************************/

int charmmRead(FILE *f,dbmNode *node)
{
  int num,ac;
  char line[256];
  char message[256];
  struct STRUCT_FILE charmm;

  while(!feof(f)) {
    fgets(line,sizeof(line),f);
    if(line[0]!='*')
      break;
  }
  if(feof(f)) {
    comMessage("empty file\n");
    return -1;
  }
  sscanf(line,"%5d",&num);
  charmm.atom_count=num;
  sprintf(message,"charmmRead: allocating memory for %d atoms",num);
  debmsg(message);
  charmm.atom_entry=Ccalloc(num+1,sizeof(struct STRUCT_FILE_ATOM_ENTRY));

  node->structNode.model_flag=0;
  node->structNode.chain_flag=0;
  node->structNode.residue_flag=1;
  
  ac=0;
  debmsg("charmmRead: parsing file");
  while(!feof(f)) {
    fgets(line,sizeof(line),f);
    if(strlen(line)>50)
      charmmLine2AtomEntry(line,&charmm.atom_entry[ac++]);
  }


  charmm.connect_count=0;
  charmm.secs_count=0;

  debmsg("charmmRead: converting to dataset");
  structFileEntry2DB(&charmm,&node->structNode);
  //node->structNode.conn_flag=1;

  Cfree(charmm.atom_entry);

  return 0;
}

int charmmLine2AtomEntry(char *line,struct STRUCT_FILE_ATOM_ENTRY *ae)
{
  int i,j;
  char field[10];

  memset(field,'\0',10);
  for(i=0;i<5;i++)
    field[i]=line[i+0];
  field[5]='\0';
  ae->anum=atoi(field);

  for(j=0,i=0;i<4;i++) {
    if(isalnum(line[i+16]))
      ae->aname[j++]=line[i+16];
  }
  ae->aname[j]='\0';

  for(j=0,i=0;i<1;i++) {
      ae->element[j++]=line[i+16];
  }
  ae->element[1]='\0';

  
  for(i=0;i<3;i++)
    ae->rname[i]=line[i+11];
  ae->rname[3]='\0';

  ae->cname[0]=line[51];
  if(!isprint(ae->cname[0]) || isspace(ae->cname[0])) {
    ae->cnum=-1;
    ae->cname[0]='0';
    ae->cname[1]='\0';
  } else {
    ae->cnum=0;
    ae->cname[1]='\0';
  }

  memset(field,'\0',10);
  for(i=0;i<4;i++)
    field[i]=line[i+55];
  field[4]='\0';
  ae->rnum=atoi(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+21];
  field[8]='\0';
  ae->x=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+30];
  field[8]='\0';
  ae->y=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+40];
  field[8]='\0';
  ae->z=atof(field);

  ae->c1=0.0;

  memset(field,'\0',10);
  for(i=0;i<6;i++)
    field[i]=line[i+60];
  field[6]='\0';
  ae->c2=atof(field);


  ae->c3=0.0;

  return 0;
}

/*****************************************
   read charmm trajectory file
   
******************************************/

/* 
   icntrl[1]: number of coordinate sets in file
   icntrl[2]: number of previous dynamic steps
   icntrl[3]: frequency for saving coordinates
   icntlr[4]: number of steps for creation run
*/

int charmmTrjRead(FILE *f, dbmStructNode *node, int sf2)
{
  int i,j;
  char message[256];
  struct CHARMM_TRJ_HEADER header;
  char dummy[4];
  float *temp;
  int pointer;
  int sf;

  sprintf(message,"loading CHARMM trajectory ...\n");
  comMessage(message);

  fread(dummy,sizeof(dummy),1,f);
  fread(header.hdrr,sizeof(char),4,f);
  fread(header.icntrl,sizeof(int),20,f);

  if(header.icntrl[1]<0 || header.icntrl[1]>1e6) {
    // nonsense atom count, try swapping
    swap_4bs((unsigned char *)header.icntrl,20);
    if(header.icntrl[1]<0 || header.icntrl[1]>1e6) {
      comMessage("error: nonsense atom count in header\n");
      return -1;
    } else {
      comMessage(" (byte-swapping) ");
      sf=1;
    }
  } else {
    sf=0;
  }

  fread(dummy,sizeof(dummy),1,f);
  fread(&header.ntitle,sizeof(int),1,f);
  if(sf) swap_4b((unsigned char *)&header.ntitle);
  fread(dummy,sizeof(dummy),1,f);
  fread(header.title,sizeof(char),header.ntitle,f);
  header.title[header.ntitle]='\0';
  fread(dummy,sizeof(dummy),1,f);
  fread(&header.t_atom_count,sizeof(int),1,f);
  if(sf) swap_4b((unsigned char *)&header.t_atom_count);
  fread(dummy,sizeof(dummy),1,f);
  header.num=header.icntrl[0];
  header.istep=header.icntrl[1];
  header.freq=header.icntrl[2];
  header.nstep=header.icntrl[3];
  header.f_atom_count=header.icntrl[8];
  header.atom_count=header.t_atom_count-header.f_atom_count;

  /*
  fprintf(stderr,"Header:\n%c%c%c%c\n",
	  header.hdrr[0],header.hdrr[1],header.hdrr[2],header.hdrr[3]);

  fprintf(stderr,"ntitle: %d\n%s\n",header.ntitle,header.title);

  fprintf(stderr,"natrec: %d nfreat: %d\n",header.atom_count, header.f_atom_count);
  */

  sprintf(message," %d trajectories with %d atoms (%d fixed) each",
	  header.num, header.atom_count, header.f_atom_count);

  comMessage(message);

  if(node->atom_count != header.t_atom_count) {
    sprintf(message,"atom count missmatch: %d in dataset, %d in trajectory\n",
	    node->atom_count,header.t_atom_count);
    comMessage(message);
    return -1;
  }

  /*
    after the trajectory file has been succesfully opened
    alocate memory for the variable atoms and fill the
    trj structure
   */    

  if(node->trj_flag) {
    /* there is already a trajectory associated */
    Cfree(node->trj.pos);
  }

  node->trj_flag=1;
  node->trj.type=STRUCT_TRJ_CHARMM;
  node->trj.atom_count=header.atom_count;
  node->trj.size=node->trj.atom_count*sizeof(struct STRUCT_TRJ_POSITION);
  node->trj.frame_count=header.num;

  node->trj.pos=Ccalloc(node->trj.frame_count,node->trj.size);
  if(node->trj.pos==NULL) {
    sprintf(message,"memory allocation error in trjRead for %dkb\n",
	    node->trj.frame_count*node->trj.size/1024);
    comMessage(message);
    return -1;
  }

  temp=Ccalloc(node->trj.atom_count,sizeof(float));
  if(temp==NULL) {
    sprintf(message,"memory allocation error for 'temp' in trjRead, %dkb\n",
	    node->trj.atom_count*sizeof(float)/1024);
    comMessage(message);
    return -1;
  }

  for(i=0;i<node->trj.frame_count;i++) {
    /* 
       read each frame
    */
    if(feof(f)) {
      /* premature EOF */
      sprintf(message,"premature EOF, %d frames read\n",i);
      comMessage(message);
      node->trj.frame_count=i;
      break;
    }
    pointer=i*node->trj.atom_count;
    fread(dummy,sizeof(dummy),1,f);
    fread(temp,sizeof(float),node->trj.atom_count,f);
    if(sf) swap_4bs((unsigned char *)temp,node->trj.atom_count);
    fread(dummy,sizeof(dummy),1,f);
    for(j=0;j<node->trj.atom_count;j++) {
      node->trj.pos[pointer+j].x=temp[j];
    }
    fread(dummy,sizeof(dummy),1,f);
    fread(temp,sizeof(float),node->trj.atom_count,f);
    if(sf) swap_4bs((unsigned char *)temp,node->trj.atom_count);
    fread(dummy,sizeof(dummy),1,f);
    for(j=0;j<node->trj.atom_count;j++) {
      node->trj.pos[pointer+j].y=temp[j];
    }
    fread(dummy,sizeof(dummy),1,f);
    fread(temp,sizeof(float),node->trj.atom_count,f);
    if(sf) swap_4bs((unsigned char *)temp,node->trj.atom_count);
    fread(dummy,sizeof(dummy),1,f);
    for(j=0;j<node->trj.atom_count;j++) {
      node->trj.pos[pointer+j].z=temp[j];
    }
  }
  return 0;
}

enum BONES_ID {BONES_NONE,
	       BONES_ATOM_XYZ,
	       BONES_ATOM_BONE,
	       BONES_CONNECTIVITY,
	       BONES_ATOM_COLOUR,
	       BONES_RESIDUE_NAME,
	       BONES_RESIDUE_TYPE,
	       BONES_RESIDUE_POINTERS};

int bonesRead(FILE *f, dbmNode *node)
{
  char line[256];
  char d1[256],d2[256],d3[256];
  int i1,i3;
  int id,pos,count,i;
  struct STRUCT_FILE_ATOM_ENTRY *entry;
  struct STRUCT_FILE_CONNECT_ENTRY *conn;
  struct STRUCT_FILE bones;
  int entry_count,conn_count,entry_max,conn_max;
  div_t dt;

  id=BONES_NONE;
  pos=0;
  count=0;

  entry_count=0;
  conn_count=0;

  while(!feof(f)) {
    fgets(line,sizeof(line),f);
    if(strstr(line,"_ATOM_XYZ")) {
      id=BONES_ATOM_XYZ;
      sscanf(line,"%34c%d",d1,&count);
     /* allocate enough memory for 'count/3' atoms */
      entry_count=0;
      entry_max=count/3;
      entry=Ccalloc(entry_max,sizeof(struct STRUCT_FILE_ATOM_ENTRY));
    } else if(strstr(line,"_ATOM_BONE")) {
      id=BONES_ATOM_BONE;
      entry_count=0;
    } else if(strstr(line,"_CONNECTIVITY")) {
      id=BONES_CONNECTIVITY;
      sscanf(line,"%34c%d",d1,&count);
      /* allocate memory for 'count' bonds */
      conn_count=0;
      conn_max=count;
      conn=Ccalloc(conn_max,sizeof(struct STRUCT_FILE_CONNECT_ENTRY));
      i3=-1;
    } else if(strstr(line,"_ATOM_COLOUR")) {
      id=BONES_ATOM_COLOUR;
    } else if(strstr(line,"_RESIDUE_NAME")) {
      id=BONES_RESIDUE_NAME;
    } else if(strstr(line,"_RESIDUE_TYPE")) {
      id=BONES_RESIDUE_TYPE;
    } else if(strstr(line,"_RESIDUE_POINTERS")) {
      id=BONES_RESIDUE_POINTERS;
    } else {
      if(id==BONES_NONE) {
	/* ignore */
      } else if(id==BONES_ATOM_XYZ){
	sscanf(line,"%10c%10c%10c",d1,d2,d3);
	d1[10]='\0';
	d2[10]='\0';
	d3[10]='\0';
	if(entry_count<entry_max) {
	  entry[entry_count].anum=entry_count+1;
	  entry[entry_count].x=atof(d1);
	  entry[entry_count].y=atof(d2);
	  entry[entry_count].z=atof(d3);
	  entry[entry_count].c1=0.0;
	  entry[entry_count].c2=0.0;
	  entry[entry_count].c3=0.0;
	  entry[entry_count].mnum=-1;
	  entry[entry_count].cnum=-1;
	  entry[entry_count].rnum=1;
	  entry_count++;
	}
      } else if(id==BONES_ATOM_BONE){
	i=0;
	while(i<strlen(line)) {
	  d1[0]=line[i++];
	  d1[1]=line[i++];
	  d1[2]='\0';
	  i1=atoi(d1);
	  if(entry_count<entry_max) {
	    sprintf(entry[entry_count].aname,"%d",i1);
	    entry_count++;
	  }
	}
      } else if(id==BONES_CONNECTIVITY){
	i=0;
	while(i<strlen(line)) {
	  d1[0]=line[i++];
	  d1[1]=line[i++];
	  d1[2]=line[i++];
	  d1[3]=line[i++];
	  d1[4]=line[i++];
	  d1[5]=line[i++];
	  d1[6]='\0';
	  i1=atoi(d1);
	  dt=div(i1,2);
	  if(i1>0) {
	    if(dt.rem==1) { /* is odd */
	      if(conn_count<conn_max) {
		conn[conn_count].a1=i3;
		conn[conn_count].a2=(i1-1)/2;
	      }
	      i3=(i1-1)/2;
	      conn_count++;
	    } else {
	      i3=i1/2;
	    }
	  }
	}
      } else if(id==BONES_ATOM_COLOUR){
	/* ignore */
      } else if(id==BONES_RESIDUE_NAME){
	/* ignore */
      } else if(id==BONES_RESIDUE_TYPE){
	sscanf(line,"%3c",d1);
	d1[3]='\0';
	for(i=0;i<entry_count;i++)
	  strcpy(entry[i].rname,d1);
      } else if(id==BONES_RESIDUE_POINTERS){
	/* ignore */
      } 
    }
  }

  /*
  for(i=0;i<entry_count;i++)
    fprintf(stderr,"%3d %3d %3d %8s %3d %4s %10.3f %10.3f %10.3f\n",
	    entry[i].mnum,entry[i].cnum,
	    entry[i].anum,entry[i].aname,entry[i].rnum,entry[i].rname,
	    entry[i].x,entry[i].y,entry[i].z);

  for(i=0;i<conn_count;i++)
    fprintf(stderr,"%3d %3d\n",conn[i].a1,conn[i].a2);
  */

  bones.atom_entry=entry;
  bones.atom_count=entry_count;
  bones.connect_entry=conn;
  bones.connect_count=conn_count;
  bones.secs_count=0;

  //node->structNode.conn_flag=0;

  structFileEntry2DB(&bones,&node->structNode);

  return 0;
}


/******************************************

          MEAD PQR Section

*******************************************/

int pqrRead(FILE *f,dbmNode *node)
{
  int i;
  char record[7];
  char line[256];
  struct STRUCT_FILE pdb;
  int model_count;
  int atom_count;
  int atom_max;
  struct STRUCT_FILE_ATOM_ENTRY *ao;
  
  model_count=-1;
  atom_count=0;
  atom_max=5000;
  pdb.atom_entry=Ccalloc(atom_max,sizeof(struct STRUCT_FILE_ATOM_ENTRY));
  
  while(!feof(f)) {
    memset(line,0,sizeof(line));
    fgets(line,sizeof(line),f);
    for(i=0;i<6;i++) {
      if(isspace(line[i]))
	record[i]='\0';
      else
	record[i]=line[i];
    }
    record[6]='\0';
    if(!strcmp(record,"MODEL")) {
      if(model_count==-1)
	model_count=0;
      model_count++;
    } else if(!strcmp(record,"ATOM") ||
	      !strcmp(record,"HETATM")) {
      pdb.atom_entry[atom_count].mnum=model_count;
      strcpy(pdb.atom_entry[atom_count].mname,"");
      pqrLine2AtomEntry(line,&pdb.atom_entry[atom_count]);
      atom_count++;
      if(atom_count>=atom_max) {
	ao=pdb.atom_entry;
	pdb.atom_entry=Ccalloc(atom_max+5000,
			      sizeof(struct STRUCT_FILE_ATOM_ENTRY));
	memcpy(pdb.atom_entry,ao,
	       atom_max*sizeof(struct STRUCT_FILE_ATOM_ENTRY));
	atom_max+=5000;
	Cfree(ao);
      }
    } else if(!strcmp(record,"ENDMDL")) {
    }
  }
  pdb.atom_count=atom_count;
  pdb.connect_count=0;
  pdb.secs_count=0;

  structFileEntry2DB(&pdb,&node->structNode);

  //node->structNode.conn_flag=1;
  node->structNode.xtal=NULL;
  
  Cfree(pdb.atom_entry);
  
  return 0;
}


int pqrLine2AtomEntry(char *line,struct STRUCT_FILE_ATOM_ENTRY *ae)
{
  int i,j;
  char field[10];
  char line2[256];

  
  memset(field,'\0',10);
  for(i=0;i<5;i++)
    field[i]=line[i+6];
  field[5]='\0';
  ae->anum=atoi(field);

  for(j=0,i=0;i<4;i++) {
    if(isalnum(line[i+12]))
      ae->aname[j++]=line[i+12];
  }
  ae->aname[j]='\0';

  ae->element[0]=line[12];
  ae->element[1]='\0';
  
  for(i=0;i<3;i++)
    ae->rname[i]=line[i+17];
  ae->rname[3]='\0';

  ae->cname[0]=line[21];

  if(!isprint(ae->cname[0]) || isspace(ae->cname[0])) {
    ae->cnum=-1;
    ae->cname[0]='0';
    ae->cname[1]='\0';
  } else {
    ae->cnum=0;
    ae->cname[1]='\0';
  }

  memset(field,'\0',10);
  for(i=0;i<5;i++)
    field[i]=line[i+22];
  field[i]='\0';
  ae->rnum=atoi(field);

  /*
    it seems that the following fields are read as free format,
    separated by spaces <SIGH>, starting from position 30
  */
  strcpy(line2,line+30);
  
  i=0,j=0;

  while(isspace(line2[i])) i++;
  j=0;
  while(!isspace(line2[i])) field[j++]=line2[i++];
  field[j]='\0';
  ae->x=atof(field);
  while(isspace(line2[i])) i++;
  j=0;
  while(!isspace(line2[i])) field[j++]=line2[i++];
  field[j]='\0';
  ae->y=atof(field);
  while(isspace(line2[i])) i++;
  j=0;
  while(!isspace(line2[i])) field[j++]=line2[i++];
  field[j]='\0';
  ae->z=atof(field);
  while(isspace(line2[i])) i++;
  j=0;
  while(!isspace(line2[i])) field[j++]=line2[i++];
  field[j]='\0';
  ae->c2=atof(field);
  while(isspace(line2[i])) i++;
  j=0;
  while(!isspace(line2[i])) field[j++]=line2[i++];
  field[j]='\0';
  ae->c1=atof(field);
  ae->c3=0.0;

  /************************
  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+30];
  field[8]='\0';
  ae->x=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+40];
  field[8]='\0';
  ae->y=atof(field);

  memset(field,'\0',10);
  for(i=0;i<8;i++)
    field[i]=line[i+50];
  field[8]='\0';
  ae->z=atof(field);

  memset(field,'\0',10);
  for(i=0;i<7;i++)
    field[i]=line[i+60];
  field[6]='\0';
  ae->c2=atof(field);

  memset(field,'\0',10);
  for(i=0;i<7;i++)
    field[i]=line[i+68];
  field[6]='\0';
  ae->c1=atof(field);

  ae->c3=0.0;
  *******************************/

  return 0;
}

/*
  very simple trajectory format

  header: 
  1 int: atomcount
  1 int: framecount

  atomcount*framecount*3 data
  (3 consecutive floats for x,y,z)
*/

int dinoTrjRead(FILE *f, dbmStructNode *node, int sf)
{
  int an,fn,ret;
  char message[256];

  fread(&an,sizeof(int),1,f);
  fread(&fn,sizeof(int),1,f);

  if(sf) {
    swap_4b((unsigned char *)&an);
    swap_4b((unsigned char *)&fn);
  }

  if(an<1 || an>100000) {
    comMessage("trjRead: nonsense atomcount\n");
    return -1;
  }
  if(fn<1 || fn>100000) {
    comMessage("trjRead: nonsense framecount\n");
    return -1;
  }

  if(node->trj_flag) {
    /* there is already a trajectory associated */
    Cfree(node->trj.pos);
  }

  node->trj_flag=0;
  node->trj.type=STRUCT_TRJ_DINO;
  node->trj.atom_count=an;
  node->trj.size=an*sizeof(struct STRUCT_TRJ_POSITION);
  node->trj.frame_count=fn;

  node->trj.pos=Ccalloc(fn*an,sizeof(struct STRUCT_TRJ_POSITION));
  if(node->trj.pos==NULL) {
    sprintf(message,"memory allocation error in trjRead for %dkb\n",
	    node->trj.frame_count*node->trj.size/1024);
    comMessage(message);
    return -1;
  }

  ret=fread(node->trj.pos,sizeof(struct STRUCT_TRJ_POSITION),fn*an,f);

  if(ret!=fn*an) {
    if(feof(f)) {
      sprintf(message,"trjRead: unexpected EOF\n");
    } else {
      sprintf(message,"trjRead: unexpected Error\n");
    }
    comMessage(message);
    Cfree(node->trj.pos);
    return -1;
  }
  sprintf(message," %d trajectories with %d atoms each",
	  fn,an);

  comMessage(message);

  node->trj_flag=1;
  return 0;

}

int binposTrjRead(FILE *f, dbmStructNode *node, int sf) 
{
  char hdr[5];
  int ac,acount,fcount,fmax,tpos;
  float *fptr;
  struct STRUCT_TRJ_POSITION *tptr;
  char message[256];

  fread(hdr,sizeof(char),4,f);
  hdr[4]='\0';

  if(!clStrcmp(hdr,"fxyz")) {
    comMessage("error: invalid binpos header\n");
    return -1;
  }

  if(node->trj_flag) {
    /* there is already a trajectory associated */
    Cfree(node->trj.pos);
  }

  fcount=0;
  fmax=10;
  acount=-1;
  while(!feof(f)) {
    if(fread(&ac,sizeof(int),1,f)<=0)
	break;

    if(sf) {
      swap_4b((unsigned char *)&ac);
    }

    if(acount==-1) {
      acount=ac;
      if(acount <1 || acount>1e6) {
	comMessage("error: binposTrjRead: nonsense atom count\n");
	return -1;
      }
      fptr=Ccalloc(sizeof(float),acount*3);
      tptr=Crecalloc(NULL,sizeof(struct STRUCT_TRJ_POSITION),acount*fmax);
    } else {
      if(ac!=acount) {
	comMessage("error: binposTrjRead: atom count missmatch error\n");
	Cfree(fptr);
	Cfree(tptr);
	return -1;
      }
    }

    fread(fptr,sizeof(float),acount*3,f);
    if(sf)
      swap_4bs((unsigned char *)fptr,acount*3);
    for(ac=0;ac<acount;ac++) {
      tpos=(fcount*acount+ac);
      (tptr+tpos)->x=fptr[ac*3+0];
      (tptr+tpos)->y=fptr[ac*3+1];
      (tptr+tpos)->z=fptr[ac*3+2];
    }
    
    fcount++;
    if(fcount>=fmax) {
      fmax+=10;
      tptr=Crecalloc(tptr,sizeof(struct STRUCT_TRJ_POSITION),acount*fmax);
    }
  }

  node->trj_flag=1;
  node->trj.type=STRUCT_TRJ_DINO;
  node->trj.atom_count=acount;
  node->trj.size=acount*sizeof(struct STRUCT_TRJ_POSITION);
  node->trj.frame_count=fcount;
  node->trj.pos=Crecalloc(tptr,sizeof(struct STRUCT_TRJ_POSITION),acount*fcount);

  sprintf(message," %d trajectories with %d atoms each",
	  fcount,acount);

  comMessage(message);
  
  
  return 0;
}

static void prep_struct_file(struct STRUCT_FILE *sf, int am, int cm, int sm)
{
  sf->atom_count=0;
  if(am>0) {
    sf->atom_max=am;
    sf->atom_entry=Crecalloc(NULL,am,
			     sizeof(struct STRUCT_FILE_ATOM_ENTRY));
  } else {
    sf->atom_max=0;
    sf->atom_entry=NULL;
  }
  sf->connect_count=0;
  if(cm>0){
    sf->connect_max=cm;
    sf->connect_entry=Crecalloc(NULL,cm,
				sizeof(struct STRUCT_FILE_CONNECT_ENTRY));
  } else {
    sf->connect_max=0;
    sf->connect_entry=NULL;
  }

  sf->secs_count=0;
  if(sm>0) {
    sf->secs_max=sm;
    sf->secs_entry=Crecalloc(NULL,sm,sizeof(struct STRUCT_FILE_SECS_ENTRY));
  } else {
    sf->secs_max=0;
    sf->secs_entry=NULL;
  }
}

static void free_struct_file(struct STRUCT_FILE *sf)
{
  if(sf->atom_max>0)
    Cfree(sf->atom_entry);
  if(sf->connect_max>0)
    Cfree(sf->connect_entry);
  if(sf->secs_max>0)
    Cfree(sf->secs_entry);
}

static void line_to_atom_entry(struct STRUCT_FILE_ATOM_ENTRY *ae, char *line, int format)
{
  switch(format) {
  case STRUCT_FILE_FORMAT_PDB:
    pdbLine2AtomEntry(line,ae);
    break;
  }

}

static void line_to_connect_entry(struct STRUCT_FILE_CONNECT_ENTRY *ce, char *line)
{
  // NOT USED
}


static void line_to_secs_entry(struct STRUCT_FILE_SECS_ENTRY *se, char *line)
{
  char tmp[8];
  if(line[0]=='H') {
    se->type=STRUCT_RTYPE_HELIX;
    se->chain[0]=line[19];
    se->chain[1]='\0';
    tmp[0]=line[21];
    tmp[1]=line[22];
    tmp[2]=line[23];
    tmp[3]=line[24];
    tmp[4]='\0';
    se->start=atoi(tmp);
    tmp[0]=line[33];
    tmp[1]=line[34];
    tmp[2]=line[35];
    tmp[3]=line[36];
    tmp[4]='\0';
    se->end=atoi(tmp);
    //fprintf(stderr,"HELIX: %s %3d %3d\n",se->chain,se->start,se->end);
  } else {
    se->type=STRUCT_RTYPE_STRAND;
    se->chain[0]=line[21];
    se->chain[1]='\0';
    tmp[0]=line[22];
    tmp[1]=line[23];
    tmp[2]=line[24];
    tmp[3]=line[25];
    tmp[4]='\0';
    se->start=atoi(tmp);
    tmp[0]=line[33];
    tmp[1]=line[34];
    tmp[2]=line[35];
    tmp[3]=line[36];
    tmp[4]='\0';
    se->end=atoi(tmp);
    //fprintf(stderr,"STRAND: %s %3d %3d\n",se->chain,se->start,se->end);
  }
}



static void add_atom_entry(struct STRUCT_FILE *sf, struct STRUCT_FILE_ATOM_ENTRY *ae)
{
  if(sf->atom_count>=sf->atom_max) {
    sf->atom_entry=Crecalloc(sf->atom_entry,sf->atom_max+5000,
			    sizeof(struct STRUCT_FILE_ATOM_ENTRY));
    sf->atom_max+=5000;
  }
  memcpy(&sf->atom_entry[sf->atom_count++],ae,
	 sizeof(struct STRUCT_FILE_ATOM_ENTRY));
}

static void add_connect_entry(struct STRUCT_FILE *sf, struct STRUCT_FILE_CONNECT_ENTRY *ce)
{
  if(sf->connect_count>=sf->connect_max) {
    sf->connect_entry=Crecalloc(sf->connect_entry,sf->connect_max+5000,
			    sizeof(struct STRUCT_FILE_CONNECT_ENTRY));
    sf->connect_max+=1000;
  }
  memcpy(&sf->connect_entry[sf->connect_count++],ce,
	 sizeof(struct STRUCT_FILE_CONNECT_ENTRY));

}

static void add_secs_entry(struct STRUCT_FILE *sf, struct STRUCT_FILE_SECS_ENTRY *se)
{
  if(sf->secs_count>=sf->secs_max) {
    sf->secs_entry=Crecalloc(sf->secs_entry,sf->secs_max+5000,
			    sizeof(struct STRUCT_FILE_SECS_ENTRY));
    sf->secs_max+=100;
  }
  memcpy(&sf->secs_entry[sf->secs_count++],se,
	 sizeof(struct STRUCT_FILE_SECS_ENTRY));

}


/*
  Convert STRUCT_FILE into struct dataset
*/

int structFileEntry2DB(struct STRUCT_FILE *sf,dbmStructNode *node)
{
  int i,j,ac,rc,*oi,cc,pass;
  struct STRUCT_FILE_ATOM_ENTRY *ae,re;
  struct STRUCT_FILE_CONNECT_ENTRY *ce;

  struct STRUCT_ATOM *atom,*oatom;
  int atom_count,atom_max;
  struct STRUCT_RESIDUE *residue,*oresidue;
  int residue_count,residue_max,residue_flag;
  struct STRUCT_CHAIN *chain,*ochain;
  int chain_count,chain_max,chain_flag;
  struct STRUCT_MODEL *model,*omodel;
  int model_count,model_max,model_flag;

  struct STRUCT_ATOM *cap;
  int cai;
  struct STRUCT_RESIDUE *crp;
  int cri;
  struct STRUCT_CHAIN *ccp;
  int cci;
  struct STRUCT_MODEL *cmp;
  int cmi;

  int secsr1,secsr2;
  char *ccname;

  /*
    allocate memory for the working
    arrays 
  */

  comMessage("a");

  debmsg("structRead: allocating working memory: atom");
  atom_max=50000;
  atom_count=0;
  atom=Ccalloc(atom_max,sizeof(struct STRUCT_ATOM));

  debmsg("structRead: allocating working memory: residue");
  residue_max=5000;
  residue_count=0;
  residue=Ccalloc(residue_max,sizeof(struct STRUCT_RESIDUE));

  debmsg("structRead: allocating working memory: chain");
  chain_max=100;
  chain_count=0;
  chain=Ccalloc(chain_max,sizeof(struct STRUCT_CHAIN));

  debmsg("structRead: allocating working memory: model");
  model_max=100;
  model_count=0;
  model=Ccalloc(model_max,sizeof(struct STRUCT_MODEL));


  debmsg("structRead: initializing atom_table");
  /* atom table */
  node->atom_table=Ccalloc(struct_atom_table_len,sizeof(struct STRUCT_ATOM_TABLE));
  node->atom_table_len=struct_atom_table_len-1;
  memcpy(node->atom_table,struct_atom_table,
	 struct_atom_table_len*sizeof(struct STRUCT_ATOM_TABLE));


  if(sf->atom_entry[0].mnum==-1) {
    model_flag=0;
  } else {
    model_flag=1;
  }

  chain_flag=0;
  for(i=0;i<sf->atom_count;i++)
    if(sf->atom_entry[i].cnum>=0) {
      chain_flag=1;
      break;
    }

  if(chain_flag==1) {
    for(i=0;i<sf->atom_count;i++)
      if(sf->atom_entry[i].cname[0]=='\0' || isspace(sf->atom_entry[i].cname[0])) {
	sf->atom_entry[i].cname[0]='~';
	sf->atom_entry[i].cname[1]='\0';
      }
  } 
  residue_flag=1;

  /* 
     fill the reference entry with
     initial values
  */

  re.anum=-1e6;
  strcpy(re.aname,"_dummy");
  re.rnum=-1e6;
  strcpy(re.rname,"_dummy");
  re.cnum=-1e6;
  strcpy(re.cname,"_dummy");
  re.mnum=-1e6;
  strcpy(re.mname,"_dummy");

  debmsg("structRead: central loop");

  comMessage("l");


  /*
    central loop
    ------------

    loop over all atom entries, generating
    new model, chain, residue and atom entries
    as necessary.

    Instead of two passes, the cross-references
    are kept as integers, and are converted to
    pointers afterwards

  */
  for(ac=0;ac<sf->atom_count;ac++) {
    ae=&sf->atom_entry[ac];

    /* make sure this is not a dummy atom */
    if(ae->x>=9999.0) {
//      fprintf(stderr,"ignored dummy atom %d\n",ac);
      continue;
    }

    /*
      check if new model applies 
    */
    if(re.mnum!=ae->mnum) {
      re.mnum=ae->mnum;

      debmsg("structRead: new model");

      /*
	at this point one should check wether
	the model has already been defined,
	and skip the generation of a new one
	if that is the case
      */ 

      cmi=model_count;
      model_count++;
      if(model_count>=model_max) {
	omodel=model;
	model=Ccalloc(model_max+10,sizeof(struct STRUCT_MODEL));
	memcpy(model,omodel,model_max*sizeof(struct STRUCT_MODEL));
	model_max+=10;
	Cfree(omodel);
      }

      cmp=&model[model_count-1];

      /* fill the model structure */
      if(model_flag)
	cmp->num=model_count;
      else
	cmp->num=-1;

      cmp->chain=NULL;
      cmp->chain_count=0;
      cmp->chain_max=10;
      cmp->chain_add=10;
      cmp->chain_index=Ccalloc(cmp->chain_max,sizeof(int));

      cmp->atom=NULL;
      cmp->atom_count=0;
      cmp->atom_max=10000;
      cmp->atom_add=10000;
      cmp->atom_index=Ccalloc(cmp->atom_max,sizeof(int));
      
      /* reset reference */
      re.anum=-1e6;
      strcpy(re.aname,"_dummy");
      re.rnum=-1e6;
      strcpy(re.rname,"_dummy");
      re.cnum=-1e6;
      strcpy(re.cname,"_dummy");

    }

    /*
      check if new chain applies 
    */
    if(strcmp(re.cname,ae->cname)) {
      strcpy(re.cname,ae->cname);

      debmsg("structRead: new chain");

      /* 
	 at this point it should be checked
	 wether this chain was already defined
	 in the structure file
      */
      for(cci=0;cci<chain_count;cci++) {
	if(!strcmp(chain[cci].name,re.cname))
	  break;
      }


      cci=chain_count;
      chain_count++;
      if(chain_count>=chain_max) {
	ochain=chain;
	chain=Ccalloc(chain_max+10,sizeof(struct STRUCT_CHAIN));
	memcpy(chain,ochain,chain_max*sizeof(struct STRUCT_CHAIN));
	chain_max+=10;
	Cfree(ochain);
      }
      ccp=&chain[chain_count-1];

      /* fill the chain structure */
      if(chain_flag)
	strcpy(ccp->name,ae->cname);
      else
	strcpy(ccp->name,"");
      ccp->num=cci;
      ccp->residue=NULL;
      ccp->residue_count=0;
      ccp->residue_count=0;
      ccp->residue_max=1000;
      ccp->residue_add=1000;
      ccp->residue_index=Ccalloc(ccp->residue_max,sizeof(int));

      ccp->atom=NULL;
      ccp->atom_count=0;
      ccp->atom_count=0;
      ccp->atom_max=1000;
      ccp->atom_add=1000;
      ccp->atom_index=Ccalloc(ccp->atom_max,sizeof(int));

      ccp->model=NULL;
      ccp->model_num=cmi;

      /* fill the current model */
      cmp->chain_index[cmp->chain_count++]=cci;
      if(cmp->chain_count>=cmp->chain_max) {
	oi=cmp->chain_index;
	cmp->chain_index=Ccalloc(cmp->chain_max+cmp->chain_add,sizeof(int));
	memcpy(cmp->chain_index,oi,cmp->chain_max*sizeof(int));
	cmp->chain_max+=cmp->chain_add;
	free(oi);
      }

      /* reset reference */
      re.anum=-1e6;
      strcpy(re.aname,"_dummy");
      re.rnum=-1e6;
      strcpy(re.rname,"_dummy");
    }


    /* 
       check if new residue applies
    */
    if(ae->rnum!=re.rnum) {
      re.rnum=ae->rnum;
      /* 
	 it is probably save to assume that
	 the residue has not been used before,
	 therefore a new one will be created
      */
      cri=residue_count;
      residue_count++;
      if(residue_count>=residue_max) {
	oresidue=residue;
	residue=Ccalloc(residue_max+1000,sizeof(struct STRUCT_RESIDUE));
	memcpy(residue,oresidue,residue_max*sizeof(struct STRUCT_RESIDUE));
	residue_max+=1000;
	Cfree(oresidue);
      }
      /* now fill the residue structure */
      crp=&residue[residue_count-1];
      strcpy(crp->name,ae->rname);
      crp->num=ae->rnum;
      crp->type=STRUCT_RTYPE_COIL;

      crp->atom=NULL;       /* to be filled later */
      crp->atom_count=0;
      crp->atom_max=50;
      crp->atom_add=50;
      crp->atom_index=Ccalloc(crp->atom_max,sizeof(int));
      crp->chain=NULL;      /* to be filled later */
      crp->chain_num=cci;

      /* update the current chain */
      ccp->residue_index[ccp->residue_count++]=cri;
      if(ccp->residue_count>=ccp->residue_max) {
	oi=ccp->residue_index;
	ccp->residue_index=Ccalloc(ccp->residue_max+ccp->residue_add,sizeof(int));
	memcpy(ccp->residue_index,oi,ccp->residue_max*sizeof(int));
	ccp->residue_max+=ccp->residue_add;
	Cfree(oi);
      }

      /* reset reference */
      re.anum=-1e6;
      strcpy(re.aname,"_dummy");

    }

    /*
      new atom (always applies) 
    */
    cai=atom_count;
    atom_count++;
    if(atom_count>=atom_max) {
      oatom=atom;
      atom=Ccalloc(atom_max+10000,sizeof(struct STRUCT_ATOM));
      memcpy(atom,oatom,atom_max*sizeof(struct STRUCT_ATOM));
      atom_max+=10000;
      Cfree(oatom);
    }
    cap=&atom[atom_count-1];

    /* fill atom structure */
    cap->n=atom_count-1;
    cap->anum=ae->anum;
    strcpy(cap->name,ae->aname);
    cap->xx=ae->x;
    cap->yy=ae->y;
    cap->zz=ae->z;
    cap->bfac=ae->c1;
    cap->weight=ae->c2;
    cap->restrict=0;
    cap->flag=0;

    strcpy(cap->chem.element,ae->element);
    for(i=0;i<node->atom_table_len;i++)
      if(!strcmp(node->atom_table[i].e,ae->element))
	break;
    cap->chem.charge=0.0;
    cap->chem.vdwr=node->atom_table[i].vdwr;
    cap->chem.an=node->atom_table[i].z;
    
    cap->def_prop.r=node->atom_table[i].r;
    cap->def_prop.g=node->atom_table[i].g;
    cap->def_prop.b=node->atom_table[i].b;
    cap->def_prop.radius=node->atom_table[i].vdwr;
    cap->def_prop.c[0][0]=cap->def_prop.r;
    cap->def_prop.c[0][1]=cap->def_prop.g;
    cap->def_prop.c[0][2]=cap->def_prop.b;
    cap->def_prop.c[1][0]=cap->def_prop.r;
    cap->def_prop.c[1][1]=cap->def_prop.g;
    cap->def_prop.c[1][2]=cap->def_prop.b;
    cap->def_prop.c[2][0]=cap->def_prop.r;
    cap->def_prop.c[2][1]=cap->def_prop.g;
    cap->def_prop.c[2][2]=cap->def_prop.b;

    /* back pointers */
    cap->residue=NULL;
    cap->residue_num=cri;
    cap->chain=NULL;
    cap->chain_num=cci;
    cap->model=NULL;
    cap->model_num=cmi;

    cap->bondc=0;
    cap->bondm=STRUCT_MAX_BOND_PER_ATOM;

    /* 
       fill in the atom info for the current
       residue, chain and model
    */
    crp->atom_index[crp->atom_count++]=cai;
    if(crp->atom_count>=crp->atom_max) {
      oi=crp->atom_index;
      crp->atom_index=Ccalloc(crp->atom_max+crp->atom_add,sizeof(int));
      memcpy(crp->atom_index,oi,crp->atom_max*sizeof(int));
      crp->atom_max+=crp->atom_add;
      Cfree(oi);
    }

    ccp->atom_index[ccp->atom_count++]=cai;
    if(ccp->atom_count>=ccp->atom_max) {
      oi=ccp->atom_index;
      ccp->atom_index=Ccalloc(ccp->atom_max+ccp->atom_add,sizeof(int));
      memcpy(ccp->atom_index,oi,ccp->atom_max*sizeof(int));
      ccp->atom_max+=ccp->atom_add;
      Cfree(oi);
    }

    cmp->atom_index[cmp->atom_count++]=cai;
    if(cmp->atom_count>=cmp->atom_max) {
      oi=cmp->atom_index;
      cmp->atom_index=Ccalloc(cmp->atom_max+cmp->atom_add,sizeof(int));
      memcpy(cmp->atom_index,oi,cmp->atom_max*sizeof(int));
      cmp->atom_max+=cmp->atom_add;
      Cfree(oi);
    }
    
  }

  comMessage("n");

  debmsg("structRead: copy to dataset");

  /* 
     allocate the memory in the node and copy
     the filled structures
  */
  node->model=Ccalloc(model_count+1,sizeof(struct STRUCT_MODEL));
  node->model_count=model_count;
  node->model_max=model_count;
  node->model_add=1;
  node->model_flag=model_flag;
  memcpy(node->model,model,model_count*sizeof(struct STRUCT_MODEL));
  Cfree(model);

  node->chain=Ccalloc(chain_count+1,sizeof(struct STRUCT_CHAIN));
  node->chain_count=chain_count;
  node->chain_max=chain_count;
  node->chain_add=1;
  node->chain_flag=chain_flag;
  memcpy(node->chain,chain,chain_count*sizeof(struct STRUCT_CHAIN));
  Cfree(chain);

  node->residue=Ccalloc(residue_count+1,sizeof(struct STRUCT_RESIDUE));
  node->residue_count=residue_count;
  node->residue_max=residue_count;
  node->residue_add=1000;
  node->residue_flag=residue_flag;
  memcpy(node->residue,residue,residue_count*sizeof(struct STRUCT_RESIDUE));
  Cfree(residue);

  node->atom=Ccalloc(atom_count+1,sizeof(struct STRUCT_ATOM));
  node->atom_count=atom_count;
  node->atom_max=atom_count;
  node->atom_add=10000;
  memcpy(node->atom,atom,atom_count*sizeof(struct STRUCT_ATOM));
  Cfree(atom);

  /*
    create the position array and fill it
  */
  node->apos=Ccalloc(node->atom_count+1,sizeof(struct STRUCT_APOS));
  node->apos_count=node->atom_count;
  node->apos_max=node->atom_count;
  node->apos_add=node->atom_add;
  for(i=0;i<node->atom_count;i++) {
    node->apos[i].x=node->atom[i].xx;
    node->apos[i].y=node->atom[i].yy;
    node->apos[i].z=node->atom[i].zz;
    node->atom[i].p=&node->apos[i];
  }

  debmsg("structRead: index to pointer");

  comMessage("i");

  /* 
     convert the integer cross references to pointers
  */
  for(i=0;i<node->model_count;i++) {
    cmp=&node->model[i];

    cmp->chain=Ccalloc(cmp->chain_count+1,sizeof(struct STRUCT_CHAIN *));
    cmp->chain_max=cmp->chain_count;
    for(j=0;j<cmp->chain_count;j++) {
      cmp->chain[j]=&node->chain[cmp->chain_index[j]];
    }
    Cfree(cmp->chain_index);

    cmp->atom=Ccalloc(cmp->atom_count+1,sizeof(struct STRUCT_ATOM *));
    cmp->atom_max=cmp->atom_count;
    for(j=0;j<cmp->atom_count;j++) {
      cmp->atom[j]=&node->atom[cmp->atom_index[j]];
    }
    Cfree(cmp->atom_index);
  }

  for(i=0;i<node->chain_count;i++) {
    ccp=&node->chain[i];

    ccp->model=&node->model[ccp->model_num];

    ccp->residue=Ccalloc(ccp->residue_count+1,sizeof(struct STRUCT_RESIDUE *));
    ccp->residue_max=ccp->residue_count;
    for(j=0;j<ccp->residue_count;j++) {
      ccp->residue[j]=&node->residue[ccp->residue_index[j]];
    }
    Cfree(ccp->residue_index);

    ccp->atom=Ccalloc(ccp->atom_count+1,sizeof(struct STRUCT_ATOM *));
    ccp->atom_max=ccp->atom_count;
    for(j=0;j<ccp->atom_count;j++) {
      ccp->atom[j]=&node->atom[ccp->atom_index[j]];
    }
    Cfree(ccp->atom_index);
  }
  
  for(i=0;i<node->residue_count;i++) {
    crp=&node->residue[i];

    crp->chain=&node->chain[crp->chain_num];

    crp->atom=Ccalloc(crp->atom_count+1,sizeof(struct STRUCT_ATOM *));
    crp->atom_max=crp->atom_count;
    for(j=0;j<crp->atom_count;j++) {
      crp->atom[j]=&node->atom[crp->atom_index[j]];
    }
    Cfree(crp->atom_index);
  }

  for(i=0;i<node->atom_count;i++) {
    cap=&node->atom[i];

    cap->residue=&node->residue[cap->residue_num];
    cap->chain=&node->chain[cap->chain_num];
    cap->model=&node->model[cap->model_num];
  }

  comMessage("s");

  /*
    secondary structure as defined in the file
  */

  if(sf->secs_count>0) {
    for(i=0;i<node->atom_count;i++) {
      cap=&node->atom[i];
      cri=cap->residue->num;
      ccname=cap->chain->name;
      for(j=0;j<sf->secs_count;j++) {
	secsr1=sf->secs_entry[j].start;
	secsr2=sf->secs_entry[j].end;
	if(clStrcmp(sf->secs_entry[j].chain,ccname)) {
	  // EXCLUSIVE setting !
	  if(cri>secsr1 && cri<secsr2) {
	    cap->residue->type=sf->secs_entry[j].type;
	  }
	}
      }
    }
  }
  debmsg("structRead: explicit connectivity");

  comMessage("x");

  /*
     now worry about the connectivity
  */
  node->conn=Ccalloc(sf->connect_count,sizeof(struct STRUCT_CONNECTIVITY));
  node->conn_count=sf->connect_count;
  node->conn_max=node->conn_count;
  node->conn_add=1000;
  cc=0;
  for(i=0;i<node->conn_count;i++) {
    ce=&sf->connect_entry[i];
    pass=0;
    for(ac=0;ac<node->atom_count;ac++) {
      if(ce->a1==node->atom[ac].anum) {
	pass++;
	node->conn[cc].ap1=&node->atom[ac];
      }
      if(ce->a2==node->atom[ac].anum) {
	pass++;
	node->conn[cc].ap2=&node->atom[ac];
      }
      if(pass==2)
	break;
    }
    if(pass==2)
      cc++;
  }
  node->conn_count=cc;

  comMessage("t");

  /* 
     determine the type of residue
  */
  for(rc=0;rc<node->residue_count;rc++) {
    if(clStrcmp(node->residue[rc].name,"ALA") ||
       clStrcmp(node->residue[rc].name,"CYS") ||
       clStrcmp(node->residue[rc].name,"ASP") ||
       clStrcmp(node->residue[rc].name,"GLU") ||
       clStrcmp(node->residue[rc].name,"PHE") ||
       clStrcmp(node->residue[rc].name,"GLY") ||
       clStrcmp(node->residue[rc].name,"HIS") ||
       clStrcmp(node->residue[rc].name,"ILE") ||
       clStrcmp(node->residue[rc].name,"LYS") ||
       clStrcmp(node->residue[rc].name,"LEU") ||
       clStrcmp(node->residue[rc].name,"MET") ||
       clStrcmp(node->residue[rc].name,"ASN") ||
       clStrcmp(node->residue[rc].name,"PRO") ||
       clStrcmp(node->residue[rc].name,"GLN") ||
       clStrcmp(node->residue[rc].name,"ARG") ||
       clStrcmp(node->residue[rc].name,"SER") ||
       clStrcmp(node->residue[rc].name,"THR") ||
       clStrcmp(node->residue[rc].name,"VAL") ||
       clStrcmp(node->residue[rc].name,"TRP") ||
       clStrcmp(node->residue[rc].name,"TYR")) {
      node->residue[rc].class=STRUCT_PROTEIN;
      /*
	fprintf(stderr,"%s %d protein\n",
	node->residue[rc].name,node->residue[rc].num);
      */
    } else if (clStrcmp(node->residue[rc].name,"A") ||
	       clStrcmp(node->residue[rc].name,"C") ||
	       clStrcmp(node->residue[rc].name,"G") ||
	       clStrcmp(node->residue[rc].name,"T") ||
	       clStrcmp(node->residue[rc].name,"U") ||
	       clStrcmp(node->residue[rc].name,"ADE") ||
	       clStrcmp(node->residue[rc].name,"CYT") ||
	       clStrcmp(node->residue[rc].name,"URI") ||
	       clStrcmp(node->residue[rc].name,"GUA") ||
	       clStrcmp(node->residue[rc].name,"THY")) {
      node->residue[rc].class=STRUCT_NA;
    } else {
      node->residue[rc].class=STRUCT_MISC;
    }
  }

  return 0;

}
