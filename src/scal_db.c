#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#include "scal_db.h"
#include "dbm.h"
#include "mat.h"
#include "com.h"
#include "rex.h"
#include "render.h"
#include "Cmalloc.h"
#include "cl.h"

//static char scal_return[256];

extern int debug_mode;

int scalNewNode(struct DBM_SCAL_NODE *node)
{
  double imat[]={1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0};

  node->attach=NULL;
  memcpy(&node->restriction,
	 lexGenerate("*"),
	 sizeof(struct LEX_STACK));
  node->obj_count=0;
  node->obj_max=64;
  node->obj=Ccalloc(node->obj_max,sizeof(struct DBM_SCAL_OBJ *));
  if(node->obj==NULL) {
    comMessage(" memory allocation error in scalNewNode()\n");
    return -1;
  }

  node->xtal=NULL;
  /*
  memcpy(node->rmat,imat,sizeof(double)*16);
  node->trans[0]=0.0;
  node->trans[1]=0.0;
  node->trans[2]=0.0;
  */
  node->center[0]=0.0;
  node->center[1]=0.0;
  node->center[2]=0.0;

  node->def_level=0.0;

  transReset(&node->transform);
  transReset(&node->transform_save);

  return 0;
}


/* return value in wl[0] */
int scalCommand(dbmScalNode *node, int wc, char **wl)
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
    return scalComNew(node, wc-1, wl+1);
  } else if(!strcmp(wl[0],"get")) {
    return scalComGet(node, wc-1, wl+1);
  } else if(!strcmp(wl[0],"set")) {
    return scalComSet(node, wc-1, wl+1);
  } else if(!strcmp(wl[0],"del") ||
	    !strcmp(wl[0],"delete")) {
    return scalComDel(node, wc-1, wl+1);
  } else if(!strcmp(wl[0],"restrict")) {
    comMessage("restriction not implemented for scalar field dataset\n");
    return -1;
    // TODO later
  } else if(!strcmp(wl[0],"copy") ||
	    !strcmp(wl[0],"cp")) {
    sprintf(message,"%s: copy not available\n",node->name);
    comMessage(message);
    return -1;
  } else if(!strcmp(wl[0],"move") ||
	    !strcmp(wl[0],"mv")) {
    sprintf(message,"%s: move not available\n",node->name);
    comMessage(message);
    return -1;
  } else if(!strcmp(wl[0],"grab")) {
    if(wc!=2) {
      comMessage("syntax: grab device\n");
      return -1;
    }
    if(comGrab(&node->transform,0,0,wl[1])<0)
      return -1;
    // set center to current center
    // should really be the rotation center of the dataset -> rcen
    //   comGetCurrentCenter(node->transform.cen);
  } else if(!strcmp(wl[0],"fix")) {
    if(wc>1)
      comMessage("warning: ignored superfluous words after fix\n");

    memcpy(&node->transform_save, &node->transform,sizeof(transMat));
    comRedraw();
  } else if(!strcmp(wl[0],"reset")) {
    if(wc>1)
      comMessage("scalar-field reset does not take additional parameters\n");

    memcpy(&node->transform, &node->transform_save,sizeof(transMat));
    comRedraw();
  } else if(!strcmp(wl[0],"rotx")) {
    if(wc<2) {
      comMessage("error: missing value after rotx\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTX,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"roty")) {
    if(wc<2) {
      comMessage("error: missing value after roty\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTY,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"rotz")) {
    if(wc<2) {
      comMessage("error: missing value after rotz\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_ROTZ,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transx")) {
    if(wc<2) {
      comMessage("error: missing value after transx\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAX,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transy")) {
    if(wc<2) {
      comMessage("error: missing value after transy\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAY,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transz")) {
    if(wc<2) {
      comMessage("error: missing value after transz\n");
      return -1;
    }
    transCommand(&node->transform,TRANS_TRAZ,-1,atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"write")) {
    comMessage("write not implemented for scalar field dataset\n");
    return -1;
    // TODO later
  } else if(!strcmp(wl[0],"sub")) {
    if(wc!=2) {
      comMessage("Syntax: .scal1 sub .scal2\n");
      return -1;
    }
    scalSub(node,wl[1]);
    // TODO add mul
  } else {
    sprintf(message,"%s: unknown command %s\n",node->name,wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}


int scalComNew(dbmScalNode *node,int wc, char **wl)
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

  type=SCAL_CONTOUR;
  name=node->name;

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
	if(clStrcmp(co.param[i].wl[0],"contour")) {
	  type=SCAL_CONTOUR;
	} else if(clStrcmp(co.param[i].wl[0],"grid")) {
	  type=SCAL_GRID;
	} else if(clStrcmp(co.param[i].wl[0],"grad")) {
	  type=SCAL_GRAD;
	} else if(clStrcmp(co.param[i].wl[0],"slab")) {
	  type=SCAL_SLAB;
#ifdef VR
	} else if(clStrcmp(co.param[i].wl[0],"volume")) {
	  type=SCAL_VR;
#endif
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

  
  ret=scalNew(node, name, type,&set,&sel);

  setDelete(&set);
  clDelete(&co);

  return ret;
}

int scalComSet(dbmScalNode *node,int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;

  ret=scalSet(node, &set);
  
  setDelete(&set);

  return ret;

}

int scalComGet(dbmScalNode *node,int wc, char **wl)
{
  int i;

  if(wc==0) {
    comMessage("error: get: missing property\n");
    return -1;
  }
  for(i=0;i<wc;i++)
    if(scalGet(node, wl[i])<0)
      return -1;
  return 0;
}

int scalComDel(dbmScalNode *node,int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"%s: missing parameter\n",node->name);
    comMessage(message);
    return -1;
  } else {
    for(i=0;i<wc;i++)
      if(scalDelObj(node,wl[i])<0)
	return -1;
  }
  comRedraw();
  return 0;
}


int scalNew(dbmScalNode *node, char *name, int type, Set *set, Select *sel)
{
  char message[256];
  scalObj *obj;

  if((obj=scalNewObj(node,name))==NULL) {
    sprintf(message,"internal error in new\n");
    comMessage(message);
    return -1;
  }

  obj->type=type;
  obj->field=node->field;
  obj->node=node;

  scalSetDefault(obj);

  memcpy(&obj->select, sel, sizeof(Select));

  return scalObjRenew(obj, set,sel);
}

int scalSet(dbmScalNode *node, Set *s)
{
  int pc;
  struct POV_VALUE *val;
  float v1[4],v2[4];

  if(s->pov_count==0) {
    return 0;
  }

  if(s->range_flag) {
    comMessage("error: set: range not expected for scal dataset");
    return -1;
  }

  for(pc=0;pc<s->pov_count;pc++) {
    if (clStrcmp(s->pov[pc].prop,"rot")) {
      s->pov[pc].id=SCAL_PROP_ROT;
    } else if (clStrcmp(s->pov[pc].prop,"trans")) {
      s->pov[pc].id=SCAL_PROP_TRANS;
    } else if (clStrcmp(s->pov[pc].prop,"rtc")) {
      s->pov[pc].id=SCAL_PROP_RTC;
    } else if (clStrcmp(s->pov[pc].prop,"center") ||
	       clStrcmp(s->pov[pc].prop,"rcen")) {
      s->pov[pc].id=SCAL_PROP_RCEN;
    } else if (clStrcmp(s->pov[pc].prop,"edge")) {
      s->pov[pc].id=SCAL_PROP_EDGE;
    } else if (clStrcmp(s->pov[pc].prop,"scale")) {
      s->pov[pc].id=SCAL_PROP_SCALE;
    } else if (clStrcmp(s->pov[pc].prop,"vm")) {
      s->pov[pc].id=SCAL_PROP_VM;
    } else if (clStrcmp(s->pov[pc].prop,"vc")) {
      s->pov[pc].id=SCAL_PROP_VC;
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
    if(s->pov[pc].val_count>1) {
      comMessage("error: set: expected only one value for property \n");
      comMessage(s->pov[pc].prop);
      return -1;
    }
  }

  for(pc=0;pc<s->pov_count;pc++) {
    val=povGetVal(&s->pov[pc],0);
    switch(s->pov[pc].id) {
    case SCAL_PROP_EDGE:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property edge\n");
	return -1;
      }
      node->field->edge=atof(val->val1);
      break;
    case SCAL_PROP_SCALE:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property edge\n");
	return -1;
      }
      node->field->scale=atof(val->val1);
      scalSetMinMax(node);
      break;
    case SCAL_PROP_VM:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property edge\n");
	return -1;
      }
      node->field->vm=atof(val->val1);
      scalSetMinMax(node);
      break;
    case SCAL_PROP_VC:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property edge\n");
	return -1;
      }
      node->field->vc=atof(val->val1);
      scalSetMinMax(node);
      break;
    case SCAL_PROP_ROT:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property rot\n");
	return -1;
      }
      if(transSetRot(&node->transform,val->val1)<0)
	return -1;
      break;
    case SCAL_PROP_TRANS:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property trans\n");
	return -1;
      }
      if(transSetTra(&node->transform,val->val1)<0)
	return -1;
      break;
    case SCAL_PROP_RTC:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property trans\n");
	return -1;
      }
      if(transSetAll(&node->transform,val->val1)<0)
	return -1;

      break;

    case SCAL_PROP_RCEN:
      if(val->range_flag) {
	comMessage("error: set: unexpected range in property trans\n");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v1)!=0) {
	comMessage("error in vector: \n");
	comMessage(val->val1);
	return -1;
      }   
      transApplyIf(&node->transform, v1);

      v2[0]=node->transform.cen[0];
      v2[1]=node->transform.cen[1];
      v2[2]=node->transform.cen[2];

      node->transform.cen[0]=v1[0];
      node->transform.cen[1]=v1[1];
      node->transform.cen[2]=v1[2];
      node->transform.cen[3]=1.0;

      v2[0]-=v1[0];
      v2[1]-=v1[1];
      v2[2]-=v1[2];

      /* adjust translation */
      node->transform.tra[0]+=
	+v2[0]*(1.0-node->transform.rot[0])
	-v2[1]*node->transform.rot[4]
	-v2[2]*node->transform.rot[8];
      node->transform.tra[1]+=
	-v2[0]*node->transform.rot[1]
	+v2[1]*(1.0-node->transform.rot[5])
	-v2[2]*node->transform.rot[9];
      node->transform.tra[2]+=
	-v2[0]*node->transform.rot[2]
	-v2[1]*node->transform.rot[6]
	+v2[2]*(1.0-node->transform.rot[10]);

      break;
    }
  }

  return 0;
}

