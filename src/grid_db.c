#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid_db.h"
#include "grid_obj.h"
#include "mat.h"
#include "render.h"
#include "Cmalloc.h"
#include "cl.h"
#include "dbm.h"
#include "com.h"

int gridNewNode(dbmGridNode *node)
{
  node->attach_cutoff=4.0;
  node->attach_flag=0;

  node->obj_max=64;
  node->obj=Ccalloc(64,sizeof(gridObj));
  node->obj_flag=Ccalloc(64,sizeof(unsigned char));
  memset(node->obj_flag,0,sizeof(unsigned char)*64);

  node->field.point=NULL;

  node->smode=GRID_SMODE_ANY;

  transReset(&node->transform);
  transReset(&node->transform_save);

  node->texture_max=8;
  node->texture_count=0;

  node->texture=Crecalloc(NULL,node->texture_max,sizeof(gridTexture));

  return 0;
}

int gridCommand(struct DBM_GRID_NODE *node,int wc, char **wl)
{
  char message[256];
  char *empty_com[]={"get","center"};

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }

  if(!strcmp(wl[0],"?") ||
     !strcmp(wl[0],"help")) {
  } else if(!strcmp(wl[0],"new")) {
    return gridComNew(node, wc-1, wl+1);
  } else if(!strcmp(wl[0],"set")) {
    return gridComSet(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"get")) {
    return gridComGet(node,wc-1, wl+1);
  } else if(!strcmp(wl[0],"restrict")) {
    return gridComRestrict(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"attach")) {
    return gridComAttach(node, wc-1, wl+1);
  } else if(!strcmp(wl[0],"delete") ||
	    !strcmp(wl[0],"del")) {
    return gridComDel(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"grab")) {
    if(wc!=2) {
      comMessage("\nsyntax: grab device");
      return -1;
    }
    if(comGrab(&node->transform,wl[1])<0)
      return -1;
    // set center to current center
    comGetCurrentCenter(node->transform.cen);
  } else if(!strcmp(wl[0],"fix")) {
    if(wc>1)
      comMessage("\nwarning: ignored superfluous words after fix");

    memcpy(&node->transform_save, &node->transform,sizeof(transMat));
    comRedraw();
  } else if(!strcmp(wl[0],"reset")) {
    if(wc>1)
      comMessage("\nwarning: ignored superfluous words after reset");

    memcpy(&node->transform, &node->transform_save,sizeof(transMat));
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
  } else if(clStrcmp(wl[0],"tex")) {
    if(wc<2) {
      comMessage("\nerror: missing filename");
      return -1;
    }
    return gridLoad(node, wl[1],"texture1");
  } else {
    sprintf(message,"\nunknown command: %s", wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}


int gridComNew(dbmGridNode *node,int wc, char **wl)
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
  type=GRID_SURFACE;

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
	if(clStrcmp(co.param[i].wl[0],"surface")) {
	  type=GRID_SURFACE;
	} else if(clStrcmp(co.param[i].wl[0],"contour")) {
	  type=GRID_CONTOUR;
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
	      clStrcmp(co.param[i].p,"sel")) {
      if(selectNew(&sel,co.param[i].wc,co.param[i].wl)<0) {
	ret=-1;
	break;
      }
      sel_flag=1;
    } else if(clStrcmp(co.param[i].p,"v")) {
      vflag=1;
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
  
  ret=gridNew(node, name,type,&set,&sel,vflag);

  setDelete(&set);
  clDelete(&co);

  return ret;
}

int gridComSet(dbmGridNode *node,int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;

  ret=gridSet(node, &set);
  
  setDelete(&set);

  return ret;

}

int gridComGet(dbmGridNode *node,int wc, char **wl)
{
  int i;

  if(wc==0) {
    comMessage("\nerror: get: missing property");
    return -1;
  }
  for(i=0;i<wc;i++)
    if(gridGet(node, wl[i])<0)
      return -1;
  return 0;
}

int gridComDel(dbmGridNode *node,int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"\n%s: missing parameter",node->name);
    comMessage(message);
    return -1;
  } else {
    for(i=0;i<wc;i++)
      if(gridDelObj(node,wl[i])<0)
	return -1;
  }
  comRedraw();
  return 0;
}

