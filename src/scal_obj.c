#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#include "dbm.h"
#include "mat.h"
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
#include "surf_obj.h"

//static char scal_obj_return[256];

static int add_vertex(struct SCAL2SURF_VERT **list, int *count, int *max, float p[3]);
static void calc_face_normal(struct SCAL2SURF_FACE *face, struct SCAL2SURF_VERT *vert);
static void add_face(struct SCAL2SURF_VERT *vert,int indx);
void check_normal(struct SCAL2SURF_FACE *face,int fi1, int fi3, struct SCAL2SURF_VERT *vert);
static int check_face_neigh(struct SCAL2SURF_FACE *f1, struct SCAL2SURF_FACE *f2, struct SCAL2SURF_VERT *vert, float b[3]);


extern struct GLOBAL_COM com;

int scalObjCommand(dbmScalNode *node,scalObj *obj,int wc,char **wl)
{
  char message[256];
  char *empty_com[]={"get","center"};
  int i;
  float tmp_tr;

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
      sprintf(message,"%s: missing expression\n", obj->name);
      comMessage(message);
      return -1;
    }

    tmp_tr=obj->render.transparency;
    if(renderSet(&obj->render,wc-1,wl+1)!=0) {
      sprintf(message,"%s: syntax error in render statement\n",obj->name);
      return -1;
    }


    if(obj->type==SCAL_CONTOUR) {
      if(obj->render.mode!=RENDER_LINE && 
	 obj->render.mode!=RENDER_POINT &&
	 obj->render.mode!=RENDER_SURFACE){
	obj->render.mode=RENDER_LINE;
	comMessage("invalid render mode\n");
	return -1;
      }

#ifdef CONTOUR_COLOR
      if(obj->render.transparency!=tmp_tr) {
	for(i=0;i<obj->point_count;i++)
	  obj->point[i].c[3]=obj->render.transparency;
	for(i=0;i<obj->face_count;i++) {
	  obj->face[i].c1[3]=obj->render.transparency;
	  obj->face[i].c2[3]=obj->render.transparency;
	  obj->face[i].c3[3]=obj->render.transparency;
	}
      }
#endif

    } else if(obj->type==SCAL_GRID) {
      if(obj->render.mode!=RENDER_ON && 
	 obj->render.mode!=RENDER_OFF) {
	obj->render.mode=RENDER_OFF;
	comMessage("invalid render mode\n");
	return -1;
      }

      for(i=0;i<obj->point_count;i++)
	obj->point[i].c[3]=obj->render.transparency;


    } else if(obj->type==SCAL_SLAB) {
      for(i=0;i<obj->slab.size;i++)
	obj->slab.tex[i*4+3]=(char)(127.0*obj->render.transparency);
      glBindTexture(GL_TEXTURE_2D,obj->slab.texname);
      glTexImage2D(GL_TEXTURE_2D,0,
		   GL_RGBA,
		   obj->slab.usize,obj->slab.vsize,0,
		   GL_RGBA,GL_BYTE,
		   obj->slab.tex);
    }
    comRedraw();
  } else if(!strcmp(wl[0],"material")) {
    if(wc<2) {
      comMessage(renderGetMaterial(&obj->render.mat));
    } else {
      if(renderMaterialSet(&obj->render.mat,wc-1,wl+1)!=0)
	return -1;
      comRedraw();
    }
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
    // TODO later
    // if scalar object is grabed should e.g. change contouring
    // level on the fly
  } else if(!strcmp(wl[0],"test")) {
    scalObj2Surf(obj,NULL);
  } else if(!strcmp(wl[0],"write")) {
    comMessage("not implemented yet\n");
    /*
    if(wc<2) {
      sprintf(message,"%s: expected filename for write command\n",obj->name);
      comMessage(message);
      return -1;
    }
    return scalObjWrite(obj, wc-1, wl+1);
    */
  } else {
    sprintf(message,"%s: unknown command %s\n",obj->name,wl[0]);
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
    sprintf(message,"%s: missing property\n", obj->name);
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
    sprintf(message,"Contouring at %f ...\n",obj->level);
    comMessage(message);

    /* check selection syntax */
    if(scalIsSelected(obj->node,0,0,0,sel)<0)
      return -1;

    ret=scalMCN(obj,sel);
  } else if(obj->type==SCAL_GRID) {
    sprintf(message,"Generating grid ...\n");
    comMessage(message);
    ret=scalGrid(obj,sel);
  } else if(obj->type==SCAL_GRAD) {
    ret=scalGrad(obj,sel);
#ifdef VR
  } else if(obj->type==SCAL_VR) {
    ret=scalVR(obj,sel);
#endif
  } else if(obj->type==SCAL_SLAB) {
    ret=scalSlab(obj,sel);
  }

  if(ret==-1)
    return -1;

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
  float r,g,b,r2,g2,b2,r3,g3,b3,v1[4],rad1,rad2;
  float rval,p[3],frac1,frac2,rval1,rval2;
  double vd1[3],vd2[3];
  float fact;
  char message[256],nv[16];
  int slab_flag=0;


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

      // pre eval color
      val=povGetVal(&s->pov[pc],0);

      if(comGetColor(val->val1,&r3,&g3,&b3)<0) {
	sprintf(message,"error: set: unknown color %s\n",val->val2);
	comMessage(message);
	return -1;
      }
      obj->r=r3;
      obj->g=g3;
      obj->b=b3;
      if(s->range_flag) {
	if(comGetColor(val->val2,&r2,&g2,&b2)<0) {
	  sprintf(message,"error: set: unknown color %s\n",val->val2);
	  comMessage(message);
	  return -1;
	}
      }
    } else if(clStrcmp(s->pov[pc].prop,"center") ||
	      clStrcmp(s->pov[pc].prop,"cen")) {
      if(s->pov[pc].op!=POV_OP_EQ) {
	comMessage("error: set: expected operator = for property \n");
	comMessage(s->pov[pc].prop);
	return -1;
      }
      s->pov[pc].id=SCAL_PROP_CENTER;
    } else if(clStrcmp(s->pov[pc].prop,"size")) {
      s->pov[pc].id=SCAL_PROP_SIZE;
    } else if(clStrcmp(s->pov[pc].prop,"scale")) {
      if(obj->type==SCAL_GRAD) {
	s->pov[pc].id=SCAL_PROP_SCALE;
      } else {
	comMessage("error: property scale only valid for object type grad\n");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"length")) {
      if(obj->type==SCAL_GRAD) {
	s->pov[pc].id=SCAL_PROP_LENGTH;
      } else {
	comMessage("error: property length only valid for object type grad\n");
	return -1;
      }
#ifdef VR
    } else if(clStrcmp(s->pov[pc].prop,"start")) {
      s->pov[pc].id=SCAL_PROP_START;
    } else if(clStrcmp(s->pov[pc].prop,"end")) {
      s->pov[pc].id=SCAL_PROP_END;
#endif
    } else if(clStrcmp(s->pov[pc].prop,"level")) {
      if(obj->type==SCAL_CONTOUR) {
	s->pov[pc].id=SCAL_PROP_LEVEL;
      } else {
	comMessage("error: property level only valid for object type contour\n");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"dir")) {
      if(obj->type==SCAL_SLAB) {
	s->pov[pc].id=SCAL_PROP_DIR;
      } else {
	comMessage("error: property dir only valid for object type slab\n");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"rad")) {
      if(obj->type==SCAL_GRID) {
	s->pov[pc].id=SCAL_PROP_RAD;
      } else {
	comMessage("error: property rad only valid for object type grid\n");
	return -1;
      }
    } else if(clStrcmp(s->pov[pc].prop,"step")) {
      s->pov[pc].id=SCAL_PROP_STEP;
    } else if(clStrcmp(s->pov[pc].prop,"method")) {
      if(s->pov[pc].op!=POV_OP_EQ) {
	comMessage("error: set: expected operator = for property \n");
	comMessage(s->pov[pc].prop);
	return -1;
      }
      comMessage("warning: set: obsolete property method ignored\n");
      s->pov[pc].id=SCAL_PROP_METHOD;
    } else {
      comMessage("error: set: unknown property \n");
      comMessage(s->pov[pc].prop);
      return -1;
    }

    op=s->pov[pc].op;
    if(!(op==POV_OP_EQ || op==POV_OP_PE || op==POV_OP_ME ||
	 op==POV_OP_SE || op==POV_OP_DE)) {
      comMessage("error: set: expected operator '=' '+=' '-=' '*=' or '/='\n");
      // /=
      return -1;
    }
    
    if(s->pov[pc].val_count>1) {
      comMessage("error: set: expected only one value for property \n");
      comMessage(s->pov[pc].prop);
      return -1;
    }
  }


  for(pc=0;pc<s->pov_count;pc++) {
    op=s->pov[pc].op;
    val=povGetVal(&s->pov[pc],0);

#ifndef CONTOUR_COLOR
    if(obj->type==SCAL_CONTOUR) {
      if(val->range_flag) {
	comMessage("error: set: range not supported for contour object\n");
	return -1;
      }
    }
#endif
    switch(s->pov[pc].id) {
    case SCAL_PROP_COLOR:
      if(flag==1) {
	if(obj->type==SCAL_CONTOUR) {
#ifdef CONTOUR_COLOR
	  for(ec=0;ec<obj->point_count;ec++) {
	    r=r3; g=g3; b=b3;
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
		if(s->range.clamp) {
		  if(frac2<0.0) {
		    frac2=0.0;
		  } else if(frac2>1.0) {
		    frac2=1.0;
		  }
		}
		if(frac2>=0.0 && frac2<=1.0) {
		  obj->point[ec].c[0]=(r2-r)*frac2+r;
		  obj->point[ec].c[1]=(g2-g)*frac2+g;
		  obj->point[ec].c[2]=(b2-b)*frac2+b;
		}
	      } else {  // range flag
		obj->point[ec].c[0]=r;
		obj->point[ec].c[1]=g;
		obj->point[ec].c[2]=b;
	      }
	      obj->point[ec].c[3]=obj->render.transparency;
	    }
	  }
	  // now the faces must be set
	  for(ec=0;ec<obj->face_count;ec++) {
	    obj->face[ec].c1[0]=obj->point[obj->face[ec].pi0].c[0];
	    obj->face[ec].c1[1]=obj->point[obj->face[ec].pi0].c[1];
	    obj->face[ec].c1[2]=obj->point[obj->face[ec].pi0].c[2];
	    obj->face[ec].c1[3]=obj->render.transparency;

	    obj->face[ec].c2[0]=obj->point[obj->face[ec].pi1].c[0];
	    obj->face[ec].c2[1]=obj->point[obj->face[ec].pi1].c[1];
	    obj->face[ec].c2[2]=obj->point[obj->face[ec].pi1].c[2];
	    obj->face[ec].c2[3]=obj->render.transparency;

	    obj->face[ec].c3[0]=obj->point[obj->face[ec].pi2].c[0];
	    obj->face[ec].c3[1]=obj->point[obj->face[ec].pi2].c[1];
	    obj->face[ec].c3[2]=obj->point[obj->face[ec].pi2].c[2];
	    obj->face[ec].c3[3]=obj->render.transparency;
	  }
#else
	  if(s->range_flag) {
	    comMessage("error: range not supported for contour object\n");
	    return -1;
	  }
	  obj->r=r;
	  obj->g=g;
	  obj->b=b;
#endif
	} else if(obj->type==SCAL_GRID) {
	  r=r3; g=g3; b=b3;
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
		if(s->range.clamp) {
		  if(frac2<0.0) {
		    frac2=0.0;
		  } else if(frac2>1.0) {
		    frac2=1.0;
		  }
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
	} else if(obj->type==SCAL_GRAD) {
	  r=r3; g=g3; b=b3;
	  for(ec=0;ec<obj->vect_count;ec++) {
	    f=0;
	    if(s->select_flag) {
	      res=scalIsSelected(obj->node,
				 obj->vect[ec].uvw[0],
				 obj->vect[ec].uvw[1],
				 obj->vect[ec].uvw[2],
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
					obj->vect[ec].v1,
					&rval)<0)
		    return -1;
		} else {
		  if(dbmGetRangeVal(&s->range,obj->vect[ec].v1,&rval)<0)
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
		if(s->range.clamp) {
		  if(frac2<0.0) {
		    frac2=0.0;
		  } else if(frac2>1.0) {
		    frac2=1.0;
		  }
		}
		if(frac2>=0.0 && frac2<=1.0) {
		  obj->vect[ec].c[0]=(r2-r)*frac2+r;
		  obj->vect[ec].c[1]=(g2-g)*frac2+g;
		  obj->vect[ec].c[2]=(b2-b)*frac2+b;
		}
	      } else {
		obj->vect[ec].c[0]=r;
		obj->vect[ec].c[1]=g;
		obj->vect[ec].c[2]=b;
	      }
	    }
	  }
	} else if (obj->type==SCAL_SLAB) {
	  /*
	    go through texture data and
	    assign colors accordingly
	  */
	  r=r3; g=g3; b=b3;
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
	      if(s->range.clamp) {
		if(frac2<0.0) {
		  frac2=0.0;
		} else if(frac2>1.0) {
		  frac2=1.0;
		}
	      }
	      if(frac2>=0.0 && frac2<=1.0) {
		obj->slab.tex[ec*4+0]=(char)(127.0*((r2-r)*frac2+r));
		obj->slab.tex[ec*4+1]=(char)(127.0*((g2-g)*frac2+g));
		obj->slab.tex[ec*4+2]=(char)(127.0*((b2-b)*frac2+b));
		obj->slab.tex[ec*4+3]=(char)(127.0*obj->render.transparency);
	      }
	    } else {
	      res=0;
	      if(s->select_flag) {
		if(scalSlabIsSelected(obj,ec,&s->select)) {
		  res=1;
		}
	      } else {
		res=1;
	      }
	      if(res==1) {
		obj->slab.tex[ec*4+0]=(char)(r*127.0);
		obj->slab.tex[ec*4+1]=(char)(g*127.0);
		obj->slab.tex[ec*4+2]=(char)(b*127.0);
		obj->slab.tex[ec*4+3]=(char)(127.0*obj->render.transparency);
	      }
	    }
	  }
	  glBindTexture(GL_TEXTURE_2D,obj->slab.texname);
	  glTexImage2D(GL_TEXTURE_2D,0,
		       GL_RGBA,
		       obj->slab.usize,obj->slab.vsize,0,
		       GL_RGBA,GL_BYTE,
		       obj->slab.tex);
	}
      }
      break;
    case SCAL_PROP_RAD:
    if(flag==1)
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
	    if(s->range.clamp) {
	      if(frac2<0.0) {
		frac2=0.0;
	      } else if(frac2>1.0) {
		frac2=1.0;
	      }
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
    case SCAL_PROP_SCALE:
      if(flag==1) {
	rad1=atof(val->val1);
	if(rad1==0.0) {
	  for(ec=0;ec<obj->vect_count;ec++) {
	    obj->vect[ec].v2[0]=obj->vect[ec].v1[0]+obj->vect[ec].n[0];
	    obj->vect[ec].v2[1]=obj->vect[ec].v1[1]+obj->vect[ec].n[1];
	    obj->vect[ec].v2[2]=obj->vect[ec].v1[2]+obj->vect[ec].n[2];
	  }
	  
	} else {
	  for(ec=0;ec<obj->vect_count;ec++) {
	    obj->vect[ec].v2[0]=obj->vect[ec].v1[0]+obj->vect[ec].grad[0]*rad1;
	    obj->vect[ec].v2[1]=obj->vect[ec].v1[1]+obj->vect[ec].grad[1]*rad1;
	    obj->vect[ec].v2[2]=obj->vect[ec].v1[2]+obj->vect[ec].grad[2]*rad1;
	  }
	}
      }
      break;
    case SCAL_PROP_LENGTH:
      if(flag==1) {
	for(ec=0;ec<obj->vect_count;ec++) {
	  f=0;
	  if(s->select_flag) {
	    res=scalIsSelected(obj->node,
			       obj->vect[ec].uvw[0],
			       obj->vect[ec].uvw[1],
			       obj->vect[ec].uvw[2],
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
				      obj->vect[ec].v1,
				      &rval)<0)
		  return -1;
	      } else {
		if(dbmGetRangeVal(&s->range,obj->vect[ec].v1,&rval)<0)
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
	      if(s->range.clamp) {
		if(frac2<0.0) {
		  frac2=0.0;
		} else if(frac2>1.0) {
		  frac2=1.0;
		}
	      }
	      if(frac2>=0.0 && frac2<=1.0) {
		obj->vect[ec].length=(rad2-rad1)*frac2+rad1;
	      }
	    } else {
	      rad1=atof(val->val1);
	      obj->vect[ec].length=rad1;
	    }
	  }
	}
      
	for(ec=0;ec<obj->vect_count;ec++) {
	  obj->vect[ec].v2[0]=obj->vect[ec].v1[0]+
	    obj->vect[ec].n[0]*obj->vect[ec].length;
	  obj->vect[ec].v2[1]=obj->vect[ec].v1[1]+
	    obj->vect[ec].n[1]*obj->vect[ec].length;
	  obj->vect[ec].v2[2]=obj->vect[ec].v1[2]+
	    obj->vect[ec].n[2]*obj->vect[ec].length;
	}
      }
    break;
    case SCAL_PROP_CENTER:
      if(matExtract1D(val->val1,3,vd1)!=0) {
	comMessage("error: set: syntax error in vector \n");
	comMessage(val->val1);
	return -1;
      }
      if(obj->type==SCAL_SLAB) {
	if(op==POV_OP_EQ) {
	  obj->slab.center[0]=vd1[0];
	  obj->slab.center[1]=vd1[1];
	  obj->slab.center[2]=vd1[2];
	} else {
	  comMessage("error: set: expected operator = for property center\n");
	  return -1;
	}
	slab_flag=1;
	//scalSlabIntersect(obj);
      } else {
	scalXYZtoUVW(obj->field,vd1,vd2);
	if(op==POV_OP_EQ) {
	  obj->u_center=(int)vd2[0];
	  obj->v_center=(int)vd2[1];
	  obj->w_center=(int)vd2[2];
	} else {
	  comMessage("error: set: expected operator = for property center\n");
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
	   os!=32 && os!=64 && os!=128 && os!=256 && os!=512 && os!=1024) {
	  comMessage("invalid size ignored\n");
	} else {
	  obj->slab.usize=os;
	  obj->slab.vsize=os;
	  //scalSlab(obj,NULL);
	}

      } else {
	if(val->val1[0]=='{') {
	  if(matExtract1Df(val->val1,3,v1)!=0) {
	    comMessage("error: set: expected {u,v,w} for size\n");
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
	  comMessage("warning: set: u size smaller than 2, reset to 2\n");
	  obj->u_size=2.0;
	}
	if(obj->v_size<2) {
	  comMessage("warning: set: v size smaller than 2, reset to 2\n");
	  obj->v_size=2.0;
	}
	if(obj->w_size<2) {
	  comMessage("warning: set: w size smaller than 2, reset to 2\n");
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
	  comMessage("error: set: expected {x,y,z} for dir\n");
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
	comMessage("error: expected operator '='\n");
	return -1;
      }
      slab_flag=1;
      //scalSlabIntersect(obj);
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
	comMessage("error: set: expected operators '=', '+=' or '-=' for property start\n");
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
	comMessage("error: set: expected operators '=', '+=' or '-=' for property end\n");
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
	comMessage("error: set: expected operators '=', '+=' or '-=' for property step\n");
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
		"warning: set: stepsize out of bounds (1-8), reset to %d\n",
		os);
	obj->step=os;
      }
      break;
    case SCAL_PROP_METHOD: break; // ignore
    }
  }

