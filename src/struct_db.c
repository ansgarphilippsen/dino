#include "gl_include.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>

#include "dino.h"
#include "struct_db.h"
#include "struct_obj.h"
#include "struct_read.h"
#include "dbm.h"
#include "com.h"
#include "rex.h"
#include "mat.h"
#include "render.h"
#include "conn.h"
#include "Cmalloc.h"
#include "cgfx.h"
#include "gfx.h"
#include "cl.h"
#include "xplor.h"
#include "gui_ext.h"
#include "io_gromacs.h"

extern struct GFX gfx;

extern int cmalloc_free,debug_mode;

int structNewNode(struct DBM_STRUCT_NODE *node)
{
  int i;
  char message[256];
  double imat[]={1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0};

  node->obj_count=0;
  node->obj_max=64;
  node->obj=0;
  node->obj=(structObj*)Crecalloc(node->obj,node->obj_max,sizeof(structObj));
  if(node->obj==NULL) {
    sprintf(message,"memory error in structNewNode\n");
    comMessage(message);
    return -1;
  }
  node->obj_flag=(int*)Ccalloc(node->obj_max,sizeof(int));
  if(node->obj_flag==NULL) {
    sprintf(message,"memory error in structNewNode\n");
    comMessage(message);
    return -1;
  }

  for(i=0;i<node->obj_max;i++)
    node->obj_flag[i]=0;

  node->model_count=0;
  node->chain_count=0;
  node->chain=NULL;
  node->residue_count=0;
  node->residue=NULL;
  node->atom_count=0;
  node->atom=NULL;
  node->bond_count=0;
  node->bond_max=0;
  node->bond=NULL;
#ifdef WITH_NCBONDS
  node->nbond_count=0;
#endif
  node->conn=NULL;
  node->xtal=NULL;
  node->helical=NULL;
  node->atom_table=NULL;
  node->ca=NULL;
  node->trj_flag=0;
  node->trj_fast=1;

  transReset(&node->transform);

  transListInit(&node->symop_list,50);

  return 0;
}

/*
  this routine is called after a structure has been read in
*/

int structPrep(dbmStructNode *node)
{
  char message[256];

  // build CubeArray
  comMessage("b");
  structBuildCA(node);

  // determine the connectivity
  comMessage("c");
  structReconnect(node);

  // determine min and max params
  comMessage("m");
  structSetMinMax(node);

  // set the center to geometric center
  comMessage("r");
  structRecenter(node);

  // calculate various res params
  comMessage("p");
  structPrepRes(node);

  comMessage("]");

  sprintf(message," %d atom(s),",node->atom_count);
  if(node->residue_flag)
    sprintf(message,"%s %d residue(s),",
	    message,node->residue_count);
  if(node->chain_flag)
    sprintf(message,"%s %d chain(s),",
	    message,node->chain_count);
  if(node->model_flag)
    sprintf(message,"%s %d model(s),",
	    message,node->model_count);
  sprintf(message,"%s %d bond(s) (%d expl)",
	  message,node->bond_count, node->conn_count);
  comMessage(message);
  comMessage("\n");
  

  return 0;
}


/* precalc dir, vect1 and vect2 */

int structPrepRes(dbmStructNode *node)
{
  int i,j;

  for(i=0;i<node->residue_count;i++) {
    for(j=0;j<3;j++) {
      node->residue[i].dir[j]=0.0;
      node->residue[i].v1[j]=0.0;
      node->residue[i].v2[j]=0.0;
      node->residue[i].v3[j]=0.0;
      node->residue[i].v4[j]=0.0;
      node->residue[i].v5[j]=0.0;
    }
    if(node->residue[i].clss==STRUCT_PROTEIN) {
      structGetVectProtein(node->residue+i);
    } else if(node->residue[i].clss==STRUCT_NA) {
      node->residue[i].res_id=structGetVectNA(node->residue+i);
    } else {
    }
  }

  return 0;
}

