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

/* 
   statically defined return value 
*/

//static char struct_obj_return[256];


int structObjCommand(struct DBM_STRUCT_NODE *node,structObj *obj,int wc,char **wl)
{
  int i,od;
  float p[]={0.0,0.0,0.0};
  float bw;
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
    od=obj->render.detail;
    bw=obj->render.bond_width;
    if(renderSet(&obj->render,wc-1,wl+1)!=0) {
      sprintf(message,"\n%s: syntax error in render statement",node->name);
      comMessage(message);
      return -1;
    }

    if(obj->type==STRUCT_CONNECT) {
      if(obj->render.mode!=RENDER_SIMPLE && 
	 obj->render.mode!=RENDER_CPK &&
	 obj->render.mode!=RENDER_CUSTOM) {
	obj->render.mode=RENDER_SIMPLE;
	comMessage("\ninvalid render mode");
	return -1;
      }
    } else {
      if(bw!=obj->render.bond_width) {
	for(i=0;i<obj->atom_count;i++)
	  obj->atom[i].prop.radius=obj->render.bond_width;
      }
      if(obj->render.mode==RENDER_HELIX ||
	 obj->render.mode==RENDER_STRAND ||
	 obj->render.mode==RENDER_STRAND2) {
	comMessage("\nrender modes helix and strand no longer supported");
	obj->render.mode=RENDER_TUBE;
      }

      if(obj->render.mode!=RENDER_SIMPLE && 
	 obj->render.mode!=RENDER_CUSTOM &&
	 obj->render.mode!=RENDER_TUBE &&
	 obj->render.mode!=RENDER_HSC &&
	 obj->render.mode!=RENDER_SLINE) {
	obj->render.mode=RENDER_SIMPLE;
	comMessage("\ninvalid render mode");
	return -1;
      } else {
	if(obj->render.mode==RENDER_TUBE ||
	   obj->render.mode==RENDER_HSC ||
	   obj->render.mode==RENDER_SLINE) {
	  structSmooth(obj);
	}
      }
    }
    if(obj->render.detail!=od) {
      comNewDisplayList(obj->sphere_list);
      cgfxSphere(1.0,obj->render.detail);
      comEndDisplayList();
    }

    if(obj->render.mode==RENDER_CUSTOM) {
      structObjGenVA(obj);
    }

    
    comRedraw();
  } else if(!strcmp(wl[0],"material")) {
    if(wc<2) {
      sprintf(message,"\n%s: missing expression", obj->name);
      comMessage(message);
      return -1;
    }

    if(renderMaterialSet(&obj->render.mat,wc-1,wl+1)!=0)
      return -1;

    comRedraw();
  } else if(!strcmp(wl[0],"set")) {
    return structObjComSet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"renew")) {
    return structObjComRenew(obj, wc-1,wl+1);
  } else if(!strcmp(wl[0],"write")) {
    return structWrite(obj->node,obj,wc-1,wl+1);
  } else {
    sprintf(message,"\n%s: unknown command %s",obj->name,wl[0]);
    comMessage(message);
    return -1;    
  }

  return 0;
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
	comMessage("\nerror: renew: expected an argument beginning with -"); 
	ret=-1;
	break;
      }
    } else if(clStrcmp(co.param[i].p,"name") || 
	      clStrcmp(co.param[i].p,"n")) {
      comMessage("\nerror: renew: -name is not allowed");
      ret=-1;
      break;
    } else if(clStrcmp(co.param[i].p,"type") ||
	      clStrcmp(co.param[i].p,"t")) {
      comMessage("\nerror: renew: -type is not allowed");
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

  comRedraw();

  return ret;
}


int structObjComGet(structObj *obj, int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"\n%s: missing property", obj->name);
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

  memset(obj->atom_flag,0,sizeof(unsigned char)*obj->node->atom_count);

  if(obj->type==STRUCT_TRACE) {
    comMessage("\ntracing ...");
    if(structObjTrace(obj->node, obj, sel)<0)
      return -1;
  } else if(obj->type==STRUCT_CONNECT) {
    comMessage("\nconnecting ...");
    if(structObjConnect(obj->node, obj, sel)<0)
      return -1;
  } else if(obj->type==STRUCT_NBOND) {
    comMessage("\nnbonding ...");
    if(structObjNbond(obj->node, obj, sel)<0)
      return -1;
  }

  sprintf(message,"%d atoms with %d bonds",obj->atom_count, obj->bond_count);
  comMessage(message);

  // set object properties after renewing
  if(structObjSet(obj,set,1)<0)
    return -1;

  return 0;
}



