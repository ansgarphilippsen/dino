#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#include "dbm.h"
#include "mat.h"
#include "shell.h"
#include "com.h"
#include "rex.h"
#include "render.h"
#include "Cmalloc.h"
#include "clist.h"
#include "scal_db.h"
#include "scal_obj.h"
#include "scal_write.h"
#include "scal_mc_new.h"
#include "cl.h"

//static char scal_obj_return[256];

extern struct GLOBAL_COM com;

int scalObjCommand(dbmScalNode *node,scalObj *obj,int wc,char **wl)
{
  char message[256];
  char *empty_com[]={"get","center"};

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }
  if(!strcmp(wl[0],"help") ||
     !strcmp(wl[0],"?")) {
  } else if(!strcmp(wl[0],"renew")) {
    return scalObjComRenew(obj, wc-1,wl+1);
  } else if(!strcmp(wl[0],"get")) {
    return scalObjComGet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"set")) {
    return scalObjComSet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"render")) {
    if(wc<2) {
      sprintf(message,"\n%s: missing expression", obj->name);
      comMessage(message);
      return -1;
    }


    if(renderSet(&obj->render,wc-1,wl+1)!=0) {
      sprintf(message,"\n%s: syntax error in render statement",obj->name);
      return -1;
    }


    if(obj->type==SCAL_CONTOUR) {
      if(obj->render.mode!=RENDER_LINE && 
	 obj->render.mode!=RENDER_POINT &&
	 obj->render.mode!=RENDER_SURFACE){
	obj->render.mode=RENDER_LINE;
	comMessage("\ninvalid render mode");
	return -1;
      }
    } else if(obj->type==SCAL_GRID) {
      if(obj->render.mode!=RENDER_ON && 
	 obj->render.mode!=RENDER_OFF) {
	obj->render.mode=RENDER_OFF;
	comMessage("\ninvalid render mode");
	return -1;
      }

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
  } else if(!strcmp(wl[0],"show")) {
    obj->render.show=1;
    comShowObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"hide")) {
    obj->render.show=0;
    comHideObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"reset")) {
  } else if(!strcmp(wl[0],"grab")) {
  } else if(!strcmp(wl[0],"ungrab")) {
  } else if(!strcmp(wl[0],"hook")) {
  } else if(!strcmp(wl[0],"unhook")) {
  } else if(!strcmp(wl[0],"write")) {
    if(wc<2) {
      sprintf(message,"\n%s: expected filename for write command",obj->name);
      comMessage(message);
      return -1;
    }
    return scalObjWrite(obj, wc-1, wl+1);
  } else {
    sprintf(message,"\n%s: unknown command %s",obj->name,wl[0]);
    comMessage(message);
    return -1;    
  }

  return 0;
}

int scalObjComRenew(scalObj *obj, int wc, char **wl)
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
    ret=scalObjRenew(obj, &set, &obj->select);
  } else {
    ret=scalObjRenew(obj, &set, &sel);
    selectDelete(&obj->select);
    memcpy(&obj->select, &sel, sizeof(Select));
  }
  
  setDelete(&set);
  clDelete(&co);
  comRedraw();
  return ret;
}

int scalObjComSet(scalObj *obj, int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  ret=scalObjSet(obj, &set,1);

  setDelete(&set);

  comRedraw();

  return ret;
}


int scalObjComGet(scalObj *obj, int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"\n%s: missing property", obj->name);
    comMessage(message);
    return -1;
  }
  for(i=0;i<wc;i++)
    if(scalObjGet(obj,wl[i])<0)
      return -1;

  return 0;
}




int scalObjRenew(scalObj *obj, Set *set, Select *sel)
{
  char message[256];
  int ret=0;

  // set object properties before renewing
  if(scalObjSet(obj,set,0)<0)
    return -1;

  if(obj->type==SCAL_CONTOUR) {
    sprintf(message,"\nContouring at %f ...",obj->level);
    comMessage(message);

    ret=scalMCN(obj,sel);
  } else if(obj->type==SCAL_GRID) {
    sprintf(message,"\nGenerating grid ...");
    comMessage(message);
    ret=scalGrid(obj,sel);
#ifdef VR
  } else if(obj->type==SCAL_VR) {
    ret=scalVR(obj,sel);
#endif
  } else if(obj->type==SCAL_SLAB) {
    ret=scalSlab(obj,sel);
  }

  obj->ou_size=obj->u_size;
  obj->ov_size=obj->v_size;
  obj->ow_size=obj->w_size;

  if(scalObjSet(obj,set,1)<0)
    return -1;

  sprintf(message,".%s.%s",obj->node->name,obj->name);
  comReturn(message);
  return 0;
}