int structCommand(dbmStructNode *node,int wc,char **wl)
{
  char message[256];
  double imat[]={1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0};
  char *empty_com[]={"get","center"};
  double v[3];
  int i;

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }
  if(clStrcmp(wl[0],"?") ||
     clStrcmp(wl[0],"help")) {
  } else if(clStrcmp(wl[0],"new")) {
    structComNew(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"get")) {
    return structComGet(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"del") ||
	    clStrcmp(wl[0],"delete")) {
    return structComDel(node, wc-1,wl+1);
  } else if(clStrcmp(wl[0],"set")) {
    return structComSet(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"copy") ||
	    clStrcmp(wl[0],"cp")) {
    sprintf(message,"%s: copy not available\n",node->name);
    comMessage(message);
    return -1;
  } else if(clStrcmp(wl[0],"move") ||
	    clStrcmp(wl[0],"mv")) {
    sprintf(message,"%s: move not available\n",node->name);
    comMessage(message);
    return -1;
  } else if(clStrcmp(wl[0],"restrict")) {
    structComRestrict(node, wc-1,wl+1);
  } else if(clStrcmp(wl[0],"load")) {
    return structComLoad(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"step")) {
    if(!node->trj_flag) {
      sprintf(message,"%s: no trajectory loaded\n",node->name);
      comMessage(message);
      return -1;
    }
    if(wc==1) {
      node->frame++;
    } else if(wc==2) {
      node->frame+=atoi(wl[1]);
    } else {
      sprintf(message,"too many parameters: usage: step [n]\n");
      comMessage(message);
      return -1;
    }
    if(node->frame<0)
      node->frame=node->trj.frame_count-1;
    else if(node->frame>=node->trj.frame_count)
      node->frame=0;
    structSetFrame(node,node->frame);
    comRedraw();
  } else if(clStrcmp(wl[0],"play")) {
    if(!node->trj_flag) {
      sprintf(message,"%s: no trajectory loaded\n",node->name);
      comMessage(message);
      return -1;
    } else {
      structComPlay(node, wc-1,wl+1);
    }
  } else if(clStrcmp(wl[0],"stop")) {
    if(!node->trj_flag) {
      comMessage("no trajectory present\n");
      return -1;
    }
    comPlay((dbmNode *)node,COM_PLAY_OFF);
    node->trj_play=0;
    structSetFrame(node,0);
    comRedraw();
  } else if(clStrcmp(wl[0],"grab")) {
    if(wc!=2) {
      sprintf(message,"Syntax: grab devicename\n");
      comMessage(message);
    } else {
      if(comGrab(&node->transform,(transCallback)structOnTransform,(void*)node,wl[1])<0)
	return -1;
      // set center to current center
      comGetCurrentCenter(node->transform.cen);
      structRecenter(node);
    }
  } else if(clStrcmp(wl[0],"write")) {
    structWrite(node,NULL,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"reset")) {
    if(wc==1) {
      transReset(&node->transform);
      structRecenter(node);
    } else {
      for(i=1;i<wc;i++) {
	if(clStrcmp(wl[i],"all")) {
	  transReset(&node->transform);
	  structRecenter(node);
	} else if(clStrcmp(wl[i],"center")) {
	  structRecenter(node);
	} else if(clStrcmp(wl[i],"rot")) {
	  transResetRot(&node->transform);
	} else if(clStrcmp(wl[i],"trans")) {
	  transResetTra(&node->transform);
	}
      }
    }
    comRedraw();
  } else if(clStrcmp(wl[0],"fix")) {
    if(wc>1) {
      sprintf(message,"warning: fix: superfluous parameter ignored\n");
      comMessage(message);
    }
    structFix(node);
    structSetMinMax(node);
    structRecenter(node);
    comRedraw();
  } else if(clStrcmp(wl[0],"rotx")) {
    if(wc<2) {
      comMessage("error: missing value after rotx\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTX,-1,atof(wl[1]));
    comRedraw();
  } else if(clStrcmp(wl[0],"roty")) {
    if(wc<2) {
      comMessage("error: missing value after roty\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTY,-1,atof(wl[1]));
    comRedraw();
  } else if(clStrcmp(wl[0],"rotz")) {
    if(wc<2) {
      comMessage("error: missing value after rotz\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTZ,-1,atof(wl[1]));
    comRedraw();
  } else if(clStrcmp(wl[0],"transx")) {
    if(wc<2) {
      comMessage("error: missing value after transx\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAX,-1,atof(wl[1]));
    comRedraw();
  } else if(clStrcmp(wl[0],"transy")) {
    if(wc<2) {
      comMessage("error: missing value after transy\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAY,-1,atof(wl[1]));
    comRedraw();
  } else if(clStrcmp(wl[0],"transz")) {
    if(wc<2) {
      comMessage("error: missing value after transz\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAZ,-1,atof(wl[1]));
    comRedraw();
  } else if(clStrcmp(wl[0],"center")) {
    if(wc<2) {
      sprintf(message,"missing value for center\n");
      comMessage(message);
      return -1;
    }
    if(matExtract1D(wl[1],3,v)!=0) {
      sprintf(message,"error in vector\n");
      comMessage(message);
      return -1;
    }
    node->transform.cen[0]=v[0];
    node->transform.cen[1]=v[1];
    node->transform.cen[2]=v[2];
    comRedraw();
  } else if(clStrcmp(wl[0],"connect")) {
    if(wc!=3) {
      sprintf(message,"syntax: .struct connect ATOM1 ATOM2\n");
      comMessage(message);
      return -1;
    }
    return structComConnect(node,wl[1],wl[2]);
  } else if(clStrcmp(wl[0],"reconnect")) {
    if(wc==1) {
      i=0xff;
    } else if(wc==2) {
      i=atoi(wl[1]);
    } else {
      sprintf(message,"syntax: .struct reconnect [flag]\n");
      comMessage(message);
      return -1;
    }
    i &= (STRUCT_CONN_IMPL | STRUCT_CONN_EXPL | STRUCT_CONN_DIST);
    node->conn_flag=i;
    if(i) {
      clStrcpy(message,"reconnecting with rules:");
      if(i & STRUCT_CONN_IMPL)
	clStrcat(message," implicit(0x1)");
      if(i & STRUCT_CONN_EXPL)
	clStrcat(message," explicit(0x2)");
      if(i & STRUCT_CONN_DIST)
	clStrcat(message," distance(0x4)");
      clStrcat(message,"\n");
    } else {
      clStrcpy(message,"reconnecting without any rules, removing all bonds");
    }
    comMessage(message);
    structReconnect(node);
    sprintf(message,"created %d bonds\n",node->bond_count);
    comMessage(message);
  } else if(clStrcmp(wl[0],"cell")) {
    if(node->xtal==NULL) {
      comMessage("no crystallographic info available\n");
      return -1;
    }
    if(wc==1) {
      sprintf(message,"%s %.2f %.2f %.2f  %.2f %.2f %.2f\n",
	      node->xtal->space_group_name,
	      node->xtal->a, node->xtal->b, node->xtal->c,
	      node->xtal->alpha, node->xtal->beta, node->xtal->gamma);
      comMessage(message);
    } else {
      if(clStrcmp(wl[1],"show")) {
	node->show_cell=1;
	comRedraw();
      } else if(clStrcmp(wl[1],"hide")) {
	node->show_cell=0;
	comRedraw();
      } else {
	sprintf(message,"error: unknown cell command '%s'\n",wl[1]);
      }
    }
  } else if(clStrcmp(wl[0],"add")) {
    if(wc<2) {
      comMessage("missing type after 'add'\n");
      return -1;
    } else {
      if(clStrcmp(wl[1],"symop")) {
	if(wc<3) {
	  comMessage("missing parameters after 'add symop'\n");
	  return -1;
	} else {
	  structAddSymop(node,wc-2,wl+2);
	}
	 
      } else {
	sprintf(message,"unknown type '%s' to add\n",wl[1]);
	comMessage(message);
	return -1;
      }
    }
  } else {
    sprintf(message,"%s: unknown command %s\n",node->name,wl[0]);
    comMessage(message);
    return -1;
  }
  
  return 0;
}


int structComNew(dbmStructNode *node, int wc, char **wl)
{
  int i,ret;
  char *name;
  int type;
  Set set;
  Select sel;
  clOrg co;
  int set_flag,sel_flag;
  char message[256];

  set_flag=sel_flag=0;

  name=node->name;
  type=STRUCT_CONNECT;

  clNew(&co,wc,wl);
  ret=0;
  for(i=0;i<co.param_count;i++) {
    if(co.param[i].p==NULL) {
      if(co.param[i].wc!=0) {
	comMessage("error: new: expected an argument beginning with -\n"); 
	ret=-1;
	break;
      }
    } else if(clStrcmp(co.param[i].p,"name") || 
	      clStrcmp(co.param[i].p,"n")) {
      if(co.param[i].wc<1) {
	comMessage("error: new: missing value for -name\n");
	ret=-1;
	break;
      } else if(co.param[i].wc>1) {
	comMessage("error: new: too many values for -name\n");
	ret=-1;
	break;
      } else {
	name=co.param[i].wl[0];
      }
    } else if(clStrcmp(co.param[i].p,"type") ||
	      clStrcmp(co.param[i].p,"t")) {
      if(co.param[i].wc<1) {
	comMessage("error: new: missing value for -type\n");
	ret=-1;
	break;
      } else if(co.param[i].wc>1) {
	comMessage("error: new: too many values for -type\n");
	ret=-1;
	break;
      } else {
	if(clStrcmp(co.param[i].wl[0],"connect")) {
	  type=STRUCT_CONNECT;
	} else if(clStrcmp(co.param[i].wl[0],"trace")) { 
	  type=STRUCT_TRACE;
#ifdef WITH_NCBONDS
	} else if(clStrcmp(co.param[i].wl[0],"nbond")) { 
	  type=STRUCT_NBOND;
#endif
	} else {
	  sprintf(message,"error: new: unknown type %s\n",co.param[i].wl[0]);
	  comMessage(message);
	  ret=-1;
	  break;
	}
      }
    } else if(clStrcmp(co.param[i].p,"set") ||
	      clStrcmp(co.param[i].p,"s")) {
      if(setNew(&set,co.param[i].wc,co.param[i].wl)<0) {
	ret=-1;
	break;
      }
      set_flag=1;
    } else if(clStrcmp(co.param[i].p,"selection") ||
	      clStrcmp(co.param[i].p,"select") ||
	      clStrcmp(co.param[i].p,"sel")) {
     if(selectNew(&sel,co.param[i].wc,co.param[i].wl)<0) {
	ret=-1;
	break;
      }
      sel_flag=1;
    } else {
      clStrcpy(message,"unknown paramater \n");
      clStrncat(message,co.param[i].p,100);
      comMessage(message);
      ret=-1;
      break;
    }
  }
  if(ret<0) {
    if(sel_flag)
      selectDelete(&sel);
    if(set_flag)
      setDelete(&set);
    clDelete(&co);
    return -1;
  }

  if(!sel_flag) {
    selectNew(&sel,0,NULL);
  }

  if(!set_flag) {
    setNew(&set,0,NULL);
  }

  ret=structNew(node, name,type,&set,&sel);

  setDelete(&set);
  clDelete(&co);

  return ret;
}

int structComSet(dbmStructNode *node, int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;

  ret=structSet(node, &set);
  
  setDelete(&set);

  return ret;
}

int structComGet(dbmStructNode *node, int wc, char **wl)
{
  int i;

  if(wc==0) {
    comMessage("error: get: missing property\n");
    return -1;
  }
  for(i=0;i<wc;i++)
    if(structGet(node, wl[i])<0)
      return -1;
  return 0;
}

int structComRestrict(dbmStructNode *node, int wc, char **wl)
{
  int i,ret,c;
  Select sel;
  char message[256];

  if(selectNew(&sel, wc,wl)<0)
    return -1;

  // reset restriction !!!
  for(i=0;i<node->atom_count;i++)
    node->atom[i].restriction=0;

  // check each atom
  c=0;
  for(i=0;i<node->atom_count;i++) {
    ret=structIsAtomSelected(node, &node->atom[i], &sel);
    if(ret<0) {
      selectDelete(&sel);
      return -1;
    }
    if(ret==0) {
      node->atom[i].restriction=1;
      c++;
   }
  }

  // output how many atoms were flaged
  if(c==0)
    sprintf(message,"all atoms unrestricted\n");
  else if(c==node->atom_count)
    sprintf(message,"restriction affects ALL of %d atoms\n",node->atom_count);
  else
    sprintf(message,"restriction affects %d of %d atoms\n",c,node->atom_count);
  comMessage(message);

  selectDelete(&sel);
  return 0;
}

int structNew(dbmStructNode *node, char *name, int type, Set *set, Select *sel)
{
  structObj *obj;

  if((obj=structNewObj(node, name))==NULL) {
    comMessage("error: new: internal error");
    return -1;
  }
  obj->type=type;
  obj->node=node;

  memcpy(&obj->select,sel,sizeof(Select));

  return structObjRenew(obj, set, sel);
}


// cell
// frame

int structSet(dbmStructNode *node, Set *s)
{
  int pc;
  struct POV_VALUE *val;
  int i,rt,op;
  Select *sel;
  float r,g,b,v1[6];

  if(s->pov_count==0) {
    return 0;
  }

  if(s->range_flag) {
    comMessage("error: set: range not expected for struct dataset\n");
    return -1;
  }

  if(s->select_flag)
    sel=&s->select;
  else
    sel=NULL;

  for(pc=0;pc<s->pov_count;pc++) {
    if (clStrcmp(s->pov[pc].prop,"rot")) {
      s->pov[pc].id=STRUCT_PROP_ROT;
    } else if (clStrcmp(s->pov[pc].prop,"trans")) {
      s->pov[pc].id=STRUCT_PROP_TRANS;
    } else if (clStrcmp(s->pov[pc].prop,"rtc")) {
      s->pov[pc].id=STRUCT_PROP_RTC;
    } else if (clStrcmp(s->pov[pc].prop,"center") ||
	       clStrcmp(s->pov[pc].prop,"rcen")) {
      s->pov[pc].id=STRUCT_PROP_RCEN;
    } else if(clStrcmp(s->pov[pc].prop,"tfast")) {
      s->pov[pc].id=STRUCT_PROP_TFAST;
    } else if(clStrcmp(s->pov[pc].prop,"rtype")) {
      s->pov[pc].id=STRUCT_PROP_RTYPE;
    } else if(clStrcmp(s->pov[pc].prop,"nci")) {
      s->pov[pc].id=STRUCT_PROP_NCI;
    } else if(clStrcmp(s->pov[pc].prop,"smode")) {
      s->pov[pc].id=STRUCT_PROP_SMODE;
    } else if(clStrcmp(s->pov[pc].prop,"vdwr")) {
      s->pov[pc].id=STRUCT_PROP_RAD;
    } else if(clStrcmp(s->pov[pc].prop,"color") ||
	      clStrcmp(s->pov[pc].prop,"colour") ||
	      clStrcmp(s->pov[pc].prop,"col")) {
      s->pov[pc].id=STRUCT_PROP_COLOR;
    } else if(clStrcmp(s->pov[pc].prop,"cell")) {
      s->pov[pc].id=STRUCT_PROP_CELL;
    } else if(clStrcmp(s->pov[pc].prop,"helicalparams")) {
      s->pov[pc].id=STRUCT_PROP_HELSYM;
    } else if(clStrcmp(s->pov[pc].prop,"sg") ||
	      clStrcmp(s->pov[pc].prop,"spacegroup")) {
      s->pov[pc].id=STRUCT_PROP_SG;
    } else if(clStrcmp(s->pov[pc].prop,"frame")) {
      s->pov[pc].id=STRUCT_PROP_FRAME;
    } else if(clStrcmp(s->pov[pc].prop,"bfac")) {
      s->pov[pc].id=STRUCT_PROP_BFAC;
    } else if(clStrcmp(s->pov[pc].prop,"occ") ||
	      clStrcmp(s->pov[pc].prop,"weight")) {
      s->pov[pc].id=STRUCT_PROP_OCC;
    } else {
      comMessage("error: set: unknown property ");
      comMessage(s->pov[pc].prop);
      comMessage("\n");
      return -1;
    }
    if(s->pov[pc].op!=POV_OP_EQ && s->pov[pc].id!=STRUCT_PROP_TFAST) {
      comMessage("error: set: expected operator = for property ");
      comMessage(s->pov[pc].prop);
      comMessage("\n");
      return -1;
    }
    op=s->pov[pc].op;
    if(s->pov[pc].val_count>1 && s->pov[pc].id!=STRUCT_PROP_NCI) {
      comMessage("error: set: expected only one value for property ");
      comMessage(s->pov[pc].prop);
      comMessage("\n");
      return -1;
    }
  }

  for(pc=0;pc<s->pov_count;pc++) {
    val=povGetVal(&s->pov[pc],0);
    switch(s->pov[pc].id) {
    case STRUCT_PROP_ROT:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rot\n");
	return -1;
      }
      if(transSetRot(&node->transform,val->val1)<0)
	return -1;
      break;
    case STRUCT_PROP_TRANS:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property trans\n");
	return -1;
      }
      if(transSetTra(&node->transform,val->val1)<0)
	return -1;
      break;
    case STRUCT_PROP_RTC:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rtc\n");
	return -1;
      }
      if(transSetAll(&node->transform,val->val1)<0)
	return -1;

      break;
    case STRUCT_PROP_RCEN:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rcen\n");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v1)!=0) {
	comMessage("error in vector: ");
	comMessage(val->val1);
	comMessage("\n");
	return -1;
      }   
      transApplyIf(&node->transform, v1);
      node->transform.cen[0]=v1[0];
      node->transform.cen[1]=v1[1];
      node->transform.cen[2]=v1[2];
      node->transform.cen[3]=1.0;
      break;
    case STRUCT_PROP_SMODE:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property smode\n");
	return -1;
      }
      if(clStrcmp(val->val1,"atom")) {
	node->smode=SEL_ATOM;
      } else if(clStrcmp(val->val1,"residue")) {
	node->smode=SEL_RESIDUE;
      } else {
	comMessage("error: unknown smode ");
	comMessage(val->val1);
	comMessage("\n");
	return -1;
      }
      break;
    case STRUCT_PROP_RTYPE:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rtype\n");
	return -1;
      }
      if(clStrcmp(val->val1,"coil")) {
	rt=STRUCT_RTYPE_COIL;
      } else if (clStrcmp(val->val1,"helix")) {
	rt=STRUCT_RTYPE_HELIX;
      } else if (clStrcmp(val->val1,"strand")) {
	rt=STRUCT_RTYPE_STRAND;
      } else {
	comMessage("unknown rtype: ");
	comMessage(val->val1);
	comMessage("\n");
	return -1;
      }

      for(i=0;i<node->atom_count;i++) {
	if(structIsAtomSelected(node, &node->atom[i], sel)) {
	  node->atom[i].residue->type=rt;
	}
      }

      break;
    case STRUCT_PROP_NCI:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property nci\n");
	return -1;
      }

      rt=atoi(val->val1);

      for(i=0;i<node->atom_count;i++) {
	if(structIsAtomSelected(node, &node->atom[i], sel)) {
	  node->atom[i].flag=rt;
	}
      }

      break;
    case STRUCT_PROP_RAD:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property vdwr\n");
	return -1;
      }

      for(i=0;i<node->atom_count;i++) {
	if(structIsAtomSelected(node, &node->atom[i], sel)) {
	  node->atom[i].def_prop.radius=atof(val->val1);
	}
      }

      break;
    case STRUCT_PROP_COLOR:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property color\n");
	return -1;
      }
      if(comGetColor(val->val1,&r,&g,&b)<0) {
	comMessage("error: set: unknown color ");
	comMessage(val->val1);
	comMessage("\n");
	return -1;
      }
      for(i=0;i<node->atom_count;i++) {
	if(structIsAtomSelected(node, &node->atom[i], sel)) {
	  node->atom[i].def_prop.r=r;
	  node->atom[i].def_prop.g=g;
	  node->atom[i].def_prop.b=b;
	  node->atom[i].def_prop.c[0][0]=node->atom[i].def_prop.r;
	  node->atom[i].def_prop.c[0][1]=node->atom[i].def_prop.g;
	  node->atom[i].def_prop.c[0][2]=node->atom[i].def_prop.b;
	  node->atom[i].def_prop.c[1][0]=node->atom[i].def_prop.r;
	  node->atom[i].def_prop.c[1][1]=node->atom[i].def_prop.g;
	  node->atom[i].def_prop.c[1][2]=node->atom[i].def_prop.b;
	  node->atom[i].def_prop.c[2][0]=node->atom[i].def_prop.r;
	  node->atom[i].def_prop.c[2][1]=node->atom[i].def_prop.g;
	  node->atom[i].def_prop.c[2][2]=node->atom[i].def_prop.b;
	}
      }
      break;
    case STRUCT_PROP_TFAST:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property tfast\n");
	return -1;
      }
      if(op==POV_OP_NOT) {
	node->trj_fast=0;
      } else if(op==POV_OP_NN) {
	node->trj_fast=1;
      } else {
	comMessage("syntax error with property tfast\n");
	return -1;
      }
      break;
    case STRUCT_PROP_FRAME:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property tfast\n");
	return -1;
      }
      if(node->trj_flag) {
	if(atoi(val->val1)<1 || atoi(val->val1)>node->trj.frame_count) {
	  comMessage("error: frame number out of limits\n"); 
	  return -1;
	} else {
	  structSetFrame(node,atoi(val->val1)-1);
	  comRedraw();
	}
      } else {
	comMessage("error: no trajectory loaded\n");
	return -1;
      }
      break;
    case STRUCT_PROP_CELL:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property cell\n");
	return -1;
      }
      if(matExtract1Df(val->val1,6,v1)==-1) {
	comMessage("error in cell expression, expected {a,b,c,alpha,beta,gamma}\n");
	return -1;
      }
      if(node->xtal==NULL) {
	node->xtal=(struct XTAL*)Cmalloc(sizeof(struct XTAL));
	strcpy(node->xtal->space_group_name,"P1");
      }
      node->xtal->a=v1[0];
      node->xtal->b=v1[1];
      node->xtal->c=v1[2];
      node->xtal->alpha=v1[3];
      node->xtal->beta=v1[4];
      node->xtal->gamma=v1[5];
      dbmCalcXtal(node->xtal);
      break;
    case STRUCT_PROP_SG:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property cell\n");
	return -1;
      }
      if(node->xtal==NULL) {
	node->xtal=(struct XTAL*)Cmalloc(sizeof(struct XTAL));
	node->xtal->a=1.0;
	node->xtal->b=1.0;
	node->xtal->c=1.0;
	node->xtal->alpha=90.0;
	node->xtal->beta=90.0;
	node->xtal->gamma=90.0;
      }
      strcpy(node->xtal->space_group_name,val->val1);
      dbmCalcXtal(node->xtal);
      break;
    case STRUCT_PROP_HELSYM:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property helicalsym\n");
	return -1;
      }
      v1[2]=1.0; // default axr
      v1[3]=0.0; // default aoffset
      if(matExtract1Df(val->val1,4,v1)==-1) {
	if(matExtract1Df(val->val1,3,v1)==-1) {
	  if(matExtract1Df(val->val1,2,v1)==-1) {
	    comMessage("error in helicalsym value, expected {angle,dist[,axialratio[,angleoffset]]}\n");
	    return -1;
	  }
	}
      }
      if(v1[2]<=0.0) {
	comMessage("value for axialration must be positive\n");
	return -1;
      }
      if(node->helical==NULL) {
	node->helical=(struct HELICAL*)Cmalloc(sizeof(struct HELICAL));
      }
      node->helical->angle=v1[0];
      node->helical->dist=v1[1];
      node->helical->axr=v1[2];
      node->helical->aoffset=v1[3];

      // TODO OTHER ATOM PROPERTIES
    case STRUCT_PROP_BFAC:
      for(i=0;i<node->atom_count;i++) {
	if(structIsAtomSelected(node, &node->atom[i], sel)) {
	  node->atom[i].bfac=atof(val->val1);
	}
      }
      break;
    case STRUCT_PROP_OCC:
      for(i=0;i<node->atom_count;i++) {
	if(structIsAtomSelected(node, &node->atom[i], sel)) {
	  node->atom[i].weight=atof(val->val1);
	}
      }
      break;
    }
  }
  return 0;
}

int structComConnect(dbmStructNode *node, char *s1, char *s2)
{
  char m1[256],m2[256],c1[256],c2[256],r1[256],r2[256],a1[256],a2[256];
  int n1,n2;
  char message[256];

  debmsg("structConnect: retrieving atoms from strings");

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

  /* both atoms were found, now connect them */

  debmsg("structConnect: connecting atoms");

  structConnectAtoms(node,
		     &node->bond,
		     &node->bond_count,
		     &node->bond_max,
		     &node->atom[n1], &node->atom[n2]);

  debmsg("structConnect: regenerating bonds");

  structRecalcBonds(node);

  return 0;
}


int structGet(dbmStructNode *node, char *prop)
{
  int ret=0,i;
  float v1[4];
  char message[256];

  if(clStrcmp(prop,"center")) {
    v1[0]=node->transform.cen[0];
    v1[1]=node->transform.cen[1];
    v1[2]=node->transform.cen[2];

    transApplyf(&node->transform,v1);

    sprintf(message,"{%.5f,%.5f,%.5f}",v1[0],v1[1],v1[2]);
    comReturn(message);
  } else if(clStrcmp(prop,"rcenter") ||
	    clStrcmp(prop,"rcen")) {
    sprintf(message,"{%.5f,%.5f,%.5f}",
	    node->transform.cen[0],
	    node->transform.cen[1],
	    node->transform.cen[2]);
    comReturn(message);
  } else if(clStrcmp(prop,"rtc")) {
    comReturn(transGetAll(&node->transform));
  } else if(clStrcmp(prop,"cell")){
    if(node->xtal==NULL) {
      sprintf(message,"%s: no cell defined\n",node->name);
      comMessage(message);
      ret=-1;
    } else {
      sprintf(message,"%s: cell: %.3f %.3f %.3f   %.2f %.2f %.2f  %s\n",
	      node->name,
	      node->xtal->a,node->xtal->b,node->xtal->c,
	      node->xtal->alpha,node->xtal->beta,node->xtal->gamma,
	      node->xtal->space_group_name);
      comMessage(message);
    }
  } else if(!strcmp(prop,"helicalparams")){
    if(node->helical==NULL) {
      sprintf(message,"%s: no helical parameters defined\n",node->name);
      comMessage(message);
      ret=-1;
    } else {
      sprintf(message,"%s: helical parameters: %.3f %.3f\n",
	      node->name,
	      node->helical->angle,node->helical->dist);
      comMessage(message);
    }
  } else if(!strcmp(prop,"rot")) {
    comReturn(transGetRot(&node->transform));
  } else if(clStrcmp(prop,"trans")) {
    comReturn(transGetTra(&node->transform));
  } else if(clStrcmp(prop,"smode")) {
    if(node->smode==SEL_ATOM)
      comReturn("atom");
    else if(node->smode==SEL_RESIDUE) {
      comReturn("residue");
    } else {
      comReturn("unknown");
    }
  } else {
    sprintf(message,"%s: unknown parameter %s\n", node->name,prop);
    comMessage(message);
    ret=-1;
  }
  
  return ret;
}

