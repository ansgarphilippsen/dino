#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "com.h"
#include "dbm.h"
#include "rex.h"
#include "mat.h"
#include "render.h"
#include "conn.h"
#include "Cmalloc.h"
#include "cgfx.h"
#include "struct_db.h"
#include "struct_obj.h"
#include "cl.h"
#include "bspline.h"
#include "symm.h"
#ifdef BUILD
#include "build.h"
#endif

/* 
   statically defined return value 
*/

//static char struct_obj_return[256];

static void prep_symview(structObj* obj);
static void gen_spline_nd(cgfxPoint *sp, int spc);
static int com_render(structObj *obj, int wc, char **wl)
{
  int od;
  float bw;
  char message[256];

  od=obj->render.detail1;
  bw=obj->render.bond_width;
  if(renderSet(&obj->render,wc-1,wl+1)!=0) {
    sprintf(message,"%s: syntax error in render statement\n",obj->node->name);
    comMessage(message);
    return -1;
  }
  
  if(obj->type==STRUCT_CONNECT) {
    if(obj->render.mode!=RENDER_SIMPLE && 
       obj->render.mode!=RENDER_CPK &&
       obj->render.mode!=RENDER_CUSTOM) {
      obj->render.mode=RENDER_SIMPLE;
      comMessage("invalid render mode\n");
      return -1;
    }
  } else {
    if(obj->render.mode==RENDER_HELIX ||
       obj->render.mode==RENDER_STRAND ||
       obj->render.mode==RENDER_STRAND2) {
      comMessage("render modes helix and strand no longer supported\n");
      obj->render.mode=RENDER_TUBE;
    }
    
    if(obj->render.mode!=RENDER_SIMPLE && 
       obj->render.mode!=RENDER_CUSTOM &&
       obj->render.mode!=RENDER_TUBE &&
       obj->render.mode!=RENDER_HSC &&
       obj->render.mode!=RENDER_SLINE) {
      obj->render.mode=RENDER_SIMPLE;
      comMessage("invalid render mode\n");
      return -1;
    } else {
      if(obj->render.mode==RENDER_TUBE ||
	 obj->render.mode==RENDER_HSC ||
	 obj->render.mode==RENDER_SLINE) {
	structSmooth(obj);
      }
    }
  }
  if(obj->render.detail1!=od) {
    comNewDisplayList(obj->sphere_list);
    cgfxSphere(1.0,obj->render.detail1);
    comEndDisplayList();
  }
  
  if(obj->render.mode==RENDER_CUSTOM ||
     obj->render.mode==RENDER_CPK) {
    structObjGenVA(obj);
  }
  
  comRedraw();
  
  return 0;
}

int structObjCommand(struct DBM_STRUCT_NODE *node,structObj *obj,int wc,char **wl)
{
  int i;
  float p[]={0.0,0.0,0.0};
  char message[256];
  char *empty_com[]={"get","center"};

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
    // could also call ComGet directly
  }  
  if(!strcmp(wl[0],"help") ||
     !strcmp(wl[0],"?")) {
  } else if(!strcmp(wl[0],"show")) {
    obj->render.show=1;
    comShowObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"hide")) {
    obj->render.show=0;
    comHideObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"clear")) {
    for(i=0;i<obj->atom_count;i++)
      obj->atom[i].label=0;
    comRedraw();
  } else if(!strcmp(wl[0],"get")) {
    return structObjComGet(obj, wc-1,wl+1);
  } else if(!strcmp(wl[0],"render")) {
    return com_render(obj,wc,wl);
  } else if(!strcmp(wl[0],"material")) {
    if(wc<2) {
      comMessage(renderGetMaterial(&obj->render.mat));
    } else {
      if(renderMaterialSet(&obj->render.mat,wc-1,wl+1)!=0)
	return -1;
      
      comRedraw();
    }
  } else if(!strcmp(wl[0],"set")) {
    return structObjComSet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"renew")) {
    return structObjComRenew(obj, wc-1,wl+1);
  } else if(!strcmp(wl[0],"write")) {
    return structWrite(obj->node,obj,wc-1,wl+1);
#ifdef BUILD
  } else if(!strcmp(wl[0],"edit")) {
    if(obj->build!=NULL) {
      sprintf(message,"already in edit mode\n");
      comMessage(message);
      return -1;
    } else {
      return structObjEdit(obj,wc-1,wl+1);
    }
  } else if(!strcmp(wl[0],"merge")) {
    if(obj->build==NULL) {
      sprintf(message,"%s only available in edit mode\n",wl[0]);
      comMessage(message);
      return -1;
    } else {
      return structObjMerge(obj,wc-1,wl+1);
    }
  } else if(!strcmp(wl[0],"unedit")) {
    if(obj->build==NULL) {
      sprintf(message,"%s only available in edit mode\n",wl[0]);
      comMessage(message);
      return -1;
    } else {
      return structObjUnedit(obj,wc-1,wl+1);
    }
  } else if(!strcmp(wl[0],"grab")) {
    if(obj->build==NULL) {
      sprintf(message,"%s only available in edit mode\n",wl[0]);
      comMessage(message);
      return -1;
    } else {
      return structObjGrab(obj,wc-1,wl+1);
    }
  } else if(!strcmp(wl[0],"reset")) {
    if(obj->build==NULL) {
      sprintf(message,"%s only available in edit mode\n",wl[0]);
      comMessage(message);
      return -1;
    } else {
      return structObjReset(obj,wc-1,wl+1);
    }
  } else if(!strcmp(wl[0],"fix")) {
    if(obj->build==NULL) {
      sprintf(message,"%s only available in edit mode\n",wl[0]);
      comMessage(message);
      return -1;
    } else {
      return structObjFix(obj,wc-1,wl+1);
    }
#endif
    /***********
  } else if(clStrcmp(wl[0],"tunnelvision")) {
    tunnelvision(obj);
    ***********/
  } else {
    sprintf(message,"%s: unknown command %s\n",obj->name,wl[0]);
    comMessage(message);
    return -1;    
  }

  return 0;
}

static void structObjRefresh(structObj *obj)
{
  if(obj->render.mode==RENDER_TUBE ||
     obj->render.mode==RENDER_HSC ||
     obj->render.mode==RENDER_SLINE) {
    structSmooth(obj);
  } else if(obj->render.mode==RENDER_CUSTOM ||
	    obj->render.mode==RENDER_CPK) {
    structObjGenVA(obj);
  }
}

int structObjComRenew(structObj *obj, int wc, char **wl)
{
  int i;
  clOrg co;
  Set set;
  Select sel;
  int set_flag,sel_flag;
  char message[256];
  int ret;

  clNew(&co,wc,wl);
  set_flag=0;
  sel_flag=0;
  ret=0;
  for(i=0;i<co.param_count;i++) {
    if(co.param[i].p==NULL) {
      if(co.param[i].wc!=0) {
	comMessage("error: renew: expected an argument beginning with -\n"); 
	ret=-1;
	break;
      }
    } else if(clStrcmp(co.param[i].p,"name") || 
	      clStrcmp(co.param[i].p,"n")) {
      comMessage("error: renew: -name is not allowed\n");
      ret=-1;
      break;
    } else if(clStrcmp(co.param[i].p,"type") ||
	      clStrcmp(co.param[i].p,"t")) {
      comMessage("error: renew: -type is not allowed\n");
      ret=-1;
      break;
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
  
  if(!set_flag) {
    setNew(&set,0,NULL);
  }

  if(!sel_flag) {
    ret=structObjRenew(obj, &set, &obj->select);
  } else {
    ret=structObjRenew(obj, &set, &sel);
    selectDelete(&obj->select);
    memcpy(&obj->select,&sel,sizeof(Select));
  }

  structObjRecalcCenter(obj);

  com_render(obj,0,NULL);
  
  setDelete(&set);
  clDelete(&co);
  comRedraw();
  return ret;
}

int structObjComSet(structObj *obj, int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  ret=structObjSet(obj, &set,1);

  setDelete(&set);

  structObjRefresh(obj);

  comRedraw();

  return ret;
}


int structObjComGet(structObj *obj, int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"%s: missing property\n", obj->name);
    comMessage(message);
    return -1;
  }
  for(i=0;i<wc;i++)
    if(structObjGet(obj,wl[i])<0)
      return -1;

  return 0;
}


int structObjRenew(structObj *obj, Set *set, Select *sel)
{
  char message[256];
  // set object properties before renewing
  if(structObjSet(obj,set,0)<0)
    return -1;

  // handle symview if requested
  prep_symview(obj);

  memset(obj->atom_flag,0,sizeof(unsigned char)*obj->node->atom_count);

  if(obj->type==STRUCT_TRACE) {
    comMessage("tracing ... ");
    if(structObjTrace(obj->node, obj, sel)<0)
      return -1;
  } else if(obj->type==STRUCT_CONNECT) {
    comMessage("connecting ... ");
    if(structObjConnect(obj->node, obj, sel)<0)
      return -1;
#ifdef WITH_NCBONDS
  } else if(obj->type==STRUCT_NBOND) {
    comMessage("nbonding ... ");
    if(structObjNbond(obj->node, obj, sel)<0)
      return -1;
#endif
  }

  sprintf(message,"%d atoms with %d bonds\n",obj->atom_count, obj->bond_count);
  comMessage(message);

  // set object properties after renewing
  if(structObjSet(obj,set,1)<0)
    return -1;

  return 0;
}