/*   if(slab_flag && flag) {
    scalSlabIntersect(obj);
  }
 */
/***
  fprintf(stderr,"%d %d %d\n%d %d %d   %d %d %d\n",
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
    sprintf(message,"%s: unknown property %s\n",obj->name, prop);
    comMessage(message);
    ret=-1;
  }
  
  return ret;
}

int scalObjCp(scalObj *o1, scalObj *o2)
{
  // TODO later
  comMessage("copy not implemented\n");
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
  spoint=Crecalloc(NULL,spoint_max,sizeof(struct SCAL_POINT));

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
	    spoint_max+=mem_add;
	    spoint=Crecalloc(spoint,spoint_max,sizeof(struct SCAL_POINT));
	  }
	}
      }

  obj->point=Crecalloc(spoint,spoint_count,sizeof(struct SCAL_POINT));
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
  fprintf(stderr,"%d %d %d  %d %d %d\n",
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
    sprintf(message,"scalVR: could not allocate %dx%dx%d 3D texture\n",
	    usize,vsize,wsize);
    comMessage(message);
    return -1;
  }

  if((obj->vr.data=Ccalloc(usize*vsize*wsize,sizeof(float)))==NULL) {
    comMessage("scalVR: memory allocation error\n");
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

//  fprintf(stderr,"slab\n");

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
  float pos[3],res;

//  fprintf(stderr,"slabIntersect\n");

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

/* 
  fprintf(stderr,"%f %f   %f %f\n",
	  mindist[0],maxdist[0],mindist[1],maxdist[1]);
 */

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
      /*
      scalXYZtoUVW(obj->field,xyz,uvw);
      obj->slab.data[v*obj->slab.usize+u]=scalReadField(obj->field,
							(int)uvw[0],
							(int)uvw[1],
							(int)uvw[2]);
      */
      pos[0]=(float)xyz[0];
      pos[1]=(float)xyz[1];
      pos[2]=(float)xyz[2];
      scalGetRangeXYZVal(obj->node, "", pos, &res);
      obj->slab.data[v*obj->slab.usize+u]=res;
    }
  }

  for(i=0;i<obj->slab.size;i++) {
    obj->slab.tex[i*4+0]=127;
    obj->slab.tex[i*4+1]=127;
    obj->slab.tex[i*4+2]=127;
    obj->slab.tex[i*4+3]=(unsigned char)(obj->render.transparency*255);
  }

  glBindTexture(GL_TEXTURE_2D,obj->slab.texname);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,0,
	       GL_RGBA,
	       obj->slab.usize,obj->slab.vsize,0,
	       GL_RGBA,GL_BYTE,
	       obj->slab.tex);

  return 0;
}