int scalObjSet(scalObj *obj, Set *s, int flag)
{
  int pc,op,os,vi[3],f,res,ec;
  struct POV_VALUE *val;
  float r,g,b,r2,g2,b2,v1[4],rad1,rad2;
  float rval,p[3],frac1,frac2,rval1,rval2;
  double vd1[3],vd2[3];
  float fact;
  char message[256],nv[16];


  if(s->pov_count==0) {
    return 0;
  }

  if(s->range_flag) {
    rval1=atof(s->range.val1);
    rval2=atof(s->range.val2);
  }

  for(pc=0;pc<s->pov_count;pc++) {
    if(clStrcmp(s->pov[pc].prop,"color") ||
       clStrcmp(s->pov[pc].prop,"colour") ||
       clStrcmp(s->pov[pc].prop,"col")) {
      s->pov[pc].id=SCAL_PROP_COLOR;
    } else if(clStrcmp(s->pov[pc].prop,"center")) {
      if(s->pov[pc].op!=POV_OP_EQ) {
	comMessage("\nerror: set: expected operator = for property ");
	comMessage(s->pov[pc].prop);
	return -1;
      }
      s->pov[pc].id=SCAL_PROP_CENTER;
    } else if(clStrcmp(s->pov[pc].prop,"size")) {
      s->pov[pc].id=SCAL_PROP_SIZE;
    } else if(clStrcmp(s->pov[pc].prop,"start")) {
      s->pov[pc].id=SCAL_PROP_START;
    } else if(clStrcmp(s->pov[pc].prop,"end")) {
      s->pov[pc].id=SCAL_PROP_END;
    } else if(clStrcmp(s->pov[pc].prop,"level")) {
      if(obj->type==SCAL_CONTOUR) {
	s->pov[pc].id=SCAL_PROP_LEVEL;
      } else {
	comMessage("\nerror: property level only valid for object type contour");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"dir")) {
      if(obj->type==SCAL_SLAB) {
	s->pov[pc].id=SCAL_PROP_DIR;
      } else {
	comMessage("\nerror: property dir only valid for object type slab");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"rad")) {
      if(obj->type==SCAL_GRID) {
	s->pov[pc].id=SCAL_PROP_RAD;
      } else {
	comMessage("\nerror: property level only valid for object type grid");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"step")) {
      s->pov[pc].id=SCAL_PROP_STEP;
    } else if(clStrcmp(s->pov[pc].prop,"method")) {
      if(s->pov[pc].op!=POV_OP_EQ) {
	comMessage("\nerror: set: expected operator = for property ");
	comMessage(s->pov[pc].prop);
	return -1;
      }
      comMessage("\nwarning: set: obsolete property method ignored");
      s->pov[pc].id=SCAL_PROP_METHOD;
    } else {
      comMessage("\nerror: set: unknown property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }

    op=s->pov[pc].op;
    if(!(op==POV_OP_EQ || op==POV_OP_PE || op==POV_OP_ME ||
	 op==POV_OP_SE || op==POV_OP_DE)) {
      comMessage("\nerror: set: expected operator '=' '+=' '-=' '*=' or '/='");
      // /=
      return -1;
    }
    
    if(s->pov[pc].val_count>1) {
      comMessage("\nerror: set: expected only one value for property ");
      comMessage(s->pov[pc].prop);
      return -1;
    }
  }


  for(pc=0;pc<s->pov_count;pc++) {
    op=s->pov[pc].op;
    val=povGetVal(&s->pov[pc],0);

    if(obj->type==SCAL_CONTOUR) {
      if(val->range_flag) {
	comMessage("\nerror: set: range not supported for contour object");
	return -1;
      }
    }
    switch(s->pov[pc].id) {
    case SCAL_PROP_COLOR:
      if(flag==1) {
	if(obj->type==SCAL_CONTOUR) {
	  if(s->range_flag) {
	    comMessage("\nerror: range not supported for contour object");
	    return -1;
	  }
	  if(comGetColor(val->val1,&r,&g,&b)<0) {
	    comMessage("\nerror: set: unknown color ");
	    comMessage(val->val1);
	    return -1;
	  }
	  obj->r=r;
	  obj->g=g;
	  obj->b=b;
	} else if(obj->type==SCAL_GRID) {
	  if(comGetColor(val->val1,&r,&g,&b)<0) {
	    comMessage("\nerror: set: unknown color ");
	    comMessage(val->val1);
	    return -1;
	  }
	  if(s->range_flag) {
	    if(comGetColor(val->val2,&r2,&g2,&b2)<0) {
	      comMessage("\nerror: set: unknown color ");
	      comMessage(val->val2);
	      return -1;
	    }
	  }
	  for(ec=0;ec<obj->point_count;ec++) {
	    f=0;
	    if(s->select_flag) {
	      res=scalIsSelected(obj->node,
				 obj->point[ec].uvw[0],
				 obj->point[ec].uvw[1],
				 obj->point[ec].uvw[2],
				 &s->select);
	      if(res==-1)
		return -1;
	      else
		f=res;
	    } else {
	      f=1;
	    }
	    if(f) {
	      if(s->range_flag) {
		if(s->range.src==NULL) {
		  // this dataset
		  if(scalGetRangeXYZVal(obj->node,
					s->range.prop,
					obj->point[ec].v,
					&rval)<0)
		    return -1;
		} else {
		  if(dbmGetRangeVal(&s->range,obj->point[ec].v,&rval)<0)
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
		if(frac2>=0.0 && frac2<=1.0) {
		  obj->point[ec].c[0]=(r2-r)*frac2+r;
		  obj->point[ec].c[1]=(g2-g)*frac2+g;
		  obj->point[ec].c[2]=(b2-b)*frac2+b;
		}
	      } else {
		obj->point[ec].c[0]=r;
		obj->point[ec].c[1]=g;
		obj->point[ec].c[2]=b;
	      }
	    }
	  }
	} else if (obj->type==SCAL_SLAB) {
	  /*
	    go through texture data and
	    assign colors accordingly
	  */
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
	  } else {
	    if(comGetColor(val->val1,&r,&g,&b)<0) {
	      comMessage("\nerror: set: unknown color ");
	      comMessage(val->val1);
	      return -1;
	    }
	  }
	  for(ec=0;ec<obj->slab.size;ec++) {
	    if(s->range_flag) {
	      rval=obj->slab.data[ec];
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
		obj->slab.tex[ec*4+0]=(unsigned char)(255.0*((r2-r)*frac2+r));
		obj->slab.tex[ec*4+1]=(unsigned char)(255.0*((g2-g)*frac2+g));
		obj->slab.tex[ec*4+2]=(unsigned char)(255.0*((b2-b)*frac2+b));
	      }
	    } else {
	      obj->slab.tex[ec*4+0]=(unsigned char)(r*255.0);
	      obj->slab.tex[ec*4+1]=(unsigned char)(g*255.0);
	      obj->slab.tex[ec*4+2]=(unsigned char)(b*255.0);
	    }
	  }
	  glBindTexture(GL_TEXTURE_2D,obj->slab.texname);
	  glTexImage2D(GL_TEXTURE_2D,0,
		       GL_RGBA,
		       obj->slab.usize,obj->slab.vsize,0,
		       GL_RGBA,GL_UNSIGNED_BYTE,
		       obj->slab.tex);
	}
      }
      break;
    case SCAL_PROP_RAD:
      for(ec=0;ec<obj->point_count;ec++) {
	f=0;
	if(s->select_flag) {
	  res=scalIsSelected(obj->node,
			     obj->point[ec].uvw[0],
			     obj->point[ec].uvw[1],
			     obj->point[ec].uvw[2],
			     &s->select);
	  if(res==-1)
	    return -1;
	  else
	    f=res;
	} else {
	  f=1;
	}
	if(f) {
	  if(s->range_flag) {
	    rad1=atof(val->val1);
	    rad2=atof(val->val2);

	    if(s->range.src==NULL) {
	      // this dataset
	      if(scalGetRangeXYZVal(obj->node,
				    s->range.prop,
				    obj->point[ec].v,
				    &rval)<0)
		return -1;
	    } else {
	      if(dbmGetRangeVal(&s->range,obj->point[ec].v,&rval)<0)
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
	    if(frac2>=0.0 && frac2<=1.0) {
	      obj->point[ec].rad=(rad2-rad1)*frac2+rad1;
	    }
	  } else {
	    rad1=atof(val->val1);
	    obj->point[ec].rad=rad1;
	  }
	}
      }
      break;
    case SCAL_PROP_CENTER:
      if(matExtract1D(val->val1,3,vd1)!=0) {
	comMessage("\nerror: set: syntax error in vector ");
	comMessage(val->val1);
	return -1;
      }
      if(obj->type==SCAL_SLAB) {
	if(op==POV_OP_EQ) {
	  obj->slab.center[0]=vd1[0];
	  obj->slab.center[1]=vd1[1];
	  obj->slab.center[2]=vd1[2];
	} else {
	  comMessage("\nerror: set: expected operator = for property center");
	  return -1;
	}
	scalSlabIntersect(obj);
      } else {
	scalXYZtoUVW(obj->field,vd1,vd2);
	if(op==POV_OP_EQ) {
	  obj->u_center=(int)vd2[0];
	  obj->v_center=(int)vd2[1];
	  obj->w_center=(int)vd2[2];
	} else {
	  comMessage("\nerror: set: expected operator = for property center");
	  return -1;
	}
	obj->u_start=obj->u_center-obj->u_size/2;
	obj->v_start=obj->v_center-obj->v_size/2;
	obj->w_start=obj->w_center-obj->w_size/2;
	obj->u_end=obj->u_center+obj->u_size/2;
	obj->v_end=obj->v_center+obj->v_size/2;
	obj->w_end=obj->w_center+obj->w_size/2;
      }
      break;
    case SCAL_PROP_SIZE:
      if(obj->type==SCAL_SLAB) {
	os=atoi(val->val1);
	if(os!=1 && os!=2 && os!=4 && os!=8 && os!=16 &&
	   os!=32 && os!=64 && os!=128 && os!=256 && os!=512) {
	  comMessage("\ninvalid size ignored");
	} else {
	  obj->slab.usize=os;
	  obj->slab.vsize=os;
	  scalSlab(obj,NULL);
	}

      } else {
	if(val->val1[0]=='{') {
	  if(matExtract1Df(val->val1,3,v1)!=0) {
	    comMessage("\nerror: set: expected {u,v,w} for size");
	    return -1;
	  }
	} else {
	  v1[0]=atof(val->val1);
	  v1[1]=atof(val->val1);
	  v1[2]=atof(val->val1);
	}
	
	switch(op) {
	case POV_OP_EQ:
	  obj->u_size=(int)v1[0];
	  obj->v_size=(int)v1[1];
	  obj->w_size=(int)v1[2]; 
	  break;
	case POV_OP_PE:
	  obj->u_size+=(int)v1[0];
	  obj->v_size+=(int)v1[1];
	  obj->w_size+=(int)v1[2];
	  break;
	case POV_OP_ME:
	  obj->u_size-=(int)v1[0];
	  obj->v_size-=(int)v1[1];
	  obj->w_size-=(int)v1[2]; 
	  break;
	case POV_OP_SE:
	  obj->u_size=(int)((float)obj->u_size*v1[0]);
	  obj->v_size=(int)((float)obj->v_size*v1[1]);
	  obj->w_size=(int)((float)obj->w_size*v1[2]);
	  break;
	case POV_OP_DE:
	  obj->u_size=(int)((float)obj->u_size/v1[0]);
	  obj->v_size=(int)((float)obj->v_size/v1[1]);
	  obj->w_size=(int)((float)obj->w_size/v1[2]);
	  break;
	}
	if(obj->u_size<2) {
	  comMessage("\nwarning: set: u size smaller than 2, reset to 2");
	  obj->u_size=2.0;
	}
	if(obj->v_size<2) {
	  comMessage("\nwarning: set: v size smaller than 2, reset to 2");
	  obj->v_size=2.0;
	}
	if(obj->w_size<2) {
	  comMessage("\nwarning: set: w size smaller than 2, reset to 2");
	  obj->w_size=2.0;
	}
	obj->u_start=obj->u_center-obj->u_size/2;
	obj->v_start=obj->v_center-obj->v_size/2;
	obj->w_start=obj->w_center-obj->w_size/2;
	obj->u_end=obj->u_center+obj->u_size/2;
	obj->v_end=obj->v_center+obj->v_size/2;
	obj->w_end=obj->w_center+obj->w_size/2;
      }
      break;
    case SCAL_PROP_DIR:
      if(val->val1[0]=='{') {
	if(matExtract1Df(val->val1,3,v1)!=0) {
	  comMessage("\nerror: set: expected {x,y,z} for dir");
	  return -1;
	}
      } else {
	v1[0]=atof(val->val1);
	v1[1]=atof(val->val1);
	v1[2]=atof(val->val1);
      }
      if(op==POV_OP_EQ) {
	obj->slab.dir[0]=v1[0];
	obj->slab.dir[1]=v1[1];
	obj->slab.dir[2]=v1[2];
      } else {
	comMessage("\nerror: expected operator '='");
	return -1;
      }
      scalSlabIntersect(obj);
      break;
    case SCAL_PROP_LEVEL:
      strncpy(nv,val->val1,15);
      if(nv[clStrlen(nv)-1]=='s') {
	nv[clStrlen(nv)-1]='\0';
	if(obj->field->sigma==0.0)
	  fact=1.0;
	else
	  fact=obj->field->sigma;
      } else {
	fact=1.0;
      }
      switch(op) {
      case POV_OP_EQ: obj->level=atof(nv)*fact; break;
      case POV_OP_PE: obj->level+=atof(nv)*fact; break;
      case POV_OP_ME: obj->level-=atof(nv)*fact; break;
      case POV_OP_SE: obj->level*=atof(nv)*fact; break;
      case POV_OP_DE: obj->level/=atof(nv)*fact; break;
      }
      break;
#ifdef VR
    case SCAL_PROP_START: 
      if(op!= POV_OP_EQ && op!=POV_OP_PE && op!=POV_OP_ME) {
	comMessage("\nerror: set: expected operators '=', '+=' or '-=' for property start");
	return -1;
      }
      switch(op) {
      case POV_OP_EQ: obj->vr.start=atof(val->val1); break;
      case POV_OP_PE: obj->vr.start+=atof(val->val1); break;
      case POV_OP_ME: obj->vr.start-=atof(val->val1); break;
      }
      break;
    case SCAL_PROP_END: 
      if(op!= POV_OP_EQ && op!=POV_OP_PE && op!=POV_OP_ME) {
	comMessage("\nerror: set: expected operators '=', '+=' or '-=' for property end");
	return -1;
      }
      switch(op) {
      case POV_OP_EQ: obj->vr.end=atof(val->val1); break;
      case POV_OP_PE: obj->vr.end+=atof(val->val1); break;
      case POV_OP_ME: obj->vr.end-=atof(val->val1); break;
      }
      break;
#endif
    case SCAL_PROP_STEP: 
      if(op!= POV_OP_EQ && op!=POV_OP_PE && op!=POV_OP_ME) {
	comMessage("\nerror: set: expected operators '=', '+=' or '-=' for property step");
	return -1;
      }
      os=obj->step;
      switch(op) {
      case POV_OP_EQ: obj->step=atof(val->val1); break;
      case POV_OP_PE: obj->step+=atof(val->val1); break;
      case POV_OP_ME: obj->step-=atof(val->val1); break;
      }
      if(obj->step<1 || obj->step>8) {
	sprintf(message,
		"\nwarning: set: stepsize out of bounds (1-8), reset to %d",
		os);
	obj->step=os;
      }
      break;
    case SCAL_PROP_METHOD: break; // ignore
    }
  }

/***
  fprintf(stderr,"\n%d %d %d\n%d %d %d   %d %d %d",
	  obj->u_center,obj->v_center, obj->w_center,
	  obj->u_start, obj->v_start, obj->w_start,
	  obj->u_end, obj->v_end, obj->w_end);
***/ 
  return 0;
}


int scalObjGet(scalObj *obj, char *prop)
{
  char message[256];
  double v1[4],v2[4];
  int ret=0;

  if(clStrcmp(prop,"center")) {
    v1[0]=(double)obj->u_center;
    v1[1]=(double)obj->v_center;
    v1[2]=(double)obj->w_center;
    scalUVWtoXYZ(obj->field,v1,v2);
    sprintf(message,"{%.3f,%.3f,%.3f}",v2[0],v2[1],v2[2]);
    comReturn(message);
  } else if(clStrcmp(prop,"level")) {
    sprintf(message,"%5.3f",obj->level);
    comReturn(message);
  } else if(clStrcmp(prop,"size")) {
    sprintf(message,"{%5d,%5d,%5d}",obj->u_size,obj->v_size,obj->w_size);
    comReturn(message);
  } else if(clStrcmp(prop,"color") ||
	    clStrcmp(prop,"colour")) {
    sprintf(message,"{%.3f,%.3f,%.3f}",obj->r, obj->g, obj->b);
    comReturn(message);
  } else if(clStrcmp(prop,"step")) {
    sprintf(message,"%2d",obj->step);
    comReturn(message);
  } else {
    sprintf(message,"\n%s: unknown property %s",obj->name, prop);
    comMessage(message);
    ret=-1;
  }
  
  return ret;
}

int scalObjCp(scalObj *o1, scalObj *o2)
{
  // TODO later
  comMessage("\ncopy not implemented");
  return -1;
}


int scalGrid(scalObj *obj, Select *sel)
{
  int u,v,w,ustart,uend,vstart,vend,wstart,wend,res;
  int step,flag;
  struct SCAL_POINT *spoint=NULL;
  struct SCAL_POINT *spoint_old=NULL;
  int spoint_count, spoint_max, mem_add;
  double v1[3],v2[3];
  double val;
  char message[1024];

  ustart=obj->u_start; uend=obj->u_end;
  vstart=obj->v_start; vend=obj->v_end;
  wstart=obj->w_start; wend=obj->w_end;

  mem_add=100000 ;
  spoint_count=0;
  spoint_max=mem_add;
  spoint=Ccalloc(spoint_max,sizeof(struct SCAL_POINT));

  step=obj->step;

  for(u=ustart;u<uend-1;u+=step)
    for(v=vstart;v<vend-1;v+=step)
      for(w=wstart;w<wend-1;w+=step) {
        flag=0;
	
	/* if step==1 check min/max */
	/* else */
	res=scalIsSelected(obj->node,u,v,w,sel);
	if(res==-1)
	  return -1;
	else
	  flag=res;
	
	if(flag==1) {
	  v1[0]=(double)u;
	  v1[1]=(double)v;
	  v1[2]=(double)w;
	  scalUVWtoXYZ(obj->field,v1,v2);
	  val=scalReadField(obj->field,u,v,w);

	  spoint[spoint_count].rad=0.1;
	  spoint[spoint_count].v[0]=v2[0];
	  spoint[spoint_count].v[1]=v2[1];
	  spoint[spoint_count].v[2]=v2[2];
	  spoint[spoint_count].c[0]=1.0;
	  spoint[spoint_count].c[1]=1.0;
	  spoint[spoint_count].c[2]=1.0;
	  spoint[spoint_count].c[3]=1.0;
	  spoint[spoint_count].val=val;
	  spoint[spoint_count].uvw[0]=u;
	  spoint[spoint_count].uvw[1]=v;
	  spoint[spoint_count].uvw[2]=w;
	  spoint_count++;


	  if(spoint_count>=spoint_max) {
	    spoint_old=spoint;
	    spoint=Ccalloc(spoint_max+mem_add,sizeof(struct SCAL_POINT));
	    memcpy(spoint,spoint_old,
		   spoint_count*sizeof(struct SCAL_POINT));
	    Cfree(spoint_old);
	    spoint_max+=mem_add;
	  }
	}
      }

  obj->point=spoint;
  obj->point_count=spoint_count;
  obj->line=NULL;
  obj->line_count=0;
  obj->face=NULL;
  obj->face_count=0;

  sprintf(message," %d points",
	  obj->point_count);
  comMessage(message);


  return 0;
}

int scalObjIsWithin(scalObj *obj, float *p, float d2)
{
  int i;
  float dx,dy,dz,dxx,dyy;

  for(i=0;i<obj->point_count;i++) {
    dx=obj->point[i].v[0]-p[0];
    if((dxx=dx*dx)<=d2) {
      dy=obj->point[i].v[1]-p[1];
      if((dyy=dxx+dy*dy)<=d2) {
	dz=obj->point[i].v[2]-p[2];
	if(dz*dz+dyy<=d2) {
	  return 1;
	}
      }
    }
  }
  return 0;
}

#ifdef VR
int scalVR(scalObj *obj, Select *sel)
{
  float uvw[3];
  int u,v,w,usize,vsize,wsize,p;
  float fval,fval2;
  char message[256];

  /*
  fprintf(stderr,"\n%d %d %d  %d %d %d",
	  obj->u_start, obj->v_start, obj->w_start,
	  obj->u_end, obj->v_end, obj->w_end);
  */

  uvw[0]=(float)obj->u_start;
  uvw[1]=(float)obj->v_start;
  uvw[2]=(float)obj->w_start;
  scalUVWtoXYZf(obj->node->field,uvw,obj->vr.xyz1);

  uvw[0]=(float)obj->u_end;
  uvw[1]=(float)obj->v_end;
  uvw[2]=(float)obj->w_end;
  scalUVWtoXYZf(obj->node->field,uvw,obj->vr.xyz2);

  usize=obj->u_end-obj->u_start;
  vsize=obj->v_end-obj->v_start;
  wsize=obj->w_end-obj->w_start;

  if(!comTestTex3D(usize,vsize,wsize)) {
    sprintf(message,"\nscalVR: could not allocate %dx%dx%d 3D texture",
	    usize,vsize,wsize);
    comMessage(message);
    return -1;
  }

  if((obj->vr.data=Ccalloc(usize*vsize*wsize,sizeof(float)))==NULL) {
    comMessage("\nscalVR: memory allocation error");
    return -1;
  }

  obj->vr.usize=usize;
  obj->vr.vsize=vsize;
  obj->vr.wsize=wsize;

  for(u=0;u<usize;u++) {
    for(v=0;v<usize;v++) {
      for(w=0;w<usize;w++) {
	p=w*usize*vsize+v*usize+u;
	fval=scalReadField(obj->field,
			   u+obj->u_start,v+obj->v_start,w+obj->w_start);
	fval2=(fval-obj->vr.start)/(obj->vr.end-obj->vr.start);
	if(fval2<0.0) fval2=0.0;
	if(fval2>1.0) fval2=1.0;
	obj->vr.data[p]=fval2;
      }
    }
  }

  obj->vr.tex1[0]=(float)(obj->u_start-obj->field->u1)/(float)(obj->field->u_size);
  obj->vr.tex1[1]=(float)(obj->v_start-obj->field->v1)/(float)(obj->field->v_size);
  obj->vr.tex1[2]=(float)(obj->w_start-obj->field->w1)/(float)(obj->field->w_size);

  obj->vr.tex2[0]=(float)(obj->u_end-obj->field->u1)/(float)(obj->field->u_size);
  obj->vr.tex2[1]=(float)(obj->v_end-obj->field->v1)/(float)(obj->field->v_size);
  obj->vr.tex2[2]=(float)(obj->w_end-obj->field->w1)/(float)(obj->field->w_size);

  if(obj->vr.tex==0) {
    glGenTextures(1,&obj->vr.tex);
    glBindTexture(GL_TEXTURE_3D_EXT,obj->vr.tex);
  }
 
  return 0;
}
#endif

int scalSlab(scalObj *obj, Select *sel)
{
  int u,v;
  int p,q,r;
  double uvw[3];

  /*
    create space for the data and texture
  */

  obj->slab.size=obj->slab.usize*obj->slab.vsize;

  if(obj->slab.data!=NULL) {
    Cfree(obj->slab.data);
    Cfree(obj->slab.tex);
    glDeleteTextures(1,&obj->slab.texname);
  }  

  obj->slab.data=Ccalloc(obj->slab.size,sizeof(float));
  obj->slab.tex=Ccalloc(obj->slab.size,4*sizeof(unsigned char));

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glGenTextures(1,&obj->slab.texname);


  // minimal and maximal corners of cube

  uvw[0]=(float)obj->field->u1;
  uvw[1]=(float)obj->field->v1;
  uvw[2]=(float)obj->field->w1;
  scalUVWtoXYZ(obj->field,uvw,obj->slab.corner[0]);
  uvw[0]=(float)obj->field->u2;
  uvw[1]=(float)obj->field->v2;
  uvw[2]=(float)obj->field->w2;
  scalUVWtoXYZ(obj->field,uvw,obj->slab.corner[1]);

  // calculate intersect

  scalSlabIntersect(obj);

  return 0;
}

int scalSlabIntersect(scalObj *obj)
{

  int corner[][3]={
    {0,0,0},{1,0,0},{0,1,0},{1,1,0},
    {0,0,1},{1,0,1},{0,1,1},{1,1,1}
  };
  int edge[][2]={
    {0,1},{0,2},{2,3},{1,3},
    {0,4},{2,6},{3,7},{1,5},
    {4,5},{4,6},{6,7},{5,7}
  };
  int i,j,pc,flag[12],indx[2];
  double r[12],point[12][3];
  double a,b,c,d,x0,y0,z0,x1,y1,z1,x2,y2,z2,q1,q2;
  double dir[3],dist,axis1[3],axis2[3],mindist[2],maxdist[2],diff[3];
  int u,v;
  double fu,fv,len1,len2,xyz[3],uvw[3];

  matNormalize(obj->slab.dir,dir);

  a=obj->slab.dir[0];
  b=obj->slab.dir[1];
  c=obj->slab.dir[2];
  
  if(a==0.0 && b==0.0 && c==0.0) {
    obj->slab.linec=0;
    return -1;
  }

  d=-(a*obj->slab.center[0]+
      b*obj->slab.center[1]+
      c*obj->slab.center[2]);

  pc=0;

  for(i=0;i<12;i++) {
    x0=obj->slab.corner[corner[edge[i][0]][0]][0];
    y0=obj->slab.corner[corner[edge[i][0]][1]][1];
    z0=obj->slab.corner[corner[edge[i][0]][2]][2];
    x1=obj->slab.corner[corner[edge[i][1]][0]][0];
    y1=obj->slab.corner[corner[edge[i][1]][1]][1];
    z1=obj->slab.corner[corner[edge[i][1]][2]][2];
    x2=x1-x0;
    y2=y1-y0;
    z2=z1-z0;

    q2=a*x2+b*y2+c*z2;
    if(q2==0.0) {
      r[i]=-1.0;
      point[i][0]=0.0;
      point[i][1]=0.0;
      point[i][2]=0.0;
      flag[i]=0;
    } else {
      q1=-(d+a*x0+b*y0+c*z0);
      r[i]=q1/q2;
      if(r[i]>=0.0 && r[i]<1.0) {
	flag[i]=1;
	point[i][0]=x0+r[i]*x2;
	point[i][1]=y0+r[i]*y2;
	point[i][2]=z0+r[i]*z2;
      } else {
	flag[i]=0;
      }
    }
  }

  obj->slab.linec=0;
  for(i=0;i<12;i++) {
    obj->slab.point[i][0]=point[i][0];
    obj->slab.point[i][1]=point[i][1];
    obj->slab.point[i][2]=point[i][2];
    for(j=i+1;j<12;j++) {
      if(flag[i] && flag[j]) {
	if(com.cube_lookup[i*12+j]) {
	  obj->slab.line[obj->slab.linec][0]=i;
	  obj->slab.line[obj->slab.linec][1]=j;
	  obj->slab.linec++;
	}	
      }
    }
  }

  /*
    algorithm for minimal square including all points
    a) find axis1 (longest line from above)
    b) find axis2 (crossproduct of axis1 with plane normal)
    c) find minimal and maximal extend of both axis from center
   */

  maxdist[0]=0.0;
  for(i=0;i<obj->slab.linec;i++) {
    diff[0]=point[obj->slab.line[i][0]][0]-point[obj->slab.line[i][1]][0];
    diff[1]=point[obj->slab.line[i][0]][1]-point[obj->slab.line[i][1]][1];
    diff[2]=point[obj->slab.line[i][0]][2]-point[obj->slab.line[i][1]][2];
    dist=diff[0]*diff[0]+diff[1]*diff[1]+diff[2]*diff[2];
    if(dist>maxdist[0]) {
      maxdist[0]=dist;
      indx[0]=obj->slab.line[i][0];
      indx[1]=obj->slab.line[i][1];
    }
  }


  axis1[0]=point[indx[0]][0]-point[indx[1]][0];
  axis1[1]=point[indx[0]][1]-point[indx[1]][1];
  axis1[2]=point[indx[0]][2]-point[indx[1]][2];

  matCalcCross(axis1,dir,axis2);

  maxdist[0]=maxdist[1]=0.0;
  mindist[0]=mindist[1]=0.0;

  for(i=0;i<12;i++)
    if(flag[i]) {
      dist=matCalcDistancePointToLine(obj->slab.center,axis1,point[i]);
      if(dist<mindist[0])
	mindist[0]=dist;
      if(dist>maxdist[0])
	maxdist[0]=dist;
      dist=matCalcDistancePointToLine(obj->slab.center,axis2,point[i]);
      if(dist<mindist[1])
	mindist[1]=dist;
      if(dist>maxdist[1])
	maxdist[1]=dist;
    }

  fprintf(stderr,"\n%f %f   %f %f",
	  mindist[0],maxdist[0],mindist[1],maxdist[1]);

  len1=maxdist[1]*2;
  len2=maxdist[0]*2;
  
  matNormalize(axis1,axis1);
  matNormalize(axis2,axis2);

  // use limits to create boundary rectangle

  for(i=0;i<3;i++) {
    obj->slab.bound[0][i]=obj->slab.center[i]-
      maxdist[0]*axis2[i]-maxdist[1]*axis1[i];
    obj->slab.bound[1][i]=obj->slab.center[i]-
      maxdist[0]*axis2[i]+maxdist[1]*axis1[i];
    obj->slab.bound[2][i]=obj->slab.center[i]+
      maxdist[0]*axis2[i]+maxdist[1]*axis1[i];
    obj->slab.bound[3][i]=obj->slab.center[i]+
      maxdist[0]*axis2[i]-maxdist[1]*axis1[i];
  }

  // fill in values from the scalar field dataset
  for(u=0;u<obj->slab.usize;u++) {
    fu=(double)u/(double)(obj->slab.usize-1);
    for(v=0;v<obj->slab.vsize;v++) {
      fv=(double)v/(double)(obj->slab.vsize-1);
      xyz[0]=obj->slab.bound[0][0]+axis1[0]*fu*len1+axis2[0]*fv*len2;
      xyz[1]=obj->slab.bound[0][1]+axis1[1]*fu*len1+axis2[1]*fv*len2;
      xyz[2]=obj->slab.bound[0][2]+axis1[2]*fu*len1+axis2[2]*fv*len2;
      scalXYZtoUVW(obj->field,xyz,uvw);
      obj->slab.data[v*obj->slab.usize+u]=scalReadField(obj->field,
							(int)uvw[0],
							(int)uvw[1],
							(int)uvw[2]);
    }
  }

  for(i=0;i<obj->slab.size;i++) {
    obj->slab.tex[i*4+0]=255;
    obj->slab.tex[i*4+1]=255;
    obj->slab.tex[i*4+2]=255;
    obj->slab.tex[i*4+3]=255;
  }

  glBindTexture(GL_TEXTURE_2D,obj->slab.texname);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,
	       GL_RGBA,
	       obj->slab.usize,obj->slab.vsize,0,
	       GL_RGBA,GL_UNSIGNED_BYTE,
	       obj->slab.tex);

  return 0;
}

