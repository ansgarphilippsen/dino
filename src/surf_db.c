#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com.h"
#include "dbm.h"
#include "surf_db.h"
#include "surf_obj.h"
#include "mat.h"
#include "render.h"
#include "Cmalloc.h"
#include "cl.h"

int surfNewNode(dbmSurfNode *node)
{
  node->restriction=NULL;

  node->attach_cutoff=4.0;
  node->attach_flag=0;
  node->last_attach=NULL;

  node->v=NULL;
  node->vc=0;
  node->f=NULL;
  node->fc=0;

  node->obj_max=64;
  node->obj=Ccalloc(64,sizeof(surfObj));
  node->obj_flag=Ccalloc(64,sizeof(unsigned char));
  memset(node->obj_flag,0,sizeof(unsigned char)*64);

  node->smode=SURF_SMODE_ALL;

  transReset(&node->transform);
  node->curr_transform = &node->transform;
  
  return 0;
}

int surfCommand(struct DBM_SURF_NODE *node,int wc, char **wl)
{
  int i;
  char message[256];
  char *empty_com[]={"get","center"};

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }

  if(clStrcmp(wl[0],"?") ||
     clStrcmp(wl[0],"help")) {
  } else if(clStrcmp(wl[0],"new")) {
    return surfComNew(node, wc-1, wl+1);
  } else if(clStrcmp(wl[0],"set")) {
    return surfComSet(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"get")) {
    return surfComGet(node,wc-1, wl+1);
  } else if(clStrcmp(wl[0],"restrict")) {
    return surfComRestrict(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"attach")) {
    return surfComAttach(node, wc-1, wl+1);
  } else if(clStrcmp(wl[0],"cotrans")) {
    return surfComCotrans(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"delete") ||
	    clStrcmp(wl[0],"del")) {
    return surfComDel(node,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"grab")) {
    if(wc!=2) {
      comMessage("syntax: grab device\n");
      return -1;
    }
    if(comGrab(&node->transform,0,0,wl[1])<0)
      return -1;
    // set center to current center
    comGetCurrentCenter(node->transform.cen);
  } else if(clStrcmp(wl[0],"fix")) {
    if(wc>1)
      comMessage("warning: ignored superfluous words after fix\n");

    surfFix(node);
    comRedraw();
  } else if(clStrcmp(wl[0],"reset")) {
    if(wc==1) {
      transReset(&node->transform);
    } else {
      for(i=1;i<wc;i++) {
	if(clStrcmp(wl[i],"all")) {
	  transReset(&node->transform);
	  surfRecenter(node);
	} else if(clStrcmp(wl[i],"center")) {
	  surfRecenter(node);
	} else if(clStrcmp(wl[i],"rot")) {
	  transResetRot(&node->transform);
	} else if(clStrcmp(wl[i],"trans")) {
	  transResetTra(&node->transform);
	}
      }
    }
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
  } else {
    sprintf(message,"unknown command: %s\n", wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}


int surfComNew(dbmSurfNode *node,int wc, char **wl)
{
  int i,ret;
  char *name;
  int type;
  Set set;
  Select sel;
  clOrg co;
  int set_flag,sel_flag;
  char message[256];
  int vflag=0;

  set_flag=sel_flag=0;
  name=node->name;
  type=SURF_NORMAL;

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
	if(clStrcmp(co.param[i].wl[0],"default")) {
	  type=SURF_NORMAL;
	} else {
	  clStrcpy(message,"error: new: unknown type \n");
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
    } else if(clStrcmp(co.param[i].p,"v")) {
      vflag=1;
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
  
  ret=surfNew(node, name,type,&set,&sel,vflag);

  setDelete(&set);
  clDelete(&co);

  return ret;
}

int surfComSet(dbmSurfNode *node,int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;

  ret=surfSet(node, &set);
  
  setDelete(&set);

  return ret;

}

int surfComGet(dbmSurfNode *node,int wc, char **wl)
{
  int i;

  if(wc==0) {
    comMessage("error: get: missing property\n");
    return -1;
  }
  for(i=0;i<wc;i++)
    if(surfGet(node, wl[i])<0)
      return -1;
  return 0;
}

int surfComDel(dbmSurfNode *node,int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"%s: missing parameter\n",node->name);
    comMessage(message);
    return -1;
  } else {
    for(i=0;i<wc;i++)
      if(surfDelObj(node,wl[i])<0)
	return -1;
  }
  comRedraw();
  return 0;
}