int scalSlabIsSelected(scalObj *obj, int c, Select *sel)
{
  int i,ec,res;

  if(sel==NULL)
    return 1;

  ec=selectGetPOVCount(sel);
  for(i=0;i<ec;i++) {
    res=scalSlabEvalPOV(obj,c,selectGetPOV(sel, i));
    if(res==-1)
      return -1;
    selectSetResult(sel, i, res);
  }  

  return selectResult(sel);

}

int scalSlabEvalPOV(scalObj *obj, int c, POV *pov)
{
  int i,vc,op;
  struct POV_VALUE *val;

  if(clStrcmp(pov->prop,"*"))
    return 1;

  op=pov->op;
  vc=pov->val_count;

  if(clStrcmp(pov->prop,"v")) {
    for(i=0;i<vc;i++) {
      val=povGetVal(pov,i);
      if(clStrcmp(val->val1,"*")) {
	return 1;
      } else {
	switch(op) {
	case POV_OP_EQ: if(obj->slab.data[c]==atof(val->val1)) return 1; break;
	case POV_OP_NE: if(obj->slab.data[c]!=atof(val->val1)) return 1; break;
	case POV_OP_LT: if(obj->slab.data[c]<atof(val->val1)) return 1; break;
	case POV_OP_LE: if(obj->slab.data[c]<=atof(val->val1)) return 1; break;
	case POV_OP_GT: if(obj->slab.data[c]>atof(val->val1)) return 1; break;
	case POV_OP_GE: if(obj->slab.data[c]>=atof(val->val1)) return 1; break;
	  
	}
      }
    }
  }
  return 0;
}


