#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com.h"
#include "dbm.h"
#include "mat.h"
#include "render.h"
#include "Cmalloc.h"
#include "surf_db.h"
#include "surf_obj.h"
#include "cl.h"

//static char surf_obj_return[256];

int surfObjCommand(struct DBM_SURF_NODE *node, surfObj *obj, int wc, char **wl)
{
  char message[256];
  char *empty_com[]={"get","center"};
  int i;
  float ot;

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }
  if(!strcmp(wl[0],"?") ||
     !strcmp(wl[0],"help")) {
  } else if(!strcmp(wl[0],"get")) {
    return surfObjComGet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"hide")) {
    obj->render.show=0;
    comHideObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"show")) {
    obj->render.show=1;
    comShowObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"render")) {
    if(wc<2) {
      sprintf(message,"\n%s: missing expression", obj->name);
      comMessage(message);
      return -1;
    }

    ot=obj->render.transparency;

    if(renderSet(&obj->render,wc-1,wl+1)!=0)
      return -1;

    if(obj->render.transparency!=ot) {
      for(i=0;i<obj->vertc;i++)
	obj->vert[i].c[3]=obj->render.transparency;
    }

    if(obj->render.mode!=RENDER_LINE && 
       obj->render.mode!=RENDER_POINT &&
       obj->render.mode!=RENDER_SURFACE){
      obj->render.mode=RENDER_SURFACE;
      comMessage("\ninvalid render mode");
      return -1;
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
    return surfObjComSet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"renew")) {
    return surfObjComRenew(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"reverse")) {
    if(wc>1) {
      comMessage("\nsuperfluous words ignored");
    }
    for(i=0;i<obj->vertc;i++) {
      obj->vert[i].n[0]=-obj->vert[i].n[0];
      obj->vert[i].n[1]=-obj->vert[i].n[1];
      obj->vert[i].n[2]=-obj->vert[i].n[2];
    }
  } else { 
    sprintf(message,"\n%s: unknow command: %s",obj->name, wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}

int surfObjComSet(surfObj *obj, int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  ret=surfObjSet(obj, &set,1);

  setDelete(&set);

  comRedraw();

  return ret;
}

int surfObjComGet(surfObj *obj, int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"\n%s: missing property", obj->name);
    comMessage(message);
    return -1;
  }
  for(i=0;i<wc;i++)
    if(surfObjGet(obj,wl[i])<0)
      return -1;

  return 0;
}

int surfObjComRenew(surfObj *obj, int wc, char **wl)
{
  int i;
  clOrg co;
  Set set;
  Select sel;
  int set_flag,sel_flag;
  char message[256];
  int ret,vflag;

  clNew(&co,wc,wl);
  set_flag=0;
  sel_flag=0;
  ret=0;
  vflag=0;
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
  
  if(!set_flag) {
    setNew(&set,0,NULL);
  }

  if(!sel_flag) {
    ret=surfObjRenew(obj, &set, &obj->select,vflag);
  } else {
    ret=surfObjRenew(obj, &set, &sel,vflag);
    selectDelete(&obj->select);
    memcpy(&obj->select,&sel,sizeof(Select));
  }

  setDelete(&set);
  clDelete(&co);
  comRedraw();
  return ret;
}