int gridComAttach(dbmGridNode *node, int wc, char **wl)
{
  float cutoff;
  int i;
  char message[256];

  if(wc==0) {
    return gridAttach(node,NULL,0);
  }
  if(!strcmp(wl[0],"none") ||
     !strcmp(wl[0],"off")) {
    return gridAttach(node,NULL,0);
  }
  if(!comIsDB(wl[0])) {
    sprintf(message,"\nunknown database %s",wl[0]);
    comMessage(message);
    return -1;
  }
  cutoff=4.0;
  for(i=1;i<wc;i++)
    if(!strcmp(wl[i],"-i")) {
      comMessage("\ninfo: attach: -i no longer supported, ignored");
    } else if(!strcmp(wl[i],"-cutoff") ||
	      !strcmp(wl[i],"-co")) {
      if(i+1>=wc) {
	sprintf(message,"\ndistance missing for -cutoff");
	comMessage(message);
	return -1;
      } else {
	cutoff=atof(wl[++i]);
      }
    } else {
      sprintf(message,"\nunknown parameter %s",wl[i]);
      comMessage(message);
      return -1;
    }
  node->attach_cutoff=cutoff;
  return gridAttach(node,comGetDB(wl[0]),0);
}

int gridComRestrict(dbmGridNode *node, int wc, char **wl)
{
  int i,ret,c;
  Select sel;
  char message[256];

  if(selectNew(&sel, wc,wl)<0)
    return -1;

  // reset restriction !!!
  for(i=0;i<node->field.point_count;i++) {
    node->field.point[i].restrict=0;
  }

  // check each element
  c=0;
  for(i=0;i<node->field.point_count;i++) {
    ret=gridIsSelected(node, &node->field.point[i], &sel);
    if(ret<0)
      return -1;
    node->field.point[i].restrict=ret;
    c+=ret;
  }

  // output how many elements were flaged
  if(c==0)
    sprintf(message,"\nall grid-points unrestricted");
  else if(c==node->field.point_count)
    sprintf(message,"\nrestriction affects ALL of %d grid-points",c);
  else
    sprintf(message,"\nrestriction affects %d of %d grid-points",
	    c,node->field.point_count);
  comMessage(message);

  selectDelete(&sel);
  return 0;
}


int gridNew(dbmGridNode *node, char *name, int type, Set *set, Select *sel,int vflag)
{
  gridObj *obj;

  if((obj=gridNewObj(node, name))==NULL) {
    comMessage("error: new: internal error");
    return -1;
  }
  obj->type=type;
  obj->node=node;
  obj->texname=-1;

  memcpy(&obj->select,sel,sizeof(Select));

  return gridObjRenew(obj, set, sel,vflag);
}


/*******

  smode
  scale
  step

 *******/