int scalGet(dbmScalNode *node, char *prop)
{
  int u,v,w;
  double v1[4],v2[4],v3[4];
  char message[256];

  if(prop[0]=='{') {
    matExtract1D(prop,3,v2);
    scalXYZtoUVW(node->field,v2,v1);
    u=(int)v1[0];
    v=(int)v1[1];
    w=(int)v1[2];
    sprintf(message,"%d %d %d: %.3f",u,v,w,scalReadField(node->field,u,v,w));
    comReturn(message);
  } else if(!strcmp(prop,"center")) {

    v1[0]=node->center[0];
    v1[1]=node->center[1];
    v1[2]=node->center[2];

    transApply(&node->transform,v1);

    sprintf(message,"{%.5f,%.5f,%.5f}",v1[0],v1[1],v1[2]);
    comReturn(message);
  } else if(clStrcmp(prop,"rcenter") ||
	    clStrcmp(prop,"rcen")) {
    sprintf(message,"{%.5f,%.5f,%.5f}",
	    node->transform.cen[0],
	    node->transform.cen[1],
	    node->transform.cen[2]);
    comReturn(message);
  } else if(!strcmp(prop,"edge")) {
    sprintf(message,"%f",node->field->edge);
    comReturn(message);
  } else if(!strcmp(prop,"rot")) {
    comReturn(transGetRot(&node->transform));
  } else if(!strcmp(prop,"trans")) {
    comReturn(transGetTra(&node->transform));
  } else if(!strcmp(prop,"rtc")) {
    comReturn(transGetAll(&node->transform));
  } else {
    sprintf(message,"%s: unknown parameter %s\n", node->name,prop);
    comMessage(message);
    return -1;
  }
  return 0;
}