/*
  recalculate the connectivity based on
  the following rules

  a) implicit: both residue- and atom-names are define
     in lookup table

  b) explicit: two atoms connected by CONECT statement in file
     or on command line

  c) distance: connect atoms within bonding distance 

  NOTE: the way it is implemented, it will first try a), then
  c), and apply b) afterwards
  
*/

int structReconnect(struct DBM_STRUCT_NODE *node)
{
  int i,mc,cc,rc,bc;
  register int ac1, ac2,id1,id2;
  int m1;
  struct STRUCT_MODEL *cmp;
  struct STRUCT_CHAIN *ccp;
  struct STRUCT_RESIDUE *crp;
  struct STRUCT_ATOM *ap1,*ap2,*prev;
  struct STRUCT_BOND *nb;
  struct CONN_ENTRY *centry;
  int b_max;
  int abc1[3],abc2[3];
  caPointer *ca_p;
  int ca_c;

  b_max=node->atom_count*2; // empirical
  nb=(struct STRUCT_BOND*)Crecalloc(NULL,b_max,sizeof(struct STRUCT_BOND));
  bc=0;

  // reset connectivity
  for(i=0;i<node->atom_count;i++)
    node->atom[i].bondc=0;
  

  if(node->conn_flag)
    for(mc=0;mc<node->model_count;mc++) {
    cmp=&node->model[mc];
    for(cc=0;cc<cmp->chain_count;cc++) {
      ccp=cmp->chain[cc];
      prev=NULL;
      for(rc=0;rc<ccp->residue_count;rc++) {
	crp=ccp->residue[rc];
	// only check lookup table if requested by flag
	if(node->conn_flag & STRUCT_CONN_IMPL) {
	  centry=conGetEntry(crp->name);
	} else {
	  centry=NULL;
	}
	if(centry==NULL) {
	  if(node->conn_flag & STRUCT_CONN_DIST) {
	    /*
	      no connectivity information is available, do 
	      distance based search only if requested by flag
	    */
	    prev=NULL;
	    for(ac1=0;ac1<crp->atom_count;ac1++) {
	      // loop over all atoms in this RESIDUE ... 
	      ap1=crp->atom[ac1];
	      //... versus all atoms in this MODEL 
	      // check distance by using the cube array
	      caXYZtoABC(node->ca,(float *)ap1->p,abc1);
	      for(abc2[0]=abc1[0]-1;abc2[0]<=abc1[0]+1;abc2[0]++)
		for(abc2[1]=abc1[1]-1;abc2[1]<=abc1[1]+1;abc2[1]++)
		  for(abc2[2]=abc1[2]-1;abc2[2]<=abc1[2]+1;abc2[2]++) {
		    caGetList(node->ca,abc2,&ca_p,&ca_c);
		    for(ac2=0;ac2<ca_c;ac2++) {
		      ap2=(struct STRUCT_ATOM*)ca_p[ac2];
		      if(ap1->model==ap2->model)
			if(ap1->n!=ap2->n)
			  if(structIsConnected(ap1,ap2))
			    structConnectAtoms(node,&nb,&bc,&b_max,ap1,ap2);
		    }
		  }
	    }
	  }
	} else {
	  /* 
	     connectivity info is available for this residue, 
	     do connectivity table lookup
	  */

	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	    ap1=crp->atom[ac1];
	    id1=conGetAtomID(centry,ap1->name);
	    if(!id1) {
	      // no entry for this particular atom
	      if(node->conn_flag & STRUCT_CONN_DIST) {
		// do distance based check only if requested by flag
		m1=ap1->model->num;
		/* 
		   check against all other atoms in residue
		   this is questionable, maybe all other atoms ???
		*/
		for(ac2=0;ac2<crp->atom_count;ac2++) {
		  ap2=crp->atom[ac2];
		  /*
		    TODO double bonds
		    connect if THIS NEEDS ADJUSTMENT TO AVOID
		    DOUBLE ALLOCATED BONDS !
		    and distance is ok
		  */
		  if(ac1!=ac2)
		    if(structIsConnected(ap1,ap2))
		      structConnectAtoms(node,&nb,&bc,&b_max,ap1,ap2);
		}
	      }
	    } else { /* !id1 */
	      /* 
		 there was info for this atom
	      */
	      for(ac2=ac1+1;ac2<crp->atom_count;ac2++) {
		// loop over remaining atoms in residue
		ap2=crp->atom[ac2];
		id2=conGetAtomID(centry,ap2->name);
		/*
		  if other atom was also found
		  and is connected to first
		*/
		if(id2)
		  if(conIsConnected(centry,id1,id2))
		    structConnectAtoms(node,&nb,&bc,&b_max,ap1,ap2);
	      }
	      
	    }
	  }
	  id2=conGetPrevID(centry);
	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	      ap1=crp->atom[ac1];
	      if(id2==conGetAtomID(centry,ap1->name))
		if(prev!=NULL)
		  if(prev->residue->num+1==ap1->residue->num)
		    structConnectAtoms(node,&nb,&bc,&b_max,ap1,prev);
	  }
	  prev=NULL;
	  id2=conGetNextID(centry);
	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	      ap1=crp->atom[ac1];
	      if(id2==conGetAtomID(centry,ap1->name)) {
		prev=ap1;
		break;
	      }
	  }
	}
      }
    }
  }

  /* 
     find all bonds that have not been found,
     but that are defined by external connectivity
  */
  if(node->conn_flag && STRUCT_CONN_EXPL) {
    for(cc=0;cc<node->conn_count;cc++) {
      // is this one already defined ?
      for(i=0;i<bc;i++) {
	if(nb[i].atom1==node->conn[cc].ap1 && 
	   nb[i].atom2==node->conn[cc].ap2) {
	  break;
	} else if(nb[i].atom1==node->conn[cc].ap2 && 
		  nb[i].atom2==node->conn[cc].ap1) {
	  break;
	} 
      }
      // not yet defined
      if(i==bc) {
	structConnectAtoms(node,&nb,&bc,&b_max,
			   node->conn[cc].ap1,node->conn[cc].ap2);
      } else {
      }
      
    }
  }
  
  node->bond_count=bc;
  node->bond_max=b_max;
  node->bond=nb;

#ifdef WITH_NCBONDS
  structReconnectNC(node);
#endif

  structRecalcBonds(node);

  return 0;
}

#ifdef WITH_NCBONDS

/*
  non-covalent bonds
*/

int structReconnectNC(struct DBM_STRUCT_NODE *node)
{
  int ac,bond_count,bond_max;
  struct STRUCT_BOND *bond;
  struct STRUCT_ATOM *cap,*lap;
  float pos[3],dx,dy,dz;
  caPointer *cp;
  int cpc,lc;
  float co=4.0,co2;

  cp=Ccalloc(node->atom_count*2,sizeof(caPointer));

  co2=co*co;

  if(node->nbond_count>0)
    Cfree(node->nbond);

  node->nbond_count=0;
  bond_count=0;
  bond_max=1000;

  bond=Crecalloc(NULL,bond_max,sizeof(struct STRUCT_BOND));

  /*
    loop over all atoms
  */
  
  for(ac=0;ac<node->atom_count;ac++) {
    cap=&node->atom[ac];
    pos[0]=cap->p->x;
    pos[1]=cap->p->y;
    pos[2]=cap->p->z;
    caGetWithinList(node->ca, pos, co, &cp,&cpc);
    /*
      loop over all atoms within cutoff
    */
    for(lc=0;lc<cpc;lc++) {
      lap=(struct STRUCT_ATOM *)cp[lc];
      // only consider atoms with larger index
      if(lap->n>cap->n) {
	if(structCheckNCB(node,cap,lap,co)) {
	  bond[bond_count].atom1=cap;
	  bond[bond_count].prop1=&cap->def_prop;
	  bond[bond_count].atom2=lap;
	  bond[bond_count].prop1=&lap->def_prop;
	  bond_count++;
	  if(bond_count>=bond_max) {
	    bond_max+=1000;
	    bond=Crecalloc(bond,bond_max,sizeof(struct STRUCT_BOND));
	  }
	}
      }
    }
  }

  structRecalcBondList(bond,bond_count);

  node->nbond=bond;
  node->nbond_count=bond_count;

  Cfree(cp);

  return 0;
}
#endif

/*
  check if two atoms are in non covalent interaction
*/

int structCheckNCB(dbmStructNode *node, struct STRUCT_ATOM *a1, struct STRUCT_ATOM *a2, float co)
{
  int f1,f2,ret;
  float co2,dx,dy,dz;
  int bc,ac;
  float pos[3];
  struct STRUCT_ATOM *a3,*a4,*a5,*a6;
  float alimit=120.0;
  

  f1=a1->flag;
  f2=a2->flag;
  co2=co*co;

  ret=0;

  /*
    first criteria:
    donor and acceptor
  */

  // Hydrogen Bonds
  if((f1&STRUCT_HBA && f2&STRUCT_HBD) ||
     (f1&STRUCT_HBD && f2&STRUCT_HBA)) {

    /* 
       second criteria:
       distance
    */
    dx=a1->p->x-a2->p->x;
    if(dx*dx<co2) {
      dy=a1->p->y-a2->p->y;
      if(dy*dy<co2) {
	dz=a1->p->z-a2->p->z;
	if(dz*dz<co2) {
	  if(dx*dx+dy*dy+dz*dz<co2) {
	    /*
	      third criteria:
	      angle
	    */

	    
	    if(f1&STRUCT_HBA && f2&STRUCT_HBD) {
	      a3=a1;
	      a4=a2;
	    } else if (f1&STRUCT_HBD && f2&STRUCT_HBA) {
	      a4=a1;
	      a3=a2;
	    }
	    
	    ret=1;

	    // a3 is acceptor, a4 donor
	    
	    // check angle for acceptor
	    for(bc=0;bc<a3->bondc;bc++) {
	      if(node->bond[a3->bondi[bc]].atom1==a3) {
		a5=node->bond[a3->bondi[bc]].atom2;
	      } else {
		a5=node->bond[a3->bondi[bc]].atom1;
	      }

	      if(matfCalcAngle((float*)a3->p,(float*)a4->p,(float*)a3->p,(float*)a5->p)<alimit) {
		ret=0;
		break;
	      }
	    }

	    // check angle for donor
	    if(a4->bondc>0 && ret==1) {
	      pos[0]=0.0;
	      pos[1]=0.0;
	      pos[2]=0.0;
	      for(bc=0;bc<a4->bondc;bc++) {
		if(node->bond[a4->bondi[bc]].atom1==a4) {
		  a5=node->bond[a4->bondi[bc]].atom2;
		} else {
		  a5=node->bond[a4->bondi[bc]].atom1;
		}
		pos[0]+=(a5->p->x-a4->p->x);
		pos[1]+=(a5->p->y-a4->p->y);
		pos[2]+=(a5->p->z-a4->p->z);
	      }
	      matfNormalize(pos,pos);
	      
	      pos[0]=a4->p->x-pos[0];
	      pos[1]=a4->p->y-pos[1];
	      pos[2]=a4->p->z-pos[2];
	      
	      /*
		dummy_pos should now
		be roughly in pos
		of H atom
	      */
	      if(matfCalcAngle((float*)a4->p,pos,(float*)a3->p,pos)<alimit) {
		ret=0;
	      } 
	    }
	  }
	}
      }
    }
  }

  // Salt Bridges
  if((f1&STRUCT_SBA && f2&STRUCT_SBD) ||
     (f1&STRUCT_SBD && f2&STRUCT_SBA)) {
    
  }
  
  return ret;
}

int structRecalcBonds(dbmStructNode *node)
{
  return structRecalcBondList(node->bond,node->bond_count);
}

int structRecalcBondList(struct STRUCT_BOND *bond, int bond_count)
{
  double v1[3],v2[3],vdiff[3];
  double vcyl[]={0.0,0.0,1.0};
  double axis[3],mat[16];
  double length, dotproduct;
  double angle=0.0;
  int i,bc;

  for(bc=0;bc<bond_count;bc++){
    v1[0]=bond[bc].atom1->p->x;
    v1[1]=bond[bc].atom1->p->y;
    v1[2]=bond[bc].atom1->p->z;
    v2[0]=bond[bc].atom2->p->x;
    v2[1]=bond[bc].atom2->p->y;
    v2[2]=bond[bc].atom2->p->z;
    vdiff[0]=v2[0]-v1[0];
    vdiff[1]=v2[1]-v1[1];
    vdiff[2]=v2[2]-v1[2];

    length=sqrt(vdiff[0]*vdiff[0]+vdiff[1]*vdiff[1]+vdiff[2]*vdiff[2]);
    
    matCalcCross(vcyl,vdiff,axis);
    
    if(axis[0]==0.0 && axis[1]==0.0)
      if(vdiff[2]>=0)
        angle=0.0;
      else
        angle=180.0;
    else {
      dotproduct=matCalcDot(vcyl,vdiff);
      angle=180.0*acos(dotproduct/length)/M_PI;
    }
    bond[bc].axis[0]=axis[0];
    bond[bc].axis[1]=axis[1];
    bond[bc].axis[2]=axis[2];
    bond[bc].length=length;
    bond[bc].angle=angle;

    for(i=0;i<16;i++)
      mat[i]=bond[bc].rotmat[i];
    matMakeRotMat(angle, axis[0], axis[1], axis[2], mat);
    for(i=0;i<16;i++)
      bond[bc].rotmat[i]=mat[i];
  }
  return 0;
}