int structObjSet(structObj *obj, Set *set, int flag)
{
  int ac,f,pc;
  struct POV_VALUE *val;
  float r,g,b,r2,g2,b2;
  float rval,p[3],frac1,frac2,rval1,rval2;

  if(flag==0)
    return 0;

  if(set->pov_count==0) {
    return 0;
  }

  if(set->range_flag) {
    rval1=atof(set->range.val1);
    rval2=atof(set->range.val2);
  }

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
    } else if(clStrcmp(set->pov[pc].prop,"uco")) {
      set->pov[pc].id=STRUCT_PROP_UCO;
    } else {
      comMessage("\nerror: set: unknown property ");
      comMessage(set->pov[pc].prop);
      return -1;
    }
    if(set->pov[pc].op!=POV_OP_EQ) {
      comMessage("\nerror: set: expected operator = for property ");
      comMessage(set->pov[pc].prop);
      return -1;
    }
    if(set->pov[pc].val_count>1) {
      comMessage("\nerror: set: expected only one value for property ");
      comMessage(set->pov[pc].prop);
      return -1;
    }
  }

  /* 
     object properties
  */
  for(pc=0;pc<set->pov_count;pc++) {
    val=povGetVal(&set->pov[pc],0);
    switch(set->pov[pc].id) {
    case STRUCT_PROP_COLOR:
      if(obj->type==STRUCT_NBOND) {
	if(comGetColor(val->val1,&r,&g,&b)<0) {
	  comMessage("\nerror: set: unknown color ");
	  comMessage(val->val1);
	  return -1;
	}
	obj->nbond_prop.r=r;
	obj->nbond_prop.g=g;
	obj->nbond_prop.b=b;
      }
      break;
    case STRUCT_PROP_UCO:
      if(val->range_flag) {
	comMessage("\nerror: unexpected range for uco");
	return -1;
      }
      if(matExtract1Df(val->val1,3,p)==-1) {
	comMessage("\nerror: expected uco={x,y,z}");
	return -1;
      }
      obj->uco[0]=p[0];
      obj->uco[1]=p[1];
      obj->uco[2]=p[2];
      if(obj->node->xtal==NULL) {
	comMessage("\nerror: uco requires cell for dataset");
      } else {
	if(p[0]==0.0 && p[1]==0.0 && p[2]==0.0) {
	  obj->uco_flag=0;
	  obj->transform_flag=0;
	} else {
	  obj->uco_flag=1;
	  obj->transform_flag=1;
	  dbmUCOTransform(obj->node->xtal,&obj->transform,p);
	  fprintf(stderr,"\n%f %f %f",
		  obj->transform.tra[0],
		  obj->transform.tra[1],
		  obj->transform.tra[2]);
	}
      }
      break;
    }
  }

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
	    // fprintf(stderr,"\n%f %f %f",rval1,rval2,rval);
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
	  if(set->range_flag) {
	    if(comGetColor(val->val1,&r,&g,&b)<0) {
	      comMessage("\nerror: set: unknown color ");
	      comMessage(val->val1);
	      return -1;
	    }
	    if(comGetColor(val->val2,&r2,&g2,&b2)<0) {
	      comMessage("\nerror: set: unknown color ");
	      comMessage(val->val2);
	      return -1;
	    }
	    if(set->range.src==NULL) {
	      // this dataset
	      if(structGetRangeVal(obj->node,obj->atom[ac].ap,set->range.prop,&rval)<0)
		return -1;
	    } else {
	      if(dbmGetRangeVal(&set->range,(float*)obj->atom[ac].ap->p,&rval)<0)
		return -1;
	    }
	    //	    fprintf(stderr,"\n%f %f %f",rval1,rval2,rval);
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
	    if(frac2>=0.0 && frac2<=1.0) {
	      obj->atom[ac].prop.r=(r2-r)*frac2+r;
	      obj->atom[ac].prop.g=(g2-g)*frac2+g;
	      obj->atom[ac].prop.b=(b2-b)*frac2+b;
	    }
	  } else {
	    if(comGetColor(val->val1,&r,&g,&b)<0) {
	      comMessage("\nerror: set: unknown color ");
	      comMessage(val->val1);
	      return -1;
	    }
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

  return 0;
}