scalObj *scalNewObj(struct DBM_SCAL_NODE *node, char *name)
{
  int i;
  
  scalDelObj(node,name);
  for(i=0;i<node->obj_max;i++) {
    if(node->obj[i]==NULL) {
      node->obj[i]=Cmalloc(sizeof(scalObj)); 
      memset(node->obj[i],0,sizeof(scalObj));
      strcpy(node->obj[i]->name,name);
      comNewObj(node->name,name);
      return node->obj[i];
    }
  }
  
  /* catch and increase obj_max */

  return NULL;
}

int scalDelObj(struct DBM_SCAL_NODE *node,char *name)
{
  int i;
  scalObj *obj;
	
  for(i=0;i<node->obj_max;i++)
    if(node->obj[i]!=NULL)
      if(!strcmp(node->obj[i]->name,name)) {
	obj=node->obj[i];

	if(obj->point_count>0)
	  Cfree(obj->point);
	if(obj->line_count>0)
	  Cfree(obj->line);
	if(obj->face_count>0)
	  Cfree(obj->face);
	Cfree(obj);

	node->obj[i]=NULL;
	node->obj_count--;
	comDelObj(node->name,name);
      }


  return 0;
}




int scalUVWtoXYZ(struct SCAL_FIELD *field,double *uvw,double *xyz)
{
  double a,b,c;

  a=uvw[0];
  b=uvw[1];
  c=uvw[2];

  xyz[0]=a*field->a[0]+b*field->b[0]+c*field->c[0];
  xyz[0]*=field->scale;
  xyz[0]+=field->offset_x;
  xyz[1]=a*field->a[1]+b*field->b[1]+c*field->c[1];
  xyz[1]*=field->scale;
  xyz[1]+=field->offset_y;
  xyz[2]=a*field->a[2]+b*field->b[2]+c*field->c[2];
  xyz[2]*=field->scale;
  xyz[2]+=field->offset_z;

  return 0;
}

int scalUVWtoXYZf(struct SCAL_FIELD *field,float *uvw,float *xyz)
{
  float a,b,c;


  a=uvw[0];
  b=uvw[1];
  c=uvw[2];

  xyz[0]=a*(float)field->a[0]+b*(float)field->b[0]+c*(float)field->c[0];
  xyz[0]*=(float)field->scale;
  xyz[0]+=(float)field->offset_x;
  xyz[1]=a*(float)field->a[1]+b*(float)field->b[1]+c*(float)field->c[1];
  xyz[1]*=(float)field->scale;
  xyz[1]+=(float)field->offset_y;
  xyz[2]=a*(float)field->a[2]+b*(float)field->b[2]+c*(float)field->c[2];
  xyz[2]*=(float)field->scale;
  xyz[2]+=(float)field->offset_z;


  return 0;
}

int scalXYZtoUVWf(struct SCAL_FIELD *field,float *xyz,float *uvw)
{
  double a[4],b[4];
  a[0]=xyz[0]; a[1]=xyz[1]; a[2]=xyz[2];
  scalXYZtoUVW(field,a,b);
  uvw[0]=b[0]; uvw[1]=b[1]; uvw[2]=b[2];
  return 0;
}

int scalXYZtoUVW(struct SCAL_FIELD *field,double *xyz,double *uvw)
{
  double da,db,dc,dd;
  double ax,ay,az,bx,by,bz,cx,cy,cz,px,py,pz;
  
  ax=field->a[0];
  ay=field->a[1];
  az=field->a[2];
  bx=field->b[0];
  by=field->b[1];
  bz=field->b[2];
  cx=field->c[0];
  cy=field->c[1];
  cz=field->c[2];
  px=xyz[0]-field->offset_x;
  py=xyz[1]-field->offset_y;
  pz=xyz[2]-field->offset_z;
  px/=field->scale;
  py/=field->scale;
  pz/=field->scale;

  da=px*(by*cz-bz*cy)-py*(bx*cz-bz*cx)+pz*(bx*cy-by*cx);
  db=ax*(py*cz-pz*cy)-ay*(px*cz-pz*cx)+az*(px*cy-py*cx);
  dc=ax*(by*pz-bz*py)-ay*(bx*pz-bz*px)+az*(bx*py-by*px);
  dd=ax*(by*cz-bz*cy)-ay*(bx*cz-bz*cx)+az*(bx*cy-by*cx);

  if(dd==0.0) {
    comMessage("scalXYZtoUVW: Error: cannot resolve singular matrix\n");
    uvw[0]=0.0;
    uvw[1]=0.0;
    uvw[2]=0.0;
    return -1;
  }

  uvw[0]=da/dd;
  uvw[1]=db/dd;
  uvw[2]=dc/dd;

  return 0;
}