int surfComAttach(dbmSurfNode *node, int wc, char **wl)
{
  float cutoff;
  int i;
  char message[256];

  if(wc==0) {
    return surfAttach(node,NULL,0);
  }
  if(clStrcmp(wl[0],"none") ||
     clStrcmp(wl[0],"off")) {
    return surfAttach(node,NULL,0);
  }
  if(!comIsDB(wl[0])) {
    sprintf(message,"unknown dataset %s\n",wl[0]);
    comMessage(message);
    return -1;
  }
  cutoff=4.0;
  for(i=1;i<wc;i++)
    if(clStrcmp(wl[i],"-i")) {
      comMessage("info: attach: -i no longer supported, ignored\n");
    } else if(clStrcmp(wl[i],"-cutoff") ||
	      clStrcmp(wl[i],"-co")) {
      if(i+1>=wc) {
	sprintf(message,"distance missing for -cutoff\n");
	comMessage(message);
	return -1;
      } else {
	cutoff=atof(wl[++i]);
      }
    } else {
      sprintf(message,"unknown parameter %s\n",wl[i]);
      comMessage(message);
      return -1;
    }
  node->attach_cutoff=cutoff;
  return surfAttach(node,comGetDB(wl[0]),0);
}

int surfComCotrans(dbmSurfNode *sn,int wc, char **wl)
{
  char message[256];
  dbmNode *nn;

  if(wc<1) {
    comMessage("required dataset as argument to cotrans\n");
    return -1;
  }
  if(clStrcmp(wl[0],"none") ||
     clStrcmp(wl[0],"off")) {
    // copy cotrans matrix to current one
    transCopy(sn->curr_transform,&sn->transform);
    // reset to own transformation matrix
    sn->curr_transform=&sn->transform;
  }
  nn = comGetDB(wl[0]);
  if(nn==NULL) {
    sprintf(message,"unknown dataset %s\n",wl[0]);
    comMessage(message);
    return -1;
  }
  sn->curr_transform = &nn->common.transform;
  return 0;
}

int surfComRestrict(dbmSurfNode *node, int wc, char **wl)
{
  int i,ret,c;
  Select sel;
  char message[256];

  if(selectNew(&sel, wc,wl)<0)
    return -1;

  // reset restriction !!!
  for(i=0;i<node->vc;i++)
    node->v[i].restriction=0;

  // check each atom
  c=0;
  for(i=0;i<node->vc;i++) {
    ret=surfIsSelected(node, &node->v[i], &sel);
    if(ret<0) {
      selectDelete(&sel);
      return -1;
    }
    if(ret==0) {
      node->v[i].restriction=1;
      c++;
   }
  }

  // output how many atoms were flaged
  if(c==0)
    sprintf(message,"all vertices unrestricted\n");
  else if(c==node->vc)
    sprintf(message,"restriction affects ALL of %d vertices\n",node->vc);
  else
    sprintf(message,"restriction affects %d of %d vertices\n",c,node->vc);
  comMessage(message);

  selectDelete(&sel);
  return 0;
}


int surfNew(dbmSurfNode *node, char *name, int type, Set *set, Select *sel,int vflag)
{
  surfObj *obj;

  if((obj=surfNewObj(node, name))==NULL) {
    comMessage("error: new: internal error");
    return -1;
  }
  obj->type=type;
  obj->node=node;

  obj->list=comGenLists(1);

  memcpy(&obj->select,sel,sizeof(Select));

  return surfObjRenew(obj, set, sel,vflag);
}

int surfSet(dbmSurfNode *node, Set *s)
{
  int pc;
  struct POV_VALUE *val;
  int i,rt,op;
  Select *sel;
  float r,g,b,v1[6];
  char message[256];

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
      s->pov[pc].id=SURF_PROP_ROT;
    } else if (clStrcmp(s->pov[pc].prop,"trans")) {
      s->pov[pc].id=SURF_PROP_TRANS;
    } else if (clStrcmp(s->pov[pc].prop,"rtc")) {
      s->pov[pc].id=SURF_PROP_RTC;
    } else if ( clStrcmp(s->pov[pc].prop,"center") ||
		clStrcmp(s->pov[pc].prop,"rcen")) {
      s->pov[pc].id=SURF_PROP_RCEN;
    } else if (clStrcmp(s->pov[pc].prop,"smode")) {
      s->pov[pc].id=SURF_PROP_SMODE;
    } else {
      comMessage("error: set: unknown property \n");
      comMessage(s->pov[pc].prop);
      return -1;
    }
    if(s->pov[pc].op!=POV_OP_EQ) {
      comMessage("error: set: expected operator = for property \n");
      comMessage(s->pov[pc].prop);
      return -1;
    }
    op=s->pov[pc].op;
    if(s->pov[pc].val_count>1) {
      comMessage("error: set: expected only one value for property \n");
      comMessage(s->pov[pc].prop);
      return -1;
    }
  }

  for(pc=0;pc<s->pov_count;pc++) {
    val=povGetVal(&s->pov[pc],0);
    switch(s->pov[pc].id) {
    case SURF_PROP_SMODE:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property smode\n");
	return -1;
      }
      if(clStrcmp(val->val1,"all")) {
	node->smode=SURF_SMODE_ALL;
      } else if(clStrcmp(val->val1,"any")) {
	node->smode=SURF_SMODE_ANY;
      } else {
	sprintf(message,"error: smode must be one of 'any' 'all'\n");
	comMessage(message);
	return -1;
      }
      break;
    case SURF_PROP_ROT:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rot\n");
	return -1;
      }
      if(transSetRot(&node->transform,val->val1)<0)
	return -1;
      break;
    case SURF_PROP_TRANS:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property trans\n");
	return -1;
      }
      if(transSetTra(&node->transform,val->val1)<0)
	return -1;
      break;
    case SURF_PROP_RTC:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rtc\n");
	return -1;
      }
      if(transSetAll(&node->transform,val->val1)<0)
	return -1;

      break;
    case SURF_PROP_RCEN:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rcen\n");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v1)!=0) {
	comMessage("error in vector: \n");
	comMessage(val->val1);
	return -1;
      }   
      transApplyIf(&node->transform, v1);
      node->transform.cen[0]=v1[0];
      node->transform.cen[1]=v1[1];
      node->transform.cen[2]=v1[2];
      node->transform.cen[3]=1.0;
      break;
    }
  }
  return 0;
}