int structConnectAtoms(dbmStructNode *node, struct STRUCT_BOND **nb,int *bc, int *bm, struct STRUCT_ATOM *ap1, struct STRUCT_ATOM *ap2)
{
  int max_add=node->atom_count;
  struct STRUCT_BOND *ob;
  int i;

  if(ap1->bondc<ap1->bondm && ap2->bondc<ap2->bondm) {
    
    for(i=0;i<ap1->bondc;i++)
      if(((*nb)[ap1->bondi[i]].atom1==ap1 && (*nb)[ap1->bondi[i]].atom2==ap2) || ((*nb)[ap1->bondi[i]].atom1==ap2 && (*nb)[ap1->bondi[i]].atom2==ap1))
	return 0;
    for(i=0;i<ap2->bondc;i++)
      if(((*nb)[ap2->bondi[i]].atom1==ap1 && (*nb)[ap2->bondi[i]].atom2==ap2) || ((*nb)[ap2->bondi[i]].atom1==ap2 && (*nb)[ap2->bondi[i]].atom2==ap1))
	return 0;
	

    (*nb)[(*bc)].atom1=ap1;
    (*nb)[(*bc)].prop1=&ap1->def_prop;
    
    (*nb)[(*bc)].atom2=ap2;
    (*nb)[(*bc)].prop2=&ap2->def_prop;
    
    ap1->bondi[ap1->bondc++]=(*bc);
    ap2->bondi[ap2->bondc++]=(*bc);
    
    (*nb)[(*bc)].n=(*bc);
    (*bc)++;
    if((*bc)>=(*bm)) {
      (*nb)=(struct STRUCT_BOND*)Crecalloc((*nb),(*bm)+max_add,sizeof(struct STRUCT_BOND));
      (*bm)+=max_add;
    }
    return 0;
  } else {
    debmsg("ConnectAtoms: bond overflow");
    return -1;
  }
}


structObj *structNewObj(dbmStructNode *node, char *name)
{
  int i;
  structObj *no;
  
  structDelObj(node,name);
  for(i=0;i<node->obj_max;i++)
    if(node->obj_flag[i]==0) 
      break;
  if(i==node->obj_max) {
    node->obj_max*=2;
    node->obj=(structObj*)Crecalloc(node->obj,node->obj_max,sizeof(structObj));
  }
  
  node->obj_flag[i]=1;
  no=&node->obj[i];
  no->node=node;
  comNewObj(node->name,name);
  structSetDefault(no);
  clStrcpy(no->name,name);
  return no;
}

int structDelObj(struct DBM_STRUCT_NODE *node,char *name)
{
  int i;
  structObj *obj;
  for(i=0;i<node->obj_max;i++)
    if(node->obj_flag[i]==1)
      if(clStrcmp(node->obj[i].name,name)) {
	node->obj_flag[i]=0;
	obj=&node->obj[i];

	structObjDelete(obj);
	comDelObj(node->name, name);
      }

  return 0;
}


int structRenew(structObj *obj, char *set, char *sel)
{
  return 0;
}


int structSubCommand(dbmStructNode *node,char *rsub, int wc, const char **wl)
{
  char sub1[256],sub2[256];
  int anum;
  char model[256];
  char chain[256];
  char residue[256];
  char atom[256];
  char com[256];
  char message[256];
  char *p;
  int i,c;
  struct STRUCT_ATOM *cap;
  char *empty_com[]={"get","xyz"};

  if(wc==0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }

  if(structSubMatch(node,rsub,model,chain,residue,atom)!=0) {
    return -1;
  }
  c=0;
  for(i=0;i<node->atom_count;i++) {
    cap=&node->atom[i];
    sprintf(sub1,"%d",cap->residue->num);
    sprintf(sub2,"%d",cap->model->num);
    if(rex(model,sub2))
      if(rex(chain,cap->chain->name))
	if(rex(residue,sub1))
	  if(rex(atom,cap->name)){
	    /* MATCH */
	    /*
	      fprintf(stdout,"found atom #%d\n",cap->n);
	    */
	    c++;
	    if(clStrcmp(wl[0],"get")) {
	      return structSubComGet(node,cap,wc-1,wl+1);
	    } else {
	      sprintf(message,"%s: unknown command %s\n",node->name,wl[0]);
	      comMessage(message);
	      return -1;
	    }
	  }
    
  }

  if(c==0) {
    sprintf(message,"%s: not found: %s\n",node->name,rsub);
    comMessage(message);
    return -1;
  }
  return 0;
}

int structSubMatch(struct DBM_STRUCT_NODE *node, char *rsub, char *model, char *chain, char *residue, char *atom)
{
  char e[6][64];
  int anum;
  char message[256];
  char sub1[256],sub2[256];
  char *p;
  int i,c;

  for(i=0;i<6;i++) {
    strcpy(e[i],"*");
  }

  if(rsub[0]=='#') {
    anum=atoi(rsub+1);
    for(i=0;i<node->atom_count;i++)
      if(node->atom[i].n==anum) {
	if(node->model_flag)
	  sprintf(model,"%d",node->atom[i].model->num);
	else
	  strcpy(model,"*");

	if(node->chain_flag)
	  sprintf(chain,"%s",node->atom[i].chain->name);
	else
	  strcpy(chain,"*");

	if(node->residue_flag)
	  sprintf(residue,"%d",node->atom[i].residue->num);
	else
	  strcpy(residue,"*");

	strcpy(atom,node->atom[i].name);
	break;
      }
    if(i==node->atom_count) {
      sprintf(message,"atom %d not found\n",anum);
      comMessage(message);
      return -1;
    }

  } else {
    strcpy(sub1,rsub);
    for(i=0;i<6;i++) {
      p=strchr(sub1,'.');
      if(p==NULL) {
	strcpy(e[i],sub1);
	break;
      } else {
	p[0]='\0';
	strcpy(e[i],sub1);
	strcpy(sub2,p+1);
	strcpy(sub1,sub2);
      }
    }
    c=i+1;
    i=0;
    if(node->model_flag)
      strcpy(model,e[i++]);
    else
      strcpy(model,"*");
    
    if(node->chain_flag)
      strcpy(chain,e[i++]);
    else
      strcpy(chain,"*");
    
    if(node->residue_flag)
      strcpy(residue,e[i++]);
    else
      strcpy(residue,"*");
    
    strcpy(atom,e[i++]);
    
    if(c-i>0) {
      comMessage("parse error in subindexing\n");
      return -1;
    }
  }
  c=0;
  return 0;
}

int structSubGetNum(struct DBM_STRUCT_NODE *node, char *model, char *chain, char *residue, char *atom)
{
  struct STRUCT_ATOM *cap;
  char sub1[256],sub2[256];
  int i;

  for(i=0;i<node->atom_count;i++) {
    cap=&node->atom[i];
    sprintf(sub1,"%d",cap->residue->num);
    sprintf(sub2,"%d",cap->model->num);
    if(rex(model,sub2))
      if(rex(chain,cap->chain->name))
	if(rex(residue,sub1))
	  if(rex(atom,cap->name)){
	    return i;
	  }
  }
  return -1;
}


int structSubComGet(dbmStructNode *node,struct STRUCT_ATOM *ap,int wc, const char **wl)
{
  float v[3];
  char message[256];
  if(wc<=0) {
    sprintf(message,"%s: missing parameter for get\n",node->name);
    comMessage(message);
    return -1;
  }
  if(clStrcmp(wl[0],"xyz")) {
    v[0]=ap->p->x;
    v[1]=ap->p->y;
    v[2]=ap->p->z;
    transApplyf(&node->transform,v);
    sprintf(message,"{%.3f,%.3f,%.3f}",v[0],v[1],v[2]);
  } else if(clStrcmp(wl[0],"bfac")) {
    sprintf(message,"%.3f",ap->bfac);
  } else if(clStrcmp(wl[0],"weight") ||
	    clStrcmp(wl[0],"occ")) {
    sprintf(message,"%.3f",ap->weight);
  } else if(clStrcmp(wl[0],"rname")) {
    sprintf(message,"%s",ap->residue->name);
  } else if(clStrcmp(wl[0],"rtype")) {
    switch(ap->residue->type) {
    case STRUCT_RTYPE_HELIX: sprintf(message,"%s","helix"); break;
    case STRUCT_RTYPE_STRAND: sprintf(message,"%s","strand"); break;
    default: sprintf(message,"%s","coil"); break;
    }
  } else if(clStrcmp(wl[0],"aname")) {
    sprintf(message,"%s",ap->name);
  } else if(clStrcmp(wl[0],"ele")) {
    sprintf(message,"%s",ap->chem.element);
  } else if(clStrcmp(wl[0],"anum")) {
    sprintf(message,"%d",ap->anum);
  } else if(clStrcmp(wl[0],"chain")) {
    sprintf(message,"%s",ap->chain->name);
  } else if(clStrcmp(wl[0],"model")) {
    sprintf(message,"%d",ap->model->num);
  } else if(clStrcmp(wl[0],"rnum")) {
    sprintf(message,"%d",ap->residue->num);
  } else {
    sprintf(message,"%s:SubGet: unknown parameter %s\n",node->name,wl[0]);
    comMessage(message);
    return -1;
  }
  comReturn(message);
  return 0;
}

#define NEW_CUTOFF

int structIsConnected(struct STRUCT_ATOM *a1, struct STRUCT_ATOM *a2)
{
  register double dx,dy,dz,d;
  double cutoff;
  double hcutoff;

#ifdef NEW_CUTOFF
  cutoff=a1->chem.vdwr+a2->chem.vdwr;
  cutoff/=2.0;
  // allow a 10% error
  cutoff*=1.1;
  cutoff*=cutoff;
#else
  cutoff=2.1*2.1;
  hcutoff=1.2*1.2;
#endif

  dx=a1->p->x-a2->p->x;
  if(dx>cutoff)
    return 0;
  dy=a1->p->y-a2->p->y;
  if(dy>cutoff)
    return 0;
  dz=a1->p->z-a2->p->z;
  if(dz>cutoff)
    return 0;

  d=dx*dx+dy*dy+dz*dz;

  if(d<=cutoff)
    if(d>0.0)
#ifndef NEW_CUTOFF
      if(a1->chem.element[0]=='H' || a2->chem.element[0]=='H') {
	if(a1->chem.element[0]=='H' && a2->chem.element[0]=='H') {
	  return 0;
	} else {
	  if(d<=hcutoff)
	    return 1;
	  else
	    return 0;
	}
      } else {
	return 1;
      }
#else
      return 1;
#endif
  return 0;
}


int structIsAtomSelected(dbmStructNode *node, struct STRUCT_ATOM *atom, Select *sel)
{
  int i,ec,r;

  if(atom->restriction)
    return 0;

  if(sel==NULL)
    return 1;

  
  ec=selectGetPOVCount(sel);

  for(i=0;i<ec;i++) {
    r=structEvalAtomPOV(node,atom,selectGetPOV(sel, i));
    if(r<0)
      return -1;
    selectSetResult(sel, i, r);
  }  

  return selectResult(sel);
}


static int structCheckFloatProp(float prop, int op, int rf, float v1, float v2)
{
  if(rf) {
    return (prop>=v1 && prop<=v2);
  } else {
    switch(op) {
    case POV_OP_EQ: return prop==v1;
    case POV_OP_NE: return prop!=v1;
    case POV_OP_LT: return prop<v1;
    case POV_OP_LE: return prop<=v1;
    case POV_OP_GT: return prop>v1;
    case POV_OP_GE: return prop>=v1;
    }
  }
  return 0;
}

static int structCheckIntProp(int prop, int op, int rf, int v1, int v2)
{
  if(rf) {
    return (prop>=v1 && prop<=v2);
  } else {
    switch(op) {
    case POV_OP_EQ: return prop==v1;
    case POV_OP_NE: return prop!=v1;
    case POV_OP_LT: return prop<v1;
    case POV_OP_LE: return prop<=v1;
    case POV_OP_GT: return prop>v1;
    case POV_OP_GE: return prop>=v1;
    }
  }
  return 0;
}

static int structCheckStringProp(const char* prop, int op, int rf, const char* v1, const char* v2,const char* name)
{
  static const char message[128];

  if(rf) {
    sprintf(message,"error: range not allowed for string property '%s'\n",name);
    comMessage(message);
    return -1;
  }

  if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
    sprintf(message,"error: expected operator = or != for string property '%s'\n",name);
    comMessage(message);
    return -1;
  }

  if(rex(v1,prop))
    return 1;
  return 0;
}