/* 
   calculate gradient based on

   |gx|   | [v(x+1,y,z)-v(x-1,y,z)] / dx |
   |gy| = | [v(x,y+1,z)-v(x,y-1,z)] / dy |
   |gz|   | [v(x,y,z+1)-v(x,y,z-1)] / dz |

   where v(x,y,z) is the value at grid coordinates xyz and
   d is the grid spacing

*/

int scalGrad(scalObj *obj, Select *sel)
{
  struct SCAL_VECT *vect;
  int vect_max, vect_count;
  int u,v,w,ustart,uend,vstart,vend,wstart,wend,res;
  int step,flag;
  int mem_add;
  double v0[3],v1[3],v2[3];
  double val1,val2,dx,dy,dz,grad[3],norm[3],length;
  char message[1024];

  mem_add=100000;
  vect_max=mem_add;
  vect_count=0;
  vect=Crecalloc(NULL, vect_max,sizeof(struct SCAL_VECT));

  ustart=obj->u_start; uend=obj->u_end;
  vstart=obj->v_start; vend=obj->v_end;
  wstart=obj->w_start; wend=obj->w_end;

  step=obj->step;

  v0[0]=(double)0;
  v0[1]=(double)0;
  v0[2]=(double)0;
  v1[0]=(double)1;
  v1[1]=(double)1;
  v1[2]=(double)1;
  scalUVWtoXYZ(obj->field,v1,v2);
  scalUVWtoXYZ(obj->field,v0,v1);

  dx=v2[0]-v1[0];
  dy=v2[1]-v1[1];
  dz=v2[2]-v1[2];

  if(dx==0.0 || dy==0.0 || dz==0.0) {
    comMessage("scalGrad: internal error #1\n");
    return -1;
  }

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
	  val1=scalReadField(obj->field,u-1,v,w);
	  val2=scalReadField(obj->field,u+1,v,w);
	  grad[0]=(val2-val1)/(2.0*dx);
	  val1=scalReadField(obj->field,u,v-1,w);
	  val2=scalReadField(obj->field,u,v+1,w);
	  grad[1]=(val2-val1)/(2.0*dy);
	  val1=scalReadField(obj->field,u,v,w-1);
	  val2=scalReadField(obj->field,u,v,w+1);
	  grad[2]=(val2-val1)/(2.0*dz);
	  matNormalize(grad,norm);
	  length=matCalcLen(grad);

	  vect[vect_count].v1[0]=v2[0];
	  vect[vect_count].v1[1]=v2[1];
	  vect[vect_count].v1[2]=v2[2];
	  vect[vect_count].grad[0]=grad[0];
	  vect[vect_count].grad[1]=grad[1];
	  vect[vect_count].grad[2]=grad[2];
	  vect[vect_count].n[0]=norm[0];
	  vect[vect_count].n[1]=norm[1];
	  vect[vect_count].n[2]=norm[2];
	  vect[vect_count].v2[0]=vect[vect_count].v1[0]+grad[0];
	  vect[vect_count].v2[1]=vect[vect_count].v1[1]+grad[1];
	  vect[vect_count].v2[2]=vect[vect_count].v1[2]+grad[2];
	  vect[vect_count].c[0]=1.0;
	  vect[vect_count].c[1]=1.0;
	  vect[vect_count].c[2]=1.0;
	  vect[vect_count].c[3]=1.0;
	  
	  vect_count++;

	  if(vect_count>=vect_max) {
	    vect_max+=mem_add;
	    vect=Crecalloc(vect,vect_max,sizeof(struct SCAL_VECT));
	  }
	}
      }

  obj->vect=Crecalloc(vect,vect_count,sizeof(struct SCAL_VECT));
  obj->vect_count=vect_count;
  obj->point=NULL;
  obj->point_count=0;
  obj->line=NULL;
  obj->line_count=0;
  obj->face=NULL;
  obj->face_count=0;

  sprintf(message," %d vectors",
	  obj->vect_count);
  comMessage(message);

  return 0;


}