// flag is 0 for pre-creation settings, 1 otherwise
int structObjSet(structObj *obj, Set *set, int flag)
{
  int ac,f,pc;
  struct POV_VALUE *val;
  float r,g,b,r2,g2,b2,r3,g3,b3;
  float rval,p[3],frac1,frac2,rval1,rval2;
  float vmin,vmax;
  char message[256];

  if(set->pov_count==0) {
    return 0;
  }

  // assign numeric ids to all pov expressions
  for(pc=0;pc<set->pov_count;pc++) {
    if(clStrcmp(set->pov[pc].prop,"color") ||
       clStrcmp(set->pov[pc].prop,"colour") ||
       clStrcmp(set->pov[pc].prop,"col")) {
      set->pov[pc].id=STRUCT_PROP_COLOR;
    } else if(clStrcmp(set->pov[pc].prop,"color1") ||
	    clStrcmp(set->pov[pc].prop,"colour1") ||
	    clStrcmp(set->pov[pc].prop,"col1")) {
      set->pov[pc].id=STRUCT_PROP_COLOR1;
    } else if(clStrcmp(set->pov[pc].prop,"color2") ||
	    clStrcmp(set->pov[pc].prop,"colour2") ||
	    clStrcmp(set->pov[pc].prop,"col2")) {
      set->pov[pc].id=STRUCT_PROP_COLOR2;
    } else if(clStrcmp(set->pov[pc].prop,"color3") ||
	    clStrcmp(set->pov[pc].prop,"colour3") ||
	    clStrcmp(set->pov[pc].prop,"col3")) {
      set->pov[pc].id=STRUCT_PROP_COLOR3;
    } else if(clStrcmp(set->pov[pc].prop,"vdwr") ||
	      clStrcmp(set->pov[pc].prop,"radius") ||
	      clStrcmp(set->pov[pc].prop,"rad")) {
      set->pov[pc].id=STRUCT_PROP_RAD;
    } else if(clStrcmp(set->pov[pc].prop,"symview")) {
      set->pov[pc].id=STRUCT_PROP_SYMVIEW;
    } else if(clStrcmp(set->pov[pc].prop,"symcount")) {
      set->pov[pc].id=STRUCT_PROP_SYMCOUNT;
    } else {
      comMessage("error: set: unknown property \n");
      comMessage(set->pov[pc].prop);
      return -1;
    }
    if(set->pov[pc].op!=POV_OP_EQ ) {
      comMessage("error: set: expected operator = for property ");
      comMessage(set->pov[pc].prop);
      comMessage("\n");
      return -1;
    }
    if(set->pov[pc].val_count>1) {
      comMessage("error: set: expected only one value for property ");
      comMessage(set->pov[pc].prop);
      comMessage("\n");
      return -1;
    }
  }


  if(flag) {
    // post-creation stuff

    if(set->range_flag) {
      // get min and max
      // if dataset is self
      if(set->range.src==NULL) {
	if(structGetMinMax(obj->node, set->range.prop,&vmin,&vmax)<0) {
	  sprintf(message,"error: unknown range property %s\n",set->range.prop);
	  comMessage(message);
	  return -1;
	}
      } else {
	if(dbmGetMinMax(set->range.src,set->range.prop,&vmin,&vmax)<0) {
	  sprintf(message,"error: unknown range property %s\n",set->range.prop);
	  comMessage(message);
	  return -1;
	}
      }
      if(clStrcmp(set->range.val1,"min"))
	rval1=vmin;
      else if(clStrcmp(set->range.val1,"max"))
	rval1=vmax;
      else
	rval1=atof(set->range.val1);
      
      if(clStrcmp(set->range.val2,"min"))
	rval2=vmin;
      else if(clStrcmp(set->range.val2,"max"))
	rval2=vmax;
      else
	rval2=atof(set->range.val2);
      
      sprintf(message,"using range of property %s from %g to %g\n",
	      set->range.prop,rval1,rval2);
      comMessage(message);
    }
    
    
    /* 
       pre-evaluation and object properties
    */
    for(pc=0;pc<set->pov_count;pc++) {
      val=povGetVal(&set->pov[pc],0);
      switch(set->pov[pc].id) {
	
      case STRUCT_PROP_COLOR:
      case STRUCT_PROP_COLOR1:
      case STRUCT_PROP_COLOR2:
      case STRUCT_PROP_COLOR3:
	if(comGetColor(val->val1,&r3,&g3,&b3)<0) {
	  sprintf(message,"error: set: unknown color %s\n",val->val1);
	  comMessage(message);
	  return -1;
	}
	if(set->range_flag) {
	  if(comGetColor(val->val2,&r2,&g2,&b2)<0) {
	    sprintf(message,"error: set: unknown color %s\n",val->val2);
	    comMessage(message);
	    return -1;
	  }
	}
#ifdef WITH_NCBONDS
	if(obj->type==STRUCT_NBOND && set->pov[pc].id==STRUCT_PROP_COLOR) {
	  obj->nbond_prop.r=r;
	  obj->nbond_prop.g=g;
	  obj->nbond_prop.b=b;
	}
#endif
	break;
      }
    }
    
    // go through all atoms of object and evaluate all "set" properties
    for(ac=0;ac<obj->atom_count;ac++) {
      f=0;
      if(set->select_flag) {
	if(structIsAtomSelected(obj->node, obj->atom[ac].ap, &set->select))
	  f=1;
      } else {
	f=1;
      }
      if(f) {
	for(pc=0;pc<set->pov_count;pc++) {
	  val=povGetVal(&set->pov[pc],0);
	  switch(set->pov[pc].id) {
	  case STRUCT_PROP_RAD:
	    if(val->range_flag) {
	      r=atof(val->val1);
	      r2=atof(val->val2);
	      if(set->range.src==NULL) {
		// this dataset
		if(structGetRangeVal(obj->node,obj->atom[ac].ap,set->range.prop,&rval)<0)
		  return -1;
	      } else {
		if(dbmGetRangeVal(&set->range,(float*)obj->atom[ac].ap->p,&rval)<0)
		  return -1;
	      }
	      // fprintf(stderr,"%f %f %f\n",rval1,rval2,rval);
	      frac1=rval2-rval1;
	      frac2=rval-rval1;
	      if(frac1==0.0) {
		if(frac2==0.0)
		  frac2=0.5;
		else
		  frac2=-2.0;
	      } else {
		frac2/=frac1;
	      }
	      if(set->range.clamp) {
		if(frac2<0.0) {
		  frac2=0.0;
		} else if(frac2>1.0) {
		  frac2=1.0;
		}
	      }
	      if(frac2>=0.0 && frac2<=1.0) {
		obj->atom[ac].prop.radius=(r2-r)*frac2+r;
	      }
	    } else {
	      obj->atom[ac].prop.radius=atof(val->val1);
	    }
	    break;
	  case STRUCT_PROP_COLOR:
	  case STRUCT_PROP_COLOR1:
	  case STRUCT_PROP_COLOR2:
	  case STRUCT_PROP_COLOR3:
	    r=r3; g=g3; b=b3;
	    if(set->range_flag) {
	      if(set->range.src==NULL) {
		// this dataset
		if(structGetRangeVal(obj->node,obj->atom[ac].ap,set->range.prop,&rval)<0)
		  return -1;
	      } else {
		if(dbmGetRangeVal(&set->range,(float*)obj->atom[ac].ap->p,&rval)<0)
		  return -1;
	      }
	      frac1=rval2-rval1;
	      frac2=rval-rval1;
	      if(frac1==0.0) {
		if(frac2==0.0)
		  frac2=0.5;
		else
		  frac2=-2.0;
	      } else {
		frac2/=frac1;
	      }
	      if(set->range.clamp) {
		if(frac2<0.0) {
		  frac2=0.0;
		} else if(frac2>1.0) {
		  frac2=1.0;
		}
	      }
	      if(frac2>=0.0 && frac2<=1.0) {
		r+=(r2-r)*frac2;
		g+=(g2-g)*frac2;
		b+=(b2-b)*frac2;
		if(set->pov[pc].id==STRUCT_PROP_COLOR) {
		  obj->atom[ac].prop.r=r;
		  obj->atom[ac].prop.g=g;
		  obj->atom[ac].prop.b=b;
		  obj->atom[ac].prop.c[0][0]=r;
		  obj->atom[ac].prop.c[0][1]=g;
		  obj->atom[ac].prop.c[0][2]=b;
		  obj->atom[ac].prop.c[1][0]=r;
		  obj->atom[ac].prop.c[1][1]=g;
		  obj->atom[ac].prop.c[1][2]=b;
		  obj->atom[ac].prop.c[2][0]=r;
		  obj->atom[ac].prop.c[2][1]=g;
		  obj->atom[ac].prop.c[2][2]=b;
		} else if(set->pov[pc].id==STRUCT_PROP_COLOR1) {
		  obj->atom[ac].prop.c[0][0]=r;
		  obj->atom[ac].prop.c[0][1]=g;
		  obj->atom[ac].prop.c[0][2]=b;
		} else if(set->pov[pc].id==STRUCT_PROP_COLOR2) {
		  obj->atom[ac].prop.c[1][0]=r;
		  obj->atom[ac].prop.c[1][1]=g;
		  obj->atom[ac].prop.c[1][2]=b;
		} else if(set->pov[pc].id==STRUCT_PROP_COLOR3) {
		  obj->atom[ac].prop.c[2][0]=r;
		  obj->atom[ac].prop.c[2][1]=g;
		  obj->atom[ac].prop.c[2][2]=b;
		}
	      }
	    } else {
	      if(set->pov[pc].id==STRUCT_PROP_COLOR) {
		obj->atom[ac].prop.r=r;
		obj->atom[ac].prop.g=g;
		obj->atom[ac].prop.b=b;
		obj->atom[ac].prop.c[0][0]=r;
		obj->atom[ac].prop.c[0][1]=g;
		obj->atom[ac].prop.c[0][2]=b;
		obj->atom[ac].prop.c[1][0]=r;
		obj->atom[ac].prop.c[1][1]=g;
		obj->atom[ac].prop.c[1][2]=b;
		obj->atom[ac].prop.c[2][0]=r;
		obj->atom[ac].prop.c[2][1]=g;
		obj->atom[ac].prop.c[2][2]=b;
	      } else if(set->pov[pc].id==STRUCT_PROP_COLOR1) {
		obj->atom[ac].prop.c[0][0]=r;
		obj->atom[ac].prop.c[0][1]=g;
		obj->atom[ac].prop.c[0][2]=b;
	      } else if(set->pov[pc].id==STRUCT_PROP_COLOR2) {
		obj->atom[ac].prop.c[1][0]=r;
		obj->atom[ac].prop.c[1][1]=g;
		obj->atom[ac].prop.c[1][2]=b;
	      } else if(set->pov[pc].id==STRUCT_PROP_COLOR3) {
		obj->atom[ac].prop.c[2][0]=r;
		obj->atom[ac].prop.c[2][1]=g;
		obj->atom[ac].prop.c[2][2]=b;
	      }
	    }
	    break;
	  }
	}
      }
    }
  } else {
    // this is evaluated for pre-creation (flag==0)
    for(pc=0;pc<set->pov_count;pc++) {
      val=povGetVal(&set->pov[pc],0);
      switch(set->pov[pc].id) {
      case STRUCT_PROP_SYMVIEW:
	obj->symview=atoi(val->val1);
	break;
      case STRUCT_PROP_SYMCOUNT:
	obj->symcount=atoi(val->val1);
	break;
      }
    }
  }

  return 0;
}