int surfGet(dbmSurfNode *node, char *prop)
{
  char message[256];
  int i;
  float v[3];

  if(clStrcmp(prop,"center")) {
    v[0]=node->transform.cen[0];
    v[1]=node->transform.cen[1];
    v[2]=node->transform.cen[2];
    transApplyf(&node->transform,v);
    sprintf(message,"{%.5f,%.5f,%.5f}",v[0],v[1],v[2]);
    comReturn(message);
  } else if(clStrcmp(prop,"smode")) {
    if(node->smode==SURF_SMODE_ALL)
      strcpy(message,"all");
    else
      strcpy(message,"any");
    comReturn(message);
  } else if(clStrcmp(prop,"rot")) {
    comReturn(transGetRot(&node->transform));
  } else if(clStrcmp(prop,"trans")) {
    comReturn(transGetTra(&node->transform));
  } else if(clStrcmp(prop,"rtc")) {
    comReturn(transGetAll(&node->transform));
  } else {
    sprintf(message,"%s: unknown property %s\n",node->name, prop);
    comMessage(message);
    return -1;
  }
  return 0;
}



surfObj *surfNewObj(dbmSurfNode *node, char *name)
{
  int i;
  surfObj *obj;

  surfDelObj(node,name);
  for(i=0;i<node->obj_max;i++) {
    if(node->obj_flag[i]==0) {
      node->obj_flag[i]=1;
      obj=&node->obj[i];
      surfSetDefault(node, obj);
      clStrcpy(obj->name,name);
      comNewObj(node->name, name);
      return obj;
    }
  }
  
  /* catch and increase obj_max */

  return NULL;
}

int surfDelObj(dbmSurfNode *node,char *name)
{
  int i;
  surfObj *obj;
	
  for(i=0;i<node->obj_max;i++)
    if(node->obj_flag[i]!=0)
      if(clStrcmp(node->obj[i].name,name)) {
	node->obj_flag[i]=0;
	obj=&node->obj[i];
	// TODO CFREE !!
	comDelObj(node->name, name);
      }
  return 0;
}



int surfIsSelected(dbmSurfNode *node, struct SURF_VERTICE *vert, Select *sel)
{
  int i,ec,ret;

  if(vert->restriction)
    return 0;

  if(sel==NULL)
    return 1;

  ec=selectGetPOVCount(sel);
  for(i=0;i<ec;i++) {
    ret=surfEvalPOV(node,vert,selectGetPOV(sel, i));
    if(ret<0)
      return -1;
    selectSetResult(sel, i, ret);
  }  
  return selectResult(sel);
}