/**********************************************************************/

int scalWriteField(struct SCAL_FIELD *field, int u, int v, int w, float value)
{
  int p;
  int a,b,c;
  
  if(field->wrap) {
    while(u>field->u2)
      u-=field->u_size;
    while(u<field->u1)
      u+=field->u_size;
    while(v>field->v2)
      v-=field->v_size;
    while(v<field->v1)
      v+=field->v_size;
    while(w>field->w2)
      w-=field->w_size;
    while(w<field->w1)
      w+=field->w_size;
    a=u-field->u1;
    b=v-field->v1;
    c=w-field->w1;

  } else {
    if(u<field->u1)
      return -1;
    else if(u>field->u2)
      return 1;
    else
      a=u-field->u1;
    
    if(v<field->v1)
      return -1;
    else if(v>field->v2)
      return 1;
    else
      b=v-field->v1;
    
    if(w<field->w1)
      return -1;
    else if(w>field->w2)
      return 1;
    else
      c=w-field->w1;
  }      
  
  p=field->u_size*field->v_size*c+field->u_size*b+a;
  
#ifdef SCAL_DEBUG
  if(p<field->size)
#endif
  field->data[p]=value;
#ifdef SCAL_DEBUG
  else
    fprintf(stderr,
	    "set map write error: out of bounds retrieval(s: %d, p:%d)\n",
	    p,field->size);
#endif
  return 0;
}

float scalReadField(struct SCAL_FIELD *field, int u, int v, int w)
{
  int p;
  int a,b,c;
  float edge=field->edge;

  if(field->wrap) {
    while(u>field->u2)
      u-=field->u_size;
    while(u<field->u1)
      u+=field->u_size;
    while(v>field->v2)
      v-=field->v_size;
    while(v<field->v1)
      v+=field->v_size;
    while(w>field->w2)
      w-=field->w_size;
    while(w<field->w1)
      w+=field->w_size;
    a=u-field->u1;
    b=v-field->v1;
    c=w-field->w1;

  } else {
    if(u<field->u1)
      return edge;
    else if(u>field->u2)
      return edge;
    else
      a=u-field->u1;
    
    if(v<field->v1)
      return edge;
    else if(v>field->v2)
      return edge;
    else
      b=v-field->v1;
    
    if(w<field->w1)
      return edge;
    else if(w>field->w2)
      return edge;
    else
      c=w-field->w1;
  }      
  p=field->u_size*field->v_size*c+field->u_size*b+a;
  
#ifdef SCAL_DEBUG
  if(p<field->size)
#endif
    return field->data[p]*field->vm+field->vc;
#ifdef SCAL_DEBUG
  else
    fprintf(stderr,
	    "set map read error: out of bounds retrieval(s: %d, p:%d)\n",
	    p,field->size);
#endif
}



int scalIsSelected(dbmScalNode *node, int u, int v, int w, Select *sel)
{
  int i,ec,res;

  if(sel==NULL)
    return 1;

  ec=selectGetPOVCount(sel);
  for(i=0;i<ec;i++) {
    res=scalEvalPOV(node,u,v,w,selectGetPOV(sel, i));
    if(res==-1)
      return -1;
    selectSetResult(sel, i, res);
  }  

  return selectResult(sel);
}