int structObjGet(structObj *obj, char *prop)
{
  int i;
  char message[256];
  float x,y,z,v[3];
  dbmStructNode *node=obj->node;

  if(!strcmp(prop,"center")) {
    sprintf(message,"{%.5f,%.5f,%.5f}",obj->center[0],obj->center[1],obj->center[2]);
    comReturn(message);
  } else {
    sprintf(message,"%s: get: unknown parameter %s\n", obj->name,prop);
    comMessage(message);
    return -1;
  }
  return 0;
}

int structObjCp(structObj *o1, structObj *o2)
{
  return -1;
}

static int atom_to_point(structAtom *a, struct STRUCT_ATOM_PROP *pr, cgfxSplinePoint *p, struct RENDER *r) {
  int ret=0;
  if(a->residue->clss==STRUCT_PROTEIN) {
    p->v[0]=a->p->x;
    p->v[1]=a->p->y;
    p->v[2]=a->p->z;
    p->colp[0]=pr->c[0];
    p->colp[1]=pr->c[1];
    p->colp[2]=pr->c[2];
    p->colp[0][3]=r->transparency;
    p->colp[1][3]=r->transparency;
    p->colp[2][3]=r->transparency;
    p->rad=pr->radius;
    switch(a->residue->type) {
    case STRUCT_RTYPE_COIL: p->id=CGFX_COIL; break;
    case STRUCT_RTYPE_HELIX: p->id=CGFX_HELIX; break;
    case STRUCT_RTYPE_STRAND: p->id=CGFX_STRAND; break;
    default: p->id=CGFX_COIL;
    }
    p->v1=a->residue->v1;
    ret=1;
  } else if(a->residue->clss==STRUCT_NA) {
    if(r->mode==RENDER_TUBE) {
      p->v[0]=a->p->x;
      p->v[1]=a->p->y;
      p->v[2]=a->p->z;
    } else {
      if(r->na_method==0) {
	p->v[0]=a->residue->v0[0];
	p->v[1]=a->residue->v0[1];
	p->v[2]=a->residue->v0[2];
      } else {
	p->v[0]=a->p->x;
	p->v[1]=a->p->y;
	p->v[2]=a->p->z;
      }
    }
    p->rad=pr->radius;
    p->id=CGFX_NA;
    p->v1=a->residue->v1;
    p->v2=a->residue->v2;
    p->v3=a->residue->v3;
    p->v4=a->residue->v4;
    p->v5=a->residue->v5;
    if(r->na_method==0)
      p->v6=a->residue->v6;
    else
      p->v6=a->residue->v7;
    p->res_id=a->residue->res_id;
    p->colp[0]=pr->c[0];
    p->colp[1]=pr->c[1];
    p->colp[2]=pr->c[2];
    p->colp[0][3]=r->transparency;
    p->colp[1][3]=r->transparency;
    p->colp[2][3]=r->transparency;
    ret=1;
  } else {
    // ignore
  }
  return ret;
}

static void make_helix_cyl(cgfxSplinePoint *pl,int h1,int h2, int pc) 
{
  int i;
  float n1[3],n2[3],n3[3],n4[3],n5[3],delta;
  n1[0]=pl[h1+0].v[0];
  n1[1]=pl[h1+0].v[1];
  n1[2]=pl[h1+0].v[2];
  n2[0]=pl[h1+1].v[0];
  n2[1]=pl[h1+1].v[1];
  n2[2]=pl[h1+1].v[2];
  n3[0]=pl[h1+2].v[0];
  n3[1]=pl[h1+2].v[1];
  n3[2]=pl[h1+2].v[2];
  n4[0]=(n1[0]+n2[0]+n3[0])/3.0;
  n4[1]=(n1[1]+n2[1]+n3[1])/3.0;
  n4[2]=(n1[2]+n2[2]+n3[2])/3.0;
  pl[h1+1].v[0]=n4[0];
  pl[h1+1].v[1]=n4[1];
  pl[h1+1].v[2]=n4[2];
  
  n1[0]=pl[h2-2].v[0];
  n1[1]=pl[h2-2].v[1];
  n1[2]=pl[h2-2].v[2];
  n2[0]=pl[h2-1].v[0];
  n2[1]=pl[h2-1].v[1];
  n2[2]=pl[h2-1].v[2];
  n3[0]=pl[h2-0].v[0];
  n3[1]=pl[h2-0].v[1];
  n3[2]=pl[h2-0].v[2];
  n4[0]=(n1[0]+n2[0]+n3[0])/3.0;
  n4[1]=(n1[1]+n2[1]+n3[1])/3.0;
  n4[2]=(n1[2]+n2[2]+n3[2])/3.0;
  pl[h2-1].v[0]=n4[0];
  pl[h2-1].v[1]=n4[1];
  pl[h2-1].v[2]=n4[2];
  
  n1[0]=pl[h1+1].v[0];
  n1[1]=pl[h1+1].v[1];
  n1[2]=pl[h1+1].v[2];
  n2[0]=pl[h2-1].v[0];
  n2[1]=pl[h2-1].v[1];
  n2[2]=pl[h2-1].v[2];
  n3[0]=n2[0]-n1[0];
  n3[1]=n2[1]-n1[1];
  n3[2]=n2[2]-n1[2];
  
  delta=1.0/(float)(h2-h1);
  /*
  n1[0]-=delta*n3[0];
  n1[1]-=delta*n3[1];
  n1[2]-=delta*n3[2];
  n2[0]+=delta*n3[0];
  n2[1]+=delta*n3[1];
  n2[2]+=delta*n3[2];
  */
  n4[0]=pl[h1+2].v[0]-n1[0];
  n4[1]=pl[h1+2].v[1]-n1[1];
  n4[2]=pl[h1+2].v[2]-n1[2];
  matfCalcCross(n3,n4,n5);

  for(i=h1+2;i<=h2-2;i++) {
    pl[i].v[0]=n1[0]+n3[0]*delta*(float)(i-h1);
    pl[i].v[1]=n1[1]+n3[1]*delta*(float)(i-h1);
    pl[i].v[2]=n1[2]+n3[2]*delta*(float)(i-h1);
  }

  matfNormalize(n3,n3);
  matfNormalize(n4,n4);
  matfNormalize(n5,n5);

  for(i=h1+1;i<=h2-1;i++) {
    pl[i].d[0]=-n4[0];
    pl[i].d[1]=-n4[1];
    pl[i].d[2]=-n4[2];
    pl[i].n[0]=n5[0];
    pl[i].n[1]=n5[1];
    pl[i].n[2]=n5[2];
  }
  /*
  pl[h1+1].d[0]=(pl[h1+1].d[0]+pl[h1].d[0])*0.5;
  pl[h1+1].d[1]=(pl[h1+1].d[1]+pl[h1].d[1])*0.5;
  pl[h1+1].d[2]=(pl[h1+1].d[2]+pl[h1].d[2])*0.5;
  pl[h1+1].n[0]=(pl[h1+1].n[0]+pl[h1].n[0])*0.5;
  pl[h1+1].n[1]=(pl[h1+1].n[1]+pl[h1].n[1])*0.5;
  pl[h1+1].n[2]=(pl[h1+1].n[2]+pl[h1].n[2])*0.5;

  if(h1>0) {
    pl[h1].d[0]=(pl[h1].d[0]+pl[h1-1].d[0])*0.5;
    pl[h1].d[1]=(pl[h1].d[1]+pl[h1-1].d[1])*0.5;
    pl[h1].d[2]=(pl[h1].d[2]+pl[h1-1].d[2])*0.5;
    pl[h1].n[0]=(pl[h1].n[0]+pl[h1-1].n[0])*0.5;
    pl[h1].n[1]=(pl[h1].n[1]+pl[h1-1].n[1])*0.5;
    pl[h1].n[2]=(pl[h1].n[2]+pl[h1-1].n[2])*0.5;
  }

  pl[h2-1].d[0]=(pl[h2-1].d[0]+pl[h2].d[0])*0.5;
  pl[h2-1].d[1]=(pl[h2-1].d[1]+pl[h2].d[1])*0.5;
  pl[h2-1].d[2]=(pl[h2-1].d[2]+pl[h2].d[2])*0.5;
  pl[h2-1].n[0]=(pl[h2-1].n[0]+pl[h2].n[0])*0.5;
  pl[h2-1].n[1]=(pl[h2-1].n[1]+pl[h2].n[1])*0.5;
  pl[h2-1].n[2]=(pl[h2-1].n[2]+pl[h2].n[2])*0.5;

  if(h2<pc-1) {
    pl[h2].d[0]=(pl[h2].d[0]+pl[h2+1].d[0])*0.5;
    pl[h2].d[1]=(pl[h2].d[1]+pl[h2+1].d[1])*0.5;
    pl[h2].d[2]=(pl[h2].d[2]+pl[h2+1].d[2])*0.5;
    pl[h2].n[0]=(pl[h2].n[0]+pl[h2+1].n[0])*0.5;
    pl[h2].n[1]=(pl[h2].n[1]+pl[h2+1].n[1])*0.5;
    pl[h2].n[2]=(pl[h2].n[2]+pl[h2+1].n[2])*0.5;
  }
  */
}