int structEvalAtomPOV(dbmStructNode *node, struct STRUCT_ATOM *atom,POV *pov)
{
  int i,j,vc,f,ret;
  int prop,rf,op;
  const char *v1,*v2;
  float v1_f,v2_f;
  int v1_i,v2_i;
  struct POV_VALUE *val;
  char message[256];
  float pos[3],dx,dy,dz,dist,dist2;

  // needed for cross-selection
  if(atom->restriction)
    return 0;

  if(clStrcmp(pov->prop,"*"))
    return 1;

  if(pov->op==POV_OP_WI)
    prop=STRUCT_SEL_WITHIN;
  else if(pov->op==POV_OP_OBJ)
    prop=STRUCT_SEL_OBJECT;
  else if(clStrcmp(pov->prop,"anum"))
    prop=STRUCT_SEL_ANUM;
  else if(clStrcmp(pov->prop,"aname"))
    prop=STRUCT_SEL_ANAME;
  else if(clStrcmp(pov->prop,"rnum"))
    prop=STRUCT_SEL_RNUM;
  else if(clStrcmp(pov->prop,"rtype"))
    prop=STRUCT_SEL_RTYPE;
  else if(clStrcmp(pov->prop,"rname"))
    prop=STRUCT_SEL_RNAME;
  else if(clStrcmp(pov->prop,"class"))
    prop=STRUCT_SEL_CLASS;
  else if(clStrcmp(pov->prop,"chain"))
    prop=STRUCT_SEL_CHAIN;
  else if(clStrcmp(pov->prop,"model"))
    prop=STRUCT_SEL_MODEL;
  else if(clStrcmp(pov->prop,"occ") ||
	  clStrcmp(pov->prop,"weight"))
    prop=STRUCT_SEL_OCC;
  else if(clStrcmp(pov->prop,"bfac"))
    prop=STRUCT_SEL_BFAC;
  else if(clStrcmp(pov->prop,"x"))
    prop=STRUCT_SEL_XC;
  else if(clStrcmp(pov->prop,"y"))
    prop=STRUCT_SEL_YC;
  else if(clStrcmp(pov->prop,"z"))
    prop=STRUCT_SEL_ZC;
  else if(clStrcmp(pov->prop,"ele"))
    prop=STRUCT_SEL_ELE;
  else {
    sprintf(message,"error: %s: unknown atom property %s\n",node->name, pov->prop);
    comMessage(message);
    return -1;
  }

  op=pov->op;

  if(!(op==POV_OP_EQ || op==POV_OP_LT || op==POV_OP_LE ||
       op==POV_OP_GT || op==POV_OP_GE || op==POV_OP_NE ||
       op==POV_OP_WI || op==POV_OP_OBJ)) {
    sprintf(message,"error: invalid operator\n");
    comMessage(message);
    return -1;
  }

  if(pov->op==POV_OP_WI) {
    dist=atof(pov->prop);
    dist2=dist*dist;
  }

  vc=pov->val_count;
  ret=0;
  for(i=0;i<vc;i++) {
    val=povGetVal(pov,i);
    if(val->range_flag) {
      if(op!=POV_OP_EQ) {
	sprintf(message,"error: %s: expected operator = for range\n",node->name);
	comMessage(message);
	return -1;
      }
      rf=1;
      v1=val->val1;
      v2=val->val2;
    } else {
      rf=0;
      v1=val->val1;
      v2=val->val1;
    }
    
    if(clStrcmp(v1,"*") || clStrcmp(v2,"*"))
      return 1;

    v1_f=atof(v1);
    v1_i=atoi(v1);
    if(rf) {
      v2_f=atof(v2);
      v2_i=atoi(v2);
    }

    switch(prop) {
    case STRUCT_SEL_ANUM: ret=structCheckIntProp(atom->anum,op,rf,v1_i,v2_i); break;

    case STRUCT_SEL_ANAME: ret=structCheckStringProp(atom->name,op,rf,v1,v2,"aname"); break;

    case STRUCT_SEL_RNUM: ret=structCheckIntProp(atom->residue->num,op,rf,v1_i,v2_i); break;

    case STRUCT_SEL_RNAME: ret=structCheckStringProp(atom->residue->name,op,rf,v1,v2,"rname"); break;
    case STRUCT_SEL_CHAIN: ret=structCheckStringProp(atom->chain->name,op,rf,v1,v2,"chain"); break;

    case STRUCT_SEL_MODEL: ret=structCheckIntProp(atom->model->num,op,rf,v1_i,v2_i); break;

    case STRUCT_SEL_OCC: ret=structCheckFloatProp(atom->weight,op,rf,v1_f,v2_f); break;

    case STRUCT_SEL_BFAC: ret=structCheckFloatProp(atom->bfac,op,rf,v1_f,v2_f); break;

    case STRUCT_SEL_XC: ret=structCheckFloatProp(atom->p->x,op,rf,v1_f,v2_f); break;

    case STRUCT_SEL_YC: ret=structCheckFloatProp(atom->p->y,op,rf,v1_f,v2_f); break;

    case STRUCT_SEL_ZC: ret=structCheckFloatProp(atom->p->z,op,rf,v1_f,v2_f); break;

    case STRUCT_SEL_ELE: ret=structCheckStringProp(atom->chem.element,op,rf,v1,v2,"ele"); break;

    case STRUCT_SEL_RTYPE: // special
      if(rf) {
	comMessage("error: range not allowed for rtype\n");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("error: expected operator = or != for rtype\n");
	return -1;
      }
      switch(atom->residue->type) {
      case STRUCT_RTYPE_HELIX: if(rex(v1,"helix")) return 1; break;
      case STRUCT_RTYPE_STRAND: if(rex(v1,"strand")) return 1; break;
      case STRUCT_RTYPE_COIL: if(rex(v1,"coil")) return 1; break;
      }
      break;
    case STRUCT_SEL_CLASS: // special
      if(rf) {
	comMessage("error: range not allowed for class\n");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("error: expected operator = or != for class\n");
	return -1;
      }
      switch(atom->residue->clss) {
      case STRUCT_PROTEIN: if(rex(v1,"protein")) return 1; break;
      case STRUCT_NA: if(rex(v1,"na")) return 1; break;
      case STRUCT_MISC: if(rex(v1,"misc")) return 1; break;
      }
      break;
    case STRUCT_SEL_OBJECT:
      if(rf) {
	comMessage("error: range not supported for .object selection\n");
	return -1;
      }
      // identify object
      if(v1[0]!='.') {
	comMessage("error: expected object to start with .\n");
	return -1;
      }
      v1++;
      v2=clStrchr(v1,'.');
      if(v2!=NULL) {
	comMessage("error: .dataset.object selection not allowed\n");
	return -1;
      }
      f=0;
      for(j=0;j<node->obj_max;j++) {
	if(node->obj_flag[j]) {
	  if(rex(v1,node->obj[j].name)) {
	    f++;
	    if(node->obj[j].atom_flag[atom->n])
	      return 1;
	  }
	}
      }
      if(!f) {
	comMessage("error: object ");
	comMessage(v1);
	comMessage(" not found");
	comMessage("\n");
	return -1;
      }
      break;
    case STRUCT_SEL_WITHIN:
      if(rf) {
	comMessage("error: range not supported for < >\n");
	return -1;
      }
      if(val->wi_flag) {
	pos[0]=atom->p->x;
	pos[1]=atom->p->y;
	pos[2]=atom->p->z;
	if(v1==NULL) {
	  ret=dbmIsWithin(pos,dist2, node->name, val->val2);
	  if(ret<0)
	    return -1;
	  if(ret>0)
	    return 1;
	} else {
	  ret=dbmIsWithin(pos,dist2, val->val1,val->val2);
	  if(ret<0)
	    return -1;
	  if(ret>0)
	    return 1;
	}
      } else {
	dx=atom->p->x-val->vect[0];
	dy=atom->p->y-val->vect[1];
	dz=atom->p->z-val->vect[2];
	if((dx*dx+dy*dy+dz*dz)<dist2)
	  return 1;
      }
      break;
    }
    if(ret!=0) return ret;
  }

  return 0;
}

#ifdef PICK_NEW
int structPick(dbmStructNode *node, double *p1, double *p2, double cutoff_d, dbmPickList *pl)
{
  double sx,sy,sz;
  double v1x,v1y,v1z;
  double vx,vy,vz;
  double dx,dy,dz,dist;
  double u,t1,t2,t3;
  double px,py,pz;
  int ac,bc,oc;
  struct STRUCT_OBJ *obj;
  double cur_d,cur_d2;
  char name[256],id[256];
  // SET TO ZERO FOR BOND PICKING
  int atom_hit=1;

  /* The formula is:

       px*vx+py*vy+pz*vz-(v1x*vx+v1y*vy+v1z*vz)
     u=----------------------------------------
                 vx^2+vy^2+vz^2
                   
     but I prefer to split it in three terms: t1, t2 and t3 (see below)
  */

  v1x=p1[0];
  v1y=p1[1];
  v1z=p1[2];
  vx=p2[0]-v1x;
  vy=p2[1]-v1y;
  vz=p2[2]-v1z;

  t2=v1x*vx+v1y*vy+v1z*vz;
  t3=vx*vx+vy*vy+vz*vz;

  if(t3==0.0)
    return 0;

  cur_d=cutoff_d;
  cur_d2=cur_d;
  
  for(oc=0;oc<node->obj_max;oc++)
    if(node->obj_flag[oc]!=0)
      if(node->obj[oc].render.show) {
	obj=&node->obj[oc];
	for(ac=0;ac<obj->atom_count;ac++){
	  px=obj->atom[ac].ap->p->x;
	  py=obj->atom[ac].ap->p->y;
	  pz=obj->atom[ac].ap->p->z;
	  
	  t1=px*vx+py*vy+pz*vz;
	  
	  u=(t1-t2)/t3;
	  
	  sx=v1x+u*vx;
	  sy=v1y+u*vy;
	  sz=v1z+u*vz;
	  
	  dx=sx-px;
	  dy=sy-py;
	  dz=sz-pz;
	  
	  dist=(dx*dx+dy*dy+dz*dz);
	  
	  if(dist<cur_d) {
	    sprintf(name,".%s:",node->name);
	    if(node->model_flag)
	      sprintf(name,"%s %d",name,obj->atom[ac].ap->model->num);
	    if(node->chain_flag)
	      sprintf(name,"%s %s",name,obj->atom[ac].ap->chain->name);
	    sprintf(name,"%s %s%d",name,
		    obj->atom[ac].ap->residue->name,
		    obj->atom[ac].ap->residue->num);
	    sprintf(name,"%s %s",name,obj->atom[ac].ap->name);
	    sprintf(id,".%s:#%d",node->name,obj->atom[ac].ap->n);
	    dbmPickAdd(pl,px,py,pz,name,id);
	    atom_hit++;
	  }
	}
      }
  if(!atom_hit)
    for(oc=0;oc<node->obj_max;oc++)
      if(node->obj_flag[oc]!=0)
	if(node->obj[oc].render.show) {
	  obj=&node->obj[oc];
	  for(bc=0;bc<obj->bond_count;bc++) {
	    px=0.5*(obj->bond[bc].atom1->p->x+obj->bond[bc].atom2->p->x);
	    py=0.5*(obj->bond[bc].atom1->p->y+obj->bond[bc].atom2->p->y);
	    pz=0.5*(obj->bond[bc].atom1->p->z+obj->bond[bc].atom2->p->z);
	    
	    t1=px*vx+py*vy+pz*vz;
	    
	    u=(t1-t2)/t3;
	    
	    sx=v1x+u*vx;
	    sy=v1y+u*vy;
	    sz=v1z+u*vz;
	    
	    dx=sx-px;
	    dy=sy-py;
	    dz=sz-pz;
	    
	    dist=(dx*dx+dy*dy+dz*dz);
	    
	    if(dist<cur_d2) {
	      sprintf(name,".%s: [",node->name);
	      if(node->model_flag)
		sprintf(name,"%s %d",name,obj->bond[bc].atom1->model->num);
	      if(node->chain_flag)
		sprintf(name,"%s %s",name,obj->bond[bc].atom1->chain->name);
	      sprintf(name,"%s %s%d",name,
		      obj->bond[bc].atom1->residue->name,
		      obj->bond[bc].atom1->residue->num);
	      sprintf(name,"%s %s",name,obj->bond[bc].atom1->name);
	      strcat(name,"] - [");
	      if(node->model_flag)
		sprintf(name,"%s %d",name,obj->bond[bc].atom2->model->num);
	      if(node->chain_flag)
		sprintf(name,"%s %s",name,obj->bond[bc].atom2->chain->name);
	      sprintf(name,"%s %s%d",name,
		      obj->bond[bc].atom2->residue->name,
		      obj->bond[bc].atom2->residue->num);
	      sprintf(name,"%s %s",name,obj->bond[bc].atom2->name);
	      strcat(name,"]");
	      
	      sprintf(id,".%s:%%%d",node->name,obj->bond[bc].n);
	      
	      dbmPickAdd(pl,px,py,pz,name,id);
	    }
	  }
	}
  
  
  return 0;
}

#else

struct STRUCT_ATOM ** structPick(struct DBM_STRUCT_NODE *node, double *p1, double *p2)
{
  double sx,sy,sz;
  double v1x,v1y,v1z;
  double vx,vy,vz;
  double dx,dy,dz,dist;
  double u,t1,t2,t3;
  double px,py,pz;
  int ac,oc;
  struct STRUCT_ATOM *cur_atom;
  struct STRUCT_ATOM **atom_list,**ol;
  struct STRUCT_OBJ *obj;
  int atom_listc,atom_listm;
  double cur_d,cur_u;

  atom_listm=100;
  atom_list=(struct STRUCT_ATOM **)Ccalloc(atom_listm, sizeof(struct STRUCT_ATOM*));
  atom_listc=0;

  /* p1 and p2 should be multiplied with the struct mmat */
  /*
  matMultMV(p1o,obj->node->rmat,p2o);
  */
  
  /* The formula is:

       px*vx+py*vy+pz*vz-(v1x*vx+v1y*vy+v1z*vz)
     u=----------------------------------------
                 vx^2+vy^2+vz^2
                   
     but I prefer to split it in three terms: t1, t2 and t3 (see below)
  */

  v1x=p1[0];
  v1y=p1[1];
  v1z=p1[2];
  vx=p2[0]-v1x;
  vy=p2[1]-v1y;
  vz=p2[2]-v1z;

  t2=v1x*vx+v1y*vy+v1z*vz;
  t3=vx*vx+vy*vy+vz*vz;

  if(t3==0.0)
    return NULL;

  cur_atom=NULL;
  cur_d=0.2;
  cur_u=1.0e8;

  
  for(oc=0;oc<node->obj_max;oc++)
    if(node->obj_flag[oc]!=0)
      if(node->obj[oc].render.show) {
	obj=&node->obj[oc];
	for(ac=0;ac<obj->atom_count;ac++){
	  px=obj->atom[ac].ap->p->x;
	  py=obj->atom[ac].ap->p->y;
	  pz=obj->atom[ac].ap->p->z;
	  
	  t1=px*vx+py*vy+pz*vz;
	  
	  u=(t1-t2)/t3;
	  
	  sx=v1x+u*vx;
	  sy=v1y+u*vy;
	  sz=v1z+u*vz;
	  
	  dx=sx-px;
	  dy=sy-py;
	  dz=sz-pz;
	  
	  dist=(dx*dx+dy*dy+dz*dz);
	  
	  if(dist<cur_d) {
	    atom_list[atom_listc++]=obj->atom[ac].ap;
	    if(atom_listc>=atom_listm) {
	      ol=atom_list;
	      atom_list=(struct STRUCT_ATOM **)Ccalloc(atom_listm+100, sizeof(struct STRUCT_ATOM*));
	      memcpy(atom_list,ol,atom_listm*sizeof(struct STRUCT_ATOM));
	      atom_listm+=100;
	      Cfree(ol);
	    }
	  }
	}
      }
  atom_list[atom_listc]=NULL;
  return atom_list;
}

#endif

float structGetAtomProperty(struct STRUCT_ATOM *ap,const char *prop)
{
  if(clStrcmp(prop,"anum"))
    return (float)ap->anum;
  else if(clStrcmp(prop,"bfac"))
    return ap->bfac;
  else if(clStrcmp(prop,"weight") ||
	  clStrcmp(prop,"occ"))
    return ap->weight;
  else if(clStrcmp(prop,"rnum"))
    return (float)ap->residue->num;
  else if(clStrcmp(prop,"model"))
    return (float)ap->model->num;
  else
    return 0.0;
}

int structSetDefault(structObj *obj)
{
  float p[]={0.0,0.0,0.0};
  obj->type=STRUCT_CONNECT;
  strcpy(obj->name,"");
  obj->r=-1.0;
  obj->g=-1.0;
  obj->b=-1.0;
  obj->model=NULL;
  obj->model_count=0;
  obj->chain=NULL;
  obj->chain_count=0;
  obj->residue=NULL;
  obj->residue_count=0;
  obj->atom=NULL;
  obj->atom_count=0;
  obj->atom_flag=(unsigned char*)Ccalloc(obj->node->atom_count,sizeof(unsigned char));
  obj->bond=NULL;
  obj->bond_count=0;
  obj->s_bond=NULL;
  obj->s_bond_count=0;
  obj->node=NULL;
  obj->transform_list.count=0;
  obj->transform_list.max=0;
  obj->symview=0;
  obj->symcount=1;
  /*********
  obj->tv=NULL;
  obj->tvm=0;
  obj->tvc=0;
  *********/

  obj->render.show=1;
  obj->render.mode=RENDER_CUSTOM;
  obj->render.detail1=3;
  obj->render.detail2=6;
  obj->render.nice=1;
  obj->render.line_width=1.0;
  obj->render.bond_width=0.2;
  obj->render.tube_ratio=1.0;
  obj->render.tube_width=0.4;
  obj->render.sphere_radius=0.2;
  obj->render.helix_width=1.0;
  obj->render.helix_thickness=0.3;
  obj->render.strand_width=1.2;
  obj->render.strand_thickness=0.3;
  obj->render.arrow_thickness=0.5;
  obj->render.sugar_thickness=0.5;
  obj->render.base_thickness=0.5;

  obj->render.strand_method=0;
  obj->render.helix_method=0;
  obj->render.na_method=0;
    
  obj->render.transparency=1.0;
  obj->render.cgfx_flag=CGFX_INTPOL_COL | CGFX_HSC_CAP;

  comSetDefMat(&obj->render.mat);
  obj->render.mat.spec[0]=0.3;
  obj->render.mat.spec[1]=0.3;
  obj->render.mat.spec[2]=0.3;
  obj->render.mat.spec[3]=1.0;
  obj->render.mat.shin=32;

  obj->render.stipple_flag=0;
  obj->render.stipplei=0.7;
  obj->render.stippleo=0.3;

  obj->va.p=NULL;

  obj->sphere_list=comGenLists(1);
  comNewDisplayList(obj->sphere_list);
  cgfxSphere(1.0,obj->render.detail1);
  comEndDisplayList();

  obj->build=NULL;

  obj->va_list=comGenLists(1);
  obj->va_list_flag=0;

  return 0;
}


