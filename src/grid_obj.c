#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com.h"
#include "dbm.h"
#include "mat.h"
#include "render.h"
#include "Cmalloc.h"
#include "grid_db.h"
#include "grid_obj.h"
#include "cl.h"

int gridObjCommand(struct DBM_GRID_NODE *node, gridObj *obj, int wc, char **wl)
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
  } else if(!strcmp(wl[0],"get")) {
    return gridObjComGet(obj, wc-1, wl+1);
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

    if(renderSet(&obj->render,wc-1,wl+1)!=0)
      return -1;

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
      sprintf(message,"Current material setting:\namb: {%.3f,%.3f,%.3f}\nspec: {%3f,%3f,%3f}\nshin: %3f\nemm: {%.3f,%.3f,%.3f}\n",
	      obj->render.mat.amb[0],
	      obj->render.mat.amb[1],
	      obj->render.mat.amb[2],
	      obj->render.mat.spec[0],
	      obj->render.mat.spec[1],
	      obj->render.mat.spec[2],
	      obj->render.mat.shin,
	      obj->render.mat.emm[0],
	      obj->render.mat.emm[1],
	      obj->render.mat.emm[2]);
      comMessage(message);
    } else {

      if(renderMaterialSet(&obj->render.mat,wc-1,wl+1)!=0)
	return -1;
      
      comRedraw();
    }
  } else if(!strcmp(wl[0],"set")) {
    return gridObjComSet(obj, wc-1, wl+1);
  } else if(!strcmp(wl[0],"renew")) {
    return gridObjComRenew(obj, wc-1, wl+1);
  } else if(clStrcmp(wl[0],"map")) {
    return gridObjComMap(obj, wc-1, wl+1);
  } else if(clStrcmp(wl[0],"unmap")) {
    obj->map=-1;
    return 0;
  } else { 
    sprintf(message,"\n%s: unknow command: %s",obj->name, wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}

int gridObjComSet(gridObj *obj, int wc, char **wl)
{
  Set set;
  int ret;

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  ret=gridObjSet(obj, &set,1);

  setDelete(&set);

  comRedraw();

  return ret;
}

int gridObjComGet(gridObj *obj, int wc, char **wl)
{
  char message[256];
  int i;

  if(wc==0) {
    sprintf(message,"\n%s: missing property", obj->name);
    comMessage(message);
    return -1;
  }
  for(i=0;i<wc;i++)
    if(gridObjGet(obj,wl[i])<0)
      return -1;

  return 0;
}

int gridObjComMap(gridObj *obj, int wc, char **wl)
{
  int i,m;
  char message[256],texname[256];
  gridTexture *tex;
  

  if(wc<1) {
    comMessage("\nerror: missing texture name");
    return -1;
  }
  clStrcpy(texname,wl[0]);

  m=-1;
  for(i=0;i<obj->node->texture_count;i++) {
    if(clStrcmp(obj->node->texture[i].name,texname)) {
      m=i;
      break;
    }
  }

  if(m==-1) {
    sprintf(message,"\nerror: texture %s not found",texname);
    comMessage(message);
    return -1;
  }

  obj->map=m;

  tex=&obj->node->texture[obj->map];

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D,obj->texname);
 
      glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,
		   tex->width,tex->height, 0,
		   GL_RGBA, GL_BYTE, tex->data);

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  return 0;
}

int gridObjComRenew(gridObj *obj, int wc, char **wl)
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
    ret=gridObjRenew(obj, &set, &obj->select,vflag);
  } else {
    ret=gridObjRenew(obj, &set, &sel,vflag);
    selectDelete(&obj->select);
    memcpy(&obj->select,&sel,sizeof(Select));
  }

  setDelete(&set);
  clDelete(&co);
  comRedraw();
  return ret;
}