int structSmooth(struct STRUCT_OBJ *obj)
{
  int i,j,bc,pc;
  cgfxSplinePoint *point_list;
  cgfxPoint *spoint_list;
  cgfxVA va;
  int detail,flag,h1,h2;
  float n1[4],n2[4],n3[4],n4[4];
  
  // reference
  n4[0]=1.0; n4[1]=0.0; n4[2]=0.0;
  
  // maximal size
  point_list=Ccalloc(obj->bond_count+10,sizeof(cgfxSplinePoint));
  
  // detail level for number of segments
  detail=obj->render.detail2;
  if(detail<1)
    detail=1;
  
  // clean obj va
  if(obj->va.max>0)
    Cfree(obj->va.p);
  obj->va.p=NULL;
  obj->va.max=0;
  obj->va.count=0;
  
  // go through each segment
  bc=0;

  while(bc<obj->bond_count) {
    
    // generate consecutive points
    pc=0;
    while(1) {
      if(atom_to_point(obj->bond[bc].atom1,
		       obj->bond[bc].prop1,
		       &point_list[pc],
		       &obj->render)) {
	pc++;
      }
      
      bc++;
      if(bc>=obj->bond_count)
	break;
      if(obj->bond[bc-1].atom2->n!=obj->bond[bc].atom1->n) {
	break;
      }
    }

    atom_to_point(obj->bond[bc-1].atom2,
		  obj->bond[bc-1].prop2,
		  &point_list[pc],
		  &obj->render);
    pc++;
    
    // set strands correctly for arrow generation
    
    for(i=0;i<pc-1;i++) {
      if(point_list[i].id==CGFX_STRAND && point_list[i+1].id!=CGFX_STRAND)
	point_list[i].id=CGFX_STRAND2;
    }
    if(point_list[i].id==CGFX_STRAND)
      point_list[i].id=CGFX_STRAND2;

    // generate normals
    for(i=0;i<pc;i++) {
      if(i==0) {
	n1[0]=point_list[0].v[0]-point_list[1].v[0];
	n1[1]=point_list[0].v[1]-point_list[1].v[1];
	n1[2]=point_list[0].v[2]-point_list[1].v[2];
	n2[0]=point_list[2].v[0]-point_list[1].v[0];
	n2[1]=point_list[2].v[1]-point_list[1].v[1];
	n2[2]=point_list[2].v[2]-point_list[1].v[2];
      } else if(i==pc-1) {
	n1[0]=point_list[i-2].v[0]-point_list[i-1].v[0];
	n1[1]=point_list[i-2].v[1]-point_list[i-1].v[1];
	n1[2]=point_list[i-2].v[2]-point_list[i-1].v[2];
	n2[0]=point_list[i].v[0]-point_list[i-1].v[0];
	n2[1]=point_list[i].v[1]-point_list[i-1].v[1];
	n2[2]=point_list[i].v[2]-point_list[i-1].v[2];
      } else {
	n1[0]=point_list[i-1].v[0]-point_list[i].v[0];
	n1[1]=point_list[i-1].v[1]-point_list[i].v[1];
	n1[2]=point_list[i-1].v[2]-point_list[i].v[2];
	n2[0]=point_list[i+1].v[0]-point_list[i].v[0];
	n2[1]=point_list[i+1].v[1]-point_list[i].v[1];
	n2[2]=point_list[i+1].v[2]-point_list[i].v[2];
      }
      // use n2 as direction
      point_list[i].d[0]=n2[0];
      point_list[i].d[1]=n2[1];
      point_list[i].d[2]=n2[2];

      matfNormalize(point_list[i].d,point_list[i].d);

      matfCalcCross(n1,n2,n3);

      // check against reference

      if(matfCalcDot(n3,n4)<0) {
	matfCalcCross(n2,n1,n3);
      }

      matfNormalize(n3,point_list[i].n);

      // take as new reference
      matfNormalize(n3,n4);
    }

    // if strand method is 1
    if(obj->render.strand_method==1) {
      n4[0]=point_list[0].v[0];
      n4[1]=point_list[0].v[1];
      n4[2]=point_list[0].v[2];
      for(i=1;i<pc-1;i++) {
	if(point_list[i].id==CGFX_STRAND) {
	  if(point_list[i-1].id==CGFX_STRAND) {
	    n1[0]=n4[0];
	    n1[1]=n4[1];
	    n1[2]=n4[2];
	  } else {
	    if(i+2>=pc) {
	      n1[0]=n4[0];
	      n1[1]=n4[1];
	      n1[2]=n4[2];
	    } else {
	      n3[0]=point_list[i+1].v[0]-point_list[i+2].v[0];
	      n3[1]=point_list[i+1].v[1]-point_list[i+2].v[1];
	      n3[2]=point_list[i+1].v[2]-point_list[i+2].v[2];
	      n1[0]=point_list[i].v[0]+n3[0];
	      n1[1]=point_list[i].v[1]+n3[1];
	      n1[2]=point_list[i].v[2]+n3[2];
	    }
	  }
	  if(point_list[i+1].id==CGFX_STRAND) {
	    n2[0]=point_list[i+1].v[0];
	    n2[1]=point_list[i+1].v[1];
	    n2[2]=point_list[i+1].v[2];
	  } else {
	    if(i-2<0) {
	      n2[0]=point_list[i+1].v[0];
	      n2[1]=point_list[i+1].v[1];
	      n2[2]=point_list[i+1].v[2];
	    } else {
	      n3[0]=point_list[i-2].v[0]-point_list[i-1].v[0];
	      n3[1]=point_list[i-2].v[1]-point_list[i-1].v[1];
	      n3[2]=point_list[i-2].v[2]-point_list[i-1].v[2];
	      n2[0]=point_list[i].v[0]+n3[0];
	      n2[1]=point_list[i].v[1]+n3[1];
	      n2[2]=point_list[i].v[2]+n3[2];
	    }
	  }
	  n3[0]=n1[0]*0.25+n2[0]*0.25+point_list[i].v[0]*0.5;
	  n3[1]=n1[1]*0.25+n2[1]*0.25+point_list[i].v[1]*0.5;
	  n3[2]=n1[2]*0.25+n2[2]*0.25+point_list[i].v[2]*0.5;
	  n4[0]=point_list[i].v[0];
	  n4[1]=point_list[i].v[1];
	  n4[2]=point_list[i].v[2];
	  point_list[i].v[0]=n3[0];
	  point_list[i].v[1]=n3[1];
	  point_list[i].v[2]=n3[2];
	  //	fprintf(stderr,"%f %f %f\n",n3[0],n3[1],n3[2]);
	} else {
	  n4[0]=point_list[i].v[0];
	  n4[1]=point_list[i].v[1];
	  n4[2]=point_list[i].v[2];
	}
      }
    }

    /***
    if(obj->render.helix_method==1) {
      i=0;
      while(i<pc) {
	for(;i<pc;i++)
	  if(point_list[i].id==CGFX_HELIX)
	    break;
	if(i>0)
	  h1=i-1;
	else
	  h2=i;

	for(;i<pc;i++)
	  if(point_list[i].id!=CGFX_HELIX)
	    break;
	if(i<pc-1)
	  h2=i;
	else
	  h2=i-1;

	if(h2-h1>2)
	  make_helix_cyl(point_list,h1,h2,pc);
      }
    }
    **/


    // generate spline, spoint_list is allocated
    bsplineGenerate(point_list, &spoint_list, pc, detail, obj->render.cgfx_flag);
    
    /*
      this is not used !
      while the algorithm is elegant, the normals for the bb
      should not be 'random' but rather dependent change of direction
      as calculated above
       gen_spline_nd(spoint_list,pc*detail+detail);
    */
    // should transparency be set here ??

    // create cgfx object
    cgfxGenHSC(&va, point_list, pc, &obj->render);

    cgfxAppend(&obj->va,&va);

    Cfree(va.p);

    //obj->sp=spoint_list;
    //obj->spc=pc*detail;
    Cfree(spoint_list);

  } // next segment

  Cfree(point_list);
  return 0;
}