int structSetMinMax(struct DBM_STRUCT_NODE *node)
{
  int i;
  float x1,y1,z1,x2,y2,z2;

  x1=0;y1=0;z1=0;
  for(i=0;i<node->atom_count;i++) {
    x1+=node->atom[i].p->x;
    y1+=node->atom[i].p->y;
    z1+=node->atom[i].p->z;
  }
  if(node->atom_count>0) {
    x2=x1/(double)node->atom_count;
    y2=y1/(double)node->atom_count;
    z2=z1/(double)node->atom_count;
  } else {
    x2=0.0;
    y2=0.0;
    z2=0.0;
  }

  if(node->atom_count<1)
    return -1;

  node->min_max.anum1=node->atom[0].anum;
  node->min_max.anum2=node->atom[0].anum;
  node->min_max.rnum1=node->atom[0].residue->num;
  node->min_max.rnum2=node->atom[0].residue->num;
  node->min_max.model1=node->atom[0].model->num;
  node->min_max.model2=node->atom[0].model->num;
  node->min_max.bfac1=node->atom[0].bfac;
  node->min_max.bfac2=node->atom[0].bfac;
  node->min_max.weight1=node->atom[0].weight;
  node->min_max.weight2=node->atom[0].weight;

  for(i=1;i<node->atom_count;i++) {
    if(node->atom[i].anum<node->min_max.anum1)
      node->min_max.anum1=node->atom[i].anum;
    if(node->atom[i].anum>node->min_max.anum2)
      node->min_max.anum2=node->atom[i].anum;
    
    if(node->atom[i].residue->num<node->min_max.rnum1)
      node->min_max.rnum1=node->atom[i].residue->num;
    if(node->atom[i].residue->num>node->min_max.rnum2)
      node->min_max.rnum2=node->atom[i].residue->num;

    if(node->atom[i].model->num<node->min_max.model1)
      node->min_max.model1=node->atom[i].model->num;
    if(node->atom[i].model->num>node->min_max.model2)
      node->min_max.model2=node->atom[i].model->num;

    if(node->atom[i].bfac<node->min_max.bfac1)
      node->min_max.bfac1=node->atom[i].bfac;
    if(node->atom[i].bfac>node->min_max.bfac2)
      node->min_max.bfac2=node->atom[i].bfac;

    if(node->atom[i].weight<node->min_max.weight1)
      node->min_max.weight1=node->atom[i].weight;
    if(node->atom[i].weight>node->min_max.weight2)
      node->min_max.weight2=node->atom[i].weight;

  }

  return 0;
}

int structGetMinMax(dbmStructNode *n, const char *prop, float *vmin, float *vmax)
{
  if(prop==NULL) {
    (*vmin)=0.0;
    (*vmax)=0.0;
    return -1;
  }
  if(clStrcmp(prop,"anum")) {
    (*vmin)=n->min_max.anum1;
    (*vmax)=n->min_max.anum2;
  } else if(clStrcmp(prop,"rnum")) {
    (*vmin)=n->min_max.rnum1;
    (*vmax)=n->min_max.rnum2;
  } else if(clStrcmp(prop,"model")) {
    (*vmin)=n->min_max.model1;
    (*vmax)=n->min_max.model2;
  } else if(clStrcmp(prop,"weight") ||
	    clStrcmp(prop,"occ")) {
    (*vmin)=n->min_max.weight1;
    (*vmax)=n->min_max.weight2;
  } else if(clStrcmp(prop,"bfac")) {
    (*vmin)=n->min_max.bfac1;
    (*vmax)=n->min_max.bfac2;
  } else if(clStrcmp(prop,"dist")) {
    // TODO
    (*vmin)=0.0;
    (*vmax)=0.0;
  } else if(clStrcmp(prop,"x")) {
    // TODO
    (*vmin)=0.0;
    (*vmax)=0.0;
  } else if(clStrcmp(prop,"y")) {
    // TODO
    (*vmin)=0.0;
    (*vmax)=0.0;
  } else if(clStrcmp(prop,"z")) {
    // TODO
    (*vmin)=0.0;
    (*vmax)=0.0;
  } else {
    (*vmin)=0.0;
    (*vmax)=0.0;
    return -1;
  }
  return 0;
}


int structBuildCA(dbmStructNode *n)
{
  int i,c,mode;
  float xmin,xmax,ymin,ymax,zmin,zmax;
  float xyz1[3],xyz2[3],xyz[3];
  int abc[3];

  if(n->atom_count<1) {
    xyz1[0]=0;
    xyz1[1]=0;
    xyz1[2]=0;
    xyz2[0]=1;
    xyz2[1]=1;
    xyz2[2]=1;
    
    n->ca=caInit(xyz1,xyz2,0,5.0);

  } else {

    xmin=n->atom[0].p->x;
    xmax=n->atom[0].p->x;
    ymin=n->atom[0].p->y;
    ymax=n->atom[0].p->y;
    zmin=n->atom[0].p->z;
    zmax=n->atom[0].p->z;
    
    for(i=1;i<n->atom_count;i++) {
      xmin = (n->atom[i].p->x < xmin) ? n->atom[i].p->x : xmin;
      xmax = (n->atom[i].p->x > xmax) ? n->atom[i].p->x : xmax;
      ymin = (n->atom[i].p->y < ymin) ? n->atom[i].p->y : ymin;
      ymax = (n->atom[i].p->y > ymax) ? n->atom[i].p->y : ymax;
      zmin = (n->atom[i].p->z < zmin) ? n->atom[i].p->z : zmin;
      zmax = (n->atom[i].p->z > zmax) ? n->atom[i].p->z : zmax;
    }
    
    xyz1[0]=xmin;
    xyz1[1]=ymin;
    xyz1[2]=zmin;
    xyz2[0]=xmax;
    xyz2[1]=ymax;
    xyz2[2]=zmax;
    
    n->ca=caInit(xyz1,xyz2,0,5.0);
    
    for(c=0;c<2;c++) {
      if(c==0) {
	mode=CA_ADD;
      } else {
	caFix(n->ca);
	mode=CA_WRITE;
      }
      for(i=0;i<n->atom_count;i++) {
	xyz[0]=n->atom[i].p->x;
	xyz[1]=n->atom[i].p->y;
	xyz[2]=n->atom[i].p->z;
	caXYZtoABC(n->ca,xyz,abc);
	caAddPointer(n->ca,abc,(caPointer)&n->atom[i],mode);
      }
    }
  }
  return 0;
}

int structComLoad(struct DBM_STRUCT_NODE *node, int wc, char **wl)
{
  int i;
  char filename[256],message[256];
  char name[64];
  char base[256],*bp;
  char ext[256];
  char type[256];
  int cmp,id,ret;
  char gunzip[256];
  FILE *f;
  int swap_flag;

  strcpy(filename,wl[0]);

  strcpy(type,"");

  if((bp=strrchr(filename,'/'))!=NULL)
    strcpy(base,bp+1);
  else
    strcpy(base,filename);
  
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
  for(i=1;i<wc;i++) {
    if(clStrcmp(wl[i],"-type") ||
       clStrcmp(wl[i],"-t")) {
      if(i+1>=wc) {
	sprintf(message,"missing parameter for -type\n");
	comMessage(message);
	return -1;
      }
      strcpy(type,wl[++i]);
    } else if(clStrcmp(wl[i],"-swap")) {
      swap_flag=1;
    } else {
      sprintf(message,"unknow option %s\n",wl[i]);
      comMessage(message);
      return -1;
    }
  }

  if(clStrcmp(ext,"gz")) {
    cmp=1;
    bp=strrchr(base,'.');
    if(bp!=NULL)
      bp[0]='\0';
    strcpy(ext,bp+1);
    sprintf(gunzip,"gunzip < %s",filename);
    if((f=popen(gunzip,"r"))==NULL) {
      sprintf(message,"Error while piping %s\n",filename);
      comMessage(message);
      return -1;
    }
  } else {
    cmp=0;
    if((f=fopen(filename,"r"))==NULL) {
      sprintf(message,"Error opening %s\n",filename);
      comMessage(message);
      return -1;
    }
  }

  if(strlen(type)==0) {
    if(clStrcmp(ext,"trj") ||
       clStrcmp(ext,"dcd")) {
      id=STRUCT_TRJ_CHARMM; /* default */
    } else if(clStrcmp(ext,"crd")) {
      id=STRUCT_TRJ_CNS; /* default */
    } else if(clStrcmp(ext,"dtrj")) {
      id=STRUCT_TRJ_DINO;
    } else if(clStrcmp(ext,"binpos")) {
      id=STRUCT_TRJ_BINPOS;
    } else if(clStrcmp(ext,"xtc")) {
      id=STRUCT_TRJ_XTC;
    } else {
      sprintf(message,"unknown extension %s, please specify type\n",ext);
      comMessage(message);
      if(cmp) pclose(f); else fclose(f);
      return -1;
    }
  } else if(clStrcmp(type,"charmm")) {
    id=STRUCT_TRJ_CHARMM;
  } else if(clStrcmp(type,"xplor")) {
    id=STRUCT_TRJ_XPLOR;
  } else if(clStrcmp(type,"cns")) {
    id=STRUCT_TRJ_CNS;
  } else if(clStrcmp(type,"dino")) {
    id=STRUCT_TRJ_DINO;
  } else if(clStrcmp(type,"xtc")) {
    id=STRUCT_TRJ_XTC;
  } else if(clStrcmp(type,"binpos")) {
    id=STRUCT_TRJ_BINPOS;
  } else {
    sprintf(message,"unknown type %s\n",type);
    comMessage(message);
    if(cmp) pclose(f); else fclose(f);
    return -1;
  }

  /* 
     at last, the trajectory is read
     the subroutine should return 
     zero on succesfull completion
     and then set the trj_flag to true,
     otherwise return is -1
  */

  if(id==STRUCT_TRJ_CHARMM) {
    ret=charmmTrjRead(f,node,swap_flag);
  } else if(id==STRUCT_TRJ_CNS) {
    ret=cnsTrjRead(f,node, swap_flag);
  } else if(id==STRUCT_TRJ_DINO) {
    ret=dinoTrjRead(f,node, swap_flag);
  } else if(id==STRUCT_TRJ_BINPOS) {
    ret=binposTrjRead(f,node, swap_flag);
  } else if(id==STRUCT_TRJ_XTC) {
    if(cmp) {
      comMessage("gzipped xtc files not supported\n");
      ret=-1;
    } else {
      fclose(f);
      return xtcTrjRead(filename, node, swap_flag);
    }
  } else {
    sprintf(message,"this type is not yet supported\n");
    comMessage(message);
    ret=-1;
  }
  
  if(cmp) pclose(f); else fclose(f);
  return ret;
}

int structPlay(struct DBM_STRUCT_NODE *node)
{
  if(node->trj_flag)
    if(node->trj_play) {
      node->play.wait_count++;
      if(node->play.wait_count>=node->play.wait) {
	node->play.wait_count=0;
	node->play.frame+=node->play.step;

	/* if trajectory is at end */
	if(node->play.frame>=node->play.end) {
	  node->play.delay_count++;
	  if(node->play.delay_count>node->play.delay) {
	    node->play.delay_count=0;
	    if(node->play.mode==STRUCT_PLAY_ROCK) {
	      node->play.step*=-1;
	    } else if (node->play.mode==STRUCT_PLAY_SINGLE) {
	      node->play.frame=0;
	      node->trj_play=0;
	    } else {
	      node->play.frame=node->play.start;
	    }
	  }
	} else if(node->play.frame<node->play.start) {
	  node->play.delay_count++;
	  if(node->play.delay_count>node->play.delay) {
	    node->play.delay_count=0;
	    if(node->play.mode==STRUCT_PLAY_ROCK) {
	      node->play.step*=-1;
	    } else if (node->play.mode==STRUCT_PLAY_SINGLE) {
	      node->play.frame=0;
	      node->trj_play=0;
	    } else {
	      node->play.frame=node->play.end;
	    }
	  }
	}


	structSetFrame(node,node->play.frame);
      }
    }
  return 0;
}


int structSetFrame(struct DBM_STRUCT_NODE *node, int frame)
{
  int pointer,i;
  struct STRUCT_TRJ_POSITION *p;
  char message[256];

  if(!node->trj_flag)
    return -1;

  if(frame<0 || frame>=node->trj.frame_count)
    return -1;

  if(node->trj.type==STRUCT_TRJ_BD) {
    node->frame=frame;
  } else {
    pointer=frame*node->trj.atom_count;
    p=&node->trj.pos[pointer];
    
    memcpy(node->apos,p,sizeof(struct STRUCT_APOS)*node->trj.atom_count);
    
    if(!node->trj_fast) {
      structRecalcBonds(node);
      for(i=0;i<node->obj_max;i++) {
	if(node->obj_flag[i]) {
	  structRecalcBondList(node->obj[i].bond,node->obj[i].bond_count);
	  if(node->obj[i].render.mode==RENDER_CUSTOM) {
	    structObjGenVA(&node->obj[i]);
	  }
	}
      }
      
    }
  }
  sprintf(message,"frame %d of %d",frame+1,node->trj.frame_count);
  guiMessage(message);
  
  return 0;
}

static char* record_atom[]={"ALA","CYS","ASP","GLU","PHE",
			    "GLY","HIS","ILE","LYS","LEU",
			    "MET","ASN","PRO","GLN","ARG",
			    "SER","THR","VAL","TRP","TYR",
			    "A","C","G","T","U",
			    "ADE","CYT","URI","GUA","THY",
			    ""};

static int record_atom_count=30;