int structObjGet(structObj *obj, char *prop)
{
  int i;
  char message[256];
  float x,y,z,v[3];
  dbmStructNode *node=obj->node;

  if(!strcmp(prop,"center")) {
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
    transApplyf(&node->transform,v);
    sprintf(message,"{%.5f,%.5f,%.5f}",v[0],v[1],v[2]);
    comReturn(message);
  } else {
    sprintf(message,"\n%s: get: unknown parameter %s", obj->name,prop);
    comMessage(message);
    return -1;
  }
  return 0;
}

int structObjCp(structObj *o1, structObj *o2)
{
  return -1;
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

  // detail level from render struct
  detail=obj->render.detail;
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
      if(obj->bond[bc].atom1->residue->class==STRUCT_PROTEIN) {
	point_list[pc].v[0]=obj->bond[bc].atom1->p->x;
	point_list[pc].v[1]=obj->bond[bc].atom1->p->y;
	point_list[pc].v[2]=obj->bond[bc].atom1->p->z;
	point_list[pc].c[0]=obj->bond[bc].prop1->r;
	point_list[pc].c[1]=obj->bond[bc].prop1->g;
	point_list[pc].c[2]=obj->bond[bc].prop1->b;
	point_list[pc].c[3]=obj->render.transparency;
	point_list[pc].rad=obj->bond[bc].prop1->radius;
	switch(obj->bond[bc].atom1->residue->type) {
	case STRUCT_RTYPE_COIL: point_list[pc].id=CGFX_COIL; break;
	case STRUCT_RTYPE_HELIX: point_list[pc].id=CGFX_HELIX; break;
	case STRUCT_RTYPE_STRAND: point_list[pc].id=CGFX_STRAND; break;
	default: point_list[pc].id=CGFX_COIL;
	}
	point_list[pc].v1=obj->bond[bc].atom1->residue->v1;
	pc++;
      } else if(obj->bond[bc].atom1->residue->class==STRUCT_NA) {
	if(obj->render.mode==RENDER_TUBE) {
	  point_list[pc].v[0]=obj->bond[bc].atom1->p->x;
	  point_list[pc].v[1]=obj->bond[bc].atom1->p->y;
	  point_list[pc].v[2]=obj->bond[bc].atom1->p->z;
	} else {
	  if(obj->render.na_method==0) {
	    point_list[pc].v[0]=obj->bond[bc].atom1->residue->v0[0];
	    point_list[pc].v[1]=obj->bond[bc].atom1->residue->v0[1];
	    point_list[pc].v[2]=obj->bond[bc].atom1->residue->v0[2];
	  } else {
	    point_list[pc].v[0]=obj->bond[bc].atom1->p->x;
	    point_list[pc].v[1]=obj->bond[bc].atom1->p->y;
	    point_list[pc].v[2]=obj->bond[bc].atom1->p->z;
	  }
	}
	point_list[pc].rad=obj->bond[bc].prop1->radius;
	// TODO
	/*
	  the DNA can have different rendering types, e.g.
	  the schematic sugar-base representation or
	  just a cylinder across each basepair
	*/ 
	point_list[pc].id=CGFX_NA;
	point_list[pc].v1=obj->bond[bc].atom1->residue->v1;
	point_list[pc].v2=obj->bond[bc].atom1->residue->v2;
	point_list[pc].v3=obj->bond[bc].atom1->residue->v3;
	point_list[pc].v4=obj->bond[bc].atom1->residue->v4;
	point_list[pc].v5=obj->bond[bc].atom1->residue->v5;
	if(obj->render.na_method==0)
	  point_list[pc].v6=obj->bond[bc].atom1->residue->v6;
	else
	  point_list[pc].v6=obj->bond[bc].atom1->residue->v7;
	point_list[pc].res_id=obj->bond[bc].atom1->residue->res_id;
	point_list[pc].c[0]=obj->bond[bc].prop1->c[0][0];
	point_list[pc].c[1]=obj->bond[bc].prop1->c[0][1];
	point_list[pc].c[2]=obj->bond[bc].prop1->c[0][2];
	point_list[pc].c[3]=obj->render.transparency;
	/* add color2 and color3 */
	point_list[pc].c2[0]=obj->bond[bc].prop1->c[1][0];
	point_list[pc].c2[1]=obj->bond[bc].prop1->c[1][1];
	point_list[pc].c2[2]=obj->bond[bc].prop1->c[1][2];
	point_list[pc].c2[3]=obj->render.transparency;
	point_list[pc].c3[0]=obj->bond[bc].prop1->c[2][0];
	point_list[pc].c3[1]=obj->bond[bc].prop1->c[2][1];
	point_list[pc].c3[2]=obj->bond[bc].prop1->c[2][2];
	point_list[pc].c3[3]=obj->render.transparency;
	pc++;
      } else {
	//	point_list[pc].id=CGFX_COIL;
	/*
	  ignore 
	*/
      }
      //      pc++;
      bc++;
      if(bc>=obj->bond_count)
	break;
      if(obj->bond[bc-1].atom2->n!=obj->bond[bc].atom1->n) {
	
	
	break;
      }
    }

    if(obj->bond[bc-1].atom2->residue->class==STRUCT_PROTEIN) {
      point_list[pc].v[0]=obj->bond[bc-1].atom2->p->x;
      point_list[pc].v[1]=obj->bond[bc-1].atom2->p->y;
      point_list[pc].v[2]=obj->bond[bc-1].atom2->p->z;
      point_list[pc].c[0]=obj->bond[bc-1].prop2->r;
      point_list[pc].c[1]=obj->bond[bc-1].prop2->g;
      point_list[pc].c[2]=obj->bond[bc-1].prop2->b;
      point_list[pc].c[3]=obj->render.transparency;
      point_list[pc].rad=obj->bond[bc-1].prop2->radius;
      switch(obj->bond[bc-1].atom2->residue->type) {
      case STRUCT_RTYPE_COIL: point_list[pc].id=CGFX_COIL; break;
      case STRUCT_RTYPE_HELIX: point_list[pc].id=CGFX_HELIX; break;
      case STRUCT_RTYPE_STRAND: point_list[pc].id=CGFX_STRAND; break;
      default: point_list[pc].id=CGFX_COIL;
      }
      point_list[pc].v1=obj->bond[bc-1].atom2->residue->v1;
    } else if(obj->bond[bc-1].atom2->residue->class==STRUCT_NA) {
      if(obj->render.mode==RENDER_TUBE) {
	point_list[pc].v[0]=obj->bond[bc-1].atom2->p->x;
	point_list[pc].v[1]=obj->bond[bc-1].atom2->p->y;
	point_list[pc].v[2]=obj->bond[bc-1].atom2->p->z;
      } else {
	point_list[pc].v[0]=obj->bond[bc-1].atom2->residue->v0[0];
	point_list[pc].v[1]=obj->bond[bc-1].atom2->residue->v0[1];
	point_list[pc].v[2]=obj->bond[bc-1].atom2->residue->v0[2];
      }
      point_list[pc].rad=obj->bond[bc-1].prop2->radius;
      point_list[pc].id=CGFX_NA;
      point_list[pc].v1=obj->bond[bc-1].atom2->residue->v1;
      point_list[pc].v2=obj->bond[bc-1].atom2->residue->v2;
      point_list[pc].v3=obj->bond[bc-1].atom2->residue->v3;
      point_list[pc].v4=obj->bond[bc-1].atom2->residue->v4;
      point_list[pc].v5=obj->bond[bc-1].atom2->residue->v5;
      point_list[pc].v6=obj->bond[bc-1].atom2->residue->v6;
      point_list[pc].res_id=obj->bond[bc-1].atom2->residue->res_id;
      point_list[pc].c[0]=obj->bond[bc-1].prop2->c[0][0];
      point_list[pc].c[1]=obj->bond[bc-1].prop2->c[0][1];
      point_list[pc].c[2]=obj->bond[bc-1].prop2->c[0][2];
      point_list[pc].c[3]=obj->render.transparency;
      /* add color2 and color3 */
      point_list[pc].c2[0]=obj->bond[bc-1].prop2->c[1][0];
      point_list[pc].c2[1]=obj->bond[bc-1].prop2->c[1][1];
      point_list[pc].c2[2]=obj->bond[bc-1].prop2->c[1][2];
      point_list[pc].c2[3]=obj->render.transparency;
      point_list[pc].c3[0]=obj->bond[bc-1].prop2->c[2][0];
      point_list[pc].c3[1]=obj->bond[bc-1].prop2->c[2][1];
      point_list[pc].c3[2]=obj->bond[bc-1].prop2->c[2][2];
      point_list[pc].c3[3]=obj->render.transparency;
    } else {
      point_list[pc].id=CGFX_COIL;
    }

    pc++;
    
    // set strands correctly for arrow generation
    
    for(i=0;i<pc-1;i++) {
      if(point_list[i].id==CGFX_STRAND && point_list[i+1].id!=CGFX_STRAND)
	point_list[i].id=CGFX_STRAND2;
    }
    if(point_list[i].id==CGFX_STRAND)
      point_list[i].id=CGFX_STRAND2;


    if(obj->render.helix_method==1) {
      i=0;
      while(i<pc) {
	for(;i<pc;i++)
	  if(point_list[i].id==CGFX_HELIX)
	    break;
	h1=i;
	for(;i<pc;i++)
	  if(point_list[i].id!=CGFX_HELIX)
	    break;
	h2=i-1;

	/* helix found between h1 and h2 */
	if(h2-h1>2) {
	  for(i=h1;i<=h2-2;i++) {
	    n1[0]=point_list[i+0].v[0];
	    n1[1]=point_list[i+0].v[1];
	    n1[2]=point_list[i+0].v[2];
	    n2[0]=point_list[i+1].v[0];
	    n2[1]=point_list[i+1].v[1];
	    n2[2]=point_list[i+1].v[2];
	    n3[0]=point_list[i+2].v[0];
	    n3[1]=point_list[i+2].v[1];
	    n3[2]=point_list[i+2].v[2];
	    n4[0]=(n1[0]+n2[0]+n3[0])/3.0;
	    n4[1]=(n1[1]+n2[1]+n3[1])/3.0;
	    n4[2]=(n1[2]+n2[2]+n3[2])/3.0;
	    point_list[i].v[0]=n4[0];
	    point_list[i].v[1]=n4[1];
	    point_list[i].v[2]=n4[2];
	  }
	}
	
      }

    }



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
      /* 
	 this is not used at the moment since its buggy
	 i.e. gives visual artefacts !

      n3[0]=point_list[i].v1[0];
      n3[1]=point_list[i].v1[1];
      n3[2]=point_list[i].v1[2];

      if(matfCalcDot(n3,n4)<0) {
	n3[0]*=-1.0;
	n3[1]*=-1.0;
	n3[2]*=-1.0;
      }
      */

      // take as new reference
      n4[0]=n3[0];
      n4[1]=n3[1];
      n4[2]=n3[2];
      matfNormalize(n4,n4);
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
	  //	fprintf(stderr,"\n%f %f %f",n3[0],n3[1],n3[2]);
	} else {
	  n4[0]=point_list[i].v[0];
	  n4[1]=point_list[i].v[1];
	  n4[2]=point_list[i].v[2];
	}
      }
    }


    // generate spline, spoint_list is allocated
    bsplineGenerate(point_list, &spoint_list, pc, detail, obj->render.cgfx_flag);

    // create cgfx object
    cgfxGenHSC(&va, point_list, pc, &obj->render);

    cgfxAppend(&obj->va,&va);

    Cfree(va.p);

    Cfree(spoint_list);

  } // next segment

  Cfree(point_list);
  return 0;
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
	      if(!cap->restrict) {
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
	if(node->residue[rc].class==STRUCT_PROTEIN) {
	  if(!strcmp("CA",node->residue[rc].atom[ac]->name)) {
	    r=structIsAtomSelected(obj->node,node->residue[rc].atom[ac],sel);
	  }
	} else if(node->residue[rc].class==STRUCT_NA) {
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

  /*
    generate a cylinder for each bond
    and a sphere for each atom
  */

  if(obj->va.p!=NULL) {
    Cfree(obj->va.p);
  }

  detail=1+(int)(obj->render.detail/2);

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

  if(sr>=bw)
    for(i=0;i<obj->atom_count;i++) {
      col[0]=obj->atom[i].prop.r;
      col[1]=obj->atom[i].prop.g;
      col[2]=obj->atom[i].prop.b;
      col[3]=1.0;
      cgfxSphereVA(sr,(float *)obj->atom[i].ap->p,col,&obj->va,detail);
  }

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
    col[3]=1.0;
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
    col[3]=1.0;
    if(sr<=bw)
      cgfxGenCylinder(&obj->va,
		      mid,(float *)obj->bond[i].atom2->p,
		      bw,sti,sto,detail,CGFX_ROUND_END,col);
    else
      cgfxGenCylinder(&obj->va,
		      mid,(float *)obj->bond[i].atom2->p,
		      bw,sti,sto,detail,CGFX_BLUNT,col);
		    
  }

  for(i=0;i<obj->s_bond_count;i++) {
    col[0]=obj->s_bond[i].prop->r;
    col[1]=obj->s_bond[i].prop->g;
    col[2]=obj->s_bond[i].prop->b;
    cgfxSphereVA(sr,(float *)obj->s_bond[i].atom->p,col,&obj->va,detail);
  }

  obj->va.max=obj->va.count;

  obj->va.p=Crecalloc(obj->va.p,obj->va.max,sizeof(cgfxVAField));

  return 0;
}