/*
  generate normal and direction vectors based on spline points
*/

static void gen_spline_nd(cgfxPoint *sp, int spc)
{
  int i,c;
  double n0[4],d0[4],ax[4],n1[4],d1[4],ra,rm[16];

  // initial direction
  for(i=0;i<3;i++)
    d0[i]=(double)(sp[1].v[i]-sp[0].v[i]);
  d0[3]=1.0;
  matNormalize(d0,d0);

  // initial normal
  d1[0]=-d0[1]; d1[1]=d0[0]; d1[2]=0.0;
  d1[3]=1.0;
  matCalcCross(d0,d1,n0);
  n0[3]=1.0;
  n0[3]=1.0;
  ax[3]=1.0;

  for(i=0;i<3;i++) {
    sp[0].d[i]=d0[i];
    sp[0].n[i]=n0[i];
  }

  for(c=1;c<spc-1;c++) {
    // new direction from point[c]
    for(i=0;i<3;i++)
      d1[i]=(double)(sp[c+1].v[i]-sp[c].v[i]);
    matNormalize(d1,d1);
    
    // check that both dir vectors are not equal
    if(!(d0[0]==d1[0] && d0[1]==d1[1] && d0[2]==d1[2])) {
      // calculate rotation matrix to go from old to new dir
      matTransformAtoB(d0,d1,rm);
      matMultMV(rm,n0,n1);
    } else {
      //fprintf(stderr,"eq\n");
      for(i=0;i<3;i++)
	n1[i]=n0[i];
    }
    
    // n1 and d1 contain new direction and normal vectors
    for(i=0;i<3;i++) {
      sp[c].d[i]=d1[i];
      sp[c].n[i]=n1[i];
      //matfNormalize(sp[c].n,sp[c].n);
      d0[i]=d1[i];
      n0[i]=n1[i];
    }
  }
  for(i=0;i<3;i++) {
    sp[spc-1].d[i]=d0[i];
    sp[spc-1].n[i]=n0[i];
  }

}

static void gen_spline_nd2(cgfxPoint *sp, int spc)
{
  int i,c;
  double n0[4],d0[4],ax[4],n1[4],d1[4],ra,rm[16];

  // initial direction
  for(i=0;i<3;i++)
    d0[i]=(double)(sp[1].v[i]-sp[0].v[i]);
  d0[3]=1.0;
  matNormalize(d0,d0);

  // initial normal
  d1[0]=d0[1]; d1[1]=d0[2]; d1[2]=d0[0];
  d1[3]=1.0;
  matCalcCross(d0,d1,n0);
  n0[3]=1.0;
  n0[3]=1.0;
  ax[3]=1.0;

  for(i=0;i<3;i++) {
    sp[0].d[i]=d0[i];
    sp[0].n[i]=n0[i];
  }

  for(c=1;c<spc-1;c++) {
    // new direction from point[c]
    for(i=0;i<3;i++)
      d1[i]=(double)(sp[c+1].v[i]-sp[c].v[i]);
    matNormalize(d1,d1);
    
    // check that both dir vectors are not equal
    if(!(d0[0]==d1[0] && d0[1]==d1[1] && d0[2]==d1[2])) {
      // axis that runs perp to both d0 and d1
      matCalcCross(d0,d1,ax);
      // angle to rotate around this axis to go from d0 to d1
      ra=asin(matCalcDot(d1,d0)/(matCalcLen(d0)*matCalcLen(d1)));
      // generate rotation matrix around axis
      matMakeRotMat(ra*180.0/M_PI,ax[0],ax[1],ax[2],rm);
      // apply this matrix to current normal
      matTranspose(rm,rm);
      matMultVM(n0,rm,n1);
      fprintf(stderr,"%f\n",180.0/M_PI*asin(matCalcDot(n0,n1)));
      matNormalize(n1,n1);
    } else {
      //fprintf(stderr,"eq\n");
      for(i=0;i<3;i++)
	n1[i]=n0[i];
    }
    
    // n1 and d1 contain new direction and normal vectors
    for(i=0;i<3;i++) {
      sp[c].d[i]=d1[i];
      sp[c].n[i]=n1[i];
      //matfNormalize(sp[c].n,sp[c].n);
      d0[i]=d1[i];
      n0[i]=n1[i];
    }
  }
  for(i=0;i<3;i++) {
    sp[spc-1].d[i]=d0[i];
    sp[spc-1].n[i]=n0[i];
  }

}



int structObjConnect(dbmStructNode *node, structObj *obj, Select *sel)
{
  int i,ac,rc,cc,mc,pass,obj_ac,mode;
  int bc,obj_bc,hit,atom1,atom2;
  struct STRUCT_ATOM *cap,*bap1,*bap2;
  struct STRUCT_RESIDUE *crp;
  struct STRUCT_CHAIN *ccp;
  struct STRUCT_MODEL *cmp;
  struct STRUCT_OBJ_ATOM *oap1,*oap2,*bap;
  int r;

  mode=node->smode;

  for(bc=0;bc<node->bond_count;bc++) {
    node->bond[bc].oap1=NULL;
    node->bond[bc].oap2=NULL;
  }

  obj_ac=0;
  for(pass=0;pass<=1;pass++) {
    for(mc=0;mc<node->model_count;mc++) {
      cmp=&node->model[mc];
      for(cc=0;cc<cmp->chain_count;cc++) {
	ccp=cmp->chain[cc];
	for(rc=0;rc<ccp->residue_count;rc++) {
	  crp=ccp->residue[rc];
	  if(mode==SEL_RESIDUE)
	    hit=0;
	  for(ac=0;ac<crp->atom_count;ac++) {
	    cap=crp->atom[ac];
	    if(pass==0)
	      r=structIsAtomSelected(obj->node,cap,sel);
	    else
	      r=obj->atom_flag[cap->n];
	    if(r<0)
	      return -1;
	    if(r>0) {
		if(mode!=SEL_ATOM) {
		  hit=1;
		  break;
		} else {
		  if(pass==1) {
		    memcpy(&obj->atom[obj_ac].prop,&cap->def_prop,sizeof(struct STRUCT_ATOM_PROP));
		    obj->atom[obj_ac].label=0;
		    obj->atom[obj_ac].ap=cap;
		    obj->atom[obj_ac].cc=0;

		    for(bc=0;bc<cap->bondc;bc++) {
		      if(node->bond[cap->bondi[bc]].atom1==cap)
			node->bond[cap->bondi[bc]].oap1=&obj->atom[obj_ac];
		      if(node->bond[cap->bondi[bc]].atom2==cap)
			node->bond[cap->bondi[bc]].oap2=&obj->atom[obj_ac];
		    }

		  } else {
		    obj->atom_flag[cap->n]=1;
		  }
		  obj_ac++;
		}
	      }
	  }
	  if(mode==SEL_RESIDUE && hit) {
	    for(ac=0;ac<crp->atom_count;ac++) {
	      cap=crp->atom[ac];
	      if(!cap->restriction) {
		if(pass==1) {
		  memcpy(&obj->atom[obj_ac].prop,&cap->def_prop,sizeof(struct STRUCT_ATOM_PROP));
		  obj->atom[obj_ac].label=0;
		  obj->atom[obj_ac].ap=cap;
		  obj->atom[obj_ac].cc=0;

		  for(bc=0;bc<cap->bondc;bc++) {
		    if(node->bond[cap->bondi[bc]].atom1==cap)
		      node->bond[cap->bondi[bc]].oap1=&obj->atom[obj_ac];
		    if(node->bond[cap->bondi[bc]].atom2==cap)
		      node->bond[cap->bondi[bc]].oap2=&obj->atom[obj_ac];
		    
		  }

		} else {
		  obj->atom_flag[cap->n]=1;
		}
		obj_ac++;
	      }
	    }
	  }
	}
      }
    }
    if(pass==0) {
      if(obj->atom_count>0)
	Cfree(obj->atom);
      obj->atom=Ccalloc(obj_ac,sizeof(struct STRUCT_OBJ_ATOM));
      obj_ac=0;
    }
  }
  obj->atom_count=obj_ac;

  if(obj->bond_count>0)
    Cfree(obj->bond);
  obj_bc=0;
  obj->bond=Ccalloc(node->bond_count,sizeof(struct STRUCT_BOND));

  comMessage(".");

  bap=obj->atom;

  for(bc=0;bc<node->bond_count;bc++) {
    oap1=node->bond[bc].oap1;
    oap2=node->bond[bc].oap2;
    if(oap1!=NULL && oap2!=NULL) {
      memcpy(&obj->bond[obj_bc],&node->bond[bc],
	     sizeof(struct STRUCT_BOND));
      obj->bond[obj_bc].prop1=&oap1->prop;
      obj->bond[obj_bc].prop2=&oap2->prop;
      obj_bc++;
      ++(oap1->cc);
      ++(oap2->cc);
    }
  }
  obj->bond_count=obj_bc;

  obj_bc=0;
  if(obj->s_bond_count>0)
    Cfree(obj->s_bond);
  /*
    TODO
    realloc memory once amount of s_bonds is determined
  */
  obj->s_bond=Ccalloc(node->atom_count,sizeof(struct STRUCT_SINGULAR_BOND));

  /* find atoms that are selected but do not have any bonds */
  for(ac=0;ac<obj->atom_count;ac++) {
    if(obj->atom[ac].cc==0) {
      obj->s_bond[obj_bc].atom=obj->atom[ac].ap;

      for(i=0;i<node->atom_table_len;i++)
	if(node->atom_table[i].z==obj->atom[ac].ap->chem.an)
	  break;
      obj->s_bond[obj_bc].prop=&obj->atom[ac].prop;

      obj_bc++;
    }
  }

  obj->s_bond_count=obj_bc;

  return 0;
}