/*
  convert a contour object into a surface dataset

  go through all faces

  generate a list of vertices and their normals
  adjusting the normals as necessary

  generate a list of faces, defined by 3 vertex indeces

*/

int scalObj2Surf(scalObj *obj,surfObj *surf) 
{
  int vc,vcount,vmax;
  int fc,fcount,fi,tc;
  struct SCAL2SURF_VERT *vert;
  struct SCAL2SURF_FACE *face;
  float ref[3];

  if(obj->type!=SCAL_CONTOUR) {
    return -1;
  }

  vmax=obj->point_count*5;
  vcount=0;
  vert=Crecalloc(NULL,vmax,sizeof(struct SCAL2SURF_VERT));
  fcount=obj->face_count;
  face=Ccalloc(fcount,sizeof(struct SCAL2SURF_FACE));

  fprintf(stderr,"1\n");
  // optimization possible by using face->pi0, pi1 and pi2

  for(fc=0;fc<fcount;fc++) {
    // assign vertex index to each face
    face[fc].i1=add_vertex(&vert,&vcount,&vmax,obj->face[fc].v1);
    face[fc].i2=add_vertex(&vert,&vcount,&vmax,obj->face[fc].v2);
    face[fc].i3=add_vertex(&vert,&vcount,&vmax,obj->face[fc].v3);
    // add the face index to each vertex
    add_face(&vert[face[fc].i1],fc);
    add_face(&vert[face[fc].i2],fc);
    add_face(&vert[face[fc].i3],fc);
    // calculate the face normals
    calc_face_normal(&face[fc],vert);
    face[fc].flag=0;
    face[fc].fc=0;
  }

  fprintf(stderr,"2\n");

  // find neighbouring faces
  // ref vector points from touching edge to opposite point
  for(fc=0;fc<fcount;fc++) {
    for(fi=0;fi<vert[face[fc].i1].fc;fi++) {
      if(check_face_neigh(&face[fc],&face[vert[face[fc].i1].fi[fi]],vert,ref)) {
	if(face[fc].fc<SCAL2SURF_MAX_FV) {
	  face[fc].fi[face[fc].fc]=vert[face[fc].i1].fi[fi];
	  face[fc].ref[3*face[fc].fc+0]=ref[0];
	  face[fc].ref[3*face[fc].fc+1]=ref[1];
	  face[fc].ref[3*face[fc].fc+2]=ref[2];
	  face[fc].fc++;
	}

      }
    }
    for(fi=0;fi<vert[face[fc].i2].fc;fi++) {
      if(check_face_neigh(&face[fc],&face[vert[face[fc].i2].fi[fi]],vert,ref)) {
	if(face[fc].fc<SCAL2SURF_MAX_FV) {
	  face[fc].fi[face[fc].fc]=vert[face[fc].i2].fi[fi];
	  face[fc].ref[3*face[fc].fc+0]=ref[0];
	  face[fc].ref[3*face[fc].fc+1]=ref[1];
	  face[fc].ref[3*face[fc].fc+2]=ref[2];
	  face[fc].fc++;

	}
      }
    }
    for(fi=0;fi<vert[face[fc].i3].fc;fi++) {
      if(check_face_neigh(&face[fc],&face[vert[face[fc].i3].fi[fi]],vert,ref)) {
	if(face[fc].fc<SCAL2SURF_MAX_FV) {
	  face[fc].fi[face[fc].fc]=vert[face[fc].i3].fi[fi];
	  face[fc].ref[3*face[fc].fc+0]=ref[0];
	  face[fc].ref[3*face[fc].fc+1]=ref[1];
	  face[fc].ref[3*face[fc].fc+2]=ref[2];
	  face[fc].fc++;
	} else {
	  fprintf(stderr,"face_neigh: max fc (%d) reached!\n",SCAL2SURF_MAX_FV);
	}
      }
    }
  }
  
  fprintf(stderr,"3\n");
  // go recursively through all faces and check normal orientation
  for(fc=0;fc<fcount;fc++) {
    for(fi=0;fi<face[fc].fc;fi++) {
      check_normal(face,fc,fi,vert);
    }
  }

  fprintf(stderr,"4\n");
  // calculate normals based on weighted face normals
  tc=0;
  for(vc=0;vc<vcount;vc++) {
    vert[vc].n[0]=face[vert[vc].fi[0]].n[0];
    vert[vc].n[1]=face[vert[vc].fi[0]].n[1];
    vert[vc].n[2]=face[vert[vc].fi[0]].n[2];
    for(fc=1;fc<vert[vc].fc;fc++) {
      fi=vert[vc].fi[fc];

      if(matfCalcDot(face[vert[vc].fi[0]].n,face[fi].n)<0) {
	tc++;
	//vert[vc].n[0]=10.0;
	//vert[vc].n[1]=10.0;
	//vert[vc].n[2]=10.0;
      } else {
      vert[vc].n[0]+=face[fi].n[0];
      vert[vc].n[1]+=face[fi].n[1];
      vert[vc].n[2]+=face[fi].n[2];
      }
    }
    matfNormalize(vert[vc].n,vert[vc].n);
    //    fprintf(stderr,"%d %f %f %f\n",vert[vc].fc,vert[vc].n[0],vert[vc].n[1],vert[vc].n[2]);
  }

  fprintf(stderr,"%d missmatches in vertice normals\n",tc);

  fprintf(stderr,"5\n");
  for(fc=0;fc<fcount;fc++) {
    obj->face[fc].v1[0]=vert[face[fc].i1].p[0];
    obj->face[fc].v1[1]=vert[face[fc].i1].p[1];
    obj->face[fc].v1[2]=vert[face[fc].i1].p[2];
    obj->face[fc].v2[0]=vert[face[fc].i2].p[0];
    obj->face[fc].v2[1]=vert[face[fc].i2].p[1];
    obj->face[fc].v2[2]=vert[face[fc].i2].p[2];
    obj->face[fc].v3[0]=vert[face[fc].i3].p[0];
    obj->face[fc].v3[1]=vert[face[fc].i3].p[1];
    obj->face[fc].v3[2]=vert[face[fc].i3].p[2];

    obj->face[fc].n1[0]=vert[face[fc].i1].n[0];
    obj->face[fc].n1[1]=vert[face[fc].i1].n[1];
    obj->face[fc].n1[2]=vert[face[fc].i1].n[2];
    obj->face[fc].n2[0]=vert[face[fc].i2].n[0];
    obj->face[fc].n2[1]=vert[face[fc].i2].n[1];
    obj->face[fc].n2[2]=vert[face[fc].i2].n[2];
    obj->face[fc].n3[0]=vert[face[fc].i3].n[0];
    obj->face[fc].n3[1]=vert[face[fc].i3].n[1];
    obj->face[fc].n3[2]=vert[face[fc].i3].n[2];
  }

  Cfree(face);
  Cfree(vert);

  return 0;
}