int scalEvalPOV(dbmScalNode *node, int u, int v, int w, POV *pov)
{
  struct SCAL_FIELD *field=node->field;
  int i,vc;
  int prop,rf,op;
  const char *val1,*val2;
  struct POV_VALUE *val;
  char message[256];
  double v1[3],v2[3],dist,dist2;
  double diff,dx,dy,dz;
  float pos[3],vv;
  int ret;

  if(clStrcmp(pov->prop,"*"))
    return 1;
  
  if(pov->op==POV_OP_WI) {
    prop=SCAL_SEL_WITHIN;
  } else if(pov->prop[0]=='.') {
    comMessage("object selection not implemented for scalar field dataset\n");
    return -1;
//    prop=SCAL_SEL_OBJ;
  } else if(clStrcmp(pov->prop,"v")) {
    prop=SCAL_SEL_V;
  } else if(clStrcmp(pov->prop,"x")) {
    prop=SCAL_SEL_X;
  } else if(clStrcmp(pov->prop,"y")) {
    prop=SCAL_SEL_Y;
  } else if(clStrcmp(pov->prop,"z")) {
    prop=SCAL_SEL_Z;
  } else {
    comMessage("error: unknown selection property \n");
    comMessage(pov->prop);
    return -1;
  }
  op=pov->op;
  vc=pov->val_count;
  for(i=0;i<vc;i++) {
    val=povGetVal(pov,i);
    if(val->range_flag) {
      if(op!=POV_OP_EQ) {
	sprintf(message,"error: expected operator = for range\n");
	return -1;
      }
      rf=1;
      val1=val->val1;
      val2=val->val2;
    } else {
      rf=0;
      val1=val->val1;
      val2=val->val1;
    }
    if(clStrcmp(val1,"*") || clStrcmp(val2,"*"))
      return 1;
    
    switch(prop) {
    case SCAL_SEL_WITHIN:
      dist=atof(pov->prop);
      dist2=dist*dist;
      
      v1[0]=(double)u;
      v1[1]=(double)v;
      v1[2]=(double)w;
      scalUVWtoXYZ(field,v1,v2);
      
      if(rf) {
	comMessage("error: range not supported for < >\n");
	return -1;
      }
      if(val->wi_flag) {
	pos[0]=v2[0];
	pos[1]=v2[1];
	pos[2]=v2[2];
	if(val->val1==NULL) {
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

	dx=v2[0]-val->vect[0];
	dy=v2[1]-val->vect[1];
	dz=v2[2]-val->vect[2];

	if((dx*dx+dy*dy+dz*dz)<dist2)
	  return 1;
      }
      break;
    case SCAL_SEL_OBJ:
      // TODO later
      break;
    /*******
    case SCAL_SEL_V:
      vv=scalReadField(field, u, v, w);
      if(rf) {
	if(vv>=atof(val1) && vv<=atof(val2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vv==atof(val1)) return 1; break;
	case POV_OP_NE: if(vv!=atof(val1)) return 1; break;
	case POV_OP_LT: if(vv<atof(val1)) return 1; break;
	case POV_OP_LE: if(vv<=atof(val1)) return 1; break;
	case POV_OP_GT: if(vv>atof(val1)) return 1; break;
	case POV_OP_GE: if(vv>=atof(val1)) return 1; break;
	default: comMessage("error: invalid operator\n"); return -1;
	}
      }
      break;
      *******************/
    case SCAL_SEL_V:
    case SCAL_SEL_X:
    case SCAL_SEL_Y:
    case SCAL_SEL_Z:
      switch(prop) {
      case SCAL_SEL_V:
	vv=scalReadField(field, u, v, w);
	break;
      case SCAL_SEL_X:
	v1[0]=u; v1[1]=v; v1[2]=w;
	scalUVWtoXYZ(field, v1, v2);
	vv=v2[0];
	break;
      case SCAL_SEL_Y:
	v1[0]=u; v1[1]=v; v1[2]=w;
	scalUVWtoXYZ(field, v1, v2);
	vv=v2[1];
	break;
      case SCAL_SEL_Z:
	v1[0]=u; v1[1]=v; v1[2]=w;
	scalUVWtoXYZ(field, v1, v2);
	vv=v2[2];
	break;
	
      }
      if(rf) {
	if(vv>=atof(val1) && vv<=atof(val2))
	  return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(vv==atof(val1)) return 1; break;
	case POV_OP_NE: if(vv!=atof(val1)) return 1; break;
	case POV_OP_LT: if(vv<atof(val1)) return 1; break;
	case POV_OP_LE: if(vv<=atof(val1)) return 1; break;
	case POV_OP_GT: if(vv>atof(val1)) return 1; break;
	case POV_OP_GE: if(vv>=atof(val1)) return 1; break;
	default: comMessage("error: invalid operator\n"); return -1;
	}
      }
      break;
    }
  }  
  return 0;
}


#ifdef SCAL_XYZ_SEL