int structObjTrace(struct DBM_STRUCT_NODE *node, structObj *obj, Select *sel)
{
  int i,rc,ac,ac2,pass,r;
  int obj_ac,obj_bc;
  char tp1[4];
  char tp2[4];
  struct STRUCT_ATOM *ap;

  double v1[3],v2[3],vdiff[3];
  double vcyl[]={0.0,0.0,1.0};
  double axis[3];
  double length, dotproduct;
  double angle=0.0;

  float cadd[4];

  strcpy(tp1,"CA");
  strcpy(tp2,"P");
  
  obj_ac=0;
  for(pass=0;pass<=1;pass++) {
    for(rc=0;rc<node->residue_count;rc++) {
      for(ac=0;ac<node->residue[rc].atom_count;ac++) {
	r=0;
	if(node->residue[rc].clss==STRUCT_PROTEIN) {
	  if(!strcmp("CA",node->residue[rc].atom[ac]->name)) {
	    r=structIsAtomSelected(obj->node,node->residue[rc].atom[ac],sel);
	  }
	} else if(node->residue[rc].clss==STRUCT_NA) {
	  if(!strcmp("P",node->residue[rc].atom[ac]->name)){
	    r=structIsAtomSelected(obj->node,node->residue[rc].atom[ac],sel);
	  }
	} else {
	  if(!strcmp("CA",node->residue[rc].atom[ac]->name) ||
	     !strcmp("P",node->residue[rc].atom[ac]->name)) {
	    r=structIsAtomSelected(obj->node,node->residue[rc].atom[ac],sel);
	  }
	}
	if(r<0)
	  return -1;
	if(r>0) {
	  if(pass==1) {
	    for(i=0;i<node->atom_table_len;i++)
	      if(node->atom_table[i].z==node->residue[rc].atom[ac]->chem.an)
		break;
	    memcpy(&obj->atom[obj_ac].prop,
		   &node->residue[rc].atom[ac]->def_prop,
		   sizeof(struct STRUCT_ATOM_PROP));
	    obj->atom[obj_ac].ap=node->residue[rc].atom[ac];
	  }
	  obj_ac++;
	  break; // goto next residue
	}
      }
    }
    if(pass==0) {
      if(obj->atom_count>0) {
	Cfree(obj->atom);
	/*
	Cfree(obj->trace_v1);
	Cfree(obj->trace_v2);
	*/
      }
      obj->atom=Ccalloc(obj_ac,sizeof(struct STRUCT_OBJ_ATOM));
      /*
      obj->trace_v1=Ccalloc(obj_ac,sizeof(structVect));
      obj->trace_v2=Ccalloc(obj_ac,sizeof(structVect));
      */
      obj->atom_count=obj_ac;
      obj_ac=0;
    }
  }
  if(obj_ac==0) {
    obj->bond_count=0;
    return -1;
  }
  
  obj_bc=0;
  for(pass=0;pass<=1;pass++) {
    for(ac=1;ac<obj_ac;ac++)
      if(obj->atom[ac-1].ap->model->num==obj->atom[ac].ap->model->num)
	if(!strcmp(obj->atom[ac-1].ap->chain->name,obj->atom[ac].ap->chain->name))
	  
	  if((obj->atom[ac].ap->residue->num-obj->atom[ac-1].ap->residue->num)==1) {
	    
	    if(pass==1) {
	      obj->bond[obj_bc].atom1=obj->atom[ac-1].ap;
	      obj->bond[obj_bc].prop1=&obj->atom[ac-1].prop;
	      
	      obj->bond[obj_bc].atom2=obj->atom[ac].ap;
	      obj->bond[obj_bc].prop2=&obj->atom[ac].prop;

	      v1[0]=obj->bond[obj_bc].atom1->p->x;
	      v1[1]=obj->bond[obj_bc].atom1->p->y;
	      v1[2]=obj->bond[obj_bc].atom1->p->z;
	      v2[0]=obj->bond[obj_bc].atom2->p->x;
	      v2[1]=obj->bond[obj_bc].atom2->p->y;
	      v2[2]=obj->bond[obj_bc].atom2->p->z;
	      vdiff[0]=v2[0]-v1[0];
	      vdiff[1]=v2[1]-v1[1];
	      vdiff[2]=v2[2]-v1[2];
	      
	      length=sqrt(vdiff[0]*vdiff[0]+
			  vdiff[1]*vdiff[1]+
			  vdiff[2]*vdiff[2]);
	      
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
	      obj->bond[obj_bc].axis[0]=axis[0];
	      obj->bond[obj_bc].axis[1]=axis[1];
	      obj->bond[obj_bc].axis[2]=axis[2];
	      obj->bond[obj_bc].length=length;
	      obj->bond[obj_bc].angle=angle;
	      
	      glMatrixMode(GL_MODELVIEW);
	      glPushMatrix();
	      glLoadIdentity();
	      glRotated(angle,
			axis[0],
			axis[1],
			axis[2]);
	      glGetFloatv(GL_MODELVIEW_MATRIX,obj->bond[obj_bc].rotmat);
	      glPopMatrix();
	      

	    }
	    obj_bc++;
	  }
    if(pass==0) {
      if(obj->bond_count>0)
	Cfree(obj->bond);
      obj->bond=Ccalloc(obj_bc,sizeof(struct STRUCT_BOND));
      obj->bond_count=obj_bc;
      obj_bc=0;
    }
  } 

  return 0;
}


int structObjIsWithin(structObj *obj, float *p, float d2)
{
  int i;
  float dx,dy,dz;
  struct STRUCT_APOS *ap;

  for(i=0;i<obj->atom_count;i++) {
    ap=obj->atom[i].ap->p;
    dx=p[0]-ap->x;
    if(dx*dx<d2) {
      dy=p[1]-ap->y;
      if(dy*dy<d2) {
	dz=p[2]-ap->z;
	if(dz*dz<d2) {
	  if(dx*dx+dy*dy+dz*dz<d2)
	    return 1;
	}
      }
    }
  }
  return 0;
}

#ifdef WITH_NCBONDS