int gridSet(dbmGridNode *node, Set *s)
{
  int pc;
  struct POV_VALUE *val;

  if(s->pov_count==0) {
    return 0;
  }

  if(s->range_flag) {
    comMessage("error: set: range not expected for grid dataset");
    return -1;
  }

  for(pc=0;pc<s->pov_count;pc++) {
    if(clStrcmp(s->pov[pc].prop,"scalez") ||
       clStrcmp(s->pov[pc].prop,"sz")) {
      s->pov[pc].id=GRID_PROP_SCALEZ;
    } else if (clStrcmp(s->pov[pc].prop,"scalexy") ||
	       clStrcmp(s->pov[pc].prop,"sxy")) {
      s->pov[pc].id=GRID_PROP_SCALEXY;
    } else if (clStrcmp(s->pov[pc].prop,"rot")) {
      s->pov[pc].id=GRID_PROP_ROT;
    } else if (clStrcmp(s->pov[pc].prop,"trans")) {
      s->pov[pc].id=GRID_PROP_TRANS;
    } else {
      comMessage("\nerror: set: unknown property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
    if(s->pov[pc].op!=POV_OP_EQ) {
      comMessage("\nerror: set: expected operator = for property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
    if(s->pov[pc].val_count>1) {
      comMessage("\nerror: set: expected only one value for property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
  }

  for(pc=0;pc<s->pov_count;pc++) {
    val=povGetVal(&s->pov[pc],0);
    switch(s->pov[pc].id) {
    case GRID_PROP_SCALEZ:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property scalez");
	return -1;
      }
      node->field.scale_z=atof(val->val1)/256.0;
      break;
    case GRID_PROP_SCALEXY:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property scalez");
	return -1;
      }
      /*
	why in the world -2 ??
      */
      node->field.scale_x=atof(val->val1)/(float)(node->field.width-2);
      /* 
	 the scale_y is set to scale_x
	 this is a feature, not a bug
      */
      // node->field.scale_y=atof(val->val1)/(float)(node->field.height-2);
      node->field.scale_y=node->field.scale_x;
      break;
    case GRID_PROP_ROT:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property rot");
	return -1;
      }
      if(transSetRot(&node->transform,val->val1)<0)
	return -1;
      break;
    case GRID_PROP_TRANS:
      if(val->range_flag) {
	comMessage("\nerror: set: unexpected range in property trans");
	return -1;
      }
      if(transSetTra(&node->transform,val->val1)<0)
	return -1;
      break;
    }
  }
  return 0;
}

int gridGet(dbmGridNode *node, char *prop)
{
  char message[256];
  int i;
  float v[4];

  if(!strcmp(prop,"center")) {
    v[0]=0.5*node->field.scale_x*(float)node->field.width-
      node->field.offset_x*node->field.scale_x;
    v[1]=0.5*node->field.scale_y*(float)node->field.height-
      node->field.offset_y*node->field.scale_y;
    v[2]=node->field.offset_z*node->field.scale_z;
    v[3]=1.0;

    transApplyf(&node->transform,v);

    sprintf(message,"{%.5f,%.5f,%.5f}",v[0],v[1],v[2]);
    comReturn(message);
  } else if(!strcmp(prop,"smode")) {
    if(node->smode==GRID_SMODE_ALL)
      strcpy(message,"all");
    else
      strcpy(message,"any");
    comReturn(message);
  } else if(!strcmp(prop,"rot")) {
    comReturn(transGetRot(&node->transform));
  } else if(!strcmp(prop,"trans")) {
    comReturn(transGetTra(&node->transform));
  } else {
    sprintf(message,"\n%s: unknown property %s",node->name, prop);
    comMessage(message);
    return -1;
  }
  return 0;
}



gridObj *gridNewObj(dbmGridNode *node, char *name)
{
  int i;
  gridObj *obj;

  gridDelObj(node,name);
  for(i=0;i<node->obj_max;i++) {
    if(node->obj_flag[i]==0) {
      node->obj_flag[i]=1;
      obj=&node->obj[i];
      gridSetDefault(node, obj);
      clStrcpy(obj->name,name);
      comNewObj(node->name, name);
      return obj;
    }
  }
  
  /* catch and increase obj_max */

  return NULL;
}

int gridDelObj(dbmGridNode *node,char *name)
{
  int i;
  gridObj *obj;
	
  for(i=0;i<node->obj_max;i++)
    if(node->obj_flag[i]!=0)
      if(!strcmp(node->obj[i].name,name)) {
	node->obj_flag[i]=0;
	obj=&node->obj[i];
	if(obj->vertc>0)
	  Cfree(obj->vert);
	if(obj->facec>0)
	  Cfree(obj->face);

	comDelObj(node->name, name);
      }
  return 0;
}



int gridIsSelected(dbmGridNode *node, gridPoint *point, Select *sel)
{
  int i,ec,ret;

  /* this is questionable, but needed for the intermediate points */
  if(point==NULL)
    return 1;

  if(point->restrict)
    return 0;

  if(sel==NULL)
    return 1;

  ec=selectGetPOVCount(sel);
  for(i=0;i<ec;i++) {
    ret=gridEvalPOV(node,point,selectGetPOV(sel, i));
    if(ret<0)
      return -1;
    selectSetResult(sel, i, ret);
  }  
  return selectResult(sel);
}

int gridEvalPOV(dbmGridNode *node, gridPoint *point, POV *pov)
{
  int prop,op,i,j,vc,f,ret,rf;
  float dist,dist2,ipos[3],pos[3],dx,dy,dz;
  struct POV_VALUE *val;
  const char *v1,*v2;
  char message[256];
  char db_s[1024],*obj_s;
  float height;
  int uv;

  if(clStrcmp(pov->prop,"*"))
    return 1;


  if(pov->op==POV_OP_WI) {
    prop=GRID_SEL_WITHIN;
  } else if(pov->op==POV_OP_OBJ) {
    prop=GRID_SEL_OBJECT;
  } else if(clStrcmp(pov->prop,"h")) {
      prop=GRID_SEL_HEIGHT;
  } else if(clStrcmp(pov->prop,"u")) {
      prop=GRID_SEL_U;
  } else if(clStrcmp(pov->prop,"v")) {
      prop=GRID_SEL_V;
  } else {
    if(clStrcmp(pov->prop,"anum"))
      prop=GRID_SEL_ANUM;
    else if(clStrcmp(pov->prop,"aname"))
      prop=GRID_SEL_ANAME;
    else if(clStrcmp(pov->prop,"rnum"))
      prop=GRID_SEL_RNUM;
    else if(clStrcmp(pov->prop,"rname"))
      prop=GRID_SEL_RNAME;
    else if(clStrcmp(pov->prop,"chain"))
      prop=GRID_SEL_CHAIN;
    else if(clStrcmp(pov->prop,"model"))
      prop=GRID_SEL_MODEL;
    else if(clStrcmp(pov->prop,"occ") ||
	    clStrcmp(pov->prop,"weight"))
      prop=GRID_SEL_OCC;
    else if(clStrcmp(pov->prop,"bfac"))
      prop=GRID_SEL_BFAC;
    else if(clStrcmp(pov->prop,"ele"))
      prop=GRID_SEL_ELE;
    else {
      sprintf(message,"\nerror: %s: unknown property %s",node->name, pov->prop);
      comMessage(message);
      return -1;
    }
    if(!node->attach_flag) {
      sprintf(message,"\nerror: %s: need attached structure for property %s",node->name, pov->prop);
      comMessage(message);
      return -1;
    }
    if(point->attach_node==NULL)
      return 0;
    ret=structEvalAtomPOV(&point->attach_node->structNode,
			  &point->attach_node->structNode.atom[point->attach_element],
			  pov);

    return ret;
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
    case GRID_SEL_OBJECT:
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
	/* .dataset.object -> use attached datasets */
	if(!node->attach_flag) {
	  comMessage("\nno datasets attached for .dataset.object selection");
	  return -1;
	}
	strncpy(db_s,v1,1023);
	obj_s=strchr(db_s,'.');
	obj_s[0]='\0';
	obj_s++;
	if(rex(db_s,point->attach_node->structNode.name)) {
	  ret=dbmIsElementInObj(db_s,obj_s, point->attach_element);
	  if(ret<0)
	    return -1;
	  if(ret>0)
	    return 1;
	} else {
	  /* ignore */
	}
      } else {
	comMessage("\nobject selection for own topog objects not implemented");
	return -1;
	// TODO later
	f=0;
	for(j=0;j<node->obj_max;j++) {
	  if(node->obj_flag[j]!=0) {
	    if(rex(v1,node->obj[j].name)) {
	      f++;
	      /*
		 if(node->obj[j].vert_flag[vert->num])
		 return 1;

		 this is difficult to do, because contour and surface
		 are very different, in worst case a loop instead
		 of a save-array must do the job
	      */
	    }
	  }
	}
	if(!f) {
	  comMessage("\nerror: object ");
	  comMessage(v1);
	  comMessage(" not found");
	  return -1;
	}
      }
      break;
    case GRID_SEL_WITHIN:
      if(rf) {
	comMessage("\nerror: range not supported for < >");
	return -1;
      }
      ipos[0]=(float)point->x;
      ipos[1]=(float)point->y;
      ipos[2]=(float)point->z;
      gridUVWtoXYZ(&node->field,ipos,pos);
      if(val->wi_flag) {
	gridUVWtoXYZ(&node->field,ipos,pos);
	// pos needs to be modified with the transformation
	transApplyf(&node->transform,pos);
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

	dx=pos[0]-val->vect[0];
	dy=pos[1]-val->vect[1];
	dz=pos[2]-val->vect[2];
	if((dx*dx+dy*dy+dz*dz)<dist2)
	  return 1;
      }
      break;
    case GRID_SEL_HEIGHT:
      height=((float)point->z)/255.0;
      if(rf) {
	if(height>=atof(v1) && height<=atof(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(height==atof(v1)) return 1; break;
	case POV_OP_NE: if(height!=atof(v1)) return 1; break;
	case POV_OP_LT: if(height<atof(v1)) return 1; break;
	case POV_OP_LE: if(height<=atof(v1)) return 1; break;
	case POV_OP_GT: if(height>atof(v1)) return 1; break;
	case POV_OP_GE: if(height>=atof(v1)) return 1; break;
	}
      }
      break;
    case GRID_SEL_U:
    case GRID_SEL_V:
      if(prop==GRID_SEL_U)
	uv=point->x;
      else
	uv=point->y;
      if(rf) {
	if(uv>=atoi(v1) && uv<=atoi(v2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(uv==atoi(v1)) return 1; break;
	case POV_OP_NE: if(uv!=atoi(v1)) return 1; break;
	case POV_OP_LT: if(uv<atoi(v1)) return 1; break;
	case POV_OP_LE: if(uv<=atoi(v1)) return 1; break;
	case POV_OP_GT: if(uv>atoi(v1)) return 1; break;
	case POV_OP_GE: if(uv>=atoi(v1)) return 1; break;
	}
      }
      break;
    }
  }

  return 0;
}

int gridAttach(dbmGridNode *node, dbmNode *attach, int iflag)
{
  register int vc;
  double dx,dy,dz,dx2,dy2,dz2,px,py,pz,dist,ndist,max_dist;
  struct STRUCT_ATOM *ap,*cap;
  struct GRID_POINT *point;

  float v_xyz1[3],v_xyz2[3],ipos[3],pos[3];
  int abc1[3],abc2[3],a_abc[3];
  int a_cac;
  cubeArray *a_ca;
  caPointer *ca_p;
  int acount;
  float limit;

  int i;


  if(attach==NULL) {
    comMessage("\nde-attaching ...");
    node->attach_flag=0;
    for(i=0;i<node->field.point_count;i++)
      node->field.point[i].attach_node=NULL;
    comMessage(" done");
    return 0;
  } else {
    node->attach_flag=1;
  }

  if(attach->common.type!=DBM_NODE_STRUCT) {
    comMessage("\ninvalid node type for attach: must be a structure db");
    return -1;
  }

  max_dist=node->attach_cutoff*node->attach_cutoff;

  comMessage("\nattaching ...");

  limit=node->attach_cutoff;
  a_ca=attach->structNode.ca;
  for(vc=0;vc<node->field.point_count;vc++) {
    point=node->field.point+vc;
    ipos[0]=point->x;
    ipos[1]=point->y;
    ipos[2]=point->z;
    gridUVWtoXYZ(&node->field,ipos,pos);
    px=pos[0];
    py=pos[1];
    pz=pos[2];
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
		    if(!cap->restrict) {
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
      point->attach_node=attach;
      point->attach_element=ap->n;
    } else {
      //      point->attach_node=NULL;
      //      vert->attach_element=0;
    }
  }
  
  comMessage(" done");
  return 0;
}

int gridSetDefault(dbmGridNode *node, gridObj *obj)
{
  obj->type=GRID_SURFACE;
  obj->name[0]='\0';
  obj->node=node;

  obj->render.show=1;
  obj->render.mode=RENDER_SURFACE;
  obj->render.detail=3;
  obj->render.nice=1;
  obj->render.line_width=1.0;
  obj->render.point_size=1.0;
  obj->render.transparency=1.0;
  obj->render.dbl_light=1;
  obj->render.face_reverse=0;          
  comSetDefMat(&obj->render.mat);

  obj->r=1.0;
  obj->g=1.0;
  obj->b=1.0;

  obj->step=1;

  obj->level_start=0.0;
  obj->level_end=255.0;
  obj->level_step=64.0;

  obj->map=-1;

  return 0;
}

int gridUVWtoXYZ(gridField *field, float *uvw, float *xyz)
{
  int u,v,p;

  u=(int)uvw[0];
  v=(int)uvw[1];
  p=(v*field->width)+u;

  xyz[0]=((float)u-field->offset_x)*field->scale_x;
  xyz[1]=((float)v-field->offset_y)*field->scale_y;
  xyz[2]=((float)field->point[p].z-field->offset_z)*field->scale_z;

  return 0;
}

int gridXYZtoUVW(gridField *field, float *xyz, float *uvw)
{
  int u1,v1,u2,v2,p;
  float a,b,c,d,e,f,du,dv;

  uvw[0]=xyz[0]/field->scale_x+field->offset_x;
  uvw[1]=xyz[1]/field->scale_y+field->offset_y;

  u1=(int)uvw[0];
  u2=u1+1;
  v1=(int)uvw[1];
  v2=v1+1;

  if(u1<0 || u2>=field->width ||
     v1<0 || v2>=field->height) {
    uvw[2]=0.0;
  } else {
    p=(v1*field->width)+u1;
    a=(float)field->point[p].z;
    p=(v1*field->width)+u2;
    b=(float)field->point[p].z;
    p=(v2*field->width)+u1;
    c=(float)field->point[p].z;
    p=(v2*field->width)+u2;
    d=(float)field->point[p].z;

    du=uvw[0]-(float)u1;
    dv=uvw[1]-(float)v1;

    e=du*b+(1.0-du)*a;
    f=du*d+(1.0-du)*c;
    
    uvw[2]=dv*f+(1.0-dv)*e;
    
  }  
  return 0;
}

int gridGetRangeVal(dbmGridNode *node, const char *prop, gridPoint *p, float *r)
{
  if(clStrcmp(prop,"h")) {
    (*r)=((float)p->z)/256.0;
  } else {
    comMessage("\nerror: range: unknown property ");
    comMessage(prop);
    (*r)=0.0;
    return -1;
  }
  return 0;
}

int gridGetRangeXYZVal(dbmGridNode *node, const char *prop, float *p, float *r)
{
  float uvw[3];

  if(clStrcmp(prop,"h")) {
    gridXYZtoUVW(&node->field,p,uvw);
    (*r)=uvw[2];
  } else {
    return -1;
  }
  return 0;
}

int gridLoad(dbmGridNode *node, char *fn, char *tn)
{
  int i;
  gridTexture *tex;
  
  for(i=0;i<node->texture_count;i++) {
    if(node->texture[i].flag)
      if(clStrcmp(tn,node->texture[i].name)) {
	Cfree(node->texture[i].data);
	break;
      }
  }
  if(i>=node->texture_max) {
    node->texture_max+=8;
    node->texture=Crecalloc(node->texture,node->texture_max,sizeof(gridTexture));
  }

  tex=node->texture+i;
  node->texture_count++;

  if(gridLoadTexture(fn,tex)==0) {
    tex->flag=1;
    strcpy(tex->name,tn);
    return 0;
  } else {
    tex->flag=0;
    strcpy(tex->name,"");
    return -1;
  }
}

int gridLoadTexture(char *file, gridTexture *tex)
{
  char base[256],*bp;
  char ext[256];
  char message[256];
  
  if((bp=strrchr(file,'/'))!=NULL)
    strcpy(base,bp+1);
  else
    strcpy(base,file);
  
  bp=strrchr(base,'.');
  if(bp!=NULL) {
    bp[0]='\0';
    strcpy(ext,bp+1);
//    strcpy(name,base);
  } else {
//    strcpy(name,base);
    strcpy(ext,"");
  }

  if(clStrcmp(ext,"tiff") || clStrcmp(ext,"tif")) {
    tiffReadTex(file,tex);
  } else {
    sprintf(message,"\nerror: unknown extension %s",ext);
    return -1;
  }

  return 0;
}