int surfEvalPOV(dbmSurfNode *node, struct SURF_VERTICE *vert, POV *pov)
{
  int prop,op,i,j,vc,f,ret,rf;
  float dist,dist2,pos[3],dx,dy,dz;
  struct POV_VALUE *val;
  const char *v1,*v2;
  char message[256];
  char db_s[1024],*obj_s;
  int cpropn;

  if(clStrcmp(pov->prop,"*"))
    return 1;
  
  if(pov->op==POV_OP_WI) {
    prop=SURF_SEL_WITHIN;
  } else if(pov->op==POV_OP_OBJ) {
    prop=SURF_SEL_OBJECT;
  } else if(clStrcmp(pov->prop,"x")) {
    prop=SURF_SEL_X;
  } else if(clStrcmp(pov->prop,"y")) {
    prop=SURF_SEL_Y;
  } else if(clStrcmp(pov->prop,"z")) {
    prop=SURF_SEL_Z;
  } else if(clStrcmp(pov->prop,"curv")) {
    prop=SURF_SEL_CURV;
  } else if(clStrcmp(pov->prop,"cp0") ||
	    clStrcmp(pov->prop,"cp1") ||
	    clStrcmp(pov->prop,"cp2") ||
	    clStrcmp(pov->prop,"cp3") ||
	    clStrcmp(pov->prop,"cp4") ||
	    clStrcmp(pov->prop,"cp5") ||
	    clStrcmp(pov->prop,"cp6") ||
	    clStrcmp(pov->prop,"cp7") ||
	    clStrcmp(pov->prop,"cp8") ||
	    clStrcmp(pov->prop,"cp9")) {
    prop=SURF_SEL_CP;
    cpropn=atoi(pov->prop+2);
  } else if (clStrcmp(pov->prop,"anum") ||
	     clStrcmp(pov->prop,"aname") ||
	     clStrcmp(pov->prop,"rnum") ||
	     clStrcmp(pov->prop,"rtype") ||
	     clStrcmp(pov->prop,"rname") ||
	     clStrcmp(pov->prop,"class") ||
	     clStrcmp(pov->prop,"chain") ||
	     clStrcmp(pov->prop,"model") ||
	     clStrcmp(pov->prop,"occ") ||
	     clStrcmp(pov->prop,"weight") ||
	     clStrcmp(pov->prop,"bfac") ||
	     clStrcmp(pov->prop,"ele")) {
      
    if(!node->attach_flag) {
      sprintf(message,"error: %s: need attached structure for property %s\n",
	      node->name, pov->prop);
      comMessage(message);
      return -1;
    }
    if(vert->attach_node==NULL) {
      return 0;
    } else {
      return structEvalAtomPOV(&vert->attach_node->structNode,
			       &vert->attach_node->structNode.atom[vert->attach_element],
			  pov);
    }
  } else {
    sprintf(message,"error: %s: unknown property %s\n",node->name, pov->prop);
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
    switch(prop) {
    case SURF_SEL_OBJECT:
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
	/* .dataset.object -> use attached datasets */
	if(!node->attach_flag) {
	  comMessage("no datasets attached for .dataset.object selection\n");
	  return -1;
	}
	strncpy(db_s,v1,1023);
	obj_s=strchr(db_s,'.');
	obj_s[0]='\0';
	obj_s++;
	if(rex(db_s,vert->attach_node->structNode.name)) {
	  ret=dbmIsElementInObj(db_s,obj_s, vert->attach_element);
	  if(ret<0)
	    return -1;
	  if(ret>0)
	    return 1;
	} else {
	  /* ignore */
	}
      } else {
	f=0;
	for(j=0;j<node->obj_max;j++) {
	  if(node->obj_flag[j]!=0) {
	    if(rex(v1,node->obj[j].name)) {
	      f++;
	      if(node->obj[j].vert_flag[vert->num])
		return 1;
	    }
	  }
	}
	if(!f) {
	  comMessage("error: object \n");
	  comMessage(v1);
	  comMessage(" not found");
	  return -1;
	}
      }
      break;
    case SURF_SEL_WITHIN:
      if(rf) {
	comMessage("error: range not supported for < >\n");
	return -1;
      }
      if(val->wi_flag) {
	pos[0]=vert->p[0];
	pos[1]=vert->p[1];
	pos[2]=vert->p[2];
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
	dx=vert->p[0]-val->vect[0];
	dy=vert->p[1]-val->vect[1];
	dz=vert->p[2]-val->vect[2];
	if((dx*dx+dy*dy+dz*dz)<dist2)
	  return 1;
      }
      break;
    case SURF_SEL_X:
      if(rf) {
	if(vert->p[0]>=atof(v1) && vert->p[0]<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vert->p[0]==atof(v1)) return 1; break;
	case POV_OP_NE: if(vert->p[0]!=atof(v1)) return 1; break;
	case POV_OP_LT: if(vert->p[0]<atof(v1)) return 1; break;
	case POV_OP_LE: if(vert->p[0]<=atof(v1)) return 1; break;
	case POV_OP_GT: if(vert->p[0]>atof(v1)) return 1; break;
	case POV_OP_GE: if(vert->p[0]>=atof(v1)) return 1; break;
	}
      }
      break;
    case SURF_SEL_Y:
      if(rf) {
	if(vert->p[1]>=atof(v1) && vert->p[1]<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vert->p[1]==atof(v1)) return 1; break;
	case POV_OP_NE: if(vert->p[1]!=atof(v1)) return 1; break;
	case POV_OP_LT: if(vert->p[1]<atof(v1)) return 1; break;
	case POV_OP_LE: if(vert->p[1]<=atof(v1)) return 1; break;
	case POV_OP_GT: if(vert->p[1]>atof(v1)) return 1; break;
	case POV_OP_GE: if(vert->p[1]>=atof(v1)) return 1; break;
	}
      }
      break;
    case SURF_SEL_Z:
      if(rf) {
	if(vert->p[2]>=atof(v1) && vert->p[2]<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vert->p[2]==atof(v1)) return 1; break;
	case POV_OP_NE: if(vert->p[2]!=atof(v1)) return 1; break;
	case POV_OP_LT: if(vert->p[2]<atof(v1)) return 1; break;
	case POV_OP_LE: if(vert->p[2]<=atof(v1)) return 1; break;
	case POV_OP_GT: if(vert->p[2]>atof(v1)) return 1; break;
	case POV_OP_GE: if(vert->p[2]>=atof(v1)) return 1; break;
	}
      }
      break; 
    case SURF_SEL_CURV:
      if(rf) {
	if(vert->curv>=atof(v1) && vert->curv<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vert->curv==atof(v1)) return 1; break;
	case POV_OP_NE: if(vert->curv!=atof(v1)) return 1; break;
	case POV_OP_LT: if(vert->curv<atof(v1)) return 1; break;
	case POV_OP_LE: if(vert->curv<=atof(v1)) return 1; break;
	case POV_OP_GT: if(vert->curv>atof(v1)) return 1; break;
	case POV_OP_GE: if(vert->curv>=atof(v1)) return 1; break;
	}
      }
      break;
    case SURF_SEL_CP:
      if(rf) {
	if(vert->cprop[cpropn]>=atof(v1) && vert->cprop[cpropn]<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vert->cprop[cpropn]==atof(v1)) return 1; break;
	case POV_OP_NE: if(vert->cprop[cpropn]!=atof(v1)) return 1; break;
	case POV_OP_LT: if(vert->cprop[cpropn]<atof(v1)) return 1; break;
	case POV_OP_LE: if(vert->cprop[cpropn]<=atof(v1)) return 1; break;
	case POV_OP_GT: if(vert->cprop[cpropn]>atof(v1)) return 1; break;
	case POV_OP_GE: if(vert->cprop[cpropn]>=atof(v1)) return 1; break;
	}
      }
      break;
    }
  }

  return 0;
}