static int add_vertex(struct SCAL2SURF_VERT **list, int *count, int *max, float p[3]) 
{
  int n;
  float dx,dy,dz,eps=1e-8;

  // either find already existing vertex
  // or add new
  // return its index at any rate

  for(n=0;n<(*count);n++) {
    dx=p[0]-(*list)[n].p[0];
    dy=p[1]-(*list)[n].p[1];
    dz=p[2]-(*list)[n].p[2];
    if(fabsf(dx)<eps &&
       fabsf(dy)<eps &&
       fabsf(dz)<eps) {
      break;
    }
  }

  if(n==(*count)) {
    if((*count)>=(*max)) {
      (*max)+=1000;
      (*list)=Crecalloc((*list),(*max)*3,sizeof(struct SCAL2SURF_VERT));
    }
    (*list)[(*count)].p[0]=p[0];
    (*list)[(*count)].p[1]=p[1];
    (*list)[(*count)].p[2]=p[2];
    (*list)[(*count)].fc=0;
    (*list)[(*count)].flag=0;
    (*count)++;
  }

  return n;
}

static void calc_face_normal(struct SCAL2SURF_FACE *face, struct SCAL2SURF_VERT *vert)
{
  float v1[3],v2[3],nn[3];

  // from the three vertices calculate the face area
  // and the normal vector
  v2[0]=vert[face->i2].p[0]-vert[face->i1].p[0];
  v2[1]=vert[face->i2].p[1]-vert[face->i1].p[1];
  v2[2]=vert[face->i2].p[2]-vert[face->i1].p[2];
  v1[0]=vert[face->i3].p[0]-vert[face->i1].p[0];
  v1[1]=vert[face->i3].p[1]-vert[face->i1].p[1];
  v1[2]=vert[face->i3].p[2]-vert[face->i1].p[2];

  face->area=matCalcTriArea(vert[face->i1].p,vert[face->i2].p,vert[face->i3].p);
  if(face->area==0.0) {
    fprintf(stderr,"face area of zero!\n");
    face->area=1.0;
  }
  matfCalcCross(v2,v1,nn);
  matfNormalize(nn,face->n);
}