int structObjNbond(struct DBM_STRUCT_NODE *node, structObj *obj, Select *sel)
{
  int bc,ret;
  struct STRUCT_ATOM *ap1,*ap2;
  int *bond_indx,bond_count;

  bond_indx=Ccalloc(node->nbond_count,sizeof(int));
  bond_count=0;

  /*
    go through all non-covalent bonds
  */
  for(bc=0;bc<node->nbond_count;bc++) {
    /*
      if both atoms of ncbond are selected
      remember this nvbond index
    */
    ret=structIsAtomSelected(node,node->nbond[bc].atom1,sel);
    if(ret<0) {
      return -1;
    } else if(ret>0) {
      ret=structIsAtomSelected(node,node->nbond[bc].atom2,sel);
      if(ret<0) {
	return -1;
      } else if(ret>0) {    
      bond_indx[bond_count++]=bc;
      }
    }
  }

  obj->model_count=0;
  obj->chain_count=0;
  obj->residue_count=0;
  obj->atom_count=0;
  obj->bond_count=bond_count;

  obj->bond=Ccalloc(bond_count,sizeof(struct STRUCT_BOND));

  obj->nbond_prop.r=1.0;
  obj->nbond_prop.g=1.0;
  obj->nbond_prop.b=0.0;

  /*
    go through all found bonds and
    transfer them to object
  */
  for(bc=0;bc<bond_count;bc++) {
    obj->bond[bc].atom1=node->nbond[bond_indx[bc]].atom1;
    obj->bond[bc].atom2=node->nbond[bond_indx[bc]].atom2;
    obj->bond[bc].prop1=&obj->nbond_prop;
    obj->bond[bc].prop2=&obj->nbond_prop;
  }

  Cfree(bond_indx);

  structRecalcBondList(obj->bond,bond_count);

  return 0;
}
#endif

/***************************
int structObjNbond2(struct DBM_STRUCT_NODE *node, structObj *obj, Select *sel)
{
  int ac;
  float co=3.0,co2,pos[3];
  caPointer *cp;
  int cpc,lc;
  float dx,dy,dz;
  struct STRUCT_ATOM *cap,*lap;
  struct STRUCT_NBOND *nbond,*onb;
  int nbond_count,nbond_max;
  
  co2=co*co;

  if(obj->nbond_count>0)
    Cfree(obj->nbond);

  obj->atom_count=0;
  obj->bond_count=0;

  nbond_count=0;
  nbond_max=node->bond_count;
  nbond=Ccalloc(nbond_max,sizeof(struct STRUCT_NBOND));

  cp=Ccalloc(node->atom_count*2,sizeof(caPointer));

  for(ac=0;ac<node->atom_count;ac++) {
    cap=&node->atom[ac];
    pos[0]=cap->p->x;
    pos[1]=cap->p->y;
    pos[2]=cap->p->z;
    caGetWithinList(node->ca, pos, co, &cp,&cpc);
    for(lc=0;lc<cpc;lc++) {
      lap=(struct STRUCT_ATOM *)cp[lc];
      if(lap->n>cap->n) {
	dx=cap->p->x-lap->p->x;
	if(dx*dx<co2) {
	  dy=cap->p->y-lap->p->y;
	  if(dy*dy<co2) {
	    dz=cap->p->z-lap->p->z;
	    if(dz*dz<co2) {
	      if(dx*dx+dy*dy+dz*dz<co2) {
		if((cap->flag&STRUCT_NBD && lap->flag&STRUCT_NBA) ||
		   (cap->flag&STRUCT_NBA && lap->flag&STRUCT_NBD)) {

		  nbond[nbond_count].atom1=cap;
		  nbond[nbond_count].atom2=lap;
		  nbond[nbond_count].c[0]=0.7;
		  nbond[nbond_count].c[1]=1.0;
		  nbond[nbond_count].c[2]=0.2;

		  if(nbond_count>=nbond_max) {
		    onb=nbond;
		    nbond=Ccalloc(nbond_max+1000,sizeof(struct STRUCT_NBOND));
		    memcpy(nbond,onb,sizeof(struct STRUCT_NBOND)*nbond_max);
		    Cfree(onb);
		    nbond_max+=1000;
		  }

		}
		   
	      }
	    }
	  }
	}
      }
    }
  }

  Cfree(cp);

  obj->nbond_count=nbond_count;
  obj->nbond=Ccalloc(nbond_count,sizeof(struct STRUCT_NBOND));
  memcpy(obj->nbond,nbond,sizeof(struct STRUCT_NBOND)*nbond_count);
  Cfree(nbond);

  return 0;
}
*******************/

int structObjGenVA(structObj *obj)
{
  int size;
  int i;
  float bw,sr,col[4],mid[3];
  int detail;
  float sti,sto;

#ifdef USE_DLIST
  obj->va_list_flag=0;
#endif

  if(obj->render.mode==RENDER_CPK) {
    // generate a sphere for each atom
    if(obj->va.p!=NULL) {
      Cfree(obj->va.p);
    }

    detail=obj->render.detail1;
    size=((detail*2-2)*detail*4*2+2*detail*2)*2*obj->atom_count;
    
    obj->va.count=0;
    /* guess initial size */
    obj->va.max=size;
    obj->va.p=Crecalloc(NULL,obj->va.max,sizeof(cgfxVAField));

    for(i=0;i<obj->atom_count;i++) {
      col[0]=obj->atom[i].prop.r;
      col[1]=obj->atom[i].prop.g;
      col[2]=obj->atom[i].prop.b;
      col[3]=obj->render.transparency;
      sr=obj->atom[i].prop.radius;
      cgfxSphereVA(sr,(float *)obj->atom[i].ap->p,col,&obj->va,detail);
    }
  } else if(obj->render.mode==RENDER_CUSTOM) {
    /*
      generate a cylinder for each bond
      and a sphere for each atom
    */
    
    if(obj->va.p!=NULL) {
      Cfree(obj->va.p);
    }

    detail=obj->render.detail1;

    size=((detail*2-2)*detail*4*2+2*detail*2)*3*obj->atom_count;
    size+=detail*4*2*3*2;

    obj->va.count=0;
    /* guess initial size */
    obj->va.max=size*2;

    obj->va.p=Crecalloc(NULL,obj->va.max,sizeof(cgfxVAField));

    bw=obj->render.bond_width;
    sr=obj->render.sphere_radius;

    if(obj->render.stipple_flag) {
      sti=obj->render.stipplei;
      sto=obj->render.stippleo;
    } else {
      sti=1.0;
      sto=0.0;
    }

    // only create explicit spheres if they are larger than the cylinders
    if(sr>bw && sr>0.0)
      for(i=0;i<obj->atom_count;i++) {
	col[0]=obj->atom[i].prop.r;
	col[1]=obj->atom[i].prop.g;
	col[2]=obj->atom[i].prop.b;
	col[3]=obj->render.transparency;
	cgfxSphereVA(sr,(float *)obj->atom[i].ap->p,col,&obj->va,detail);
      }

    if(bw>0.0) {
      for(i=0;i<obj->bond_count;i++) {
	mid[0]=obj->bond[i].atom1->p->x+obj->bond[i].atom2->p->x;
	mid[1]=obj->bond[i].atom1->p->y+obj->bond[i].atom2->p->y;
	mid[2]=obj->bond[i].atom1->p->z+obj->bond[i].atom2->p->z;
	mid[0]*=0.5;
	mid[1]*=0.5;
	mid[2]*=0.5;
	col[0]=obj->bond[i].prop1->r;
	col[1]=obj->bond[i].prop1->g;
	col[2]=obj->bond[i].prop1->b;
	col[3]=obj->render.transparency;
	if(sr<=bw)
	  cgfxGenCylinder(&obj->va,
			  (float *)obj->bond[i].atom1->p,mid,
			  bw,sti,sto,detail,CGFX_ROUND_BEGIN,col);
	else
	  cgfxGenCylinder(&obj->va,
			  (float *)obj->bond[i].atom1->p,mid,
			  bw,sti,sto,detail,CGFX_BLUNT,col);
      
	col[0]=obj->bond[i].prop2->r;
	col[1]=obj->bond[i].prop2->g;
	col[2]=obj->bond[i].prop2->b;
	col[3]=obj->render.transparency;
	if(sr<=bw)
	  cgfxGenCylinder(&obj->va,
			  mid,(float *)obj->bond[i].atom2->p,
			  bw,sti,sto,detail,CGFX_ROUND_END,col);
	else
	  cgfxGenCylinder(&obj->va,
			  mid,(float *)obj->bond[i].atom2->p,
			  bw,sti,sto,detail,CGFX_BLUNT,col);
      }
    }

    if(sr>0.0) {
      for(i=0;i<obj->s_bond_count;i++) {
	col[0]=obj->s_bond[i].prop->r;
	col[1]=obj->s_bond[i].prop->g;
	col[2]=obj->s_bond[i].prop->b;
	col[3]=obj->render.transparency;
	cgfxSphereVA(sr,(float *)obj->s_bond[i].atom->p,col,&obj->va,detail);
      }
    }

    obj->va.max=obj->va.count;

    obj->va.p=Crecalloc(obj->va.p,obj->va.max,sizeof(cgfxVAField));
  }
  return 0;
}

#ifdef BUILD

