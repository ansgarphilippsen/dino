#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include "dino.h"
#include "dbm.h"
#include "com.h"
#include "struct_db.h"
#include "struct_read.h"
#include "struct_write.h"
#include "scal_db.h"
#include "scal_read.h"
#include "scal_write.h"
#include "surf_db.h"
#include "surf_read.h"
#include "surf_write.h"
#include "grid_db.h"
#include "shell.h"
#include "rex.h"
#include "mat.h"
#include "conn.h"
#include "Cmalloc.h"
#include "cubearray.h"
#include "cl.h"

//static char dbm_return[256];
struct DBM dbm;

extern int debug_mode,gfx_mode;

static struct EXT_DEF {
  char ext[16],type[16];
}ext_def[] = {
  {"pdb","pdb"},
  {"ent","pdb"},
  {"map","ccp4"},
  {"ccp4","ccp4"},
  {"uhbd","uhbd"},
  {"uhb","uhbd"},
  {"xmap","xplorb"},
  {"cmap","cnsb"},
  {"cmp","cnsb"},
  {"xmp","xplorb"},
  {"xpl","xplorc"},
  {"cnsc","cnsc"},
  {"pot","uhbd"},
  {"crd","charmm"},
  {"odb","bones"},
  {"tiff","topo"},
  {"msp","msp"},
  {"vet","msp"},
  {"fld","mead"},
  {"mead","mead"},
  {"pqr","pqr"},
  {"grd","delphi"},
  {"ins","delphi"},
  {"cpot","charmmb"},
  {"dgrd","dgrid"},
  {"grasp","grasp"},
  {"bdtrj","bdtrj"},
  {"",""}
};

int dbmInit(void)
{
  dbm.nodec_max=128;
  dbm.node=Ccalloc(dbm.nodec_max,sizeof(dbmNode));
  dbm.node_count=0;

  debmsg("dbmInit: calling conInit");
  conInit();
  return 0;
}