static void add_face(struct SCAL2SURF_VERT *vert,int indx) 
{
  if(vert->fc<SCAL2SURF_MAX_FV) {
    vert->fi[vert->fc++]=indx;
  } else {
    fprintf(stderr,"add_face: max fc (%d) reached!\n",SCAL2SURF_MAX_FV);
  }
}

void check_normal(struct SCAL2SURF_FACE *face,int fr, int fci, struct SCAL2SURF_VERT *vert)
{
  int fc;
  static int i,fi2;
  static float v1[3],v2[3];
    
  // fr one is the reference, fci index to the current face
  fi2=face[fr].fi[fci];
  
  if(face[fi2].flag<10) {
    face[fi2].flag++;

    /*
      compare current face to reference, if 
      angle between normals >90 deg, swap
      two face indices to change orientation
      of triangle, then recalc normal
    */

    if((matfCalcDot(face[fr].n,face[fi2].n)<0.0)) {
      i=face[fi2].i2;
      face[fi2].i2=face[fi2].i3;
      face[fi2].i3=i;
      calc_face_normal(&face[fi2],vert);
    }
    
    // check all neighbours, using current face as reference
    for(fc=0;fc<face[fi2].fc;fc++) {
      check_normal(face,fi2,fc,vert);
    }
  }
}