int surfAttach(dbmSurfNode *node, dbmNode *attach, int iflag)
{
  register int vc;
  double dx,dy,dz,dx2,dy2,dz2,px,py,pz,dist,ndist,max_dist;
  struct STRUCT_ATOM *ap,*cap;
  struct SURF_VERTICE *vert;

  float v_xyz1[3],v_xyz2[3];
  int abc1[3],abc2[3],a_abc[3];
  int a_cac;
  cubeArray *a_ca;
  caPointer *ca_p;
  int acount;
  float limit;

  int i;


  if(attach==NULL) {
    comMessage("de-attaching ...");
    node->attach_flag=0;
    node->last_attach=NULL;
    for(i=0;i<node->vc;i++)
      node->v[i].attach_node=NULL;
    comMessage(" done\n");
    return 0;
  } else {
    node->attach_flag=1;
  }

  if(attach->common.type!=DBM_NODE_STRUCT) {
    comMessage("invalid node type for attach: must be a structure db\n");
    return -1;
  }

  max_dist=node->attach_cutoff*node->attach_cutoff;

  comMessage("attaching ...");
  //  restrict=&attach->structNode.restrict;

  node->last_attach=attach;

  limit=node->attach_cutoff;
  a_ca=attach->structNode.ca;
  for(vc=0;vc<node->vc;vc++) {
    vert=node->v+vc;
    px=vert->p[0];
    py=vert->p[1];
    pz=vert->p[2];
    ap=NULL;
    dist=max_dist;
    v_xyz1[0]=px-limit;
    v_xyz1[1]=py-limit;
    v_xyz1[2]=pz-limit;
    v_xyz2[0]=px+limit;
    v_xyz2[1]=py+limit;
    v_xyz2[2]=pz+limit;
    caXYZtoABC(a_ca,v_xyz1,abc1);
    caXYZtoABC(a_ca,v_xyz2,abc2);
    
    for(a_abc[0]=abc1[0];a_abc[0]<=abc2[0];a_abc[0]++)
      for(a_abc[1]=abc1[1];a_abc[1]<=abc2[1];a_abc[1]++)
	for(a_abc[2]=abc1[2];a_abc[2]<=abc2[2];a_abc[2]++) {
	  caGetList(a_ca, a_abc, &ca_p, &acount);
	  for(a_cac=0;a_cac<acount;a_cac++) {
	    cap=ca_p[a_cac];
	    dx=cap->p->x-px;
	    dx2=dx*dx;
	    if(dx2<=dist) {
	      dy=cap->p->y-py;
	      dy2=dy*dy;
	      if(dy2<=dist) {
		dz=cap->p->z-pz;
		dz2=dz*dz;
		if(dz2<=dist) {
		  ndist=dx2+dy2+dz2;
		  if(ndist<dist) {
		    if(!cap->restriction) {
		      dist=ndist;
		      ap=cap;
		    }
		  }
		}
	      }
	      
	    }
	  }
	}
    //    vert->ap=ap;
    if(ap!=NULL) {
      vert->attach_node=attach;
      vert->attach_element=ap->n;
    } else {
      /**
	 THIS SHOULD NOT BE HERE !
	 vert->attach_node=NULL;
	 vert->attach_element=0;
      **/
    }
  }
  
  comMessage(" done\n");
  return 0;
}