int dbmLoad(int wc, char **wl)
{
  char type[64];
  char file[256];
  char name[64];
  char base[256],*bp;
  char ext[256];
  char file2[256];
  char file3[256];
  char message[256];
  FILE *f,*f2;
  int n;
  dbmNode *node;
  struct stat st;
  char gunzip[256],gunzip2[256];
  int cmp,ret,swap_flag,rn_flag;
  
  if(wc<1) {
    comMessage("\nfilename missing");
    return -1;
  }

  strcpy(file,wl[0]);
  if((bp=strrchr(file,'/'))!=NULL)
    strcpy(base,bp+1);
  else
    strcpy(base,file);
  
  bp=strrchr(base,'.');
  if(bp!=NULL) {
    bp[0]='\0';
    strcpy(ext,bp+1);
    strcpy(name,base);
  } else {
    strcpy(name,base);
    strcpy(ext,"");
  }

  swap_flag=0;
  rn_flag=0;

  n=1;
  strcpy(type,"");
  while(n<wc) {
    if(!strcmp(wl[n],"-type") ||
       !strcmp(wl[n],"-t")) {
      if(n+1>=wc) {
	sprintf(message,"\nmissing parameter for %s",wl[n]);
	comMessage(message);
	return -1;
      }
      strcpy(type,wl[n+1]);
      n++;
    } else if(!strcmp(wl[n],"-name") ||
	      !strcmp(wl[n],"-n")) {
      if(n+1>=wc) {
	sprintf(message,"\nmissing parameter for %s",wl[n]);
	comMessage(message);
	return -1;
      }
      strcpy(name,wl[n+1]);
      n++;
    } else if(!strcmp(wl[n],"-swap")) {
      swap_flag=1;
    } else if(!strcmp(wl[n],"-rn")) {
      rn_flag=1;
    } else {
      sprintf(message,"\nunknown option %s",wl[n]);
      comMessage(message);
      return -1;
    }
    n++;
  }

  if(strcmp(type,"msms")) {
    if(stat(file,&st)!=0) {
      strcpy(file2,file);
      strcat(file,".gz");
      if(stat(file,&st)!=0) {
	sprintf(message,"\nerror accessing %s",file2);
	comMessage(message);
	return -1;
      } else {
	if((bp=strrchr(file,'/'))!=NULL)
	  strcpy(base,bp+1);
	else
	  strcpy(base,file);
	
	bp=strrchr(base,'.');
	if(bp!=NULL)
	  bp[0]='\0';
	strcpy(ext,bp+1);
      }
    }
    
    if(!strcmp(ext,"gz")) {
      cmp=1;
      bp=strrchr(base,'.');
      if(bp!=NULL)
	bp[0]='\0';
      strcpy(ext,bp+1);
      sprintf(gunzip,"gunzip < %s",file);
      if((f=popen(gunzip,"r"))==NULL) {
	sprintf(message,"\nError while piping %s",file);
	comMessage(message);
	return -1;
      }
    } else {
      cmp=0;
      if((f=fopen(file,"r"))==NULL) {
	sprintf(message,"\nError opening %s",file);
	comMessage(message);
	return -1;
      }
    }
  }
  if(strlen(type)==0) {
    n=0;
    while(strlen(ext_def[n].ext)>0) {
      if(!strcmp(ext_def[n].ext,ext)) {
	strcpy(type,ext_def[n].type);
	break;
      }
      n++;
    }
    if(strlen(ext_def[n].ext)==0) {
      sprintf(message,"\nunknown extension %s, please specify type",ext);
      comMessage(message);
      if(cmp) pclose(f); else fclose(f);

      return -1;
    }
    
  }

  if(!strcmp(type,"pdb")) {
    /*
      PDB Format
    */
    node=dbmNewNode(DBM_NODE_STRUCT,name);

    sprintf(message,"\nloading %s, type pdb ",name);
    comMessage(message);
    if(pdbRead(f,node)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
    comMessage(".");
    structPrep(&node->structNode);
    sprintf(message," %d atom(s),",node->structNode.atom_count);
    if(node->structNode.residue_flag)
      sprintf(message,"%s %d residue(s),",
	      message,node->structNode.residue_count);
    if(node->structNode.chain_flag)
      sprintf(message,"%s %d chain(s),",
	      message,node->structNode.chain_count);
    if(node->structNode.model_flag)
      sprintf(message,"%s %d model(s),",
	      message,node->structNode.model_count);
    sprintf(message,"%s %d bond(s)",
	    message,node->structNode.bond_count);
    comMessage(message);
  } else if(!strcmp(type,"xplorc") ||
	    !strcmp(type,"cnsc")) {
    /*
      XPLOR coordinate format
    */
    node=dbmNewNode(DBM_NODE_STRUCT,name);
    
    sprintf(message,"\nloading %s, type %s ...",name, type);
    comMessage(message);
    if(xplorPDBRead(f,node)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
    comMessage(".");
    structPrep(&node->structNode);
    sprintf(message," %d atom(s),",node->structNode.atom_count);
    if(node->structNode.residue_flag)
      sprintf(message,"%s %d residue(s),",
	      message,node->structNode.residue_count);
    if(node->structNode.chain_flag)
      sprintf(message,"%s %d chain(s),",
	      message,node->structNode.chain_count);
    if(node->structNode.model_flag)
      sprintf(message,"%s %d model(s),",
	      message,node->structNode.model_count);
    sprintf(message,"%s %d bond(s)",
	    message,node->structNode.bond_count);
    comMessage(message);
  } else if(!strcmp(type,"charmm")) {
    /*
      CHARMM coordinate format
    */
    node=dbmNewNode(DBM_NODE_STRUCT,name);
    
    sprintf(message,"\nloading %s, type charmm ...",name);
    comMessage(message);
    if(charmmRead(f,node)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
    comMessage(".");
    structPrep(&node->structNode);
    sprintf(message," %d atom(s),",node->structNode.atom_count);
    if(node->structNode.residue_flag)
      sprintf(message,"%s %d residue(s),",
	      message,node->structNode.residue_count);
    if(node->structNode.chain_flag)
      sprintf(message,"%s %d chain(s),",
	      message,node->structNode.chain_count);
    if(node->structNode.model_flag)
      sprintf(message,"%s %d model(s),",
	      message,node->structNode.model_count);
    sprintf(message,"%s %d bond(s)",
	    message,node->structNode.bond_count);
    comMessage(message);
  } else if(!strcmp(type,"bdtrj")) {
    /*
      CHARMM BDTRJ
    */
    node=dbmNewNode(DBM_NODE_STRUCT,name);
    sprintf(message,"\nloading %s, type bdtrj ...",name);
    comMessage(message);
    if(bdtrjRead(&node->structNode, f, swap_flag)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
    //structPrep(&node->structNode);
    debmsg("structPrep: build CA");       
    structBuildCA(&node->structNode);
    debmsg("structPrep: prep res");       
    structPrepRes(&node->structNode);
    structSetMinMax(&node->structNode);
    structRecenter(&node->structNode);
  } else if(!strcmp(type,"dgrid")) {
    /*
      DINO GRID
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type dgrid ...",name);
    comMessage(message);

    ret=scalRead(&node->scalNode, SCAL_READ_DINO,f,swap_flag);
    if(cmp) pclose(f); else fclose(f);
    if(ret!=0) {
      dbmDeleteNode(name);
      return -1;
    } else {
      scalSetMinMax(&node->scalNode);
    }
  } else if(!strcmp(type,"charmmb")) {
    /*
      CHARMM binary potential
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type binary charmm ...",name);
    comMessage(message);

    ret=scalRead(&node->scalNode, SCAL_READ_CHARMM_BINARY,f,swap_flag);
    if(cmp) pclose(f); else fclose(f);
    if(ret!=0) {
      dbmDeleteNode(name);
      return -1;
    } else {
      scalSetMinMax(&node->scalNode);
    }
  } else if(!strcmp(type,"pqr")) {
    /*
      PQR (MEAD) coordinate format
    */
    node=dbmNewNode(DBM_NODE_STRUCT,name);
    
    sprintf(message,"\nloading %s, type pqr ...",name);
    comMessage(message);
    if(pqrRead(f,node)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
    comMessage(".");
    structPrep(&node->structNode);
    sprintf(message," %d atom(s),",node->structNode.atom_count);
    if(node->structNode.residue_flag)
      sprintf(message,"%s %d residue(s),",
	      message,node->structNode.residue_count);
    if(node->structNode.chain_flag)
      sprintf(message,"%s %d chain(s),",
	      message,node->structNode.chain_count);
    if(node->structNode.model_flag)
      sprintf(message,"%s %d model(s),",
	      message,node->structNode.model_count);
    sprintf(message,"%s %d bond(s)",
	    message,node->structNode.bond_count);
    comMessage(message);

  } else if(!strcmp(type,"xplorb")) {
    /*
      XPLOR binary map
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type binary xplor map ...",name);
    comMessage(message);

    ret=scalRead(&node->scalNode, SCAL_READ_XPLOR_BINARY,f,swap_flag);
    if(cmp) pclose(f); else fclose(f);
    if(ret!=0) {
      dbmDeleteNode(name);
      return -1;
    } else {
      scalSetMinMax(&node->scalNode);
    }
  } else if(!strcmp(type,"cnsb")) {
    /*
      CNS binary map
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type binary cns map ...",name);
    comMessage(message);

    ret=scalRead(&node->scalNode, SCAL_READ_CNS_BINARY,f,swap_flag);
    if(cmp) pclose(f); else fclose(f);
    if(ret!=0) {
      dbmDeleteNode(name);
      return -1;
    } else {
      scalSetMinMax(&node->scalNode);
    }
  } else if(!strcmp(type,"cnsa")) {
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type ascii cns map ...",name);
    comMessage(message);

    ret=scalRead(&node->scalNode, SCAL_READ_CNS_ASCII,f,swap_flag);
    if(cmp) pclose(f); else fclose(f);
    if(ret!=0) {
      dbmDeleteNode(name);
      return -1;
    } else {
      scalSetMinMax(&node->scalNode);
    }
  } else if(!strcmp(type,"xplora")) {
    /*
      XPLOR ascii map
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type ascii xplor map ...",name);
    comMessage(message);

    ret=scalRead(&node->scalNode, SCAL_READ_XPLOR_ASCII,f,swap_flag);
    if(cmp) pclose(f); else fclose(f);
    if(ret!=0) {
      dbmDeleteNode(name);
      return -1;
    } else {
      scalSetMinMax(&node->scalNode);
    }
  } else if(!strcmp(type,"ccp4")) {
    /*
      CCP4 map format
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type binary ccp4 map ...",name);
    comMessage(message);

    if(scalRead(&node->scalNode,SCAL_READ_CCP4_BINARY,f,swap_flag)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }

    if(cmp) pclose(f); else fclose(f);
  } else if(!strcmp(type,"uhbd")) {
    /*
      UHBD potential map
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type binary uhbd map ...",name);
    comMessage(message);

    if(scalRead(&node->scalNode,SCAL_READ_UHBD_BINARY,f,swap_flag)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }

    if(cmp) pclose(f); else fclose(f);

  } else if(!strcmp(type,"mead")) {
    /*
      MEAD potential map
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type mead map ...",name);
    comMessage(message);

    if(scalRead(&node->scalNode,SCAL_READ_MEAD,f,swap_flag)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }

    if(cmp) pclose(f); else fclose(f);

  } else if(!strcmp(type,"delphig")) {
    /*
      DELPHI 2 potential map aka GRASP
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type delphi/grasp map ...",name);
    comMessage(message);

    if(scalRead(&node->scalNode,SCAL_READ_DELPHIG,f,swap_flag)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }

    if(cmp) pclose(f); else fclose(f);

  } else if(!strcmp(type,"ibios") ||
	    !strcmp(type,"delphi")) {
    /*
      DELPHI potential map
    */
    node=dbmNewNode(DBM_NODE_SCAL,name);
    sprintf(message,"\nloading %s, type delphi map ...",name);
    comMessage(message);

    if(scalRead(&node->scalNode,SCAL_READ_DELPHI,f,swap_flag)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }

    if(cmp) pclose(f); else fclose(f);

  } else if(!strcmp(type,"msms")) {
    /* MSMS surface format */
    strcpy(file2,file);
    strcat(file2,".vert");
    if((f=fopen(file2,"r"))==NULL) {
      sprintf(message,"\nError opening %s",file2);
      comMessage(message);
      return -1;
    }
    strcpy(file3,file);
    strcat(file3,".face");
    if((f2=fopen(file3,"r"))==NULL) {
      sprintf(message,"\nError opening %s",file3);
      comMessage(message);
      return -1;
    }
    node=dbmNewNode(DBM_NODE_SURF, name);
    sprintf(message,"\nloading %s, type msms ...",name);
    comMessage(message);
    if(msmsRead(f,f2,node,swap_flag)!=0) {
      fclose(f);
      fclose(f2);
      dbmDeleteNode(name);
      return -1;
    }
    fclose(f);
    fclose(f2);
    sprintf(message," %d vertices, %d faces",
	    node->surfNode.vc, node->surfNode.fc);
    comMessage(message);
    surfBuildCA(&node->surfNode);
    surfCalcMinMax(&node->surfNode);
  } else if(!strcmp(type,"msp")) {
    /* MSP surface format */
    node=dbmNewNode(DBM_NODE_SURF, name);
    if((f=fopen(file,"r"))==NULL) {
      sprintf(message,"\nError opening %s",file);
      comMessage(message);
      return -1;
    }
    sprintf(message,"\nloading %s, type msp ...",name);
    comMessage(message);
    if(mspRead(f,node)!=0) {
      fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    fclose(f);
    sprintf(message," %d vertices, %d faces",
	    node->surfNode.vc, node->surfNode.fc);
    comMessage(message);
    surfBuildCA(&node->surfNode);
    surfCalcMinMax(&node->surfNode);
  } else if(!strcmp(type,"grasp")) {
    /* GRASP surface format */
    node=dbmNewNode(DBM_NODE_SURF, name);
    if((f=fopen(file,"r"))==NULL) {
      sprintf(message,"\nError opening %s",file);
      comMessage(message);
      return -1;
    }
    sprintf(message,"\nloading %s, type grasp ...",name);
    comMessage(message);
    if(graspRead(f,node,swap_flag)!=0) {
      fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    fclose(f);
    sprintf(message," %d vertices, %d faces",
	    node->surfNode.vc, node->surfNode.fc);
    comMessage(message);
    surfBuildCA(&node->surfNode);
    surfCalcMinMax(&node->surfNode);
    if(rn_flag)
      surfRenormalize(&node->surfNode);
  } else if(!strcmp(type,"ads")) {
    /* ADS surface interface format */
    node=dbmNewNode(DBM_NODE_SURF, name);
    if((f=fopen(file,"r"))==NULL) {
      sprintf(message,"\nError opening %s",file);
      comMessage(message);
      return -1;
    }
    sprintf(message,"\nloading %s, type ads ...",name);
    comMessage(message);
    if(adsRead(f,node)!=0) {
      fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    fclose(f);
    sprintf(message," %d vertices, %d faces",
	    node->surfNode.vc, node->surfNode.fc);
    comMessage(message);
    surfBuildCA(&node->surfNode);
    //    surfCalcMinMax(&node->surfNode);
  } else if(!strcmp(type,"topo")) {
    /* topography format requested */
    if((node=dbmNewNode(DBM_NODE_GRID,name))==NULL)
      return -1;
    sprintf(message,"\nloading %s, type topo ...",name);
    comMessage(message);
    if(tiffRead(fileno(f),file,&node->gridNode)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
  } else if(!strcmp(type,"bones")) {
    /* uppsala bones format requested */
    node=dbmNewNode(DBM_NODE_STRUCT,name);
    
    sprintf(message,"\nloading %s, type bones ...",name);
    comMessage(message);
    if(bonesRead(f,node)!=0) {
      if(cmp) pclose(f); else fclose(f);
      dbmDeleteNode(name);
      return -1;
    }
    if(cmp) pclose(f); else fclose(f);
    comMessage(".");
    structPrep(&node->structNode);
    sprintf(message," %d atom(s),",node->structNode.atom_count);
    if(node->structNode.residue_flag)
      sprintf(message,"%s %d residue(s),",
	      message,node->structNode.residue_count);
    if(node->structNode.chain_flag)
      sprintf(message,"%s %d chain(s),",
	      message,node->structNode.chain_count);
    if(node->structNode.model_flag)
      sprintf(message,"%s %d model(s),",
	      message,node->structNode.model_count);
    sprintf(message,"%s %d bond(s)",
	    message,node->structNode.bond_count);
    comMessage(message);
  } else {
    if(cmp) pclose(f); else fclose(f);
    shellOut("\nUnknown type ");
    shellOut(type);
    return -1;
  }
  sprintf(message,".%s",name);
  comReturn(message);

  return 0;
}

dbmNode *dbmNewNode(int type, char *oname)
{
  int i,c;
  char name[256];
  int owflag=0;

  strcpy(name,oname);

  if(owflag) {
    dbmDeleteNode(name);
  } else {
    c=2;
    i=0;
    while(i<dbm.nodec_max) {
      if(dbm.node[i].common.type!=DBM_NODE_EMPTY)
	if(!strcmp(dbm.node[i].common.name,name)) {
	  i=-1;
	  sprintf(name,"%s%d",oname,c);
	  c++;
	}
      i++;
    }
    strcpy(oname,name);
  }
  for(i=0;i<dbm.nodec_max;i++) {
    if(dbm.node[i].common.type==DBM_NODE_EMPTY) {
      memset(&dbm.node[i],0,sizeof(dbmNode));

      dbm.node[i].common.type=type;
      strcpy(dbm.node[i].common.name,name);
      comNewDB(name);
      switch(type) {
      case DBM_NODE_STRUCT:
	if(structNewNode(&dbm.node[i].structNode)!=0)
	  return NULL;
	break;
      case DBM_NODE_SCAL:
	if(scalNewNode(&dbm.node[i].scalNode)!=0)
	  return NULL;
	break;
      case DBM_NODE_VECT:
	break;
      case DBM_NODE_SURF:
	if(surfNewNode(&dbm.node[i].surfNode)!=0)
	  return NULL;
	break;
      case DBM_NODE_GRID:
	if(gridNewNode(&dbm.node[i].gridNode)!=0)
	  return NULL;
	break;
      case DBM_NODE_GEOM:
	strcpy(dbm.node[i].geomNode.path,"");
	dbm.node[i].geomNode.obj_count=0;
	dbm.node[i].geomNode.obj_max=64;
	transReset(&dbm.node[i].geomNode.transform);
	if((dbm.node[i].geomNode.obj=Ccalloc(64,sizeof(geomObj *)))==NULL) {
	  comMessage("\nmemory allocation error in NewNode");
	  return NULL;
	}
	break;
      }
      return &dbm.node[i];
    }
  }
  comMessage("\ndbmNewNode: No more free Nodes");
  return NULL;
}

int dbmGetNodeType(char *name)
{
  int i;
  for(i=0;i<dbm.nodec_max;i++)
    if(!strcmp(dbm.node[i].common.name,name))
      return dbm.node[i].common.type;

  return DBM_NODE_EMPTY;
}

int dbmDeleteNode(char *name)
{
  // TODO later
  // all specific memory freeing calls
  // best is probably a separate function for each
  int i,j;
  for(i=0;i<dbm.nodec_max;i++) {
    if(!strcmp(dbm.node[i].common.name,name)) {
      switch(dbm.node[i].common.type) {
      case DBM_NODE_STRUCT:
	for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	  if(dbm.node[i].structNode.obj_flag[i]!=0) {
	    structDelObj(&dbm.node[i].structNode,
			 dbm.node[i].structNode.obj[i].name);
	  }
	Cfree(dbm.node[i].structNode.obj);
	break;
      case DBM_NODE_SCAL:
	for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	  if(dbm.node[i].scalNode.obj[i]!=NULL) {
	    scalDelObj(&dbm.node[i].scalNode,
		       dbm.node[i].scalNode.obj[i]->name);
	  }
	Cfree(dbm.node[i].scalNode.obj);
	break;
      case DBM_NODE_SURF:
	for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	  if(dbm.node[i].surfNode.obj_flag[i]!=0) {
	    surfDelObj(&dbm.node[i].surfNode,
		       dbm.node[i].surfNode.obj[i].name);
	  }
	Cfree(dbm.node[i].surfNode.obj);
	break;
      case DBM_NODE_GRID:
	for(j=0;j<dbm.node[i].gridNode.obj_max;j++)
	  if(dbm.node[i].gridNode.obj_flag[i]!=0) {
	    gridDelObj(&dbm.node[i].gridNode,
		       dbm.node[i].gridNode.obj[i].name);
	  }
	Cfree(dbm.node[i].gridNode.obj);
	break;
      }
      
      dbm.node[i].common.type=DBM_NODE_EMPTY;
      strcpy(dbm.node[i].common.name,"");
      comDelDB(name);
      return 0;
    }
  }
  return -1;
}



int dbmScalCheckDist(scalObj *obj, double *v, double d)
{
  register int pc;
  double x,y,z,dx,dy,dz,dd;

  for(pc=0;pc<obj->point_count;pc++) {
    x=obj->point[pc].v[0]-v[0];
    y=obj->point[pc].v[1]-v[1];
    z=obj->point[pc].v[2]-v[2];
    dx=x*x;
    dy=y*y;
    dz=z*z;
    dd=dx+dy+dz;
    if(dd<d)
      return 1;
  }
  return 0;
}

float dbmScalGetProperty(dbmScalNode *node, float *pos, const char *prop)
{
  double uvw[3],p[3];
  double du,dv,dw;
  int u,v,w;
  float d0;

  if(!strcmp(prop,"v") || strlen(prop)==0) {
    p[0]=pos[0];
    p[1]=pos[1];
    p[2]=pos[2];
    scalXYZtoUVW(node->field,p,uvw);
    
    du=uvw[0]-floor(uvw[0]);
    dv=uvw[1]-floor(uvw[1]);
    dw=uvw[2]-floor(uvw[2]);
    
    u=(int)floor(uvw[0]);
    v=(int)floor(uvw[1]);
    w=(int)floor(uvw[2]);
    
    d0=(float)scalReadField(node->field,u,v,w);
    
    return d0;
  } else {
    return 0.0;
  }
}



int dbmStructCheckDist(structObj *obj, double *v, double d)
{
  register int ac;
  struct STRUCT_ATOM *ap;
  double x,y,z,dd;

  for(ac=obj->atom_count-1;ac>=0;--ac) {
    ap=obj->atom[ac].ap;
    x=ap->p->x-v[0];
    y=ap->p->y-v[1];
    z=ap->p->z-v[2];
    dd=x*x+y*y+z*z;
    if(dd<d)
      return 1;
  }
  return 0;
}


float dbmStructGetProperty(dbmStructNode *node, float *pos, const char *prop)
{
  int i;
  float dx,dy,dz,dd,dist;
  struct STRUCT_ATOM *ap;

  dx=node->atom[0].p->x-pos[0];
  dy=node->atom[0].p->y-pos[1];
  dz=node->atom[0].p->z-pos[2];
  dd=dx*dx+dy*dy+dz*dz;
  ap=node->atom+0;
  for(i=1;i<node->atom_count;i++) {
    dx=node->atom[i].p->x-pos[0];
    if(dx<dd) {
      dy=node->atom[i].p->y-pos[1];
      if(dy<dd) {
	dz=node->atom[i].p->z-pos[2];
	if(dz<dd) {
	  dist=dx*dx+dy*dy+dz*dz;
	  if(dist<dd) {
	    dd=dist;
	    ap=node->atom+i;
	  }
	}
      }
    }
  }
  
  return structGetAtomProperty(ap,prop);
}

int dbmSurfCheckDist(surfObj *obj, double *v, double d)
{
  register int vc;
  struct SURF_VERTICE *vp;
  double x,y,z,dd;

  for(vc=0;vc<obj->vertc;vc++) {
    vp=obj->vert[vc].vp;
    x=vp->p[0]-v[0];
    y=vp->p[1]-v[1];
    z=vp->p[2]-v[2];
    dd=x*x+y*y+z*z;
    if(dd<d)
      return 1;
  }
  return 0;
}

int dbmGridCheckDist(gridObj *obj, double *v, double d)
{
  fprintf(stderr,"\nnot implemented");
  return -1;
}


float dbmSurfGetProperty(dbmSurfNode *node, float *pos, const char *prop)
{
  /* 
     the surface in itself cannot carry a property (yet)
     but an attached structure does
  */
  return 0.0;
}


int dbmCalcXtal(struct XTAL *xtal)
{
  double f[8][3]={
    {0.0,0.0,0.0},
    {1.0,0.0,0.0},
    {0.0,1.0,0.0},
    {1.0,1.0,0.0},
    {0.0,0.0,1.0},
    {1.0,0.0,1.0},
    {0.0,1.0,1.0},
    {1.0,1.0,1.0}
  };
  int i;
  double m[16];

  matMakeT2O(xtal->a,xtal->b,xtal->c,
	     xtal->alpha,xtal->beta,xtal->gamma,
	     m);
  xtal->va[0]=m[0];
  xtal->va[1]=m[1];
  xtal->va[2]=m[2];
  xtal->vb[0]=m[4];
  xtal->vb[1]=m[5];
  xtal->vb[2]=m[6];
  xtal->vc[0]=m[8];
  xtal->vc[1]=m[9];
  xtal->vc[2]=m[10];

  for(i=0;i<8;i++) {
    xtal->cc[i][0]=f[i][0]*xtal->a*xtal->a*xtal->va[0]+
      f[i][1]*xtal->b*xtal->b*xtal->vb[0]+
      f[i][2]*xtal->c*xtal->c*xtal->vc[0];
    xtal->cc[i][1]=f[i][0]*xtal->a*xtal->a*xtal->va[1]+
      f[i][1]*xtal->b*xtal->b*xtal->vb[1]+
      f[i][2]*xtal->c*xtal->c*xtal->vc[1];
    xtal->cc[i][2]=f[i][0]*xtal->a*xtal->a*xtal->va[2]+
      f[i][1]*xtal->b*xtal->b*xtal->vb[2]+
      f[i][2]*xtal->c*xtal->c*xtal->vc[2];
  }
  return 0;
}

int dbmUCOTransform(struct XTAL *xtal,transMat *transform, float uco[3])
{
  float va[3],vb[3],vc[3];

  transReset(transform);

  va[0]=xtal->va[0]*xtal->a*xtal->a;
  va[1]=xtal->va[1]*xtal->b*xtal->b;
  va[2]=xtal->va[2]*xtal->c*xtal->c;
  vb[0]=xtal->vb[0]*xtal->a*xtal->a;
  vb[1]=xtal->vb[1]*xtal->b*xtal->b;
  vb[2]=xtal->vb[2]*xtal->c*xtal->c;
  vc[0]=xtal->vc[0]*xtal->a*xtal->a;
  vc[1]=xtal->vc[1]*xtal->b*xtal->b;
  vc[2]=xtal->vc[2]*xtal->c*xtal->c;

  transform->tra[0]=va[0]*uco[0]+vb[0]*uco[1]+vc[0]*uco[2];
  transform->tra[1]=va[1]*uco[0]+vb[1]*uco[1]+vc[1]*uco[2];
  transform->tra[2]=va[2]*uco[0]+vb[2]*uco[1]+vc[2]*uco[2];
  return 0;
}

int dbmSplitPOV(char *oexpr, char *prop, char *op, char *val)
{
  int c,wc;
  char *cp,expr[2048];

  strcpy(expr,oexpr);
  wc=0;
  c=0;
  if(expr[0]=='!') {
    strcpy(op,"!");
    strcpy(prop,expr+1);
    strcpy(val,"");
  } else if((cp=strstr(expr,"*="))!=NULL) {
    op[0]=cp[0];
    op[1]=cp[1];
    op[2]='\0';
    cp[0]='\0';
    strcpy(prop,expr);
    strcpy(val,cp+2);
  } else if((cp=strstr(expr,"/="))!=NULL) {
    op[0]=cp[0];
    op[1]=cp[1];
    op[2]='\0';
    cp[0]='\0';
    strcpy(prop,expr);
    strcpy(val,cp+2);
  } else if((cp=strstr(expr,"+="))!=NULL) {
    op[0]=cp[0];
    op[1]=cp[1];
    op[2]='\0';
    cp[0]='\0';
    strcpy(prop,expr);
    strcpy(val,cp+2);
  } else if((cp=strstr(expr,"-="))!=NULL) {
    op[0]=cp[0];
    op[1]=cp[1];
    op[2]='\0';
    cp[0]='\0';
    strcpy(prop,expr);
    strcpy(val,cp+2);
  } else if((cp=strstr(expr,"="))!=NULL) {
    op[0]=cp[0];
    op[1]='\0';
    cp[0]='\0';
    strcpy(prop,expr);
    strcpy(val,cp+1);
  } else {
    strcpy(prop, expr);
    strcpy(op,"");
    strcpy(val,"");
  }


  return 0;
}


int dbmGetColorHash(char *expr, double *r, double *g, double *b)
{
  char hexr[5], hexg[5], hexb[5];
  int rv,gv,bv;
  double div;

  if(strlen(expr)==4) {    
    hexr[0]=expr[1];
    hexr[1]='\0';
    hexg[0]=expr[2];
    hexg[1]='\0';
    hexb[0]=expr[3];
    hexb[1]='\0';
    div=15.0;
  } else if(strlen(expr)==7) {
    hexr[0]=expr[1];
    hexr[1]=expr[2];
    hexr[2]='\0';
    hexg[0]=expr[3];
    hexg[1]=expr[4];
    hexg[2]='\0';
    hexb[0]=expr[5];
    hexb[1]=expr[6];
    hexb[2]='\0';
    div=255.0;
  } else if(strlen(expr)==13) {
    hexr[0]=expr[1];
    hexr[1]=expr[2];
    hexr[2]=expr[3];
    hexr[3]=expr[4];
    hexr[4]='\0';
    hexg[0]=expr[5];
    hexg[1]=expr[6];
    hexg[2]=expr[7];
    hexg[3]=expr[8];
    hexg[4]='\0';
    hexb[0]=expr[9];
    hexb[1]=expr[10];
    hexb[2]=expr[11];
    hexb[3]=expr[12];
    hexb[4]='\0';
    div=65535.0;
  } else {
    (*r)=0.0;
    (*g)=0.0;
    (*b)=0.0;
    return -1;
  }

  sscanf(hexr,"%0xd",&rv);
  sscanf(hexg,"%0xd",&gv);
  sscanf(hexb,"%0xd",&bv);


  return 0;
}

/***************************************

  dbmSetExtract
  -------------

  extracts a set statement with a
  possible 

     -selection POV...
  or
     -range POV...

  appended, and fills the DBM_SET struct
  the original 'set' or '-set' are NOT
  expected !
  
*****************************************/


int dbmSetExtract(dbmNode *node,struct DBM_SET *s,char *expr,int type)
{
  int i;
  int wc;
  char **wl;
  char set[2560],selection[2560],range[2560];
  char prop[64],op[16],val1[2560],val2[2560],val[2560];
  char message[256];
  char *cp;
  int ct=0,op_id,rf;
  struct LEX_STACK *lsp;
  float v1[3];

  strcpy(set,"");
  strcpy(selection,"");
  strcpy(range,"");
  
  dbmSplit(expr,' ',&wc,&wl);
  for(i=0;i<wc;i++) {
    /*
      go thru the word list, up to a hash
      everything is taken as a set POV
    */
    if(wl[i][0]=='-') {
      /* parameter */
      if(!strcmp(wl[i],"-set")) {
	ct=0;
      } else if(!strcmp(wl[i],"-selection") ||
		!strcmp(wl[i],"-sel")) {
	ct=1;
      } else if(!strcmp(wl[i],"-range")) {
	if(type==DBM_NODE_GEOM) {
	  comMessage("\nset: -range not supported for geometric objects");
	  return -1;
	}
	ct=2;
      } else {
	sprintf(message,"\nset: unknown parameter %s",wl[i]);
	comMessage(message);
	return -1;
      }
    } else {
      if(ct==0) {
	strcat(set,wl[i]);
	/* is it correct to put in a space here ?? */
      } else if(ct==1) {
	strcat(selection,wl[i]);
	strcat(selection," ");
      } else {
	strcat(range,wl[i]);
	strcat(range," ");
      }
    }
  }

  /* pre-process selection */
  if(strlen(selection)>0)
    selection[strlen(selection)-1]='\0';
  else 
    strcpy(selection,"*");

  if(type==DBM_NODE_GEOM) {
    strcpy(s->sel_string,selection);
  } else {
    if((lsp=lexGenerate(selection))==NULL) {
      sprintf(message,"\nset: syntax error in selection: %s",selection);
      comMessage(message);
      return -1;
    } else {
      memcpy(&s->selection,lsp,sizeof(s->selection));
      s->selection_flag=1;
    }
  }
  /* pre-process range */
  if(strlen(range)>0) {
    s->range_flag=1;
    range[strlen(range)-1]='\0';
    strcpy(s->range.expr,range);
    if(dbmRangeExtract(node,&s->range)==-1) {
      sprintf(message,"\nset: syntax error in range: %s",range);
      comMessage(message);
      return -1;
    }
  } else {
    s->range_flag=0;
  }  
  
  /*
  fprintf(stderr,"\nset:%s\nselection:%s\nrange:%s",set,selection,range);
  */

  dbmSplit(set,',',&wc,&wl);
  s->ec=0;
  for(i=0;i<wc;i++) {
    dbmSplitPOV(wl[i],prop,op,val);

    /* check for a range in the set statement */
    if(val[0]=='<') {
      rf=1;
      if(val[strlen(val)-1]!='>') {
	sprintf(message,"\nsyntax error in range");
	comMessage(message);
	return -1;
      }
      if((cp=strchr(val,','))==NULL) {
	strcpy(val1,val);
	strcpy(val2,"");
      } else {
	val[strlen(val)-1]='\0';
	strcpy(val2,cp+1);
	cp[0]='\0';
	strcpy(val1,val+1);
      }
    } else {
      rf=0;
      strcpy(val1,val);
      strcpy(val2,"");
    }

    /* get the operator */
    if(!strcmp(op,"="))
      op_id=DBM_OP_EQ;
    else if(!strcmp(op,"+="))
      op_id=DBM_OP_PE;
    else if(!strcmp(op,"-="))
      op_id=DBM_OP_ME;
    else if(!strcmp(op,"*="))
      op_id=DBM_OP_SE;
    else if(!strcmp(op,"/="))
      op_id=DBM_OP_DE;
    else
      op_id=DBM_OP_NN;

    /* finally evaluate the properties */

    /* generic cases */
    if(!strcmp(prop,"color") ||
       !strcmp(prop,"colour") ||
       !strcmp(prop,"c")) {
      switch(type) {
      case DBM_NODE_STRUCT:
	s->e[s->ec].id=STRUCT_COLOR;
	break;
      case DBM_NODE_SCAL:
	s->e[s->ec].id=SCAL_COLOR;
	break;
      case DBM_NODE_SURF:
	s->e[s->ec].id=SURF_COLOR;
	break;
      case DBM_NODE_GEOM:
	s->e[s->ec].id=GEOM_COLOR;
	break;
      default:
	sprintf(message,"\ninternal error #39 during SetExtr");
	comMessage(message);
	return -1;
      }
      s->e[s->ec].op=op_id;
      if(comGetColor(val1,
		     &s->e[s->ec].value.v[0][0],
		     &s->e[s->ec].value.v[0][1],
		     &s->e[s->ec].value.v[0][2])<0) {
	sprintf(message,"\nset: unknown color: %s", val1);
	comMessage(message);
	return -1;
      }
      if(rf) {
	if(comGetColor(val2,
		       &s->e[s->ec].value.v[1][0],
		       &s->e[s->ec].value.v[1][1],
		       &s->e[s->ec].value.v[1][2])<0) {
	  sprintf(message,"\nset: unknown color: %s", val2);
	  comMessage(message);
	  return -1;
	}
      } else {
	s->e[s->ec].value.v[1][0]=s->e[s->ec].value.v[0][0];
	s->e[s->ec].value.v[1][1]=s->e[s->ec].value.v[0][1];
	s->e[s->ec].value.v[1][2]=s->e[s->ec].value.v[0][2];
      }
      s->ec++;
    } else {
      /* special cases */
      switch(type) {
      case DBM_NODE_STRUCT:
	break;
      case DBM_NODE_SCAL:
	if(!strcmp(prop,"center")) {
	  if(rf) {
	    comMessage("set: range not supported for center");
	    return -1;
	  }
	  s->e[s->ec].id=SCAL_CENTER;
	  s->e[s->ec].op=op_id;
	  matExtract1Df(val1,3,s->e[s->ec].value.v[0]);
	} else if(!strcmp(prop,"method")) {
	  if(rf) {
	    comMessage("set: range not supported for method");
	    return -1;
	  }
	  s->e[s->ec].id=SCAL_METHOD;
	  s->e[s->ec].op=op_id;
	  if(!strcmp(val1,"mc") ||
	     !strcmp(val1,"MC"))
	    s->e[s->ec].value.i[0]=0;
	  else if(!strcmp(val1,"bono") ||
		  !strcmp(val1,"BONO")) 
	    s->e[s->ec].value.i[0]=1;
	  else if(!strcmp(val1,"mc2") ||
		  !strcmp(val1,"MC2")) 
	    s->e[s->ec].value.i[0]=2;
	  else if(!strcmp(val1,"mc3") ||
		  !strcmp(val1,"MC3")) 
	    s->e[s->ec].value.i[0]=3;
	  else {
	    sprintf(message,"\nunknown method %s",val1);
	    comMessage(message);
	    return -1;
	  }
	} else if(!strcmp(prop,"level")) {
	  if(rf) {
	    comMessage("set: range not supported for level");
	    return -1;
	  }
	  if(val1[strlen(val1)-1]=='s') {
	    s->e[s->ec].id=SCAL_LEVELS;
	    val1[strlen(val1)-1]='\0';
	  } else {
	    s->e[s->ec].id=SCAL_LEVEL;
	  }
	  s->e[s->ec].op=op_id;
	  s->e[s->ec].value.f[0]=atof(val1);
	} else if(!strcmp(prop,"points")) {
	  s->e[s->ec].id=SCAL_POINTS;
	  s->e[s->ec].op=op_id;

	  s->e[s->ec].value.i[0]=atoi(val1);
	  if(rf)
	    s->e[s->ec].value.i[1]=atoi(val2);
	  else 
	    s->e[s->ec].value.i[1]=atoi(val1);
	} else if(!strcmp(prop,"t")) {
	  s->e[s->ec].id=SCAL_T;
	  s->e[s->ec].op=op_id;

	  s->e[s->ec].value.f[0]=atof(val1);
	  if(rf)
	    s->e[s->ec].value.f[1]=atof(val2);
	  else 
	    s->e[s->ec].value.f[1]=atof(val1);
	} else if(!strcmp(prop,"size")) {
	  if(rf) {
	    comMessage("set: range not supported for size");
	    return -1;
	  }
	  s->e[s->ec].id=SCAL_SIZE;
	  s->e[s->ec].op=op_id;
	  if(val1[0]=='{') {
	    matExtract1Df(val1,3,v1);
	    s->e[s->ec].value.v[0][0]=v1[0];
	    s->e[s->ec].value.v[0][1]=v1[1];
	    s->e[s->ec].value.v[0][2]=v1[2];
	  } else {
	    s->e[s->ec].value.v[0][0]=atof(val1);
	    s->e[s->ec].value.v[0][1]=atof(val1);
	    s->e[s->ec].value.v[0][2]=atof(val1);
	  }
	} else if(!strcmp(prop,"step")) {
	  if(rf) {
	    comMessage("set: range not supported for step");
	    return -1;
	  }
	  s->e[s->ec].id=SCAL_STEP;
	  s->e[s->ec].op=op_id;
	  s->e[s->ec].value.i[0]=atoi(val1);
	} else {
	  s->e[s->ec].id=SCAL_EMPTY;
	}
	s->ec++;
	break;
      case DBM_NODE_SURF:
	break;
      case DBM_NODE_GEOM:
	if(rf) {
	  comMessage("\nset: range not supported for geometric object");
	  return -1;
	}
	if(!strcmp(prop,"position") ||
	   !strcmp(prop,"pos") ||
	   !strcmp(prop,"p")){
	  s->e[s->ec].id=GEOM_POSITION;
	  s->e[s->ec].op=op_id;
	  matExtract1D(val1,3,s->e[s->ec].value.vd[0]);
	} else if(!strcmp(prop,"radius") ||
		  !strcmp(prop,"r")) {
	  s->e[s->ec].id=GEOM_RADIUS;
	  s->e[s->ec].op=op_id;
	  s->e[s->ec].value.f[0]=atof(val1);
	} else if(!strcmp(prop,"transparency") ||
		  !strcmp(prop,"t")) {
	  s->e[s->ec].id=GEOM_TRANSPARENCY;
	  s->e[s->ec].op=op_id;
	  s->e[s->ec].value.f[0]=atof(val1);
	} else if(!strcmp(prop,"fill") ||
		  !strcmp(prop,"f")) {
	  s->e[s->ec].id=GEOM_FILL;
	  s->e[s->ec].op=op_id;
	  if(op_id==DBM_OP_NN) 
	    s->e[s->ec].value.i[0]=1;
	  else
	    s->e[s->ec].value.i[0]=atoi(val1);
	} else if(!strcmp(prop,"dir") ||
		  !strcmp(prop,"d")) {
	  s->e[s->ec].id=GEOM_DIRECTION;
	  s->e[s->ec].op=op_id;
	  matExtract1D(val1,3,s->e[s->ec].value.vd[0]);
	}
	s->ec++;
	break;
      }
    }
    
  }
  return 0;
}

/***************************

  dbmRangeExtract
  ---------------

  translates an expression
  into a range structure

****************************/

int dbmRangeExtract(dbmNode *node, struct DBM_RANGE *range)
{
  int i;
  char expr[2048],prop[128],op[16],val[1024],*v1,*v2;
  char message[256];
  char **wl;
  int wc;

  strcpy(expr,range->expr);
  dbmSplit(expr,',',&wc,&wl);

  /*
    fill the structure with default values
  */

  range->src=node;
  range->v1=0.0;
  range->v2=0.0;
  strcpy(range->prop,"");


  for(i=0;i<wc;i++) {
    dbmSplitPOV(wl[i],prop,op,val);
    if(!strcmp(prop,"src")) {
      /******************
             src
      *******************/
      if(strcmp(op,"=")) {
	sprintf(message,"\ninvalid or missing operator");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"\nmissing value");
	comMessage(message);
	return -1;
      }
      if(!strcmp(val,".") ||
	 !strcmp(val,". ")) {
	range->src=node;
      } else {
	if(comIsDB(val)) {
	  range->src=comGetDB(val);
	} else {
	  sprintf(message,"\nunknown database: %s",val);
	  comMessage(message);
	  return -1;
	}
      }
    } else if(!strcmp(prop,"prop")) {
      /******************
             prop
      *******************/
      if(strcmp(op,"=")) {
	sprintf(message,"\ninvalid or missing operator");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"\nmissing value");
	comMessage(message);
	return -1;
      }
      strcpy(range->prop,val);
    } else if(!strcmp(prop,"val")) {
      /******************
              val
      *******************/
      if(strcmp(op,"=")) {
	sprintf(message,"\ninvalid or missing operator");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"\nmissing value");
	comMessage(message);
	return -1;
      }
      if(val[0]!='<' || val[strlen(val)-1]!='>') {
	sprintf(message,"\nsyntax error in range: %s",val);
	comMessage(message);
	return -1;
      }
      v2=strchr(val,',');
      if(v2==NULL) {
      } else {
	val[strlen(val)-1]='\0';
	v2[0]='\0';
	v2++;
	v1=val+1;
      }
      /*
	check if min, resp max was given,
	and substitute accordingly
      */
      if(!strcmp(v1,"min")) {
	range->v1=(float)dbmGetMin(range->src,range->prop);
      } else if(!strcmp(v1,"max")) {
	range->v1=(float)dbmGetMax(range->src,range->prop);
      } else {
	range->v1=(float)atof(v1);
      }
      if(!strcmp(v2,"min")) {
	range->v2=(float)dbmGetMin(range->src,range->prop);
      } else if(!strcmp(v2,"max")) {
	range->v2=(float)dbmGetMax(range->src,range->prop);
      } else {
	range->v2=(float)atof(v2);
      }
      /*
      fprintf(stderr,"\n%f %f",range->v1,range->v2);
      */
    } else {
      sprintf(message,"\nunknown range property: %s",prop);
      comMessage(message);
      return -1;
    }
  }

  return 0;
}

/********* 

float dbmGetProperty(float *pos, struct DBM_RANGE *range)
{
  switch(range->src->common.type) {
  case DBM_NODE_EMPTY:
    return -1e12;
  case DBM_NODE_STRUCT:
    return dbmStructGetProperty(&range->src->structNode,pos,range->prop);
  case DBM_NODE_SCAL:
    return dbmScalGetProperty(&range->src->scalNode,pos,range->prop);
  case DBM_NODE_SURF:
    return dbmSurfGetProperty(&range->src->surfNode,pos,range->prop);
  }
  return 1e12;
}
***************/

int dbmGetMinMax(dbmNode *node, char *prop, float *min, float *max)
{
  (*min)=0.0;
  (*max)=0.0;
  switch(node->common.type) {
  case DBM_NODE_STRUCT:
    return structGetMinMax(& node->structNode,prop,min,max);
  case DBM_NODE_SURF:
    return surfGetMinMax(&node->surfNode,prop,min,max);
  case DBM_NODE_SCAL:
    if(prop==NULL || clStrcmp(prop,"v")) {
      (*min)=node->scalNode.min_max.v1;
      (*max)=node->scalNode.min_max.v2;
    } else {
      return -1;
    }
  }
  return 0;
}


float dbmGetMin(dbmNode *node, char *prop)
{
  float a,b;
  switch(node->common.type) {
  case DBM_NODE_STRUCT:
    structGetMinMax(& node->structNode,prop,&a,&b);
    return a;
    break;
  case DBM_NODE_SCAL:
    if(!strcmp(prop,"v"))
      return node->scalNode.min_max.v1;
    break;
  case DBM_NODE_SURF:
    surfGetMinMax(&node->surfNode,prop,&a,&b);
    return a;
    break;
  case DBM_NODE_GRID:
    // TODO later
    break;
  }
    return -1e12;
}

float dbmGetMax(dbmNode *node, char *prop)
{
  float a,b;
  switch(node->common.type) {
  case DBM_NODE_STRUCT:
    structGetMinMax(& node->structNode,prop,&a,&b);
    return b;
    break;
  case DBM_NODE_SCAL:
    if(!strcmp(prop,"v"))
      return node->scalNode.min_max.v2;
    break;
  case DBM_NODE_SURF:
    surfGetMinMax(&node->surfNode,prop,&a,&b);
    return b;
    break;
  case DBM_NODE_GRID:
    // TODO later
    break;
  }
  return 1e12;
}

/************************************
  dbmNew
  ------

  Create a new dataset from scratch
  Currently implemented for the
  geom, but should later be extended
  to all!!!
*************************************/

int dbmNew(int wc, char **wl)
{
  int i;
  char message[256];
  char type[256],name[256];
  dbmNode *node;

  if(wc<1) {
    comMessage("\ndbmNew: missing type");
    return -1;
  }

  
  strcpy(type,wl[0]);
  strcpy(name,"");
  i=1;
  while(i<wc) {
    if(!strcmp(wl[i],"-name")) {
      i++;
      if(i>=wc) {
	comMessage("\ndbmNew: missing name");
	return -1;
      } else {
	strcpy(name,wl[i]);
      }
    } else {
      sprintf(message,"\ndbmNew: unknown parameter %s",wl[i]);
      comMessage(message);
      return -1;
    }
    i++;
  }

  if(strlen(name)==0)
    strcpy(name,type);

  sprintf(message,".%s",name);
  comReturn(message);

  if(!strcmp(type,"struct")) {
    comMessage("\ndbmNew: type 'struct' not yet supported");
    return -1;
  } else if(!strcmp(type,"scal")) {
    comMessage("\ndbmNew: type 'scal' not yet supported");
    return -1;
  } else if(!strcmp(type,"vect")) {
    comMessage("\ndbmNew: type 'vect' not yet supported");
    return -1;
  } else if(!strcmp(type,"surf")) {
    comMessage("\ndbmNew: type 'surf' not yet supported");
    return -1;
  } else if(!strcmp(type,"geom")) {
    node=dbmNewNode(DBM_NODE_GEOM,name);
  }
  return 0;
}



int dbmIsWithin(float *p, float d2, const char *db, const char *obj)
{
  int i,j,flag;

  // TODO later
  // take transform into consideration

  flag=0;
  for(i=0;i<dbm.nodec_max;i++)
    if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
      if(rex(db,dbm.node[i].common.name)) {
	switch(dbm.node[i].common.type) {
	case DBM_NODE_STRUCT:
	  for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	    if(dbm.node[i].structNode.obj_flag[j]!=0)
	      if(rex(obj,dbm.node[i].structNode.obj[j].name)) {
		if(structObjIsWithin(&dbm.node[i].structNode.obj[j],p,d2))
		  return 1;
		flag++;
	      }
	    break;
	case DBM_NODE_SCAL:
	  for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	    if(dbm.node[i].scalNode.obj[j]!=NULL)
	      if(rex(obj,dbm.node[i].scalNode.obj[j]->name)) {
		if(scalObjIsWithin(dbm.node[i].scalNode.obj[j],p,d2))
		  return 1;
		flag++;
	      }
	    break;
	case DBM_NODE_SURF:
	  for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	    if(dbm.node[i].surfNode.obj_flag[j]!=0)
	      if(rex(obj,dbm.node[i].surfNode.obj[j].name)) {
		if(surfObjIsWithin(&dbm.node[i].surfNode.obj[j],p,d2))
		  return 1;
		flag++;
	      }
	  break;
	case DBM_NODE_GRID:
	  for(j=0;j<dbm.node[i].gridNode.obj_max;j++)
	    if(dbm.node[i].gridNode.obj_flag[j]!=0)
	      if(rex(obj,dbm.node[i].gridNode.obj[j].name)) {
		if(gridObjIsWithin(&dbm.node[i].gridNode.obj[j],p,d2))
		  return 1;
		flag++;
	      }
	  break;
	case DBM_NODE_GEOM:
	  for(j=0;j<dbm.node[i].geomNode.obj_max;j++)
	    if(dbm.node[i].geomNode.obj[j]!=NULL)
	      if(rex(obj,dbm.node[i].geomNode.obj[j]->name)) {
		flag++;
	      }
	  break;
	}
      }
    }
  if(!flag) {
    comMessage("\nerror: unknown dataset or object: .");
    comMessage(db);
    comMessage(".");
    comMessage(obj);
    return -1;
  }
  return 0;
}

int dbmIsElementInObj(const char *db, const char *obj, int ele_num) 
{
  int i,j,flag;

  flag=0;
  for(i=0;i<dbm.nodec_max;i++)
    if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
      if(rex(db,dbm.node[i].common.name)) {
	switch(dbm.node[i].common.type) {
	case DBM_NODE_STRUCT:
	  if(ele_num<dbm.node[i].structNode.atom_count)
	    for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	      if(dbm.node[i].structNode.obj_flag[j]!=0)
		if(rex(obj,dbm.node[i].structNode.obj[j].name))
		  return dbm.node[i].structNode.obj[j].atom_flag[ele_num];
	  break;
	case DBM_NODE_SCAL:
	  for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	    if(dbm.node[i].scalNode.obj[j]!=NULL)
	      if(rex(obj,dbm.node[i].scalNode.obj[j]->name))
		return 0;
	  break;
	case DBM_NODE_SURF:
	  if(ele_num<dbm.node[i].surfNode.vc)
	    for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	      if(dbm.node[i].surfNode.obj_flag[j]!=0)
		if(rex(obj,dbm.node[i].surfNode.obj[j].name))
		  return dbm.node[i].surfNode.obj[j].vert_flag[ele_num];
	  break;
	case DBM_NODE_GRID:
	  // TODO later
	  // same problem with the save-array
	  break;
	case DBM_NODE_GEOM:
	  for(j=0;j<dbm.node[i].geomNode.obj_max;j++)
	    if(dbm.node[i].geomNode.obj[j]!=NULL)
	      if(rex(obj,dbm.node[i].geomNode.obj[j]->name)) {
		flag++;
	      }
	  break;
	}
      }
    }
  if(!flag) {
    comMessage("\nerror: unknown dataset or object: .");
    comMessage(db);
    comMessage(".");
    comMessage(obj);
    return -1;
  }
  return 0;
}


int dbmGetRangeVal(Range *range, float *p, float *r)
{
  int i;
  (*r)=0.0;

  for(i=0;i<dbm.nodec_max;i++) {
    if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
      if(rex(range->src->common.name,dbm.node[i].common.name)) {
	switch(dbm.node[i].common.type) {
	case DBM_NODE_STRUCT:
	  return structGetRangeXYZVal(&range->src->structNode,range->prop,p,r);
	case DBM_NODE_SCAL:
	  return scalGetRangeXYZVal(&range->src->scalNode,range->prop,p,r);
	case DBM_NODE_SURF:
	  return surfGetRangeXYZVal(&range->src->surfNode,range->prop,p,r);
	case DBM_NODE_GRID:
	  return gridGetRangeXYZVal(&range->src->gridNode,range->prop,p,r);
	}
      }
    }
  }

  return 0;
}

static string dbm_wl[2048];

int dbmSplit(char *expr, char split_char, int *nwc, char ***wl)
{
  int c,s,wc;
  char **nwl;
  int bc;

  nwl=dbm_wl;

  c=0;
  wc=0;
  s=strlen(expr);
  nwl[wc]=&expr[c];
  while(c<s) {
    if(expr[c]==split_char) {
      expr[c]='\0';
      c++;
      if(c>s)
	break;
      nwl[++wc]=&expr[c];
    } else if(expr[c]=='<' && expr[c+1]!='=') {
      bc=1;
      c++;
      while(bc>=1) {
	if(expr[c]=='<')
	  bc++;
	if(expr[c]=='>')
	  bc--;
	c++;
	if(c>=s)
	  break;
      }
    } else if(expr[c]=='{') {
      bc=1;
      c++;
      while(bc>=1) {
	if(expr[c]=='{')
	  bc++;
	if(expr[c]=='}')
	  bc--;
	c++;
	if(c>=s)
	  break;
      }
    } else if(expr[c]=='\"') {  // "."
      bc=1;
      c++;
      while(bc==1) {
	if(expr[c]=='\"')
	  bc--;
	c++;
	if(c>=s)
	  break;
      }
    } else {
      c++;
    }
  }

  if(strlen(nwl[wc])>0)
    wc++;

  (*nwc)=wc;  
  (*wl)=nwl;
  return 0;
}