static int check_face_neigh(struct SCAL2SURF_FACE *f1, struct SCAL2SURF_FACE *f2, struct SCAL2SURF_VERT *vert, float b[3])
{
  int c=0;
  float *p1,*p2,*p3;
  int t1,t2;

  // TODO init t1 and t2, use |= instead of &= ???
  t1=t2=0;

  if(f1->i1==f2->i1) {
    t1+=0x1; t2+=0x1;
    c++;
  } else if(f1->i1==f2->i2) {
    t1+=0x1; t2+=0x2;
    c++;
  } else if(f1->i1==f2->i3) {
    t1+=0x1; t2+=0x4;
    c++;
  }

  if(f1->i2==f2->i1) {
    t1+=0x2; t2+=0x1;
    c++;
  } else if(f1->i2==f2->i2) {
    t1+=0x2; t2+=0x2;
    c++;
  } else if(f1->i2==f2->i3) {
    t1+=0x2; t2+=0x4;
    c++;
  }

  if(f1->i3==f2->i1) {
    t1+=0x4; t2+=0x1;
    c++;
  } else if(f1->i3==f2->i2) {
    t1+=0x4; t2+=0x2;
    c++;
  } else if(f1->i3==f2->i3) {
    t1+=0x4; t2+=0x4;
    c++;
  }

  if(c==2) {
    if(t1==0x3) {
      // i1+i2 are neigh, i3 is opposite
      p1=vert[f1->i3].p;
      p2=vert[f1->i2].p;
      p3=vert[f1->i1].p;
    } else if(t1==0x5) {
      // i2 opposite
      p1=vert[f1->i2].p;
      p2=vert[f1->i3].p;
      p3=vert[f1->i1].p;
    } else if(t1==0x6) {
      // i1 opposite
      p1=vert[f1->i1].p;
      p2=vert[f1->i2].p;
      p3=vert[f1->i3].p;
    } else {
      //fprintf(stderr,"internal error (%x)\n",t1);
      return 0;
    }

    b[0]=p1[0]-(p3[0]+p2[0])*0.5;
    b[1]=p1[1]-(p3[1]+p2[1])*0.5;
    b[2]=p1[2]-(p3[2]+p2[2])*0.5;
    
    return 1;
  }

  return 0;
}