int surfBuildCA(dbmSurfNode *n)
{
  int i,c,mode;
  float xmin,xmax,ymin,ymax,zmin,zmax;
  float xyz1[3],xyz2[3],xyz[3];
  int abc[3];

  xmin=n->v[0].p[0];
  xmax=n->v[0].p[0];
  ymin=n->v[0].p[1];
  ymax=n->v[0].p[1];
  zmin=n->v[0].p[2];
  zmax=n->v[0].p[2];

  for(i=1;i<n->vc;i++) {
    xmin = (n->v[i].p[0] < xmin) ? n->v[i].p[0] : xmin;
    xmax = (n->v[i].p[0] > xmax) ? n->v[i].p[0] : xmax;
    ymin = (n->v[i].p[1] < ymin) ? n->v[i].p[1] : ymin;
    ymax = (n->v[i].p[1] > ymax) ? n->v[i].p[1] : ymax;
    zmin = (n->v[i].p[2] < zmin) ? n->v[i].p[2] : zmin;
    zmax = (n->v[i].p[2] > zmax) ? n->v[i].p[2] : zmax;
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
    for(i=0;i<n->vc;i++) {
      xyz[0]=n->v[i].p[0];
      xyz[1]=n->v[i].p[1];
      xyz[2]=n->v[i].p[2];
      caXYZtoABC(n->ca,xyz,abc);
      caAddPointer(n->ca,abc,(caPointer)&n->v[i],mode);
    }
  }

  return 0;
}

int surfSetDefault(dbmSurfNode *node, surfObj *obj)
{
  obj->type=SURF_NORMAL;
  obj->name[0]='\0';
  obj->node=node;

  obj->render.show=1;
  obj->render.mode=RENDER_SURFACE;
  obj->render.detail1=3;
  obj->render.detail2=3;
  obj->render.nice=1;
  obj->render.line_width=1.0;
  obj->render.point_size=1.0;
  obj->render.transparency=1.0;
  obj->render.dbl_light=0;
  obj->render.face_reverse=0;          
  obj->render.cull=0;
  obj->render.solid=0;
  obj->render.solidc[0]=1.0;
  obj->render.solidc[1]=1.0;
  obj->render.solidc[2]=1.0;
  comSetDefMat(&obj->render.mat);

  obj->r=1.0;
  obj->g=1.0;
  obj->b=1.0;

  obj->face=NULL;
  obj->facec=0;

  obj->vert=NULL;
  obj->vertc=0;

  obj->vert_flag=Ccalloc(node->vc+16,sizeof(unsigned char));

  return 0;
}

int surfGetRangeVal(dbmSurfNode *node, struct SURF_VERTICE *v, const char *prop, float *rval)
{
  //float *cp=comGetCP();
  float cp[3];
  cp[0] = node->transform.cen[0];
  cp[1] = node->transform.cen[1];
  cp[2] = node->transform.cen[2];

  (*rval)=0.0;
  if(prop==NULL)
    return -1;
  if(clStrlen(prop)==0)
    return -1;

  if(clStrcmp(prop,"cp0")) {
    (*rval)=v->cprop[0];
  } else if(clStrcmp(prop,"cp1")) {
    (*rval)=v->cprop[1];
  } else if(clStrcmp(prop,"cp2")) {
    (*rval)=v->cprop[2];
  } else if(clStrcmp(prop,"cp3")) {
    (*rval)=v->cprop[3];
  } else if(clStrcmp(prop,"cp4")) {
    (*rval)=v->cprop[4];
  } else if(clStrcmp(prop,"cp5")) {
    (*rval)=v->cprop[5];
  } else if(clStrcmp(prop,"cp6")) {
    (*rval)=v->cprop[6];
  } else if(clStrcmp(prop,"cp7")) {
    (*rval)=v->cprop[7];
  } else if(clStrcmp(prop,"cp8")) {
    (*rval)=v->cprop[8];
  } else if(clStrcmp(prop,"cp9")) {
    (*rval)=v->cprop[9];
  } else if(clStrcmp(prop,"curv")) {
    (*rval)=v->curv;
  } else if(clStrcmp(prop,"dist")) {
    (*rval) = sqrtf(
		    (v->p[0]-cp[0])*(v->p[0]-cp[0])+
		    (v->p[1]-cp[1])*(v->p[1]-cp[1])+
		    (v->p[2]-cp[2])*(v->p[2]-cp[2])
		    );
  } else {
    return -1;
  }
  return 1;
}