int structObjEdit(structObj *obj, int wc, char **wl)
{
  char message[256];
  int i;

  if(obj->build!=NULL) {
    sprintf(message,".%s.%s: already in editing mode\n",
	    obj->node->name, obj->name);
    return -1;
  } else {
    obj->build = Cmalloc(sizeof(buildInst));

    obj->build->atom=Ccalloc(obj->atom_count,sizeof(struct STRUCT_ATOM));
    obj->build->apos=Ccalloc(obj->atom_count,sizeof(struct STRUCT_APOS));
    obj->build->apos_orig=Ccalloc(obj->atom_count,sizeof(struct STRUCT_APOS));
    obj->build->ap_save=Ccalloc(obj->atom_count,sizeof(structAtom *));
    obj->build->atom_count=obj->atom_count;
    obj->build->apos_size=obj->atom_count*sizeof(struct STRUCT_APOS);

    for(i=0;i<obj->atom_count;i++) {
      memcpy(&obj->build->atom[i],obj->atom[i].ap,sizeof(struct STRUCT_ATOM));
      memcpy(&obj->build->apos[i],obj->atom[i].ap->p,sizeof(struct STRUCT_APOS));    
      memcpy(&obj->build->apos_orig[i],obj->atom[i].ap->p,sizeof(struct STRUCT_APOS));
      obj->build->ap_save[i]=obj->atom[i].ap;
      obj->build->atom[i].p=&obj->build->apos[i];
      obj->atom[i].ap=&obj->build->atom[i];
    }

    buildGenHierarchy(obj->build);

    transReset(&obj->build->trans);
    //    transReset(&obj->build->trans_frag);

    //    buildGenTor(obj->build);
  }
  return 0;
}

int structObjMerge(structObj *obj, int wc, char **wl)
{
  if(obj->build==NULL)
    return -1;

  // merge the temp atoms back to ds

  structObjUnedit(obj,0,NULL);

  return 0;
}

int structObjUnedit(structObj *obj, int wc, char **wl)
{
  int i;
  if(obj->build==NULL)
    return -1;

  for(i=0;i<obj->atom_count;i++) {
    obj->atom[i].ap=obj->build->ap_save[i];
  }

  Cfree(obj->build->atom);
  Cfree(obj->build->apos);
  Cfree(obj->build->ap_save);
  Cfree(obj->build);
  obj->build=NULL;

  return 0;
}

int structObjReset(structObj *obj, int wc, char **wl)
{
  int i;
  // set all torsion angle deltas to 0
  /*
  for(i=0;i<obj->build->tor_count;i++)
    ;
  */
  // copy apos_orig to apos

  // reset rigid body transformation
  transReset(&obj->build->trans);
  comRedraw();
  return 0;
}

int structObjFix(structObj *obj, int wc, char **wl)
{
  int i;
  double v[3];

  // apply all transformations (torsion and rigid body)
  // then copy apos to apos_orig

  //  structRecalcBondList(obj->bond,obj->bond_count);
  // structObjReset(obj);
  //  comRedraw();

  return 0;
}

int structObjGrab(structObj *obj, int wc, char **wl)
{
  char dev[256],*ax;
  char target[256];
  float fact=1.0;
  int i;

  if(wc<=0) {
    comMessage("error: grab requires at least a device\n");
    return -1;
  }

  clStrncpy(dev,wl[0],256);
  ax=clStrchr(dev,'.');
  if(ax!=NULL) {
    ax[0]='\0';
    ax++;
  }

  clStrcpy(target,"default");
  i=1;
  while(i<wc) {
    if(clStrcmp(wl[i],"-t") ||
       clStrcmp(wl[i],"-target")) {
      i++;
      if(i>=wc) {
	comMessage("error: missing parameter after -target\n");
	return -1;
      }
      strncpy(target,wl[i++],256);
    }
  }

  fprintf(stderr,"%s %s %s\n",dev,ax,target);

  if(comGrab(&obj->build->trans,0,0,dev)<0)
    return -1;

  //  comGetCurrentCenter(obj->build->trans.cen);

  return 0;
}

#endif

int structObjDelete(structObj *obj)
{
  /*
    free all pointers associated with an object
  */
  Cfree(obj->model);
  Cfree(obj->chain);
  Cfree(obj->residue);
  Cfree(obj->atom);
  Cfree(obj->bond);
  Cfree(obj->atom_flag);
  if(obj->va.max>0)
    Cfree(obj->va.p);
  return 0;
}

/*
  called upon each dataset transformation
*/
void structObjUpdateSymview(structObj* obj)
{
  int sc;
  double *cen, *tra, ea, eb,cost,sint,cosp,sinp,posi[2],posa[2],rprime,theta,psi;
  transMat *trans;

  if(obj->symview==2 && obj->node->helical>0) { // helical symmetry
    if(obj->node->helical->axr!=1.0) {
      cen = obj->node->transform.cen;
      tra = obj->node->transform.tra;

      // angle of actual object
      psi = atan2(tra[1]+cen[1],tra[0]+cen[0]);
      cosp = cos(psi);
      sinp = sin(psi);
      // distance from 0,0 is short axis
      ea = sqrt((tra[0]+cen[0])*(tra[0]+cen[0])+(tra[1]+cen[1])*(tra[1]+cen[1]));
      // long axis
      eb = ea * obj->node->helical->axr;

      for(sc=0;sc<obj->symcount*2;sc++) {
	trans = transListGetEntry(&obj->transform_list,sc);

	// relative angle of symobj
	theta = trans->custom[0];
	sint=sin(theta);
	cost=cos(theta);

	// ideal position of symobj, corrected for psi
	rprime=sqrt((ea*ea*eb*eb)/(eb*eb*cost*cost+ea*ea*sint*sint));
	posi[0]=rprime*(cost*cosp-sint*sinp);
	posi[1]=rprime*(sint*cosp+sint*cosp);

	// actual position of symobj due to transformation, corrected for psi
	posa[0]=ea*(cost*cosp-sint*sinp);
	posa[1]=ea*(sint*cosp+sint*cosp);

	// correction
	trans->tra[0]=posi[0]-posa[0];
	trans->tra[1]=posi[1]-posa[1];

      }
    }
  }
}

/*
  prepare the transformations for symmetry
*/
static void prep_symview(structObj* obj)
{
  transMat tmat;
  struct SYMM_INFO sinfo;
  int sc;
  double angle,dist,dr,ea,eb,sina,cosa;
  double *cen,*tra;
  char msg[256];
  /* 
     based on symview settings, generate a list
     of additional transformations to be used
  */
  if(obj->symview!=0) {
    transReset(&tmat);
    if(obj->symview==1) { // xtal symmetry
      if(obj->node->xtal) {
	clStrcpy(sinfo.name,obj->node->xtal->space_group_name);
	if(symGetMatrixByName(&sinfo)<0) {
	  // error
	  sprintf(msg,"no info for spacegroup %s\n",sinfo.name);
	  comMessage(msg);
	  return;
	} else {
	  transListInit(&obj->transform_list,sinfo.mcount);
	  sprintf(msg,"%d symmetry mates generated\n",sinfo.mcount-1);
	  comMessage(msg);
	  for(sc=0;sc<sinfo.mcount;sc++) {
	    transFromSymm(&tmat,&sinfo.mat[sc]);
	    transListAddEntry(&obj->transform_list,&tmat);
	    fprintf(stderr,"%s\n",transGetAll(&tmat));
	  }
	}
      } else { // xtal==0
	comMessage("No crystallographic info in dataset\n");
	return;
      }
    } else if(obj->symview==2) {
      if(obj->node->helical) { // helical info
	transListInit(&obj->transform_list,obj->symcount);
	for(sc=-obj->symcount;sc<=obj->symcount;sc++) {
	  if(sc!=0) {
	    angle = obj->node->helical->angle*(double)sc;
	    tmat.custom[0]=angle*M_PI/180.0;
	    dist = obj->node->helical->dist*(double)sc;
	    tmat.custom[1]=dist;
	    matMakeRotMat(angle,0.0,0.0,1.0,tmat.rot);
	    tmat.tra[2]=dist;
	    transListAddEntry(&obj->transform_list,&tmat);
	  }
	}
	structObjUpdateSymview(obj);
      } else {
	comMessage("no helical symmetry info in dataset\n");
	return;
      }
    } else if(obj->symview==3) {
      transListCopy(&obj->node->symop_list,&obj->transform_list);
    } else {
      sprintf(msg,"invalid value %d for symview, must be 0 (off), 1 (xtal), 2 (helical), or 3 (custom)\n", obj->symview);
    }
  } else {
    transListDelete(&obj->transform_list);
  }
}

void structObjRecalcCenter(structObj* obj)
{
  float x,y,z,v[4];
  v[3]=1.0;
  int i;

  x=0;y=0;z=0;
  for(i=0;i<obj->atom_count;i++) {
    x+=obj->atom[i].ap->p->x;
    y+=obj->atom[i].ap->p->y;
    z+=obj->atom[i].ap->p->z;
  }
  if(obj->atom_count>0) {
    v[0]=x/(double)obj->atom_count;
    v[1]=y/(double)obj->atom_count;
    v[2]=z/(double)obj->atom_count;
  } else {
    v[0]=0.0;
    v[1]=0.0;
    v[2]=0.0;
  }
  transApplyf(&obj->node->transform,v);
  obj->center[0]=v[0];
  obj->center[1]=v[1];
  obj->center[2]=v[2];
}