int scalIsXYZSelected(dbmScalNode *n, float x, float y, float z, Select *sel)
{
  int i,ec;

  if(sel==NULL)
    return 1;

  ec=selectGetPOVCount(sel);
  for(i=0;i<ec;i++)
    selectSetResult(sel, i, scalEvalXYZPOV(n,x,y,z,selectGetPOV(sel, i)));
  
  return selectResult(sel);
}
int scalEvalXYZPOV(dbmScalNode *node, float x, float y, float z, POV *pov)
{
  int i,vc;
  int prop,rf,op;
  const char *val1,*val2;
  struct POV_VALUE *val;
  char message[256];
  double v1[3],v2[3],dist;
  double diff,dx,dy,dz;
  float pos[3];


  if(clStrcmp(pov->prop,"*"))
    return 1;
  
  if(pov->op==POV_OP_WI)
    prop=STRUCT_SEL_WITHIN;
  else if(pov->prop[0]=='.')
    prop=STRUCT_SEL_OBJECT;
  else {
    // is attached ?
  }

  vc=pov->val_count;
  for(i=0;i<vc;i++) {
    val=povGetVal(pov,i);
    if(val->range_flag) {
      if(op!=POV_OP_EQ) {
	sprintf(message,"error: expected operator = for range\n");
	return -1;
      }
      rf=1;
      val1=val->val1;
      val2=val->val2;
    } else {
      rf=0;
      val1=val->val1;
      val2=val->val1;
    }
    if(clStrcmp(val1,"*") || clStrcmp(val2,"*"))
      return 1;
    
    switch(prop) {
    case SCAL_SEL_WITHIN:
      dist=atof(pov->prop);
      
      v2[0]=x;
      v2[1]=y;
      v2[2]=z;
      
      if(rf) {
	comMessage("error: range not supported for < >\n");
	return -1;
      }
      if(val->wi_flag) {
	pos[0]=v2[0];
	pos[1]=v2[1];
	pos[2]=v2[2];
	if(val->val1==NULL) {
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

	dx=v2[0]-val->vect[0];
	dy=v2[1]-val->vect[1];
	dz=v2[2]-val->vect[2];
	if((dx*dx+dy*dy+dz*dz)<dist2)
	  return 1;
      }
      break;
    }
  }  
  return 0;
}
#endif


 
int scalCELLtoVECT(struct SCAL_FIELD *field, double a, double b, double c, double alpha, double beta, double gamma, double fa, double fb, double fc)
{
  double sa,sb,sc,ca,cb,cc,av,bv,cv;


  sa=sin(alpha*M_PI/180.0);
  sb=sin(beta*M_PI/180.0);
  sc=sin(gamma*M_PI/180.0);
  ca=cos(alpha*M_PI/180.0);
  cb=cos(beta*M_PI/180.0);
  cc=cos(gamma*M_PI/180.0);

  av=a*fa;
  bv=b*fb;
  cv=c*fc;

  field->a[0]=av;
  field->a[1]=0.0;
  field->a[2]=0.0;
  field->b[0]=bv*cc;
  field->b[1]=bv*sc;
  field->b[2]=0.0;
  field->c[0]=cv*cb;
  field->c[1]=(bv*cv*ca-bv*cv*cc*cb)/(bv*sc);
  field->c[2]=sqrt(cv*cv-field->c[0]*field->c[0]-field->c[1]*field->c[1]);
  return 0;
}

int scalSetMinMax(dbmScalNode *node)
{
  int i;
  float v1,v2,vp,vt,vt2,*data;
  char dbmesg[256];
  double uvw[3],xyz1[3],xyz2[3],xyz3[3],vm,vc;

  data=node->field->data;
  vm=node->field->vm;
  vc=node->field->vc;
  v1=data[0]*vm+vc;
  v2=data[0]*vm+vc;

  vt=0.0;
  for(i=1;i<node->field->size;i++) {
    vp=data[i]*vm+vc;
    vt+=vp;
    v1 = (vp<v1) ? vp : v1;
    v2 = (vp>v2) ? vp : v2;
  }
  node->min_max.v1=v1;
  node->min_max.v2=v2;

  vt/=(double)node->field->size;

  vt2=0.0;
  for(i=0;i<node->field->size;i++) {
    vp=data[i]*vm+vc;
    vt2 += (vp-vt)*(vp-vt);
  }
  vt2 = sqrt(vt2/(double)(node->field->size-1));
  
  sprintf(dbmesg,"min: %g  max: %g  stdev: %g\n",
	  node->min_max.v1, node->min_max.v2, vt2);
  comMessage(dbmesg);

  node->field->vmin=node->min_max.v1;
  node->field->vmax=node->min_max.v2;
  node->field->sigma=vt2;

  uvw[0]=node->field->u1;
  uvw[1]=node->field->v1;
  uvw[2]=node->field->w1;
  scalUVWtoXYZ(node->field, uvw, xyz1);

  uvw[0]=node->field->u2;
  uvw[1]=node->field->v2;
  uvw[2]=node->field->w2;
  scalUVWtoXYZ(node->field, uvw, xyz2);

  xyz3[0]=(xyz1[0]+xyz2[0])*0.5;
  xyz3[1]=(xyz1[1]+xyz2[1])*0.5;
  xyz3[2]=(xyz1[2]+xyz2[2])*0.5;

  node->center[0]=xyz3[0];
  node->center[1]=xyz3[1];
  node->center[2]=xyz3[2];

  node->transform.cen[0]=xyz3[0];
  node->transform.cen[1]=xyz3[1];
  node->transform.cen[2]=xyz3[2];

  return 0;
}

int scalCalcOffset(struct SCAL_FIELD *field, int u, int v,int w)
{
  int p;
  int a,b,c;
  float edge=0.0;

  if(field->wrap) {
    while(u>field->u2)
      u-=field->u_size;
    while(u<field->u1)
      u+=field->u_size;
    while(v>field->v2)
      v-=field->v_size;
    while(v<field->v1)
      v+=field->v_size;
    while(w>field->w2)
      w-=field->w_size;
    while(w<field->w1)
      w+=field->w_size;
    a=u-field->u1;
    b=v-field->v1;
    c=w-field->w1;

  } else {
    if(u<field->u1)
      return edge;
    else if(u>field->u2)
      return edge;
    else
      a=u-field->u1;
    
    if(v<field->v1)
      return edge;
    else if(v>field->v2)
      return edge;
    else
      b=v-field->v1;
    
    if(w<field->w1)
      return edge;
    else if(w>field->w2)
      return edge;
    else
      c=w-field->w1;
  }      
  p=field->u_size*field->v_size*c+field->u_size*b+a;
  
#ifdef SCAL_DEBUG
  if(p<field->size)
#endif
    return p;
#ifdef SCAL_DEBUG
  else
    fprintf(stderr,
	    "set map read error: out of bounds retrieval(s: %d, p:%d)\n",
	    p,field->size);
#endif
}


int scalUnCalcOffset(struct SCAL_FIELD *field, int o, int *u, int *v,int *w)
{
  int a,b;
  float f;

  /*
  p_u=(float)o/(float)field->u_size;
  p_uv=floorf(p_u)/(float)field->v_size;

  (*u)=(int)((float)field->u_size*(p_u-floorf(p_u)))+field->u1;
  (*v)=(int)((float)field->v_size*(p_uv-floorf(p_uv)))+field->v1;
  (*w)=(int)floorf(p_uv)+field->w1;
  */

  /*
  f=(float)o/(float)field->u_size;
  a=o-field->u_size*(int)floorf(f);
  (*u)=field->u1+a;
  o-=a;
  f=(float)o/(float)(field->u_size*field->v_size);
  b=o-field->u_size*field->v_size*(int)floorf(f);
  (*v)=field->v1+b/field->u_size;
  o-=b;
  (*w)=field->w1+(int)floorf(f);
  */

  f=(float)o/(float)field->u_size;
  a=o-field->u_size*(int)f;
  (*u)=field->u1+a;
  o-=a;
  f=(float)o/(float)(field->u_size*field->v_size);
  b=o-field->u_size*field->v_size*(int)f;
  (*v)=field->v1+b/field->u_size;
  o-=b;
  (*w)=field->w1+(int)f;
  return 0;
}


#ifdef BONO
int scalOctGetMinMax(struct SCAL_FIELD *field, int u, int v, int w,
		     float *min, float *max)
{
  int a,b,c;
  float cmin,cmax,val;

  cmin=scalReadField(field,u,v,w);
  cmax=cmin;

  for(a=0;a<3;a++)
    for(b=0;b<3;b++)
      for(c=0;c<3;c++) {
	val=scalReadField(field,u+a,v+b,w+c);
	if(val<cmin)
	  cmin=val;
	if(val>cmax)
	  cmax=val;
      }
  (*min)=cmin;
  (*max)=cmax;
  
  return 0;
}

float scalReadFieldO(struct SCAL_FIELD *field, int u, int v, int w, int off)
{
  int p;

  p=off+field->u_size*field->v_size*w+field->u_size*v+u;

#ifdef SCAL_DEBUG
  if(p<field->size)
#endif
    return field->data[p];
#ifdef SCAL_DEBUG
  else
    fprintf(stderr,
	    "set map read error: out of bounds retrieval(s: %d, p:%d)\n",
	    p,field->size);
#endif

}
#endif

int scalSetDefault(scalObj *obj)
{
  obj->point=NULL;
  obj->point_count=0;
  obj->line=NULL;
  obj->line_count=0;
  obj->face=NULL;
  obj->face_count=0;

  obj->r=0.7;
  obj->g=0.8;
  obj->b=1.0;
  obj->a=1.0;

  obj->u_size=30;
  obj->v_size=30;
  obj->w_size=30;
  obj->u_center=(obj->field->u2-obj->field->u1)/2+obj->field->u1;
  obj->v_center=(obj->field->v2-obj->field->v1)/2+obj->field->v1;
  obj->w_center=(obj->field->w2-obj->field->w1)/2+obj->field->w1;
  obj->u_start=obj->u_center-obj->u_size/2;
  obj->v_start=obj->v_center-obj->v_size/2;
  obj->w_start=obj->w_center-obj->w_size/2;
  obj->u_end=obj->u_center+obj->u_size/2;
  obj->v_end=obj->v_center+obj->v_size/2;
  obj->w_end=obj->w_center+obj->w_size/2;

  obj->level=obj->node->def_level;
  obj->step=1;

  obj->render.show=1;
  if(obj->type==SCAL_CONTOUR) {
    obj->render.mode=RENDER_LINE;
  } else if(obj->type==SCAL_GRID) {
    obj->render.mode=RENDER_OFF;
  }
  obj->render.nice=1;
  obj->render.point_size=1.0;
  obj->render.line_width=1.0;
  obj->render.transparency=0.3;
  obj->render.detail1=3;
  obj->render.detail2=3;
  obj->render.dbl_light=0;
  obj->render.face_reverse=0;
  obj->render.cull=0;
  obj->render.solid=0;
  obj->render.solidc[0]=1.0;
  obj->render.solidc[1]=1.0;
  obj->render.solidc[2]=1.0;

  comSetDefMat(&obj->render.mat);

#ifdef VR
  obj->vr.tex=0;
  obj->vr.start=0.0;
  obj->vr.end=1.0;
#endif

  obj->slab.usize=128;
  obj->slab.vsize=128;
  obj->slab.size=obj->slab.usize*obj->slab.vsize;
  obj->slab.data=NULL;
  obj->slab.tex=NULL;
  obj->slab.dir[0]=0.0;
  obj->slab.dir[1]=0.0;
  obj->slab.dir[2]=1.0;
  obj->slab.center[0]=0.0;
  obj->slab.center[1]=0.0;
  obj->slab.center[2]=0.0;

  return 0;
}


void swap_4b(unsigned char *a)
{
  unsigned char b[4];
  b[0]=a[3];
  b[1]=a[2];
  b[2]=a[1];
  b[3]=a[0];
  a[0]=b[0];
  a[1]=b[1];
  a[2]=b[2];
  a[3]=b[3];
}

void swap_4bs(unsigned char *b, int n)
{
  int i;
  for(i=0;i<n;i++) {
    swap_4b(b+i*4);
  }
}

void swap_int(int *p, int n)
{
  int k;
  unsigned char *a,b[4];
  
  for(k=0;k<n;k++) {
    a=(unsigned char *)&p[k];

    b[0]=a[3];
    b[1]=a[2];
    b[2]=a[1];
    b[3]=a[0];
    a[0]=b[0];
    a[1]=b[1];
    a[2]=b[2];
    a[3]=b[3];

  }
}

void swap_float(float *p, int n)
{
  int k;
  unsigned char *a,b[4],*c;

  for(k=0;k<n;k++) {
    a=(unsigned char *)&p[k];

    b[0]=a[3];
    b[1]=a[2];
    b[2]=a[1];
    b[3]=a[0];
    a[0]=b[0];
    a[1]=b[1];
    a[2]=b[2];
    a[3]=b[3];
  }
}

void swap_double(double *p, int n)
{
  int k;
  unsigned char *a,tmp;
  
  for(k=0;k<n;k++) {
    a=(unsigned char *)&p[k];
    
    tmp = a[0];
    a[0] = a[7];
    a[7] = tmp;
    tmp = a[1];
    a[1] = a[6];
    a[6] = tmp;
    tmp = a[2];
    a[2] = a[5];
    a[5] =tmp;
    tmp = a[3];
    a[3] = a[4];
    a[4] = tmp;
  } 
}

/*
  the formula for cubic interpolation is

  a=(E-A)r+A
  b=(F-B)r+B
  c=(G-C)r+C
  d=(H-D)r+D
  e=(c-a)q+a
  f=(d-b)q+b
  m=(f-e)p+e

  with A-H the corners of the cube (000 100 010 110 001 101 110 111)
  and p q r the rations (0-1) of the u v w sides

  m=
  A(-pqr+pq+pr+qr-p-q-r+1) +
  B(pqr-pq-pr+p) +
  C(pqr-pq-qr+q) +
  D(-pqr+pq) +
  E(pqr-pr-qr+r) +
  F(-pqr+pr) +
  G(-pqr+qr) +
  H(pqr)

*/

int scalGetRangeXYZVal(dbmScalNode *node, const char *prop, float *pos, float *res)
{
  double xyz[3],uvw[3],p,q,r,pq,pr,qr,pqr;
  double p1,q1,r1;
  int u,v,w;
  float vA,vB,vC,vD,vE,vF,vG,vH;

  float cp[3];
  cp[0] = node->transform.cen[0];
  cp[1] = node->transform.cen[1];
  cp[2] = node->transform.cen[2];

  if(clStrcmp(prop,"dist")) {
    (*res) = sqrtf(
		    (pos[0]-cp[0])*(pos[0]-cp[0])+
		    (pos[1]-cp[1])*(pos[1]-cp[1])+
		    (pos[2]-cp[2])*(pos[2]-cp[2])
		    );
  } else {
    // default
    xyz[0]=pos[0];
    xyz[1]=pos[1];
    xyz[2]=pos[2];
    
    scalXYZtoUVW(node->field, xyz, uvw);
    
    u=(int)floor(uvw[0]);
    v=(int)floor(uvw[1]);
    w=(int)floor(uvw[2]);
    
    p=uvw[0]-floor(uvw[0]);
    q=uvw[1]-floor(uvw[1]);
    r=uvw[2]-floor(uvw[2]);
    p1=1.0-p;
    q1=1.0-q;
    r1=1.0-r;
    
    //  pq=p*q; pr=p*r; qr=q*r; pqr=p*q*r;
    
    vA=scalReadField(node->field,u+0,v+0,w+0);
    vB=scalReadField(node->field,u+1,v+0,w+0);
    vC=scalReadField(node->field,u+0,v+1,w+0);
    vD=scalReadField(node->field,u+1,v+1,w+0);
    vE=scalReadField(node->field,u+0,v+0,w+1);
    vF=scalReadField(node->field,u+1,v+0,w+1);
    vG=scalReadField(node->field,u+0,v+1,w+1);
    vH=scalReadField(node->field,u+1,v+1,w+1);
    
    (*res)=vA*p1*q1*r1+
      vB*p*q1*r1+
      vC*p1*q*r1+
      vD*p*q*r1+
      vE*p1*q1*r+
      vF*p*q1*r+
      vG*p1*q*r+
      vH*p*q*r;
    
    /*
      (*res)=vA*(-pqr+pq+pr+qr-p-q-r+1.0)+
      vB*(pqr-pq-pr+p) +
      vC*(pqr-pq-qr+q) +
      vD*(-pqr+pq) +
      vE*(pqr-pr-qr+r) +
      vF*(-pqr+pr) +
      vG*(-pqr+qr) +
      vH*(pqr);
    */
    /*
      if(fabs((*res))>fabs(vA) &&
      fabs((*res))>fabs(vB) &&
      fabs((*res))>fabs(vC) &&
      fabs((*res))>fabs(vD) &&
      fabs((*res))>fabs(vE) &&
      fabs((*res))>fabs(vF) &&
      fabs((*res))>fabs(vG) &&
      fabs((*res))>fabs(vH)) {
      }
    */
  }
  return 0;
}

int scalSub(dbmScalNode *n,char *s)
{
  char message[256];
  dbmScalNode *n2;
  scalField *f1,*f2;
  int u,v,w;
  float v1,v2;

  if(!comIsDB(s)) {
    sprintf(message,"scalSub: Unknown db: %s\n",s);
    comMessage(message);
    return -1;
  }
  if((comGetDB(s))->common.type!=DBM_NODE_SCAL) {
    sprintf(message,"scalSub: %s is not a scalar field db\n",s);
    comMessage(message);
    return -1;
  }
  n2=&(comGetDB(s))->scalNode;
  
  f1=n->field;
  f2=n2->field;

  if(f1->u_size!=f2->u_size ||
     f1->v_size!=f2->v_size ||
     f1->w_size!=f2->w_size) {
    comMessage("scalSub: size missmatch\n");
    return -1;
  }
     
  for(u=f1->u1;u<f1->u2;u++)
    for(v=f1->v1;v<f1->v2;v++)
      for(w=f1->w1;w<f1->w2;w++) {
	v1=scalReadField(f1,u,v,w);
	v2=scalReadField(f2,u,v,w);
	scalWriteField(f1,u,v,w,v1-v2);
      }

  return 0;
}

float scalRMSD(scalField *f)
{
  int i,size;
  double count,sum,mean,sqd;
  
  size = f->u_size*f->v_size*f->w_size;
  count = (double)size;
  sum=0.0;
  for(i=0;i<size;i++) {
    sum+=(double)f->data[i];
  }

  mean = sum/count;
  sqd = 0.0;
  for(i=0;i<size;i++) {
    sqd +=((double)f->data[i]-mean)*((double)f->data[i]-mean);
  }
  return (float) (sqd/(count-1.0));
}