static void export_atom(dbmStructNode *node,struct STRUCT_ATOM *ap, int ac, float *pos, int tid, FILE *f)
{
  int i;
  char record[16];
  char *chain,spc[]="     ";
  char aname[32];

  strcpy(record,"HETATM");
  for(i=0;i<record_atom_count;i++)
    if(clStrcmp(record_atom[i],ap->residue->name)) {
      strcpy(record,"ATOM  ");
      break;
    }
  
  memset(aname,0,sizeof(aname));
  if(clStrlen(ap->chem.element)==2) {
    sprintf(aname,"%s",ap->name);
  } else {
    if(clStrcmp(ap->chem.element,"H")) {
      if(isdigit(ap->name[0])) {
	sprintf(aname,"%s",ap->name);
      } else if(strlen(ap->name)>3) {
	aname[0]=ap->name[3];
	aname[1]=ap->name[0];
	aname[2]=ap->name[1];
	aname[3]=ap->name[2];
	aname[4]='\0';
      } else {
	sprintf(aname," %s",ap->name);
      }
    } else {
      sprintf(aname," %s",ap->name);
    }
  }    
  if(node->chain_flag)
    chain=ap->chain->name;
  else
    chain=spc;
  
  
  switch(tid) {
  case STRUCT_WRITE_PDB:
    fprintf(f,"%6s%5d %-4s %3s %c%4d    %8.3f%8.3f%8.3f%6.2f%6.2f        \n",
	    record,ac+1,aname,ap->residue->name,chain[0],ap->residue->num,
	    pos[0],pos[1],pos[2],ap->weight,ap->bfac);
    break;
  case STRUCT_WRITE_XPL:
    fprintf(f,"%6s%5d %-4s %3s  %4d    %8.3f%8.3f%8.3f%6.2f%6.2f   %4s\n",
	    record,ac+1,aname,ap->residue->name,ap->residue->num,
	    pos[0],pos[1],pos[2],ap->weight,ap->bfac,chain);
    break;
  case STRUCT_WRITE_CRD:
    fprintf(f,"%5d %4d %3s  %-4s%10.5f%10.5f%10.5f %c  %4d   %9.5f\n",
	    ap->anum,ap->residue->num,ap->residue->name,ap->name,
	    pos[0],pos[1],pos[2],chain[0],ap->residue->num,ap->weight);
    break;
  case STRUCT_WRITE_XYZR:
    fprintf(f,"%.3f %.3f %.3f %.3f\n",
	    pos[0],pos[1],pos[2],ap->chem.vdwr);
    
    break;
  }
}

int structWrite(struct DBM_STRUCT_NODE *node, structObj *obj, int wc, char **wl)
{
  char message[256];
  char file[256];
  char name[64];
  char base[256],*bp;
  char ext[256];
  char type[64];
  int tid;
  FILE *f;
  time_t t;
  char *c;
  struct STRUCT_ATOM *ap;
  struct STRUCT_ATOM tmp_atom;
  transMat *symtransform;
  int i,ac,tec;
  float v[4];

  if(wc<=0) {
    sprintf(message,"write: missing filename\n");
    comMessage(message);
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

  i=1;
  strcpy(type,"");
  while(i<wc) {
    if(clStrcmp(wl[i],"-type") ||
       clStrcmp(wl[i],"-t")) {
      if(i+1>=wc) {
	sprintf(message,"missing parameter for %s\n",wl[i]);
	comMessage(message);
	return -1;
      }
      strcpy(type,wl[i+1]);
      i+=2;
    } else {
      sprintf(message,"unknown option %s\n",wl[i]);
      comMessage(message);
      return -1;
    }
  }

  if(strlen(type)==0) {
    if(clStrcmp(ext,"pdb")) {
      tid=STRUCT_WRITE_PDB;
    } else if(clStrcmp(ext,"xpl")) {
      tid=STRUCT_WRITE_XPL;
    } else if(clStrcmp(ext,"crd")) {
      tid=STRUCT_WRITE_CRD;
    } else if(clStrcmp(ext,"xyzr")) {
      tid=STRUCT_WRITE_XYZR;
    } else {
      sprintf(message,"unknown extension %s, please specify type\n",ext);
      comMessage(message);
      return -1;
    }
  } else {
    if(clStrcmp(type,"pdb")) {
      tid=STRUCT_WRITE_PDB;
    } else if(clStrcmp(type,"xplorc") ||
	      clStrcmp(type,"cnsc")) {
      tid=STRUCT_WRITE_XPL;
    } else if(clStrcmp(type,"charmm")){
      tid=STRUCT_WRITE_CRD;
    } else if(clStrcmp(type,"xyzr")){
      tid=STRUCT_WRITE_XYZR;
    } else {
      sprintf(message,"unknown type %s\n",type);
      comMessage(message);
      return -1;
    }
  }


  /*
    now that the type is set,
    open file
  */
  if((f=fopen(file,"w"))==NULL) {
    sprintf(message,"cannot open %s\n",file);
    comMessage(message);
    return -1;
  }

  t=time(NULL);
  c=asctime(localtime(&t));

  if(tid==STRUCT_WRITE_PDB) {
    fprintf(f,"REMARK     created by DINO %s",c);
  } else if(tid==STRUCT_WRITE_CRD) {
    fprintf(f,"* Created by DINO %s",c);
    if(obj==NULL)
      fprintf(f,"%5d\n",node->atom_count);
    else
      fprintf(f,"%5d\n",obj->atom_count);
  }

  if(obj==NULL) {
    // dump complete dataset
    for(ac=0;ac<node->atom_count;ac++) {
      ap = &node->atom[ac];

      if(!ap->restriction) {
	// apply dataset transformation
	v[0]=ap->p->x;
	v[1]=ap->p->y;
	v[2]=ap->p->z;
	transApplyf(&node->transform,v);
	// to file
	export_atom(node,ap,ac,v,tid,f);
      }
    }
  } else {
    tec=transListGetEntryCount(&obj->transform_list);
    if(tec>0) {
      // if symview is active
      for(i=0;i<tec;i++) {
	if(tid==STRUCT_WRITE_PDB) {
	  fprintf(f,"MODEL %d\n",i+1);
	}
	symtransform=transListGetEntry(&obj->transform_list,i);
	for(ac=0;ac<obj->atom_count;ac++) {
	  ap=obj->atom[ac].ap;
	  if(!ap->restriction) {
	    // apply dataset transformation
	    v[0]=ap->p->x;
	    v[1]=ap->p->y;
	    v[2]=ap->p->z;
	    transApplyf(symtransform,v);
	    transApplyf(&node->transform,v);
	    export_atom(node,ap,ac,v,tid,f);
	  }
	  
	}
	if(tid==STRUCT_WRITE_PDB) {
	  fprintf(f,"ENDMDL\n");
	}

      }
    } else {
      // just object
      for(ac=0;ac<obj->atom_count;ac++) {
	ap=obj->atom[ac].ap;
	if(!ap->restriction) {
	  // apply dataset transformation
	  v[0]=ap->p->x;
	  v[1]=ap->p->y;
	  v[2]=ap->p->z;
	  transApplyf(&node->transform,v);
	  export_atom(node,ap,ac,v,tid,f);
	}
      }
    }
  }

  if(tid==STRUCT_WRITE_PDB) {
    fprintf(f,"END\n");
  }
  
  fclose(f);
  return 0;
}

int structFix(struct DBM_STRUCT_NODE *node)
{
  int i;
  float v1[3];

  for(i=0;i<node->atom_count;i++) {
    v1[0]=node->atom[i].p->x;
    v1[1]=node->atom[i].p->y;
    v1[2]=node->atom[i].p->z;

    transApplyf(&node->transform,v1);

    node->atom[i].p->x=v1[0];
    node->atom[i].p->y=v1[1];
    node->atom[i].p->z=v1[2];

  }

  transReset(&node->transform);

  structRecalcBonds(node);
  for(i=0;i<node->obj_max;i++) {
    if(node->obj_flag[i])
      structRecalcBondList(node->obj[i].bond,node->obj[i].bond_count);
  }

  return 0;
}

/****************************
int structApplyMat(struct DBM_STRUCT_NODE *node,float *v1)
{
  double a[4],b[4];

  a[0]=v1[0]-node->transform.cen[0];
  a[1]=v1[1]-node->transform.cen[1];
  a[2]=v1[2]-node->transform.cen[2];
  a[3]=1.0;

  matMultVM(a,node->transform.rot,b);
  b[0]+=node->transform.tra[0];
  b[1]+=node->transform.tra[1];
  b[2]+=node->transform.tra[2];

  v1[0]=b[0]+node->transform.cen[0];
  v1[1]=b[1]+node->transform.cen[1];
  v1[2]=b[2]+node->transform.cen[2];

  return 0;
}
************************/

int structComDel(dbmStructNode *node, int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"%s: missing parameter\n",node->name);
    comMessage(message);
    return -1;
  } else {
    for(i=0;i<wc;i++)
      if(structDelObj(node,wl[i])<0)
	return -1;
  }
  comRedraw();
  return 0;
}

int structComPlay(dbmStructNode *node, int wc, char **wl)
{
  int i;
  char message[256];

  node->trj_play=0; /* turn off until all params are OK */
  node->play.frame=0;
  node->play.wait=0;
  node->play.wait_count=0;
  node->play.start=0;
  node->play.end=node->trj.frame_count;
  node->play.step=1;
  node->play.mode=STRUCT_PLAY_LOOP;
  node->play.delay=0;
  node->play.delay_count=0;
  
  for(i=0;i<wc;i++) {
    if(clStrcmp(wl[i],"-wait") ||
       clStrcmp(wl[i],"-w")) {
      if(i+1>=wc) {
	sprintf(message,"missing value for -wait\n");
	comMessage(message);
	return -1;
      }
      node->play.wait=atoi(wl[++i]);
    } else if(clStrcmp(wl[i],"-delay") ||
	      clStrcmp(wl[i],"-d")) {
      if(i+1>=wc) {
	sprintf(message,"missing value for -delay\n");
	comMessage(message);
	return -1;
      }
      node->play.delay=atoi(wl[++i]);
    } else if(clStrcmp(wl[i],"-begin") ||
	      clStrcmp(wl[i],"-b")) {
      if(i+1>=wc) {
	sprintf(message,"missing value for -begin\n");
	comMessage(message);
	return -1;
      }
      node->play.start=atoi(wl[++i]);
      if(node->play.start<0 || node->play.start>node->trj.frame_count) {
	sprintf(message,"begin out of bounds\n");
	comMessage(message);
	return -1;
      }
    } else if(clStrcmp(wl[i],"-end") ||
	      clStrcmp(wl[i],"-e")) {
      if(i+1>=wc) {
	sprintf(message,"missing value for -end\n");
	comMessage(message);
	return -1;
      }
      node->play.end=atoi(wl[++i]);
      if(node->play.end<0 || node->play.end>=node->trj.frame_count) {
	sprintf(message,"end out of bounds\n");
	comMessage(message);
	return -1;
      }
    } else if(clStrcmp(wl[i],"-step") ||
	      clStrcmp(wl[i],"-s")) {
      if(i+1>=wc) {
	sprintf(message,"missing value for -end\n");
	comMessage(message);
	return -1;
      }
      node->play.step=atoi(wl[++i]);
      if(node->play.step==0) {
	sprintf(message,"step must be nonzero\n");
	comMessage(message);
	return -1;
      }
      if(abs(node->play.step)>=node->trj.frame_count) {
	sprintf(message,"step larger than trajectory\n");
	comMessage(message);
	return -1;
      }
    } else if(clStrcmp(wl[i],"-mode") ||
	      clStrcmp(wl[i],"-m")) {
      if(i+1>=wc) {
	sprintf(message,"missing value for -mode\n");
	comMessage(message);
	return -1;
      }
      i++;
      if(clStrcmp(wl[i],"loop")) {
	node->play.mode=STRUCT_PLAY_LOOP;
      } else if(clStrcmp(wl[i],"rock")) {
	node->play.mode=STRUCT_PLAY_ROCK;
      } else if(clStrcmp(wl[i],"single")) {
	node->play.mode=STRUCT_PLAY_SINGLE;
      } else {
	sprintf(message,"unknown mode %s\n",wl[i]);
	comMessage(message);
	return -1;
      }
    } else {
      sprintf(message,"unknown parameter %s\n",wl[i]);
      comMessage(message);
      return -1;
    }
  }
  
  node->trj_play=1;
  comPlay((dbmNode *)node,COM_PLAY_ON);
  return 0;
}

int structGetRangeVal(dbmStructNode *node, struct STRUCT_ATOM *atom, const char *prop, float *r)
{
  //float *cp=comGetCP();
  float cp[3];
  cp[0] = node->transform.cen[0];
  cp[1] = node->transform.cen[1];
  cp[2] = node->transform.cen[2];

  if(prop==NULL) {
    comMessage("error: range: prop missing in range statement\n");
    (*r)=0.0;
    return -1;
  }
  if(clStrcmp(prop,"anum")) {
    (*r)=(float)atom->anum;
  } else if(clStrcmp(prop,"rnum")) {
    (*r)=(float)atom->residue->num;
  } else if(clStrcmp(prop,"model")) {
    (*r)=(float)atom->model->num;
  } else if(clStrcmp(prop,"occ") ||
	    clStrcmp(prop,"weight")) {
    (*r)=atom->weight;
  } else if(clStrcmp(prop,"bfac")) {
    (*r)=atom->bfac;
  } else if(clStrcmp(prop,"x")) {
    (*r)=atom->p->x;
  } else if(clStrcmp(prop,"y")) {
    (*r)=atom->p->y;
  } else if(clStrcmp(prop,"z")) {
    (*r)=atom->p->z;
  } else if(clStrcmp(prop,"dist")) {
    (*r) = sqrtf(
		 (atom->p->x-cp[0])*(atom->p->x-cp[0])+
		 (atom->p->y-cp[1])*(atom->p->y-cp[1])+
		 (atom->p->z-cp[2])*(atom->p->z-cp[2])
		 );
      
  } else {
    comMessage("error: range: unknown atom property \n");
    comMessage(prop);
    (*r)=0.0;
    return -1;
  }
  return 0;
}

int structGetRangeXYZVal(dbmStructNode *node, const char *prop, float *p, float *r)
{
  comMessage("not implemented\n");
  return -1;
}

int structGetVectProtein(struct STRUCT_RESIDUE *res)
{
  int ac;
  double p1[3],p2[3],p3[3],p4[3];
  double q1[3],q2[3],q3[3],q4[3];
  int f1,f2,f3,f4;

  f1=f2=f3=f4=0;

  for(ac=0;ac<res->atom_count;ac++) {
    if(clStrcmp("C",res->atom[ac]->name)) {
      f1=1;
      p1[0]=res->atom[ac]->p->x;
      p1[1]=res->atom[ac]->p->y;
      p1[2]=res->atom[ac]->p->z;
    } else if(clStrcmp("O",res->atom[ac]->name)) {
      f2=1;
      p2[0]=res->atom[ac]->p->x;
      p2[1]=res->atom[ac]->p->y;
      p2[2]=res->atom[ac]->p->z;
    }
  }

  q1[0]=p2[0]-p1[0];
  q1[1]=p2[1]-p1[1];
  q1[2]=p2[2]-p1[2];

  res->v1[0]=q1[0];
  res->v1[1]=q1[1];
  res->v1[2]=q1[2];

  return 0;
}

