#include <GL/gl.h>

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
  node->obj=Ccalloc(node->obj_max,sizeof(structObj));
  if(node->obj==NULL) {
    sprintf(message,"\nmemory error in structNewNode");
    comMessage(message);
    return -1;
  }
  node->obj_flag=Ccalloc(node->obj_max,sizeof(int));
  if(node->obj_flag==NULL) {
    sprintf(message,"\nmemory error in structNewNode");
    comMessage(message);
    return -1;
  }

  for(i=0;i<node->obj_max;i++)
    node->obj_flag[i]=0;

  node->model_count=0;
  node->chain_count=0;
  node->residue_count=0;
  node->atom_count=0;
  node->bond_count=0;
  node->nbond_count=0;
  node->conn=NULL;
  node->xtal=NULL;
  node->atom_table=NULL;
  node->ca=NULL;
  node->trj_flag=0;
  node->trj_fast=1;

  transReset(&node->transform);

  return 0;
}

/*
  this routine is called after a structure has been read in
*/

int structPrep(dbmStructNode *node)
{
  // build CubeArray
  debmsg("structPrep: build CA");       
  structBuildCA(node);

  // determine the connectivity
  debmsg("structPrep: reconnect");
  structReconnect(node);

  // determine min and max params
  structSetMinMax(node);

  // set the center to geometric center
  structRecenter(node);

  // calculate various res params
  structPrepRes(node);

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
    if(node->residue[i].class==STRUCT_PROTEIN) {
      structGetVectProtein(node->residue+i);
    } else if(node->residue[i].class==STRUCT_NA) {
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
  if(!strcmp(wl[0],"?") ||
     !strcmp(wl[0],"help")) {
  } else if(!strcmp(wl[0],"new")) {
    structComNew(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"get")) {
    return structComGet(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"del") ||
	    !strcmp(wl[0],"delete")) {
    return structComDel(node, wc-1,wl+1);
  } else if(!strcmp(wl[0],"set")) {
    return structComSet(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"copy") ||
	    !strcmp(wl[0],"cp")) {
    sprintf(message,"\n%s: copy not available",node->name);
    comMessage(message);
    return -1;
  } else if(!strcmp(wl[0],"move") ||
	    !strcmp(wl[0],"mv")) {
    sprintf(message,"\n%s: move not available",node->name);
    comMessage(message);
    return -1;
  } else if(!strcmp(wl[0],"restrict")) {
    structComRestrict(node, wc-1,wl+1);
  } else if(!strcmp(wl[0],"load")) {
    return structComLoad(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"step")) {
    if(!node->trj_flag) {
      sprintf(message,"%s: no trajectory loaded",node->name);
      comMessage(message);
      return -1;
    }
    if(wc==1) {
      node->frame++;
    } else if(wc==2) {
      node->frame+=atoi(wl[1]);
    } else {
      sprintf(message,"\ntoo many parameters: usage: step [n]");
      comMessage(message);
      return -1;
    }
    if(node->frame<0)
      node->frame=node->trj.frame_count-1;
    else if(node->frame>=node->trj.frame_count)
      node->frame=0;
    structSetFrame(node,node->frame);
    comRedraw();
  } else if(!strcmp(wl[0],"play")) {
    if(!node->trj_flag) {
      sprintf(message,"\n%s: no trajectory loaded",node->name);
      comMessage(message);
      return -1;
    } else {
      structComPlay(node, wc-1,wl+1);
    }
  } else if(!strcmp(wl[0],"stop")) {
    if(!node->trj_flag) {
      comMessage("\nno trajectory present");
      return -1;
    }
    comPlay((dbmNode *)node,COM_PLAY_OFF);
    node->trj_play=0;
    structSetFrame(node,0);
    comRedraw();
  } else if(!strcmp(wl[0],"grab")) {
    if(wc!=2) {
      sprintf(message,"\nSyntax: grab devicename");
      comMessage(message);
    } else {
      if(comGrab(&node->transform,wl[1])<0)
	return -1;
      // set center to current center
      comGetCurrentCenter(node->transform.cen);
      structRecenter(node);
    }
  } else if(!strcmp(wl[0],"write")) {
    structWrite(node,NULL,wc-1,wl+1);
  } else if(!strcmp(wl[0],"reset")) {
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
	}
      }
    }
    comRedraw();
  } else if(!strcmp(wl[0],"fix")) {
    if(wc>1) {
      sprintf(message,"\nwarning: fix: superfluous parameter ignored");
      comMessage(message);
    }
    structFix(node);
    structSetMinMax(node);
    structRecenter(node);
    comRedraw();
  } else if(!strcmp(wl[0],"rotx")) {
    if(wc<2) {
      comMessage("\nerror: missing value after rotx");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTX,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"roty")) {
    if(wc<2) {
      comMessage("\nerror: missing value after roty");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTY,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"rotz")) {
    if(wc<2) {
      comMessage("\nerror: missing value after rotz");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTZ,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transx")) {
    if(wc<2) {
      comMessage("\nerror: missing value after transx");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAX,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transy")) {
    if(wc<2) {
      comMessage("\nerror: missing value after transy");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAY,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transz")) {
    if(wc<2) {
      comMessage("\nerror: missing value after transz");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAZ,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"center")) {
    if(wc<2) {
      sprintf(message,"\nmissing value for center");
      comMessage(message);
      return -1;
    }
    if(matExtract1D(wl[1],3,v)!=0) {
      sprintf(message,"\nerror in vector");
      comMessage(message);
      return -1;
    }
    node->transform.cen[0]=v[0];
    node->transform.cen[1]=v[1];
    node->transform.cen[2]=v[2];
    comRedraw();
  } else if(!strcmp(wl[0],"connect")) {
    if(wc!=3) {
      sprintf(message,"\nsyntax: .struct connect ATOM1 ATOM2");
      comMessage(message);
      return -1;
    }
    return structComConnect(node,wl[1],wl[2]);
  } else if(!strcmp(wl[0],"reconnect")) {
    if(wc==1) {
      sprintf(message,"\nmissing parameter");
      comMessage(message);
      return -1;
    } else if(wc==2) {
      if(!strcmp(wl[1],"ncb")) {
	structReconnectNC(node);
      } else {
	sprintf(message,"\nunknown parameter: %s",wl[1]);
	comMessage(message);
	return -1;
      }
    } else {
      sprintf(message,"\ntoo many words");
      comMessage(message);
      return -1;
    }
  } else if(!strcmp(wl[0],"cell")) {
    if(node->xtal==NULL) {
      comMessage("\nno crystallographic info available");
      return -1;
    }
    if(wc==1) {
      sprintf(message,"\n%s %f.2 %f.2 %f.2  %f.2 %f.2 %f.2",
	      node->xtal->space_group_name,
	      node->xtal->a, node->xtal->b, node->xtal->c,
	      node->xtal->alpha, node->xtal->beta, node->xtal->gamma);
      comMessage(message);
    } else {
      if(!strcmp(wl[1],"show")) {
	node->show_cell=1;
	comRedraw();
      } else if(!strcmp(wl[1],"hide")) {
	node->show_cell=0;
	comRedraw();
      } else {
	sprintf(message,"\nerror: unknown cell command '%s'",wl[1]);
      }
    }
  } else {
    sprintf(message,"\n%s: unknown command %s",node->name,wl[0]);
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
	comMessage("\nerror: new: expected an argument beginning with -"); 
	ret=-1;
	break;
      }
    } else if(clStrcmp(co.param[i].p,"name") || 
	      clStrcmp(co.param[i].p,"n")) {
      if(co.param[i].wc<1) {
	comMessage("\nerror: new: missing value for -name");
	ret=-1;
	break;
      } else if(co.param[i].wc>1) {
	comMessage("\nerror: new: too many values for -name");
	ret=-1;
	break;
      } else {
	name=co.param[i].wl[0];
      }
    } else if(clStrcmp(co.param[i].p,"type") ||
	      clStrcmp(co.param[i].p,"t")) {
      if(co.param[i].wc<1) {
	comMessage("\nerror: new: missing value for -type");
	ret=-1;
	break;
      } else if(co.param[i].wc>1) {
	comMessage("\nerror: new: too many values for -type");
	ret=-1;
	break;
      } else {
	if(clStrcmp(co.param[i].wl[0],"connect")) {
	  type=STRUCT_CONNECT;
	} else if(clStrcmp(co.param[i].wl[0],"trace")) { 
	  type=STRUCT_TRACE;
	} else if(clStrcmp(co.param[i].wl[0],"nbond")) { 
	  type=STRUCT_NBOND;
	} else {
	  clStrcpy(message,"\nerror: new: unknown type ");
	  clStrncat(message,co.param[i].wl[0],100);
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
      clStrcpy(message,"\nunknown paramater ");
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
    comMessage("\nerror: get: missing property");
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
    node->atom[i].restrict=0;

  // check each atom
  c=0;
  for(i=0;i<node->atom_count;i++) {
    ret=structIsAtomSelected(node, &node->atom[i], &sel);
    if(ret<0) {
      selectDelete(&sel);
      return -1;
    }
    if(ret==0) {
      node->atom[i].restrict=1;
      c++;
   }
  }

  // output how many atoms were flaged
  if(c==0)
    sprintf(message,"\nall atoms unrestricted");
  else if(c==node->atom_count)
    sprintf(message,"\nrestriction affects ALL of %d atoms",node->atom_count);
  else
    sprintf(message,"\nrestriction affects %d of %d atoms",c,node->atom_count);
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
    comMessage("error: set: range not expected for struct dataset");
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
    } else if(clStrcmp(s->pov[pc].prop,"sg")) {
      s->pov[pc].id=STRUCT_PROP_SG;
    } else if(clStrcmp(s->pov[pc].prop,"frame")) {
      s->pov[pc].id=STRUCT_PROP_FRAME;
    } else {
      comMessage("\nerror: set: unknown property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
    if(s->pov[pc].op!=POV_OP_EQ && s->pov[pc].id!=STRUCT_PROP_TFAST) {
      comMessage("\nerror: set: expected operator = for property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
    op=s->pov[pc].op;
    if(s->pov[pc].val_count>1 && s->pov[pc].id!=STRUCT_PROP_NCI) {
      comMessage("\nerror: set: expected only one value for property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
  }

  for(pc=0;pc<s->pov_count;pc++) {
    val=povGetVal(&s->pov[pc],0);
    switch(s->pov[pc].id) {
    case STRUCT_PROP_ROT:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property rot");
	return -1;
      }
      if(transSetRot(&node->transform,val->val1)<0)
	return -1;
      break;
    case STRUCT_PROP_TRANS:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property trans");
	return -1;
      }
      if(transSetTra(&node->transform,val->val1)<0)
	return -1;
      break;
    case STRUCT_PROP_RTC:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property rtc");
	return -1;
      }
      if(transSetAll(&node->transform,val->val1)<0)
	return -1;

      break;
    case STRUCT_PROP_RCEN:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property rcen");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v1)!=0) {
	comMessage("\nerror in vector: ");
	comMessage(val->val1);
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
	comMessage("\nerror: set: unexpected range in property smode");
	return -1;
      }
      if(clStrcmp(val->val1,"atom")) {
	node->smode=SEL_ATOM;
      } else if(clStrcmp(val->val1,"residue")) {
	node->smode=SEL_RESIDUE;
      } else {
	comMessage("\nerror: unknown smode ");
	comMessage(val->val1);
	return -1;
      }
      break;
    case STRUCT_PROP_RTYPE:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property rtype");
	return -1;
      }
      if(clStrcmp(val->val1,"coil")) {
	rt=STRUCT_RTYPE_COIL;
      } else if (clStrcmp(val->val1,"helix")) {
	rt=STRUCT_RTYPE_HELIX;
      } else if (clStrcmp(val->val1,"strand")) {
	rt=STRUCT_RTYPE_STRAND;
      } else {
	comMessage("\nunknown rtype: ");
	comMessage(val->val1);
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
	comMessage("\nerror: set: unexpected range in property nci");
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
	comMessage("\nerror: set: unexpected range in property vdwr");
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
	comMessage("\nerror: set: unexpected range in property color");
	return -1;
      }
      if(comGetColor(val->val1,&r,&g,&b)<0) {
	comMessage("\nerror: set: unknown color ");
	comMessage(val->val1);
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
	comMessage("\nerror: set: unexpected range in property tfast");
	return -1;
      }
      if(op==POV_OP_NOT) {
	node->trj_fast=0;
      } else if(op==POV_OP_NN) {
	node->trj_fast=1;
      } else {
	comMessage("\nsyntax error with property tfast");
	return -1;
      }
      break;
    case STRUCT_PROP_FRAME:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property tfast");
	return -1;
      }
      if(node->trj_flag) {
	if(atoi(val->val1)<1 || atoi(val->val1)>node->trj.frame_count) {
	  comMessage("\nerror: frame number out of limits"); 
	  return -1;
	} else {
	  structSetFrame(node,atoi(val->val1)-1);
	  comRedraw();
	}
      } else {
	comMessage("\nerror: no trajectory loaded");
	return -1;
      }
      break;
    case STRUCT_PROP_CELL:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property cell");
	return -1;
      }
      if(matExtract1Df(val->val1,6,v1)==-1) {
	comMessage("\nerror in cell expression, expected {a,b,c,alpha,beta,gamma}");
	return -1;
      }
      if(node->xtal==NULL) {
	node->xtal=Cmalloc(sizeof(struct XTAL));
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
	comMessage("\nerror: set: unexpected range in property cell");
	return -1;
      }
      if(node->xtal==NULL) {
	node->xtal=Cmalloc(sizeof(struct XTAL));
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
    }
  }
  return 0;
}

int structComConnect(dbmStructNode *node, char *s1, char *s2)
{
  char m1[256],m2[256],c1[256],c2[256],r1[256],r2[256],a1[256],a2[256];
  int n1,n2;
  char message[256];

  if(structSubMatch(node,s1,m1,c1,r1,a1)!=0)
    return -1;
  if(structSubMatch(node,s2,m2,c2,r2,a2)!=0)
    return -1;

  n1=structSubGetNum(node,m1,c1,r1,a1);
  if(n1<0) {
    sprintf(message,"\natom not found: %s",s1);
    comMessage(message);
    return -1;
  }
  n2=structSubGetNum(node,m2,c2,r2,a2);
  if(n2<0) {
    sprintf(message,"\natom not found: %s",s2);
    comMessage(message);
    return -1;
  }

  /* both atoms were found, now connect them */

  structConnectAtoms(node,
		     &node->bond,
		     &node->bond_count,
		     &node->bond_max,
		     &node->atom[n1], &node->atom[n2]);

  structRecalcBonds(node);

  return 0;
}


int structGet(dbmStructNode *node, char *prop)
{
  int ret=0,i;
  float v1[4];
  char message[256];

  if(!strcmp(prop,"center")) {
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
  } else if(!strcmp(prop,"rtc")) {
    comReturn(transGetAll(&node->transform));
  } else if(!strcmp(prop,"cell")){
    if(node->xtal==NULL) {
      sprintf(message,"\n%s: no cell defined",node->name);
      comMessage(message);
      ret=-1;
    } else {
      sprintf(message,"\n%s: cell: %.3f %.3f %.3f   %.2f %.2f %.2f  %s",
	      node->name,
	      node->xtal->a,node->xtal->b,node->xtal->c,
	      node->xtal->alpha,node->xtal->beta,node->xtal->gamma,
	      node->xtal->space_group_name);
      comMessage(message);
    }
  } else if(!strcmp(prop,"rot")) {
    comReturn(transGetRot(&node->transform));
  } else if(!strcmp(prop,"trans")) {
    comReturn(transGetTra(&node->transform));
  } else if(!strcmp(prop,"smode")) {
    if(node->smode==SEL_ATOM)
      comReturn("atom");
    else if(node->smode==SEL_RESIDUE) {
      comReturn("residue");
    } else {
      comReturn("unknown");
    }
  } else {
    sprintf(message,"\n%s: unknown parameter %s", node->name,prop);
    comMessage(message);
    ret=-1;
  }
  
  return ret;
}


int structReconnect(struct DBM_STRUCT_NODE *node)
{
  int i,mc,cc,rc,bc,pass;
  register int ac1, ac2,id1,id2;
  int m1;
  struct STRUCT_MODEL *cmp;
  struct STRUCT_CHAIN *ccp;
  struct STRUCT_RESIDUE *crp;
  struct STRUCT_ATOM *ap1,*ap2,*prev;
  struct STRUCT_BOND *nb;
  int b_max=10000;
  struct CONN_ENTRY *centry;
  int abc1[3],abc2[3];
  caPointer *ca_p;
  int ca_c;

  if(node->bond_count>0)
    Cfree(node->bond);
  
  nb=Crecalloc(NULL,b_max,sizeof(struct STRUCT_BOND));

  bc=0;

//  fprintf(stderr,"\n%d",node->model_count);

  if(node->conn_flag)
    for(mc=0;mc<node->model_count;mc++) {
    cmp=&node->model[mc];
    for(cc=0;cc<cmp->chain_count;cc++) {
      ccp=cmp->chain[cc];
      prev=NULL;
//      fprintf(stderr,"\n%d %s %d",mc,ccp->name, ccp->residue_count);
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
		    ap2=ca_p[ac2];
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
  node->bond=Ccalloc(bc+100,sizeof(struct STRUCT_BOND));
  memcpy(node->bond,nb,bc*sizeof(struct STRUCT_BOND));
  Cfree(nb);

  structReconnectNC(node);

  structRecalcBonds(node);

  return 0;
}

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

	      if(matfCalcAngle(a3->p,a4->p,a3->p,a5->p)<alimit) {
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
	      if(matfCalcAngle(a4->p,pos,a3->p,pos)<alimit) {
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
//      ob=(*nb);
      (*nb)=Crecalloc((*nb),(*bm)+5000,sizeof(struct STRUCT_BOND));
//      memcpy((*nb),ob,(*bm)*sizeof(struct STRUCT_BOND));
      (*bm)+=5000;
//      Cfree(ob);
    }
    return 0;
  } else {
    debmsg("Connect: bond overflow");
    return -1;
  }
}


structObj *structNewObj(dbmStructNode *node, char *name)
{
  int i;
  structObj *no;
  
  structDelObj(node,name);
  for(i=0;i<node->obj_max;i++) {
    if(node->obj_flag[i]==0) {
      node->obj_flag[i]=1;
      no=&node->obj[i];
      no->node=node;
      comNewObj(node->name,name);
      structSetDefault(no);
      clStrcpy(no->name,name);
      return no;
    }
  }

  /* catch and increase obj_max */

  return NULL;
}

int structDelObj(struct DBM_STRUCT_NODE *node,char *name)
{
  int i;
  structObj *obj;
  for(i=0;i<node->obj_max;i++)
    if(node->obj_flag[i]==1)
      if(!strcmp(node->obj[i].name,name)) {
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


int structSubCommand(dbmStructNode *node,char *rsub, int wc, char **wl)
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
	      fprintf(stdout,"\nfound atom #%d",cap->n);
	    */
	    c++;
	    if(!strcmp(wl[0],"get")) {
	      return structSubComGet(node,cap,wc-1,wl+1);
	    } else {
	      sprintf(message,"\n%s: unknown command %s",node->name,wl[0]);
	      comMessage(message);
	      return -1;
	    }
	  }
    
  }

  if(c==0) {
    sprintf(message,"\n%s: not found: %s",node->name,rsub);
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
      sprintf(message,"\natom %d not found",anum);
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
      comMessage("\nparse error in subindexing");
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


int structSubComGet(dbmStructNode *node,struct STRUCT_ATOM *ap,int wc, char **wl)
{
  float v[3];
  char message[256];
  if(wc<=0) {
    sprintf(message,"\n%s: missing parameter for get",node->name);
    comMessage(message);
    return -1;
  }
  if(!strcmp(wl[0],"xyz")) {
    v[0]=ap->p->x;
    v[1]=ap->p->y;
    v[2]=ap->p->z;
    transApplyf(&node->transform,v);
    sprintf(message,"{%.3f,%.3f,%.3f}",v[0],v[1],v[2]);
  } else if(!strcmp(wl[0],"bfac")) {
    sprintf(message,"%.3f",ap->bfac);
  } else if(!strcmp(wl[0],"weight")) {
    sprintf(message,"%.3f",ap->weight);
  } else if(!strcmp(wl[0],"rname")) {
    sprintf(message,"%s",ap->residue->name);
  } else if(!strcmp(wl[0],"rtype")) {
    switch(ap->residue->type) {
    case STRUCT_RTYPE_HELIX: sprintf(message,"%s","helix"); break;
    case STRUCT_RTYPE_STRAND: sprintf(message,"%s","strand"); break;
    default: sprintf(message,"%s","coil"); break;
    }
  } else if(!strcmp(wl[0],"aname")) {
    sprintf(message,"%s",ap->name);
  } else if(!strcmp(wl[0],"ele")) {
    sprintf(message,"%s",ap->chem.element);
  } else if(!strcmp(wl[0],"anum")) {
    sprintf(message,"%d",ap->anum);
  } else if(!strcmp(wl[0],"chain")) {
    sprintf(message,"%s",ap->chain->name);
  } else if(!strcmp(wl[0],"model")) {
    sprintf(message,"%d",ap->model->num);
  } else if(!strcmp(wl[0],"rnum")) {
    sprintf(message,"%d",ap->residue->num);
  } else {
    sprintf(message,"\n%s:SubGet: unknown parameter %s",node->name,wl[0]);
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

  if(atom->restrict)
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

int structEvalAtomPOV(dbmStructNode *node, struct STRUCT_ATOM *atom,POV *pov)
{
  int i,j,vc,f,ret;
  int prop,rf,op;
  const char *v1,*v2;
  struct POV_VALUE *val;
  char message[256];
  float pos[3],dx,dy,dz,dist,dist2;

  // needed for cross-selection
  if(atom->restrict)
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
  else if(clStrcmp(pov->prop,"ele"))
    prop=STRUCT_SEL_ELE;
  else {
    sprintf(message,"\nerror: %s: unknown atom property %s",node->name, pov->prop);
    comMessage(message);
    return -1;
  }

  op=pov->op;

  if(!(op==POV_OP_EQ || op==POV_OP_LT || op==POV_OP_LE ||
       op==POV_OP_GT || op==POV_OP_GE || op==POV_OP_NE ||
       op==POV_OP_WI || op==POV_OP_OBJ)) {
    sprintf(message,"\nerror: invalid operator");
    comMessage(message);
    return -1;
  }

  if(pov->op==POV_OP_WI) {
    dist=atof(pov->prop);
    dist2=dist*dist;
  }


  vc=pov->val_count;
  for(i=0;i<vc;i++) {
    val=povGetVal(pov,i);
    if(val->range_flag) {
      if(op!=POV_OP_EQ) {
	sprintf(message,"\nerror: %s: expected operator = for range",node->name);
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
    switch(prop) {
    case STRUCT_SEL_OBJECT:
      if(rf) {
	comMessage("\nerror: range not supported for .object selection");
	return -1;
      }
      // identify object
      if(v1[0]!='.') {
	comMessage("\nerror: expected object to start with .");
	return -1;
      }
      v1++;
      v2=clStrchr(v1,'.');
      if(v2!=NULL) {
	comMessage("\nerror: .dataset.object selection not allowed");
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
	comMessage("\nerror: object ");
	comMessage(v1);
	comMessage(" not found");
	return -1;
      }
      break;
    case STRUCT_SEL_WITHIN:
      if(rf) {
	comMessage("\nerror: range not supported for < >");
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
    case STRUCT_SEL_ANUM:
      if(rf) {
	if(atom->anum>=atoi(v1) && atom->anum<=atoi(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(atom->anum==atoi(v1)) return 1; break;
	case POV_OP_NE: if(atom->anum!=atoi(v1)) return 1; break;
	case POV_OP_LT: if(atom->anum<atoi(v1)) return 1; break;
	case POV_OP_LE: if(atom->anum<=atoi(v1)) return 1; break;
	case POV_OP_GT: if(atom->anum>atoi(v1)) return 1; break;
	case POV_OP_GE: if(atom->anum>=atoi(v1)) return 1; break;
	}
      }
      break;
    case STRUCT_SEL_ANAME:
      if(rf) {
	comMessage("\nerror: range not allowed for aname");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("\nerror: expected operator = or != for aname");
	return -1;
      }
      if(rex(v1,atom->name))
	return 1;
      break;
    case STRUCT_SEL_RNUM:
      if(rf) {
	if(atom->residue->num>=atoi(v1) && atom->residue->num<=atoi(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(atom->residue->num==atoi(v1)) return 1; break;
	case POV_OP_NE: if(atom->residue->num!=atoi(v1)) return 1; break;
	case POV_OP_LT: if(atom->residue->num<atoi(v1)) return 1; break;
	case POV_OP_LE: if(atom->residue->num<=atoi(v1)) return 1; break;
	case POV_OP_GT: if(atom->residue->num>atoi(v1)) return 1; break;
	case POV_OP_GE: if(atom->residue->num>=atoi(v1)) return 1; break;
	}
      }
      break;
    case STRUCT_SEL_RNAME:
      if(rf) {
	comMessage("\nerror: range not allowed for rname");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("\nerror: expected operator = or != for rname");
	return -1;
      }
      if(rex(v1,atom->residue->name))
	return 1;
      break;
    case STRUCT_SEL_RTYPE:
      if(rf) {
	comMessage("\nerror: range not allowed for rtype");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("\nerror: expected operator = or != for rtype");
	return -1;
      }
      switch(atom->residue->type) {
      case STRUCT_RTYPE_HELIX: if(rex(v1,"helix")) return 1; break;
      case STRUCT_RTYPE_STRAND: if(rex(v1,"strand")) return 1; break;
      case STRUCT_RTYPE_COIL: if(rex(v1,"coil")) return 1; break;
      }
      break;
    case STRUCT_SEL_CLASS:
      if(rf) {
	comMessage("\nerror: range not allowed for class");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("\nerror: expected operator = or != for class");
	return -1;
      }
      switch(atom->residue->class) {
      case STRUCT_PROTEIN: if(rex(v1,"protein")) return 1; break;
      case STRUCT_NA: if(rex(v1,"na")) return 1; break;
      case STRUCT_MISC: if(rex(v1,"misc")) return 1; break;
      }
      break;
    case STRUCT_SEL_CHAIN:
      if(rf) {
	comMessage("\nerror: range not allowed for chain");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("\nerror: expected operator = or != for chain");
	return -1;
      }
      if(rex(v1,atom->chain->name))
	return 1;
      break;
    case STRUCT_SEL_MODEL:
      if(rf) {
	if(atom->model->num>=atoi(v1) && atom->model->num<=atoi(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(atom->model->num==atoi(v1)) return 1; break;
	case POV_OP_NE: if(atom->model->num!=atoi(v1)) return 1; break;
	case POV_OP_LT: if(atom->model->num<atoi(v1)) return 1; break;
	case POV_OP_LE: if(atom->model->num<=atoi(v1)) return 1; break;
	case POV_OP_GT: if(atom->model->num>atoi(v1)) return 1; break;
	case POV_OP_GE: if(atom->model->num>=atoi(v1)) return 1; break;
	}
      }
      break;
    case STRUCT_SEL_OCC:
      if(rf) {
	if(atom->weight>=atof(v1) && atom->weight<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(atom->weight==atof(v1)) return 1; break;
	case POV_OP_NE: if(atom->weight!=atof(v1)) return 1; break;
	case POV_OP_LT: if(atom->weight<atof(v1)) return 1; break;
	case POV_OP_LE: if(atom->weight<=atof(v1)) return 1; break;
	case POV_OP_GT: if(atom->weight>atof(v1)) return 1; break;
	case POV_OP_GE: if(atom->weight>=atof(v1)) return 1; break;
	}
      }
      break;
    case STRUCT_SEL_BFAC:
      if(rf) {
	if(atom->bfac>=atof(v1) && atom->bfac<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(atom->bfac==atof(v1)) return 1; break;
	case POV_OP_NE: if(atom->bfac!=atof(v1)) return 1; break;
	case POV_OP_LT: if(atom->bfac<atof(v1)) return 1; break;
	case POV_OP_LE: if(atom->bfac<=atof(v1)) return 1; break;
	case POV_OP_GT: if(atom->bfac>atof(v1)) return 1; break;
	case POV_OP_GE: if(atom->bfac>=atof(v1)) return 1; break;
	}
      }
      break;
    case STRUCT_SEL_ELE:
      if(rf) {
	comMessage("\nerror: range not allowed for ele");
	return -1;
      }
      if(!(op==POV_OP_EQ || op==POV_OP_NE)) {
	comMessage("\nerror: expected operator = or != for aname");
	return -1;
      }
      if(rex(v1,atom->chem.element))
	return 1;
      break;
    }
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
  atom_list=Ccalloc(atom_listm, sizeof(struct STRUCT_ATOM*));
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
	      atom_list=Ccalloc(atom_listm+100, sizeof(struct STRUCT_ATOM*));
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
  if(!strcmp(prop,"anum"))
    return (float)ap->anum;
  else if(!strcmp(prop,"bfac"))
    return ap->bfac;
  else if(!strcmp(prop,"weight"))
    return ap->weight;
  else if(!strcmp(prop,"rnum"))
    return (float)ap->residue->num;
  else if(!strcmp(prop,"model"))
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
  obj->atom_flag=Ccalloc(obj->node->atom_count,sizeof(unsigned char));
  obj->bond=NULL;
  obj->bond_count=0;
  obj->s_bond=NULL;
  obj->s_bond_count=0;
  obj->node=NULL;

  obj->render.show=1;
  obj->render.mode=RENDER_SIMPLE;
  obj->render.detail=5;
  obj->render.detail2=3;
  obj->render.nice=1;
  obj->render.line_width=1.0;
  obj->render.bond_width=0.2;
  obj->render.tube_ratio=1.0;
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
  obj->render.cgfx_flag=CGFX_INTPOL_COL;

  comSetDefMat(&obj->render.mat);
  obj->render.mat.spec[0]=0.6;
  obj->render.mat.spec[1]=0.6;
  obj->render.mat.spec[2]=0.6;
  obj->render.mat.spec[3]=1.0;
  obj->render.mat.shin=32;

  obj->render.stipple_flag=0;
  obj->render.stipplei=0.7;
  obj->render.stippleo=0.3;

  obj->va.p=NULL;

  obj->sphere_list=comGenLists(1);
  comNewDisplayList(obj->sphere_list);
  cgfxSphere(1.0,obj->render.detail);
  comEndDisplayList();

  obj->build=NULL;

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
  } else if(clStrcmp(prop,"weight")) {
    (*vmin)=n->min_max.weight1;
    (*vmax)=n->min_max.weight2;
  } else if(clStrcmp(prop,"bfac")) {
    (*vmin)=n->min_max.bfac1;
    (*vmax)=n->min_max.bfac2;
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
    if(!strcmp(wl[i],"-type") ||
       !strcmp(wl[i],"-t")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing parameter for -type");
	comMessage(message);
	return -1;
      }
      strcpy(type,wl[++i]);
    } else if(!strcmp(wl[i],"-swap")) {
      swap_flag=1;
    } else {
      sprintf(message,"\nunknow option %s",wl[i]);
      comMessage(message);
      return -1;
    }
  }

  if(!strcmp(ext,"gz")) {
    cmp=1;
    bp=strrchr(base,'.');
    if(bp!=NULL)
      bp[0]='\0';
    strcpy(ext,bp+1);
    sprintf(gunzip,"gunzip < %s",filename);
    if((f=popen(gunzip,"r"))==NULL) {
      sprintf(message,"\nError while piping %s",filename);
      comMessage(message);
      return -1;
    }
  } else {
    cmp=0;
    if((f=fopen(filename,"r"))==NULL) {
      sprintf(message,"\nError opening %s",filename);
      comMessage(message);
      return -1;
    }
  }

  if(strlen(type)==0) {
    if(!strcmp(ext,"trj")) {
      id=STRUCT_TRJ_CHARMM; /* default */
    } else if(!strcmp(ext,"crd")) {
      id=STRUCT_TRJ_CNS; /* default */
    } else if(!strcmp(ext,"dtrj")) {
      id=STRUCT_TRJ_DINO;
    } else if(!strcmp(ext,"binpos")) {
      id=STRUCT_TRJ_BINPOS;
    } else {
      sprintf(message,"\nunknown extension %s, please specify type",ext);
      comMessage(message);
      if(cmp) pclose(f); else fclose(f);
      return -1;
    }
  } else if(!strcmp(type,"charmm")) {
    id=STRUCT_TRJ_CHARMM;
  } else if(!strcmp(type,"xplor")) {
    id=STRUCT_TRJ_XPLOR;
  } else if(!strcmp(type,"cns")) {
    id=STRUCT_TRJ_CNS;
  } else if(!strcmp(type,"dino")) {
    id=STRUCT_TRJ_DINO;
  } else if(!strcmp(type,"binpos")) {
    id=STRUCT_TRJ_BINPOS;
  } else {
    sprintf(message,"\nunknown type %s",type);
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
  } else {
    sprintf(message,"\nthis type is not yet supported");
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

int structWrite(struct DBM_STRUCT_NODE *node, structObj *obj, int wc, char **wl)
{
  int i,ac;
  char message[256];
  char type[64];
  char file[256];
  char name[64];
  char base[256],*bp;
  char ext[256];
  char record[16];
  int tid;
  FILE *f;
  time_t t;
  char *c;
  struct STRUCT_ATOM *ap;
  char *chain,spc[]="     ";
  char aname[32];

  if(wc<=0) {
    sprintf(message,"\nwrite: missing filename");
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
    if(!strcmp(wl[i],"-type") ||
       !strcmp(wl[i],"-t")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing parameter for %s",wl[i]);
	comMessage(message);
	return -1;
      }
      strcpy(type,wl[i+1]);
      i+=2;
    } else {
      sprintf(message,"\nunknown option %s",wl[i]);
      comMessage(message);
      return -1;
    }
  }

  if(strlen(type)==0) {
    if(!strcmp(ext,"pdb")) {
      tid=STRUCT_WRITE_PDB;
    } else if(!strcmp(ext,"xpl")) {
      tid=STRUCT_WRITE_XPL;
    } else if(!strcmp(ext,"crd")) {
      tid=STRUCT_WRITE_CRD;
    } else if(!strcmp(ext,"xyzr")) {
      tid=STRUCT_WRITE_XYZR;
    } else {
      sprintf(message,"\nunknown extension %s, please specify type",ext);
      comMessage(message);
      return -1;
    }
  } else {
    if(!strcmp(type,"pdb")) {
      tid=STRUCT_WRITE_PDB;
    } else if(!strcmp(type,"xplorc") ||
	      !strcmp(type,"cnsc")) {
      tid=STRUCT_WRITE_XPL;
    } else if(!strcmp(type,"charmm")){
      tid=STRUCT_WRITE_CRD;
    } else if(!strcmp(type,"xyzr")){
      tid=STRUCT_WRITE_XYZR;
    } else {
      sprintf(message,"\nunknown type %s",type);
      comMessage(message);
      return -1;
    }
  }


  /*
    now that the type is set,
    open file
  */
  if((f=fopen(file,"w"))==NULL) {
    sprintf(message,"\ncannot open %s",file);
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
  
  ac=0;
  while(-1) {
    if(obj==NULL)
      ap=&node->atom[ac];
    else
      ap=obj->atom[ac].ap;

    strcpy(record,"HETATM");
    for(i=0;i<record_atom_count;i++)
      if(!strcmp(record_atom[i],ap->residue->name)) {
	strcpy(record,"ATOM  ");
	break;
      }

    memset(aname,0,sizeof(aname));
    if(strlen(ap->chem.element)==2) {
      sprintf(aname,"%s",ap->name);
    } else {
      if(!strcmp(ap->chem.element,"H")) {
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
	      ap->p->x,ap->p->y,ap->p->z,ap->weight,ap->bfac);
      break;
    case STRUCT_WRITE_XPL:
      fprintf(f,"%6s%5d %-4s %3s  %4d    %8.3f%8.3f%8.3f%6.2f%6.2f   %4s\n",
	      record,ac+1,aname,ap->residue->name,ap->residue->num,
	      ap->p->x,ap->p->y,ap->p->z,ap->weight,ap->bfac,chain);
      break;
    case STRUCT_WRITE_CRD:
      fprintf(f,"%5d %4d %3s  %-4s%10.5f%10.5f%10.5f %c  %4d   %9.5f\n",
	      ap->anum,ap->residue->num,ap->residue->name,ap->name,
	      ap->p->x,ap->p->y,ap->p->z,chain[0],ap->residue->num,ap->weight);
      break;
    case STRUCT_WRITE_XYZR:
      fprintf(f,"%.3f %.3f %.3f %.3f\n",
	      ap->p->x,ap->p->y,ap->p->z,ap->chem.vdwr);
      
      break;
    }
    ac++;
    if(obj==NULL) {
      if(ac>=node->atom_count)
	break;
    } else {
      if(ac>=obj->atom_count)
	break;
    }
  }

  if(tid==STRUCT_WRITE_PDB) {
    fprintf(f,"END");
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
    sprintf(message,"\n%s: missing parameter",node->name);
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
    if(!strcmp(wl[i],"-wait") ||
       !strcmp(wl[i],"-w")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing value for -wait");
	comMessage(message);
	return -1;
      }
      node->play.wait=atoi(wl[++i]);
    } else if(!strcmp(wl[i],"-delay") ||
	      !strcmp(wl[i],"-d")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing value for -delay");
	comMessage(message);
	return -1;
      }
      node->play.delay=atoi(wl[++i]);
    } else if(!strcmp(wl[i],"-begin") ||
	      !strcmp(wl[i],"-b")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing value for -begin");
	comMessage(message);
	return -1;
      }
      node->play.start=atoi(wl[++i]);
      if(node->play.start<0 || node->play.start>node->trj.frame_count) {
	sprintf(message,"\nbegin out of bounds");
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(wl[i],"-end") ||
	      !strcmp(wl[i],"-e")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing value for -end");
	comMessage(message);
	return -1;
      }
      node->play.end=atoi(wl[++i]);
      if(node->play.end<0 || node->play.end>=node->trj.frame_count) {
	sprintf(message,"\nend out of bounds");
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(wl[i],"-step") ||
	      !strcmp(wl[i],"-s")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing value for -end");
	comMessage(message);
	return -1;
      }
      node->play.step=atoi(wl[++i]);
      if(node->play.step==0) {
	sprintf(message,"\nstep must be nonzero");
	comMessage(message);
	return -1;
      }
      if(abs(node->play.step)>=node->trj.frame_count) {
	sprintf(message,"\nstep larger than trajectory");
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(wl[i],"-mode") ||
	      !strcmp(wl[i],"-m")) {
      if(i+1>=wc) {
	sprintf(message,"\nmissing value for -mode");
	comMessage(message);
	return -1;
      }
      i++;
      if(!strcmp(wl[i],"loop")) {
	node->play.mode=STRUCT_PLAY_LOOP;
      } else if(!strcmp(wl[i],"rock")) {
	node->play.mode=STRUCT_PLAY_ROCK;
      } else if(!strcmp(wl[i],"single")) {
	node->play.mode=STRUCT_PLAY_SINGLE;
      } else {
	sprintf(message,"\nunknown mode %s",wl[i]);
	comMessage(message);
	return -1;
      }
    } else {
      sprintf(message,"\nunknown parameter %s",wl[i]);
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
  if(prop==NULL) {
    comMessage("\nerror: range: prop missing in range statement");
    (*r)=0.0;
    return -1;
  }
  if(clStrcmp(prop,"anum")) {
    (*r)=(float)atom->anum;
  } else if(clStrcmp(prop,"rnum")) {
    (*r)=(float)atom->residue->num;
  } else if(clStrcmp(prop,"model")) {
    (*r)=(float)atom->model->num;
  } else if(clStrcmp(prop,"occ")) {
    (*r)=atom->weight;
  } else if(clStrcmp(prop,"bfac")) {
    (*r)=atom->bfac;
  } else if(clStrcmp(prop,"x")) {
    (*r)=atom->p->x;
  } else if(clStrcmp(prop,"y")) {
    (*r)=atom->p->y;
  } else if(clStrcmp(prop,"z")) {
    (*r)=atom->p->z;
  } else {
    comMessage("\nerror: range: unknown atom property ");
    comMessage(prop);
    (*r)=0.0;
    return -1;
  }
  return 0;
}

int structGetRangeXYZVal(dbmStructNode *node, const char *prop, float *p, float *r)
{
  comMessage("\nnot implemented");
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