int surfObjSet(surfObj *obj, Set *s, int flag)
{
  int vc,f,pc,ret;
  struct POV_VALUE *val;
  float r,g,b,r2,g2,b2;
  float rval,p[3],frac1,frac2,rval1,rval2;
  float vmin,vmax;
  char message[256];

  if(flag==0)
    return 0;

  if(s->pov_count==0) {
    return 0;
  }

  if(s->range_flag) {
    // get min and max
    // if dataset is self
    if(s->range.src==NULL) {
      if(surfGetMinMax(obj->node, s->range.prop,&vmin,&vmax)<0) {
	if(obj->node->attach_flag) {
	  if(dbmGetMinMax(obj->node->last_attach,s->range.prop,&vmin,&vmax)<0) {
	    sprintf(message,"\nerror: unknown range property %s",s->range.prop);
	    comMessage(message);
	    return -1;
	  }
	} else {
	  sprintf(message,"\nerror: unknown range property %s",s->range.prop);
	  comMessage(message);
	  return -1;
	}
      }
    } else {
      if(dbmGetMinMax(s->range.src,s->range.prop,&vmin,&vmax)<0) {
	sprintf(message,"\nerror: unknown range property %s",s->range.prop);
	comMessage(message);
	return -1;
      }
    }
    if(clStrcmp(s->range.val1,"min"))
      rval1=vmin;
    else if(clStrcmp(s->range.val1,"max"))
      rval1=vmax;
    else
      rval1=atof(s->range.val1);

    if(clStrcmp(s->range.val2,"min"))
      rval2=vmin;
    else if(clStrcmp(s->range.val2,"max"))
      rval2=vmax;
    else
      rval2=atof(s->range.val2);
    
    sprintf(message,"\nusing range of property %s from %g to %g",
	    s->range.prop,rval1,rval2);
    comMessage(message);
  }
  
  for(pc=0;pc<s->pov_count;pc++) {
    if(clStrcmp(s->pov[pc].prop,"color") ||
       clStrcmp(s->pov[pc].prop,"colour") ||
       clStrcmp(s->pov[pc].prop,"col")) {
      s->pov[pc].id=SURF_PROP_COLOR;
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

  for(vc=0;vc<obj->vertc;vc++) {
    f=0;
    if(s->select_flag) {
      ret=surfIsSelected(obj->node, obj->vert[vc].vp, &s->select);
      if(ret<0) {
	return -1;
      } else if(ret==1) {
	f=1;
      }
    } else {
      f=1;
    }
    if(f) {
      for(pc=0;pc<s->pov_count;pc++) {
	val=povGetVal(&s->pov[pc],0);
	switch(s->pov[pc].id) {
	case SURF_PROP_COLOR:
	  if(s->range_flag) {
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
	    if(s->range.src==NULL) {
	      // this dataset
	      if(!obj->node->attach_flag) {
		if(surfGetRangeVal(obj->node,obj->vert[vc].vp, s->range.prop,&rval)<0)
		  return -1;
	      } else if(obj->vert[vc].vp->attach_node!=NULL) {
		if(structGetRangeVal(&obj->vert[vc].vp->attach_node->structNode,
				     &obj->vert[vc].vp->attach_node->structNode.atom[obj->vert[vc].vp->attach_element],
				     s->range.prop,&rval)<0)
		  return -1;
	      } else {
		continue;
	      }
	    } else {
	      if(dbmGetRangeVal(&s->range,(float*)obj->vert[vc].p,&rval)<0)
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
	      if(s->blend) {
		obj->vert[vc].c[0]+=(r2-r)*frac2+r;
		obj->vert[vc].c[1]+=(g2-g)*frac2+g;
		obj->vert[vc].c[2]+=(b2-b)*frac2+b;	
		if(obj->vert[vc].c[0]>1.0)
		   obj->vert[vc].c[0]=1.0;
		if(obj->vert[vc].c[1]>1.0)
		   obj->vert[vc].c[1]=1.0;
		if(obj->vert[vc].c[2]>1.0)
		   obj->vert[vc].c[2]=1.0;
	      } else {
		obj->vert[vc].c[0]=(r2-r)*frac2+r;
		obj->vert[vc].c[1]=(g2-g)*frac2+g;
		obj->vert[vc].c[2]=(b2-b)*frac2+b;
	      }
	    }
	  } else {
	    if(comGetColor(val->val1,&r,&g,&b)<0) {
	      comMessage("\nerror: set: unknown color ");
	      comMessage(val->val1);
	      return -1;
	    }
	    if(s->blend) {
	      obj->vert[vc].c[0]+=r;
	      obj->vert[vc].c[1]+=g;
	      obj->vert[vc].c[2]+=b;
	      if(obj->vert[vc].c[0]>1.0)
		obj->vert[vc].c[0]=1.0;
	      if(obj->vert[vc].c[1]>1.0)
		obj->vert[vc].c[1]=1.0;
	      if(obj->vert[vc].c[2]>1.0)
		obj->vert[vc].c[2]=1.0;
	    } else {
	      obj->vert[vc].c[0]=r;
	      obj->vert[vc].c[1]=g;
	      obj->vert[vc].c[2]=b;
	    }
	  }
	  break;
	}
      }
    }

  }

  return 0;
}

int surfObjGet(surfObj *obj, char *prop)
{
  int i;
  char message[256];
  float x,y,z;

  if(!strcmp(prop,"center")) {
    x=0.0;
    y=0.0;
    z=0.0;
    for(i=0;i<obj->vertc;i++) {
      x+=obj->vert[i].p[0];
      y+=obj->vert[i].p[1];
      z+=obj->vert[i].p[2];
    }
    if(i>0) {
      x/=(double)i;
      y/=(double)i;
      z/=(double)i;
    }
    sprintf(message,"{%.5f,%.5f,%.5f}",x,y,z);
    comReturn(message);
  } else {
    sprintf(message,"\n%s: get: unknown property %s",obj->name, prop); 
    comMessage(message);
    return -1;
  }
  return 0;
}


int surfObjRenew(surfObj *obj, Set *set, Select *sel, int vflag)
{
  float area;
  int i;
  char message[256];

  // objSet 0

  if(surfGenerate(obj,sel)<0)
    return -1;

  // objSet 1

  if(vflag) {
    area=0.0;
    for(i=0;i<obj->facec;i++)
      area+=matCalcTriArea(obj->vert[obj->face[i*3+0]].p,
			   obj->vert[obj->face[i*3+1]].p,
			   obj->vert[obj->face[i*3+2]].p);
    sprintf(message,"\narea: %.4f A^2",area);
    comMessage(message);
  }
  

  comRedraw();

  return 0;
}


int surfGenerate(surfObj *obj, Select *sel)
{
  int i;
  int vi1,vi2,vi3;
  char message[256];
  struct DBM_SURF_NODE *node=obj->node;
  int flag;
  int ret;
  struct SURF_OBJ_V *vert;
  int vertc;
  int *face;
  int facec;

  int *vert_id;



  vert=Ccalloc(node->vc+16,sizeof(struct SURF_OBJ_V));
  vertc=0;
  face=Ccalloc(3*(node->fc+16),sizeof(int));
  facec=0;

  vert_id=Ccalloc(node->vc+16,sizeof(int));
  
  comMessage("\nGenerating ...");

  memset(obj->vert_flag,0,sizeof(unsigned char)*obj->node->vc);

  for(i=0;i<node->vc;i++) {
    ret=surfIsSelected(node,&node->v[i],sel);
    if(ret<0) {
      Cfree(vert);
      Cfree(face);
      return -1;
    }
    if(ret>0) {
      obj->vert_flag[i]=1;
      vert_id[i]=vertc;
      vert[vertc].p[0]=node->v[i].p[0];
      vert[vertc].p[1]=node->v[i].p[1];
      vert[vertc].p[2]=node->v[i].p[2];
      vert[vertc].n[0]=node->v[i].n[0];
      vert[vertc].n[1]=node->v[i].n[1];
      vert[vertc].n[2]=node->v[i].n[2];
      vert[vertc].c[0]=1.0;
      vert[vertc].c[1]=1.0;
      vert[vertc].c[2]=1.0;
      vert[vertc].c[3]=1.0;
      vert[vertc].vp=&node->v[i];
      vertc++;
    }
  }

  for(i=0;i<node->fc;i++) {
    vi1=node->f[i].v[0];
    vi2=node->f[i].v[1];
    vi3=node->f[i].v[2];

    flag=0;
    if(obj->node->smode==SURF_SMODE_ALL) {
      if(obj->vert_flag[vi1] && obj->vert_flag[vi2] && obj->vert_flag[vi3])
	flag=1;
    } else {
      if(obj->vert_flag[vi1] || obj->vert_flag[vi2] || obj->vert_flag[vi3])
	flag=1;
    }
    if(flag) {
      face[facec++]=vert_id[vi1];
      face[facec++]=vert_id[vi2];
      face[facec++]=vert_id[vi3];
    }
  }

  obj->vert=Ccalloc(vertc+16,sizeof(struct SURF_OBJ_V));
  memcpy(obj->vert,vert,sizeof(struct SURF_OBJ_V)*vertc);
  obj->vertc=vertc;
  Cfree(vert);

  obj->face=Ccalloc(facec+16,sizeof(int));
  memcpy(obj->face,face,sizeof(int)*facec);
  obj->facec=facec/3;
  Cfree(face);
  
  Cfree(vert_id);

//  surfPrepObj(obj);

  sprintf(message,"%d faces",facec/3);
  comMessage(message);
  
  return 0;
}

int surfObjIsWithin(surfObj *obj, float *p, float d2)
{
  int i;
  float dx,dy,dz;
  float *vp;

  for(i=0;i<obj->vertc;i++) {
    vp=obj->vert[i].p;
    dx=p[0]-vp[0];
    if(dx*dx<d2) {
      dy=p[1]-vp[1];
      if(dy*dy<d2) {
	dz=p[2]-vp[2];
	if(dz*dz<d2) {
	  if(dx*dx+dy*dy+dz*dz<d2)
	    return 1;
	}
      }
    }
  }
  return 0;
}