int gridObjSet(gridObj *obj, Set *s, int flag)
{
  int vc,f,pc;
  struct POV_VALUE *val;
  float r,g,b,r2,g2,b2;
  float rval,rvals,p[3],frac1,frac2,rval1,rval2;
  const char *defprop="h\0";
  char *value1,*value2;
  int id;

  if(s->pov_count==0) {
    return 0;
  }

  if(s->range_flag) {
    if(s->range.val1==NULL)
      rval1=0;
    else
    rval1=atof(s->range.val1);
    if(s->range.val2==NULL)
      rval2=1.0;
    else
      rval2=atof(s->range.val2);
  }
  
  for(pc=0;pc<s->pov_count;pc++) {
    if(clStrcmp(s->pov[pc].prop,"color") ||
       clStrcmp(s->pov[pc].prop,"colour") ||
       clStrcmp(s->pov[pc].prop,"col")) {
      s->pov[pc].id=GRID_PROP_COLOR;
    } else if(clStrcmp(s->pov[pc].prop,"step") ||
	      clStrcmp(s->pov[pc].prop,"s")) {
      s->pov[pc].id=GRID_PROP_STEP;
    } else if(clStrcmp(s->pov[pc].prop,"lstart")) {
      s->pov[pc].id=GRID_PROP_LSTART;
    } else if(clStrcmp(s->pov[pc].prop,"lend")) {
      s->pov[pc].id=GRID_PROP_LEND;
    } else if(clStrcmp(s->pov[pc].prop,"lstep")) {
      s->pov[pc].id=GRID_PROP_LSTEP;
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
    id=s->pov[pc].id;
    
    if(id==GRID_PROP_COLOR) {  /* vertice based properties */
      for(vc=0;vc<obj->vertc;vc++) {
	f=0;
	if(s->select_flag) {
	  if(gridIsSelected(obj->node, obj->vert[vc].gp, &s->select))
	    f=1;
	} else {
	  f=1;
	}
	if(f) {
	  switch(id) {
	  case GRID_PROP_COLOR:
	    if(flag==1) {
	      if(s->range_flag) {
		value1=val->val1;
		if(val->val2==NULL)
		  value2=value1;
		else
		  value2=val->val2;
		if(comGetColor(value1,&r,&g,&b)<0) {
		  comMessage("\nerror: set: unknown color ");
		  comMessage(value1);
		  return -1;
		}
		if(comGetColor(value2,&r2,&g2,&b2)<0) {
		  comMessage("\nerror: set: unknown color ");
		  comMessage(value2);
		  return -1;
		}
		if(s->range.src==NULL) {
		  // this dataset
		  if(s->range.prop==NULL || clStrcmp(s->range.prop,"h")) {
		    if(obj->type==GRID_SURFACE) {
		      if(gridGetRangeVal(obj->node,"h", obj->vert[vc].gp,&rval)<0)
			return -1;
		    } else { /* obj!=SURFACE */
		      rval=obj->vert[vc].z/255.0;
		    }
		  } else {
		    if(obj->type==GRID_CONTOUR) {
		      comMessage("\ncross range not supported for type contour");
		      return -1;
		    }
		    if(!obj->node->attach_flag) {
		      comMessage("\nerror: -range only meaningful with attached structure");
		      return -1;
		    }
		    if(obj->vert[vc].gp->attach_node!=NULL) {
		      if(structGetRangeVal(&obj->vert[vc].gp->attach_node->structNode,
					   &obj->vert[vc].gp->attach_node->structNode.atom[obj->vert[vc].gp->attach_element],
					   s->range.prop,&rval)<0)
			return -1;
		    } else {
		      continue;
		    }
		  }
		} else {
		  p[0]=obj->vert[vc].v[0];
		  p[1]=obj->vert[vc].v[1];
		  p[2]=obj->vert[vc].v[2];
		  transApplyf(&obj->node->transform,p);
		  if(dbmGetRangeVal(&s->range,p,&rval)<0)
		    return -1;
		}
		if(obj->vert[vc].gp!=NULL || obj->type!=GRID_SURFACE) {
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
		obj->vert[vc].c[0]=r;
		obj->vert[vc].c[1]=g;
		obj->vert[vc].c[2]=b;
	      }
	    }	
	    break;
	  }
	}
      }
    } else {   /* object based properties */
      switch(id) {
      case GRID_PROP_STEP:
	if(s->range_flag) {
	  comMessage("\nerror: set: unexpected range for step");
	  return -1;
	}
	if(atoi(val->val1)<1) {
	  comMessage("\nerror: set: expected positive integer step value");
	  return -1;
	}
	obj->step=atoi(val->val1);
	break;
      case GRID_PROP_LSTART:
	if(s->range_flag) {
	  comMessage("\nerror: set: unexpected range for step");
	  return -1;
	}
	obj->level_start=atof(val->val1)*255.0;
	break;
      case GRID_PROP_LEND:
	if(s->range_flag) {
	  comMessage("\nerror: set: unexpected range for step");
	  return -1;
	}
	obj->level_end=atof(val->val1)*255.0;
	break;
      case GRID_PROP_LSTEP:
	if(s->range_flag) {
	  comMessage("\nerror: set: unexpected range for step");
	  return -1;
	}
	obj->level_step=atof(val->val1)*255.0;
	break;
      }
    }
  }  
  return 0;
}

int gridObjGet(gridObj *obj, char *prop)
{
  int i;
  char message[256];
  float x,y,z;

  if(!strcmp(prop,"center")) {
    x=0.0;
    y=0.0;
    z=0.0;
    for(i=0;i<obj->vertc;i++) {
      x+=obj->vert[i].v[0];
      y+=obj->vert[i].v[1];
      z+=obj->vert[i].v[2];
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


int gridObjRenew(gridObj *obj, Set *set, Select *sel, int vflag)
{
  float area;
  int i;
  char message[256];

  gridObjSet(obj,set,0);

  strcpy(message,"");

  if(obj->type==GRID_SURFACE) {
    if(gridGenerate(obj,sel)<0)
      return -1;
    sprintf(message," %d vertices and %d faces",obj->vertc,obj->facec);
  } else {
    if(gridContour(obj,sel)<0)
      return -1;
    sprintf(message," %d lines",obj->vertc*2);
  }
  gridObjSet(obj,set,1);

  comMessage(message);

  if(vflag) {
    area=0.0;
    for(i=0;i<obj->facec;i++)
      area+=matCalcTriArea(obj->face[i].v1,
			   obj->face[i].v2,
			   obj->face[i].v3);
    sprintf(message,"\narea: %.4f A^2",area);
    comMessage(message);
  }
  
  if(obj->texname>-1) {
    glDeleteTextures(1,&obj->texname);
  }
  glGenTextures(1,&obj->texname);

  comRedraw();

  return 0;
}


int gridGenerate(gridObj *obj, Select *sel)
{
  gridPoint *data;
  gridField *field;
  int width,height,count;
  int u,v,p,u2,v2,p2,p3;
  int f1,f2,f3,f4;
  int step=obj->step;
  gridVert *vert;
  int vertc;
  gridFace *face;
  int facec,flag;
  double vv[3];
  float vvf[3],vv1[3],vv2[3],n1[3],n2[3],n3[3],n4[3],t[2];
  float fx,fy,fz;

  field=&obj->node->field;

  fx=field->scale_x;
  fy=field->scale_y;
  fz=field->scale_z;

  comMessage("\nGenerating ");

  // if step size is larger then 1 generate subgrid
  if(step>1) {
    width=field->width/step;
    height=field->height/step;
    count=width*height;
    data=Ccalloc(count,sizeof(gridPoint));
    for(u=0;u<width;u++) {
      for(v=0;v<height;v++) {
	p=v*width+u;
	v2=v*step;
	u2=u*step;
	p2=v2*field->width+u2;
	memcpy(&data[p],&field->point[p2],sizeof(gridPoint));
      }
    }
  } else {
    width=field->width;
    height=field->height;
    count=width*height;
    data=field->point;
  }

  comMessage(".");

  vertc=0;
  vert=Ccalloc(count+height+16,sizeof(gridVert));

  for(u=0;u<width;u++) {
    for(v=0;v<height;v++) {
      p=v*width+u;
      if(gridIsSelected(obj->node,&data[p],sel))
	vert[p].sel=1;
      else
	vert[p].sel=0;
      vvf[0]=(float)(u*step);
      vvf[1]=(float)(v*step);
      gridUVWtoXYZ(field,vvf,vert[p].v);
      v2=v*step;
      u2=u*step;
      p2=v2*field->width+u2;
      vert[p].p=p2;
      vert[p].gp=&field->point[p2];

      vert[p].t[0]=(float)u/(float)(width-1);
      vert[p].t[1]=(float)v/(float)(height-1);

      vertc++;
    }
  }

  comMessage(".");

  fx=field->scale_x/(float)step;
  fy=field->scale_y/(float)step;
  fz=field->scale_z;

  // calculate normals
  for(u=0;u<width;u++) {
    for(v=0;v<height;v++) {
      p=v*width+u;

      if(u+1<width && v+1<height) {
	p2=(v+0)*width+(u+1);
	p3=(v+1)*width+(u+0);
	vv1[0]=vert[p2].v[0]-vert[p].v[0];
	vv1[1]=vert[p2].v[1]-vert[p].v[1];
	//	vv1[0]=fx; vv1[1]=0.0;
	vv1[2]=fz*(float)(data[p2].z);
	//	vv2[0]=0.0; vv2[1]=fy;
	vv2[0]=vert[p3].v[0]-vert[p].v[0];
	vv2[1]=vert[p3].v[1]-vert[p].v[1];
	vv2[2]=fz*(float)(data[p3].z);
	matfCalcCross(vv1,vv2,vvf);
	matfNormalize(vvf,n1);
      } else {
	n1[0]=n1[1]=n1[2]=0.0;
      }

      if(u>0 && v+1<height) {
	p2=(v+0)*width+(u-1);
	p3=(v+1)*width+(u+0);
	vv1[0]=vert[p2].v[0]-vert[p].v[0];
	vv1[1]=vert[p2].v[1]-vert[p].v[1];
	//	vv1[0]=-fx; vv1[1]=0.0; 
	vv1[2]=fz*(float)(data[p2].z);
	//	vv2[0]=0.0; vv2[1]=fy; 
	vv2[0]=vert[p3].v[0]-vert[p].v[0];
	vv2[1]=vert[p3].v[1]-vert[p].v[1];
	vv2[2]=fz*(float)(data[p3].z);
	matfCalcCross(vv2,vv1,vvf);
	matfNormalize(vvf,n2);
      } else {
	n2[0]=n2[1]=n2[2]=0.0;
      }

      if(u>0 && v>0) {
	p2=(v+0)*width+(u-1);
	p3=(v-1)*width+(u+0);
	//	vv1[0]=-fx; vv1[1]=0.0; 
	vv1[0]=vert[p2].v[0]-vert[p].v[0];
	vv1[1]=vert[p2].v[1]-vert[p].v[1];
	vv1[2]=fz*(float)(data[p2].z);
	//	vv2[0]=0.0; vv2[1]=-fy; 
	vv2[0]=vert[p3].v[0]-vert[p].v[0];
	vv2[1]=vert[p3].v[1]-vert[p].v[1];
	vv2[2]=fz*(float)(data[p3].z);
	matfCalcCross(vv1,vv2,vvf);
	matfNormalize(vvf,n3);
      } else {
	n3[0]=n3[1]=n3[2]=0.0;
      }

      if(u+1<width && v>0) {
	p2=(v+0)*width+(u+1);
	p3=(v-1)*width+(u+0);
	//	vv1[0]=fx; vv1[1]=0.0;
	vv1[0]=vert[p2].v[0]-vert[p].v[0];
	vv1[1]=vert[p2].v[1]-vert[p].v[1];
	vv1[2]=fz*(float)(data[p2].z);
	//	vv2[0]=0.0; vv2[1]=-fy;
	vv2[0]=vert[p3].v[0]-vert[p].v[0];
	vv2[1]=vert[p3].v[1]-vert[p].v[1];
	vv2[2]=fz*(float)(data[p3].z);
	matfCalcCross(vv2,vv1,vvf);
	matfNormalize(vvf,n4);
      } else {
	n4[0]=n4[1]=n4[2]=0.0;
      }

      vvf[0]=n1[0]+n2[0]+n3[0]+n4[0];
      vvf[1]=n1[1]+n2[1]+n3[1]+n4[1];
      vvf[2]=n1[2]+n2[2]+n3[2]+n4[2];

      matfNormalize(vvf,vert[p].n);

    }
  }

  comMessage(".");


  // reset color
  for(u=0;u<width;u++) {
    for(v=0;v<height;v++) {
      p=v*width+u;
      vert[p].c[0]=1.0;
      vert[p].c[1]=1.0;
      vert[p].c[2]=1.0;
    }
  }


  if(obj->node->smode==GRID_SMODE_ALL) {
    flag=2;
  } else {
    flag=1;
  }
  
  // fill face structure

  facec=0;
  face=Ccalloc(count*2,sizeof(gridFace));

  for(u=0;u<width-1;u++) {
    for(v=0;v<height-1;v++) {
      f1=(v+0)*width+(u+0);
      f2=(v+0)*width+(u+1);
      f3=(v+1)*width+(u+0);
      f4=(v+1)*width+(u+1);

      
      if(vert[f1].sel+vert[f2].sel+vert[f3].sel>=flag) {
	face[facec].v1=vert[f1].v;
	face[facec].v2=vert[f2].v;
	face[facec].v3=vert[f3].v;
	face[facec].n1=vert[f1].n;
	face[facec].n2=vert[f2].n;
	face[facec].n3=vert[f3].n;
	face[facec].c1=vert[f1].c;
	face[facec].c2=vert[f2].c;
	face[facec].c3=vert[f3].c;
	face[facec].t1=vert[f1].t;
	face[facec].t2=vert[f2].t;
	face[facec].t3=vert[f3].t;
	facec++;
      }
      if(vert[f2].sel+vert[f3].sel+vert[f4].sel>=flag) {
	face[facec].v1=vert[f2].v;
	face[facec].v2=vert[f4].v;
	face[facec].v3=vert[f3].v;
	face[facec].n1=vert[f2].n;
	face[facec].n2=vert[f4].n;
	face[facec].n3=vert[f3].n;
	face[facec].c1=vert[f2].c;
	face[facec].c2=vert[f4].c;
	face[facec].c3=vert[f3].c;
	face[facec].t1=vert[f2].t;
	face[facec].t2=vert[f4].t;
	face[facec].t3=vert[f3].t;
	facec++;
      }
    }
  }

  comMessage(".");

  if(obj->vertc>0)
    Cfree(obj->vert);
  if(obj->facec>0)
    Cfree(obj->face);

  obj->vert=vert;
  obj->vertc=vertc;
  obj->face=face;
  obj->facec=facec;

  if(step>1)
    Cfree(data);

  return 0;

}

int gridGenerate2(gridObj *obj, Select *sel)
{
  int u,v,p,p1,p2,p3,p4,p5;
  float uvw[3],v1[3],v2[3],v3[3],v4[3],v5[3];
  float n1[3],n2[3],n3[3],n4[3],n5[3];
  gridField *field=&obj->node->field;
  gridVert *vert;
  int vertc;
  gridFace *face;
  int fc;
  int step=obj->step;
  int vmax,fmax;

  comMessage("\nGenerating ");

  if(obj->vertc>0)
    Cfree(obj->vert);

  if(obj->facec>0)
    Cfree(obj->face);

  vmax=obj->node->field.point_count*2;
  fmax=obj->node->field.point_count*4;

  obj->vert=Ccalloc(obj->node->field.point_count*2,sizeof(gridVert));
  vert=obj->vert;
  vertc=0;
  obj->face=Ccalloc(obj->node->field.point_count*4,sizeof(gridFace));
  face=obj->face;
  fc=0;

  for(u=0;u<field->width;u++)
    for(v=0;v<field->height;v++) {
      p=v*field->width+u;
      if(gridIsSelected(obj->node,&field->point[p],sel))
	vert[p].sel=1;
      else
	vert[p].sel=0;
      uvw[0]=(float)(u+0);
      uvw[1]=(float)(v+0);
      gridUVWtoXYZ(field,uvw,vert[p].v);
      vert[p].p=p;
      vert[p].gp=&obj->node->field.point[p];
      vertc++;
    }
  comMessage(".");
  // reset the color
  for(u=1;u<(field->width-1);u++)
    for(v=1;v<(field->height-1);v++) {
      p=v*field->width+u;
      vert[p].c[0]=1.0;
      vert[p].c[1]=1.0;
      vert[p].c[2]=1.0;
      vert[p].c[3]=1.0;
    }

  comMessage(".");

  // calculate normals
  for(u=step;u<(field->width-step);u+=step)
    for(v=step;v<(field->height-step);v+=step) {
      p=v*field->width+u;
      p1=(v+0)*field->width+(u+step);
      p2=(v+step)*field->width+(u+0);
      p3=(v+0)*field->width+(u-step);
      p4=(v-step)*field->width+(u+0);
      // diff
      matfCalcDiff(vert[p1].v,vert[p].v,v1);
      matfCalcDiff(vert[p2].v,vert[p].v,v2);
      matfCalcDiff(vert[p3].v,vert[p].v,v3);
      matfCalcDiff(vert[p4].v,vert[p].v,v4);
      matfCalcCross(v1,v2,n1);
      matfCalcCross(v2,v3,n2);
      matfCalcCross(v3,v4,n3);
      matfCalcCross(v4,v1,n4);
      n5[0]=n1[0]+n2[0]+n3[0]+n4[0];
      n5[1]=n1[1]+n2[1]+n3[1]+n4[1];
      n5[2]=n1[2]+n2[2]+n3[2]+n4[2];
      matfNormalize(n5,vert[p].n);
    }
  comMessage(".");
  // go through all the faces, note -2 !
  for(u=step;u<(field->width-step-1);u+=step)
    for(v=step;v<(field->height-step-1);v+=step) {
      // calc middle point
      p1=(v+0)*field->width+(u+0);
      p2=(v+0)*field->width+(u+step);
      p3=(v+step)*field->width+(u+0);
      p4=(v+step)*field->width+(u+step);
      v5[0]=vert[p1].v[0]+vert[p2].v[0]+vert[p3].v[0]+vert[p4].v[0];
      v5[1]=vert[p1].v[1]+vert[p2].v[1]+vert[p3].v[1]+vert[p4].v[1];
      v5[2]=vert[p1].v[2]+vert[p2].v[2]+vert[p3].v[2]+vert[p4].v[2];
      v5[0]*=0.25;
      v5[1]*=0.25;
      v5[2]*=0.25;
      matfCopyVV(v5,vert[vertc].v);
      vert[vertc].p=vertc;
      vert[vertc].gp=NULL;
      vert[vertc].sel=0;  // set middle point false
      vert[vertc].p1=p1;
      vert[vertc].p2=p2;
      vert[vertc].p3=p3;
      vert[vertc].p4=p4;

      n5[0]=vert[p1].n[0]+vert[p2].n[0]+vert[p3].n[0]+vert[p4].n[0];
      n5[1]=vert[p1].n[1]+vert[p2].n[1]+vert[p3].n[1]+vert[p4].n[1];
      n5[2]=vert[p1].n[2]+vert[p2].n[2]+vert[p3].n[2]+vert[p4].n[2];
      n5[0]*=0.25;
      n5[1]*=0.25;
      n5[2]*=0.25;
      matfCopyVV(n5,vert[vertc].n);
      vert[vertc].c[0]=1.0; vert[vertc].c[1]=1.0; vert[vertc].c[2]=1.0;
      p5=vertc;
      vertc++;

      // check selection for each face
      // p5 is NOT checked, because it is the middle point

      // add 4 faces: 153 354 452 251
      if(vert[p1].sel && vert[p3].sel) {
	face[fc].v1=vert[p1].v; face[fc].n1=vert[p1].n; face[fc].c1=vert[p1].c;
	face[fc].v2=vert[p5].v; face[fc].n2=vert[p5].n; face[fc].c2=vert[p5].c;
	face[fc].v3=vert[p3].v; face[fc].n3=vert[p3].n; face[fc].c3=vert[p3].c;
	fc++;
      }
      if(vert[p3].sel  && vert[p4].sel) {
	face[fc].v1=vert[p3].v; face[fc].n1=vert[p3].n; face[fc].c1=vert[p3].c;
	face[fc].v2=vert[p5].v; face[fc].n2=vert[p5].n; face[fc].c2=vert[p5].c;
	face[fc].v3=vert[p4].v; face[fc].n3=vert[p4].n; face[fc].c3=vert[p4].c;
	fc++;
      } 
      if(vert[p4].sel && vert[p2].sel) {
	face[fc].v1=vert[p4].v; face[fc].n1=vert[p4].n; face[fc].c1=vert[p4].c;
	face[fc].v2=vert[p5].v; face[fc].n2=vert[p5].n; face[fc].c2=vert[p5].c;
	face[fc].v3=vert[p2].v; face[fc].n3=vert[p2].n; face[fc].c3=vert[p2].c;
	fc++;
      }
      if(vert[p2].sel && vert[p1].sel) {
	face[fc].v1=vert[p2].v; face[fc].n1=vert[p2].n; face[fc].c1=vert[p2].c;
	face[fc].v2=vert[p5].v; face[fc].n2=vert[p5].n; face[fc].c2=vert[p5].c;
	face[fc].v3=vert[p1].v; face[fc].n3=vert[p1].n; face[fc].c3=vert[p1].c;
	fc++;
      }      
    }

  obj->vertc=vertc;
  obj->facec=fc;

  return 0;
}

/*********

  contour
  2  2   3
   -----
1 |     | 3
  |     |
   -----
  0  0  1

**********/

int grid_contour_lookup[][9]={
  /* 0000 */ {0,0,0,0,0,0,0,0,0},
  /* 0001 */ {0,0,0,0,0,0,0,0,0},
  /* 0010 */ {0,0,0,0,0,0,0,0,0},
  /* 0011 */ {1, 0,1,0,0,0,0,0,0},
  /* 0100 */ {0,0,0,0,0,0,0,0,0},
  /* 0101 */ {1, 0,2,0,0,0,0,0,0},
  /* 0110 */ {1, 1,2,0,0,0,0,0,0},
  /* 0111 */ {3, 0,1, 1,2, 0,2,0,0},
  /* 1000 */ {0,0,0,0,0,0,0,0,0},
  /* 1001 */ {1, 0,3,0,0,0,0,0,0},
  /* 1010 */ {1, 1,3,0,0,0,0,0,0},
  /* 1011 */ {3, 0,1, 0,3, 1,3,0,0},
  /* 1100 */ {1, 2,3,0,0,0,0,0,0},
  /* 1101 */ {3, 0,2, 2,3, 0,3,0,0},
  /* 1110 */ {3, 1,2, 2,3, 1,3,0,0},
  /* 1111 */ {4, 0,1, 1,2, 2,3, 0,3}
};

int gridContour(gridObj *obj, Select *sel)
{
  float level,level_start, level_end, level_step;
  int u,v,step,p;
  float uvw[3],v1[3],v2[3],v3[3],v4[3];
  float z1,z2,z3,z4,r;
  gridField *field;
  gridVert vertp[4];
  gridVert *vert,*ov;
  int vertc,vertm;
  int id,i,j,l1,l2;

  if(obj->vertc>0)
    Cfree(obj->vert);
  if(obj->facec>0)
    Cfree(obj->face);

  level_start=obj->level_start;
  level_end=obj->level_end;
  level_step=obj->level_step;
  step=obj->step;
  field=&obj->node->field;

  vertm=1000;
  vertc=0;
  vert=Crecalloc(NULL,vertm,sizeof(gridVert));

  for(level=level_start; level<=level_end; level+=level_step) {
    for(u=step+1;u<(field->width-step-1);u+=step) {
      for(v=step+1;v<(field->height-step-1);v+=step) {
	uvw[0]=u;
	uvw[1]=v;
	p=uvw[1]*field->width+uvw[0];
	z1=(float)(field->point[p].z);
	gridUVWtoXYZ(field,uvw,v1);
	uvw[0]=u+step;
	uvw[1]=v;
	p=uvw[1]*field->width+uvw[0];
	z2=(float)(field->point[p].z);
	gridUVWtoXYZ(field,uvw,v2);
	uvw[0]=u;
	uvw[1]=v+step;
	p=uvw[1]*field->width+uvw[0];
	z3=(float)(field->point[p].z);
	gridUVWtoXYZ(field,uvw,v3);
	uvw[0]=u+step;
	uvw[1]=v+step;
	p=uvw[1]*field->width+uvw[0];
	z4=(float)(field->point[p].z);
	gridUVWtoXYZ(field,uvw,v4);
	id=0;
	if((z1>=level && z2<=level) || (z1<=level && z2>=level)) {
	  if(z1==z2) {
	    // a) don't touch id, rather add line between two corners ?
	    // b) just add corner 0 to vertices ?
	  } else {
	    r=(level-z1)/(z2-z1);
	    vertp[0].v[0]=r*(v2[0]-v1[0])+v1[0];
	    vertp[0].v[1]=r*(v2[1]-v1[1])+v1[1];
	    vertp[0].v[2]=r*(v2[2]-v1[2])+v1[2];
	    vertp[0].c[0]=1.0;
	    vertp[0].c[1]=1.0;
	    vertp[0].c[2]=1.0;
	    id|=0x1;
	  }
	}
	if((z1>=level && z3<=level) || (z1<=level && z3>=level)) {
	  if(z1==z3) {
	    // a) don't touch id, rather add line between two corners ?
	    // b) just add corner 0 to vertices ?
	  } else {
	    r=(level-z1)/(z3-z1);
	    vertp[1].v[0]=r*(v3[0]-v1[0])+v1[0];
	    vertp[1].v[1]=r*(v3[1]-v1[1])+v1[1];
	    vertp[1].v[2]=r*(v3[2]-v1[2])+v1[2];
	    id|=0x2;
	  }
	} 
	if((z3>=level && z4<=level) || (z3<=level && z4>=level)) {
	  if(z3==z4) {
	    // a) don't touch id, rather add line between two corners ?
	    // b) just add corner 0 to vertices ?
	  } else {
	    r=(level-z3)/(z4-z3);
	    vertp[2].v[0]=r*(v4[0]-v3[0])+v3[0];
	    vertp[2].v[1]=r*(v4[1]-v3[1])+v3[1];
	    vertp[2].v[2]=r*(v4[2]-v3[2])+v3[2];
	    id|=0x4;
	  }
	}
	if((z2>=level && z4<=level) || (z2<=level && z4>=level)) {
	  if(z2==z4) {
	    // a) don't touch id, rather add line between two corners ?
	    // b) just add corner 0 to vertices ?
	  } else {
	    r=(level-z2)/(z4-z2);
	    vertp[3].v[0]=r*(v4[0]-v2[0])+v2[0];
	    vertp[3].v[1]=r*(v4[1]-v2[1])+v2[1];
	    vertp[3].v[2]=r*(v4[2]-v2[2])+v2[2];
	    id|=0x8;
	  }
	}

	if(vertc+4>=vertm) {
	  vert=Crecalloc(vert,vertm+1000,sizeof(gridVert));
	  vertm+=1000;
	}

	if(id>0) {
	  for(j=0;j<grid_contour_lookup[id][0];j++) {
	    l1=grid_contour_lookup[id][j*2+1];
	    l2=grid_contour_lookup[id][j*2+2];
	    vert[vertc].v[0]=vertp[l1].v[0];
	    vert[vertc].v[1]=vertp[l1].v[1];
	    vert[vertc].v[2]=vertp[l1].v[2];
	    vert[vertc].c[0]=1.0;
	    vert[vertc].c[1]=1.0;
	    vert[vertc].c[2]=1.0;
	    vert[vertc].z=level;
	    vertc++;
	    vert[vertc].v[0]=vertp[l2].v[0];
	    vert[vertc].v[1]=vertp[l2].v[1];
	    vert[vertc].v[2]=vertp[l2].v[2];
	    vert[vertc].c[0]=1.0;
	    vert[vertc].c[1]=1.0;
	    vert[vertc].c[2]=1.0;
	    vert[vertc].z=level;
	    vertc++;
	  }
	}
      }      
    }
  }
  
  obj->vertc=vertc;
  obj->vert=vert;
  obj->facec=0;
  obj->face=NULL;

  return 0;
}

int gridObjIsWithin(gridObj *obj, float *p, float d2)
{
  int i;
  float dx,dy,dz;
  float *vp;

  for(i=0;i<obj->vertc;i++) {
    if(obj->vert[i].sel) {
      vp=obj->vert[i].v;
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
  }
  return 0;
}