int structGetVectNA(struct STRUCT_RESIDUE *res)
{
  int ac;
  double p0[3],p1[3],p2[3],p3[3],p4[3],p5[3],p6[3],p7[3];
  double q1[3],q2[3],q3[3],q4[3],q5[3],q6[4];
  int f0,f1,f2,f3,f4,f5,f6,f7;
  int rtype=0;
  char message[256];

  f0=f1=f2=f3=f4=f5=f6=f7=0;

  for(ac=0;ac<res->atom_count;ac++) {
    if(clStrcmp("P",res->atom[ac]->name)) {
      f0=1;
      p0[0]=res->atom[ac]->p->x;
      p0[1]=res->atom[ac]->p->y;
      p0[2]=res->atom[ac]->p->z;
    } else if(clStrcmp("C3'",res->atom[ac]->name)) {
      f1=1;
      p1[0]=res->atom[ac]->p->x;
      p1[1]=res->atom[ac]->p->y;
      p1[2]=res->atom[ac]->p->z;
    } else if(clStrcmp("C1'",res->atom[ac]->name)) {
      f2=1;
      p2[0]=res->atom[ac]->p->x;
      p2[1]=res->atom[ac]->p->y;
      p2[2]=res->atom[ac]->p->z;
    } else if(clStrcmp("C4'",res->atom[ac]->name)) {
      f3=1;
      p3[0]=res->atom[ac]->p->x;
      p3[1]=res->atom[ac]->p->y;
      p3[2]=res->atom[ac]->p->z;
    }

    if(clStrcmp("C",res->name) || clStrcmp("T",res->name) ||
       clStrcmp("U",res->name) || clStrcmp("CYT",res->name) ||
       clStrcmp("THY",res->name) || clStrcmp("URI",res->name)) {
      /* pyrimidine */
      rtype=1;
      if(clStrcmp("N1",res->atom[ac]->name)) {
	f4=1;
	p4[0]=res->atom[ac]->p->x;
	p4[1]=res->atom[ac]->p->y;
	p4[2]=res->atom[ac]->p->z;
      } else if(clStrcmp("C5",res->atom[ac]->name)) {
	f5=1;
	p5[0]=res->atom[ac]->p->x;
	p5[1]=res->atom[ac]->p->y;
	p5[2]=res->atom[ac]->p->z;
      } else if(clStrcmp("C2",res->atom[ac]->name)) {
	f6=1;
	p6[0]=res->atom[ac]->p->x;
	p6[1]=res->atom[ac]->p->y;
	p6[2]=res->atom[ac]->p->z;
      } else if(clStrcmp("N3",res->atom[ac]->name)) {
	f7=1;
	p7[0]=res->atom[ac]->p->x;
	p7[1]=res->atom[ac]->p->y;
	p7[2]=res->atom[ac]->p->z;
      } 
    } else if (clStrcmp("A",res->name) || clStrcmp("G",res->name) ||
	       clStrcmp("ADE",res->name) || clStrcmp("GUA",res->name)) {
      /* purin */
      rtype=2;
      if(clStrcmp("N9",res->atom[ac]->name)) {
	f4=1;
	p4[0]=res->atom[ac]->p->x;
	p4[1]=res->atom[ac]->p->y;
	p4[2]=res->atom[ac]->p->z;
      } else if(clStrcmp("C4",res->atom[ac]->name)) {
	f5=1;
	p5[0]=res->atom[ac]->p->x;
	p5[1]=res->atom[ac]->p->y;
	p5[2]=res->atom[ac]->p->z;
      } else if(clStrcmp("C2",res->atom[ac]->name)) {
	f6=1;
	p6[0]=res->atom[ac]->p->x;
	p6[1]=res->atom[ac]->p->y;
	p6[2]=res->atom[ac]->p->z;
      } else if(clStrcmp("N1",res->atom[ac]->name)) {
	f7=1;
	p7[0]=res->atom[ac]->p->x;
	p7[1]=res->atom[ac]->p->y;
	p7[2]=res->atom[ac]->p->z;
      } 
    }
  }

  if((f0+f1+f2+f3+f4+f5+f6+f7)!=8) {
    sprintf(message,"error in NA recognizion: residue %s%d is missing: ",
	    res->name,res->num);

    if(!f0)
      strcat(message," P");
    if(!f1)
      strcat(message," C1'");
    if(!f2)
      strcat(message," C3'");
    if(!f3)
      strcat(message," C4'");

    if(!f4 && rtype==1)
      strcat(message," N1");
    if(!f5 && rtype==1)
      strcat(message," C5");
    if(!f6 && rtype==1)
      strcat(message," C2");
    if(!f7 && rtype==1)
      strcat(message,"N3");

    if(!f4 && rtype==2)
      strcat(message," N9");
    if(!f5 && rtype==2)
      strcat(message," C4");
    if(!f6 && rtype==2)
      strcat(message," C2");
    if(!f7 && rtype==2)
      strcat(message,"N1");

    debmsg(message);

    return -1;
  }

  q1[0]=p2[0]-p1[0];
  q1[1]=p2[1]-p1[1];
  q1[2]=p2[2]-p1[2];
  q2[0]=p3[0]-p1[0];
  q2[1]=p3[1]-p1[1];
  q2[2]=p3[2]-p1[2];
  q4[0]=p4[0]-p2[0];
  q4[1]=p4[1]-p2[1];
  q4[2]=p4[2]-p2[2];
  q5[0]=p5[0]-p2[0];
  q5[1]=p5[1]-p2[1];
  q5[2]=p5[2]-p2[2];

  matCalcCross(q1,q2,q3);

  //  matfNormalize(q1,q1);
  // matfNormalize(q3,q3);

  /*

    v1: sugar vector {0,0,0} to {1,0,0}
    v2: normal of sugar plane
    v3: vector of sugar end to nitrogen of base
    v4: offset of nitrogen to 0,0,0 of base
    v5: {1,0,0} direction of base
    v6: normal of base
    v7: vector pointing from phosphate to middle
    
  */

  res->v0[0]=p1[0];
  res->v0[1]=p1[1];
  res->v0[2]=p1[2];

  res->v1[0]=q1[0];
  res->v1[1]=q1[1];
  res->v1[2]=q1[2];

  res->v2[0]=q3[0];
  res->v2[1]=q3[1];
  res->v2[2]=q3[2];

  res->v3[0]=q4[0];
  res->v3[1]=q4[1];
  res->v3[2]=q4[2];

  res->v7[0]=p7[0]-p0[0];
  res->v7[1]=p7[1]-p0[1];
  res->v7[2]=p7[2]-p0[2];

  if(rtype==1) {
    /* pyrimidine */
    res->v4[0]=0.0;
    res->v4[1]=0.0;
    res->v4[2]=0.0;

    q1[0]=p5[0]-p4[0];
    q1[1]=p5[1]-p4[1];
    q1[2]=p5[2]-p4[2];
    res->v5[0]=q1[0];
    res->v5[1]=q1[1];
    res->v5[2]=q1[2];
    
    q2[0]=p6[0]-p4[0];
    q2[1]=p6[1]-p4[1];
    q2[2]=p6[2]-p4[2];
    matCalcCross(q1,q2,q3);
    res->v6[0]=q3[0];
    res->v6[1]=q3[1];
    res->v6[2]=q3[2];
  } else if(rtype==2) {
    /* purin */
    res->v4[0]=p5[0]-p4[0];
    res->v4[1]=p5[1]-p4[1];
    res->v4[2]=p5[2]-p4[2];


    q1[0]=p6[0]-p5[0];
    q1[1]=p6[1]-p5[1];
    q1[2]=p6[2]-p5[2];
    matNormalize(q1,q1);
    res->v5[0]=q1[0];
    res->v5[1]=q1[1];
    res->v5[2]=q1[2]; 
   
    q2[0]=p5[0]-p4[0];
    q2[1]=p5[1]-p4[1];
    q2[2]=p5[2]-p4[2];
    matNormalize(q2,q2);
    matCalcCross(q1,q2,q3);
    matNormalize(q3,q3);
    res->v6[0]=q3[0];
    res->v6[1]=q3[1];
    res->v6[2]=q3[2];
  }

  return rtype;
}

int structRecenter(dbmStructNode *node)
{
  float v1[4],v2[4];
  int i;

  v1[0]=0.0; v1[1]=0.0; v1[2]=0.0;
  for(i=0;i<node->atom_count;i++) {
    v1[0]+=node->atom[i].p->x;
    v1[1]+=node->atom[i].p->y;
    v1[2]+=node->atom[i].p->z;
  }
  if(node->atom_count>0) {
    v2[0]=v1[0]/(float)node->atom_count;
    v2[1]=v1[1]/(float)node->atom_count;
    v2[2]=v1[2]/(float)node->atom_count;
  } else {
    v2[0]=0.0;
    v2[1]=0.0;
    v2[2]=0.0;
  }

  node->transform.cen[0]=v2[0];
  node->transform.cen[1]=v2[1];
  node->transform.cen[2]=v2[2];

  return 0;
}

void structSetFlag(dbmStructNode *node,int mask)
{
  int i;
  for(i=0;i<node->atom_count;i++)
    node->atom[i].flag |= mask;
}

void structClearFlag(dbmStructNode *node,int mask)
{
  int i;
  for(i=0;i<node->atom_count;i++)
    node->atom[i].flag &= (~mask);
}

// old

int structReconnect2(struct DBM_STRUCT_NODE *node)
{
  int i,mc,cc,rc,bc,pass;
  register int ac1, ac2,id1,id2;
  int m1;
  struct STRUCT_MODEL *cmp;
  struct STRUCT_CHAIN *ccp;
  struct STRUCT_RESIDUE *crp;
  struct STRUCT_ATOM *ap1,*ap2,*prev;
  struct STRUCT_BOND *nb;
  struct CONN_ENTRY *centry;
  int b_max;
  int abc1[3],abc2[3];
  caPointer *ca_p;
  int ca_c;

  if(node->bond_count>0)
    Cfree(node->bond);
  
  b_max=10000;
  nb=(struct STRUCT_BOND*)Crecalloc(NULL,b_max,sizeof(struct STRUCT_BOND));
  bc=0;

  // implicit connectivity based on lookup tables
  if(node->conn_flag & STRUCT_CONN_IMPL)
    for(mc=0;mc<node->model_count;mc++) {
    cmp=&node->model[mc];
    for(cc=0;cc<cmp->chain_count;cc++) {
      ccp=cmp->chain[cc];
      prev=NULL;
      for(rc=0;rc<ccp->residue_count;rc++) {
	crp=ccp->residue[rc];
	centry=conGetEntry(crp->name);
	/*
	  no connectivity information is available
	*/
	if(centry==NULL) {
	  prev=NULL;
	  /*
	    loop over all atoms in this RESIDUE ... 
	  */
	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	    ap1=crp->atom[ac1];
	    /*
	      ... versus all atoms in this MODEL 
	    */

	    caXYZtoABC(node->ca,(float *)ap1->p,abc1);
	    for(abc2[0]=abc1[0]-1;abc2[0]<=abc1[0]+1;abc2[0]++)
	      for(abc2[1]=abc1[1]-1;abc2[1]<=abc1[1]+1;abc2[1]++)
		for(abc2[2]=abc1[2]-1;abc2[2]<=abc1[2]+1;abc2[2]++) {
		  caGetList(node->ca,abc2,&ca_p,&ca_c);
		  for(ac2=0;ac2<ca_c;ac2++) {
		    ap2=(struct STRUCT_ATOM*)ca_p[ac2];
		    if(ap1->model==ap2->model)
		      if(ap1->n!=ap2->n)
			if(structIsConnected(ap1,ap2))
			  structConnectAtoms(node,&nb,&bc,&b_max,ap1,ap2);
		  }
		}
	  }
	  /* 
	     connectivity info is available for this residue
	  */
	} else {
	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	    ap1=crp->atom[ac1];
	    id1=conGetAtomID(centry,ap1->name);
	    /*
	      but no info for this atom 
	    */
	    if(!id1) {
	      m1=ap1->model->num;
	      /* 
		 check against all other atoms in residue
		 this is questionable, maybe all other atoms ???
	      */
	      for(ac2=0;ac2<crp->atom_count;ac2++) {
		ap2=crp->atom[ac2];
		/*
		  TODO double bonds
		  connect if THIS NEEDS ADJUSTMENT TO AVOID
		  DOUBLE ALLOCATED BONDS !
		  and distance is ok
		*/
		if(ac1!=ac2)
		  if(structIsConnected(ap1,ap2))
		    structConnectAtoms(node,&nb,&bc,&b_max,ap1,ap2);
	      }
	      /* 
		 there was info for this atom
	      */
	    } else { /* !id1 */
	      /* 
		 loop over remaining atoms in residue
	      */
	      for(ac2=ac1+1;ac2<crp->atom_count;ac2++) {
		ap2=crp->atom[ac2];
		id2=conGetAtomID(centry,ap2->name);
		/*
		  if other atom was also found
		  and is connected to first
		*/
		if(id2)
		  if(conIsConnected(centry,id1,id2))
		    structConnectAtoms(node,&nb,&bc,&b_max,ap1,ap2);
	      }
	      
	    }
	  }
	  id2=conGetPrevID(centry);
	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	      ap1=crp->atom[ac1];
	      if(id2==conGetAtomID(centry,ap1->name))
		if(prev!=NULL)
		  if(prev->residue->num+1==ap1->residue->num)
		    structConnectAtoms(node,&nb,&bc,&b_max,ap1,prev);
	  }
	  prev=NULL;
	  id2=conGetNextID(centry);
	  for(ac1=0;ac1<crp->atom_count;ac1++) {
	      ap1=crp->atom[ac1];
	      if(id2==conGetAtomID(centry,ap1->name)) {
		prev=ap1;
		break;
	      }
	  }
	}
      }
    }
  }

  /* 
     find all bonds that have not been found,
     but that are defined by external connectivity
  */
  pass=0;
  for(cc=0;cc<node->conn_count;cc++) {
    for(i=0;i<bc;i++) {
      if(nb[i].atom1==node->conn[cc].ap1 && 
	 nb[i].atom2==node->conn[cc].ap2) {
	break;
      } else if(nb[i].atom1==node->conn[cc].ap2 && 
		nb[i].atom2==node->conn[cc].ap1) {
	break;
      } 
    }
    if(i==bc) {
      pass++;
      structConnectAtoms(node,&nb,&bc,&b_max,
			 node->conn[cc].ap1,node->conn[cc].ap2);
    }
    
  }


  node->bond_count=bc;
  node->bond_max=bc+100;
  node->bond=(struct STRUCT_BOND*)Ccalloc(bc+100,sizeof(struct STRUCT_BOND));
  memcpy(node->bond,nb,bc*sizeof(struct STRUCT_BOND));
  Cfree(nb);
#ifdef WITH_NCBONDS
  structReconnectNC(node);
#endif

  structRecalcBonds(node);

  return 0;
}

/*
  add symmetry operators
*/
void structAddSymop(dbmStructNode* node, int wc, char**wl)
{
  // first word contains rotation matrix
  // second word contains translation matrix
  transMat tmat;
  transReset(&tmat);
  if(transSetRot(&tmat,wl[0])<0) return;
  if(transSetTra(&tmat,wl[1])<0) return;

  transListAddEntry(&node->symop_list,&tmat);
}

void structOnTransform(void* cdata)
{
  dbmStructNode *node = (dbmStructNode*)cdata;
  int j;
  for(j=0;j<node->obj_max;j++) {
      if(node->obj_flag[j]!=0) {
	if(node->obj[j].symview>0) {
	  structObjUpdateSymview(&node->obj[j]);
	}
      }
  }
}