int surfGetRangeXYZVal(dbmSurfNode *node, const char *prop, float *p, float *r)
{
  comMessage("not implemented\n");
  return -1;
}

int surfFix(dbmSurfNode *node)
{
  int i,k;
  double v[4];
  double imat[16];
  surfObj *obj;

  matInverse2(node->transform.rot,imat);
  matTranspose(imat,imat);

  for(i=0;i<node->vc;i++) {
    v[0]=node->v[i].p[0];
    v[1]=node->v[i].p[1];
    v[2]=node->v[i].p[2];
    transApply(&node->transform,v);
    node->v[i].p[0]=v[0];
    node->v[i].p[1]=v[1];
    node->v[i].p[2]=v[2];

    v[0]=node->v[i].n[0];
    v[1]=node->v[i].n[1];
    v[2]=node->v[i].n[2];
    v[3]=1.0;
    matMultVM(v,imat,v);
    matNormalize(v,v);
    node->v[i].n[0]=v[0];
    node->v[i].n[1]=v[1];
    node->v[i].n[2]=v[2];
  }
  transReset(&node->transform);

  for(i=0;i<node->obj_max;i++)
    if(node->obj_flag[i]!=0) {
      obj=&node->obj[i];
      for(k=0;k<obj->vertc;k++) {
	obj->vert[k].p[0]=obj->vert[k].vp->p[0];
	obj->vert[k].p[1]=obj->vert[k].vp->p[1];
	obj->vert[k].p[2]=obj->vert[k].vp->p[2];
	obj->vert[k].n[0]=obj->vert[k].vp->n[0];
	obj->vert[k].n[1]=obj->vert[k].vp->n[1];
	obj->vert[k].n[2]=obj->vert[k].vp->n[2];
      }
    }
      

  return 0;
}

int surfCalcMinMax(dbmSurfNode *n)
{
  int i,vc;
  float vmin[PROP_MAX_VALUES],vmax[PROP_MAX_VALUES];

  if(n->vc==0) {
    for(i=0;i<PROP_MAX_VALUES;i++) {
      n->cprop_min[i]=0.0;
      n->cprop_max[i]=0.0;
    }
    return -1;
  }
  for(i=0;i<PROP_MAX_VALUES;i++) {
    vmin[i]=n->v[0].cprop[i];
    vmax[i]=n->v[0].cprop[i];
  }

  for(vc=1;vc<n->vc;vc++) {
    for(i=0;i<PROP_MAX_VALUES;i++) {
      if(n->v[vc].cprop[i]<vmin[i])
	vmin[i]=n->v[vc].cprop[i];
      if(n->v[vc].cprop[i]>vmax[i])
	vmax[i]=n->v[vc].cprop[i];
    }
  }
  for(i=0;i<PROP_MAX_VALUES;i++) {
    n->cprop_min[i]=vmin[i];
    n->cprop_max[i]=vmax[i];
  }
  return 0;
}


int surfGetMinMax(dbmSurfNode *n,const char *p, float *vmin, float *vmax)
{
  (*vmin)=0.0;
  (*vmax)=0.0;
  if(p==NULL) {
    return -1;
  } else if(clStrlen(p)==0) {
    return -1;
  } else if(clStrcmp(p,"curv")) {
    (*vmin)=-1.0;
    (*vmax)=1.0;
  } else if(clStrcmp(p,"cp0")) {
    (*vmin)=n->cprop_min[0];
    (*vmax)=n->cprop_max[0];
  } else if(clStrcmp(p,"cp1")) {
    (*vmin)=n->cprop_min[1];
    (*vmax)=n->cprop_max[1];
  } else if(clStrcmp(p,"cp2")) {
    (*vmin)=n->cprop_min[2];
    (*vmax)=n->cprop_max[2];
  } else if(clStrcmp(p,"cp3")) {
    (*vmin)=n->cprop_min[3];
    (*vmax)=n->cprop_max[3];
  } else if(clStrcmp(p,"cp4")) {
    (*vmin)=n->cprop_min[4];
    (*vmax)=n->cprop_max[4];
  } else if(clStrcmp(p,"cp5")) {
    (*vmin)=n->cprop_min[5];
    (*vmax)=n->cprop_max[5];
  } else if(clStrcmp(p,"cp6")) {
    (*vmin)=n->cprop_min[6];
    (*vmax)=n->cprop_max[6];
  } else if(clStrcmp(p,"cp7")) {
    (*vmin)=n->cprop_min[7];
    (*vmax)=n->cprop_max[7];
  } else if(clStrcmp(p,"cp8")) {
    (*vmin)=n->cprop_min[8];
    (*vmax)=n->cprop_max[8];
  } else if(clStrcmp(p,"cp9")) {
    (*vmin)=n->cprop_min[9];
    (*vmax)=n->cprop_max[9];
  } else if(clStrcmp(p,"dist")) {
    // TODO
    (*vmin)=0.0;
    (*vmax)=0.0;
  } else {
    // unknown property
    return -1;
  }
  return 0;
}

/*
  recalculate vertex normals based
  on the average of the face normals
  touching each vertex, weighted by
  the area of each face
*/

int surfRenormalize(dbmSurfNode *n)
{
  struct SURF_RENORMAL_VERTICE *rv;
  int i,j,k,indx,flag,v1,v2,v3;

  float area,norm[3],d1[3],d2[3];

  rv=Ccalloc(n->vc,sizeof(struct SURF_RENORMAL_VERTICE));

  for(i=0;i<n->vc;i++) {
    rv[i].v=&n->v[i];
    rv[i].facec=0;
    rv[i].n[0]=0.0;
    rv[i].n[1]=0.0;
    rv[i].n[2]=0.0;
  }

  for(i=0;i<n->fc;i++) {
    for(j=0;j<3;j++) {
      indx=n->f[i].v[j];
      flag=0;
      for(k=0;k<rv[indx].facec;k++) {
	if(rv[indx].facei[k]==i) {
	  flag=1;
	  break;
	}
      }
      if(!flag && rv[indx].facec<SURF_RENORMAL_MAX_FACEI)
	rv[indx].facei[rv[indx].facec++]=i;
      
    }
  }

  for(i=0;i<n->vc;i++) {
    //    fprintf(stderr," %4d: \n",i);
    for(j=0;j<rv[i].facec;j++) {
      //      fprintf(stderr," %d",rv[i].facei[j]);
      indx=rv[i].facei[j];
      v1=n->f[indx].v[0];
      v2=n->f[indx].v[1];
      v3=n->f[indx].v[2];

      d1[0]=rv[v1].v->p[0]-rv[v2].v->p[0];
      d1[1]=rv[v1].v->p[1]-rv[v2].v->p[1];
      d1[2]=rv[v1].v->p[2]-rv[v2].v->p[2];
      d2[0]=rv[v3].v->p[0]-rv[v2].v->p[0];
      d2[1]=rv[v3].v->p[1]-rv[v2].v->p[1];
      d2[2]=rv[v3].v->p[2]-rv[v2].v->p[2];

      area=matCalcTriArea(rv[v1].v->p,rv[v2].v->p,rv[v3].v->p);
      matfCalcCross(d2,d1,norm);
      rv[v1].n[0]+=norm[0]*area;
      rv[v1].n[1]+=norm[1]*area;
      rv[v1].n[2]+=norm[2]*area;
      rv[v2].n[0]+=norm[0]*area;
      rv[v2].n[1]+=norm[1]*area;
      rv[v2].n[2]+=norm[2]*area;
      rv[v3].n[0]+=norm[0]*area;
      rv[v3].n[1]+=norm[1]*area;
      rv[v3].n[2]+=norm[2]*area;
    }
  }

  for(i=0;i<n->vc;i++) {
    matfNormalize(rv[i].n,rv[i].n);
    rv[i].v->n[0]=rv[i].n[0];
    rv[i].v->n[1]=rv[i].n[1];
    rv[i].v->n[2]=rv[i].n[2];
  }  

  Cfree(rv);

  return 0;
}

void surfPrep(dbmSurfNode *node)
{
  surfRecenter(node);
  surfBuildCA(node);
  surfCalcMinMax(node);
}

void surfRecenter(dbmSurfNode *node)
{
  float v[3];
  int i;
  v[0]=0.0;
  v[1]=0.0;
  v[2]=0.0;
  if(node->vc>0) {
    for(i=0;i<node->vc;i++) {
      v[0]+=node->v[i].p[0];
      v[1]+=node->v[i].p[1];
      v[2]+=node->v[i].p[2];
    }
    v[0]/=(double)i;
    v[1]/=(double)i;
    v[2]/=(double)i;
  }
  node->transform.cen[0]=v[0];
  node->transform.cen[1]=v[1];
  node->transform.cen[2]=v[2];
}

