#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dino.h"
#include "com.h"
#include "dbm.h"
#include "geom_db.h"
#include "geom_obj.h"
#include "Cmalloc.h"
#include "mat.h"
#include "bspline.h"
#include "cl.h"


int geomObjCommand(struct DBM_GEOM_NODE *node, struct GEOM_OBJ *obj, int wc, char **wl)
{
  int i;
  char message[256];
  char set[2560];
  dbmSet s;

  if(wc<=0) {
    comMessage("missing command\n");
    return -1;
  }

  if(!strcmp(wl[0],"help") ||
     !strcmp(wl[0],"?")) {
    comMessage("possible commands: add del get set list show hide help\n");
  } else if(!strcmp(wl[0],"add")) {
    geomObjAdd(obj,wc-1,wl+1);
  } else if(!strcmp(wl[0],"del")) {
    geomObjDel(obj,wc-1,wl+1);
    geomObjRegen(obj);
  } else if(!strcmp(wl[0],"get")) {
    geomObjGet(obj,wc-1,wl+1);
  } else if(!strcmp(wl[0],"set")) {
    strcpy(set,"");
    for(i=1;i<wc;i++) {
      strcat(set,wl[i]);
      strcat(set," ");
    }
    if(dbmSetExtract((dbmNode *)obj->node,&s,set,DBM_NODE_GEOM)==0)
      geomObjSet(obj,&s,1);

    comRedraw();
  } else if(!strcmp(wl[0],"list")) {
    geomObjList(obj,wc-1,wl+1);
  } else if(!strcmp(wl[0],"hide")) {
    obj->render.show=0;
    comHideObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"show")) {
    obj->render.show=1;
    comShowObj(node->name, obj->name);
    comRedraw();
  } else if(!strcmp(wl[0],"render")) {
    if(renderSet(&obj->render,wc-1,wl+1)!=0) {
      sprintf(message,"%s: syntax error in render statement\n",node->name);
      comMessage(message);
      return -1;
    }
    if(obj->render.mode!=RENDER_OFF && 
       obj->render.mode!=RENDER_ON &&
       obj->render.mode!=RENDER_TUBE) {
      obj->render.mode=RENDER_OFF;
      comMessage("invalid render mode\n");
    }
  } else if(!strcmp(wl[0],"material")) {
    if(wc<2) {
      comMessage(renderGetMaterial(&obj->render.mat));
    } else {

      if(renderMaterialSet(&obj->render.mat,wc-1,wl+1)!=0)
	return -1;
      
      comRedraw();
    }
  } else {
    sprintf(message,"unknown command %s\n",wl[0]);
  }

  geomObjRegen(obj);
  
  comRedraw();

  return 0;
}

int geomObjRegen(geomObj *obj)
{
  int i;
  if(obj->render.mode==RENDER_ON) {
    if(obj->va.count>0)
      Cfree(obj->va.p);
    obj->va.count=0;
    obj->va.max=0;
    obj->va.p=NULL;

    for(i=0;i<obj->point_count;i++) {
      cgfxSphereVA(obj->point[i].r,obj->point[i].v,obj->point[i].c,
		   &obj->va,obj->render.detail1);
    }
    
    for(i=0;i<obj->line_count;i++) {
      if(obj->render.stipple_flag) {
	cgfxGenCylinder(&obj->va,
			obj->line[i].v1,obj->line[i].v2,obj->line[i].r,
			obj->render.stipplei,obj->render.stippleo,
			obj->render.detail1,
			CGFX_CAP,
			obj->line[i].c);
      } else {
	cgfxGenCylinder(&obj->va,
			obj->line[i].v1,obj->line[i].v2,obj->line[i].r,
			0.0,0.0,
			obj->render.detail1,
			CGFX_CAP,
			obj->line[i].c);
      }
    }
  } else if (obj->render.mode==RENDER_TUBE) {
    geomSmooth(obj);
  }
  return 0;
}

geomObj *geomNewObj(struct DBM_GEOM_NODE *node, char *name, int type)
{
  int i;
  geomObj *no;
  
  geomDelObj(node,name);
  for(i=0;i<node->obj_max;i++) {
    if(node->obj[i]==NULL) {
      node->obj[i]=Cmalloc(sizeof(geomObj)); 
      comNewObj(node->name,name);
      no=node->obj[i];
      memset(no,0,sizeof(geomObj));
      strcpy(no->name,name);
      no->node=node;

      no->type=type;
      if(no->type==GEOM_PRIM) {
	no->point_max=500;
	no->point_count=0;
	no->point=Crecalloc(NULL,no->point_max,sizeof(geomPoint));
	
	no->line_max=500;
	no->line_count=0;
	no->line=Crecalloc(NULL,no->line_max,sizeof(geomLine));
	
	no->tri_max=500;
	no->tri_count=0;
	no->tri=Crecalloc(NULL,no->tri_max,sizeof(geomTri));
	
	no->rect_max=500;
	no->rect_count=0;
	no->rect=Crecalloc(NULL,no->rect_max,sizeof(geomRect));

	no->label_max=500;
	no->label_count=0;
	no->label=Crecalloc(NULL,no->label_max,sizeof(geomLabel));
	
	no->va.count=0;
	no->va.max=0;
	no->va.p=NULL;
      } else if (no->type==GEOM_SLAB) {

      }
      no->r=1.0;
      no->g=1.0;
      no->b=1.0;

      no->render.show=1;
      no->render.mode=RENDER_OFF;
      no->render.detail1=3;
      no->render.detail2=3;
      no->render.tube_ratio=1.0;
      no->render.nice=1;
      no->render.stipple_flag=0;
      no->render.stipple_factor=1;
      no->render.stipple_pattern=0x0f0f;
      no->render.stipplei=0.2;
      no->render.stippleo=0.2;
      no->render.line_width=1.0;
      no->render.point_size=1.0;
      no->render.bond_width=1.0;
      no->render.transparency=1.0;
      /* the other render stats are unused */

      return node->obj[i];
    }
  }

  /* catch and increase obj_max */

  return NULL;
}
int geomDelObj(struct DBM_GEOM_NODE *node,char *name)
{
  int i;
  geomObj *obj;
  for(i=0;i<node->obj_max;i++)
    if(node->obj[i]!=NULL)
      if(!strcmp(node->obj[i]->name,name)) {
	obj=node->obj[i];

	Cfree(obj->point);
	Cfree(obj->line);
	Cfree(obj->tri);
	Cfree(obj->rect);

	node->obj[i]=NULL;
	comDelObj(node->name, name);
      }

  return 0;
}

int geomObjGet(geomObj *obj, int wc, char **wl)
{
  char mesg[256];
  float v[3];
  int i,n;

  if(wc<1) {
    comMessage("missing parameter for get\n");
    return -1;
  }
  if(clStrcmp(wl[0],"center")) {
    v[0]=0.0; v[1]=0.0; v[2]=0.0;
    n=0;
    for(i=0;i<obj->point_count;i++) {
      v[0]+=obj->point[i].v[0];
      v[1]+=obj->point[i].v[1];
      v[2]+=obj->point[i].v[2];
      n++;
    }
    for(i=0;i<obj->line_count;i++) {
      v[0]+=obj->line[i].v1[0];
      v[1]+=obj->line[i].v1[1];
      v[2]+=obj->line[i].v1[2];
      v[0]+=obj->line[i].v2[0];
      v[1]+=obj->line[i].v2[1];
      v[2]+=obj->line[i].v2[2];
      n+=2;
    }
    if(n>0) {
      v[0]/=(float)n;
      v[1]/=(float)n;
      v[2]/=(float)n;
      sprintf(mesg,"{%.5f,%.5f,%.5f}",v[0],v[1],v[2]);
      comReturn(mesg);
    } else {
      comReturn("{0,0,0}");
    }
  } else {
    sprintf(mesg,"error in get: unknown property %s\n",wl[0]);
    comMessage(mesg);
  }
  return 0;
}

/******************************

   add element to object

   the elements are
        point
	line
	triangle
	rectangle
	label

  with the properties
        p (position) 
	c (color)
	r (radius)
	t (transparency)

*******************************/

int geomObjAdd(geomObj *obj,int wc, char **wl)
{
  int i,k,ele;
  char message[256];
  char param[2048];
  int pc,d1,d2,pflag;
  char **pl;
  char prop[256],op[256],val[2048];
  float p[128],r,c[3],t;
  geomPoint *npoint,*opoint;
  geomLine *nline,*oline;
  geomTri *ntri,*otri;
  geomRect *nrect,*orect;
  geomLabel *nlabel;
  double vcyl[]={0.0,0.0,1.0};
  double axis[3],vdiff[3],length,angle,dotproduct;
  double mat[16];
  char lab[128];

  if(wc<=0) {
    comMessage("no element given\n");
    return -1;
  }

  strcpy(lab,"");

  if(!strcmp(wl[0],"point"))
    ele=GEOM_ELE_POINT;
  else if(!strcmp(wl[0],"line"))
    ele=GEOM_ELE_LINE;
  else if(!strcmp(wl[0],"triangle") ||
	  !strcmp(wl[0],"tri"))
    ele=GEOM_ELE_TRI;
  else if(!strcmp(wl[0],"rectangle") ||
	  !strcmp(wl[0],"rect"))
    ele=GEOM_ELE_RECT;
  else if(!strcmp(wl[0],"label"))
    ele=GEOM_ELE_LABEL;
  else {
    sprintf(message,"unknown element %s\n",wl[0]);
    comMessage(message);
    return -1;
  }
  
  strcpy(param,"");
  for(i=1;i<wc;i++)
    strcat(param,wl[i]);

  dbmSplit(param,',',&pc,&pl);

  /* default values */
  c[0]=obj->r;
  c[1]=obj->g;
  c[2]=obj->b;
  r=1.0;
  t=1.0;

  pflag=0;
  for(i=0;i<pc;i++) {
    dbmSplitPOV(pl[i],prop,op,val);

    if(strcmp(op,"=")) {
      sprintf(message,"invalid operator %s\n",op);
      comMessage(message);
      return -1;
    }

    if(!strcmp(prop,"p")) {
      pflag=1;
      if(matExtractMatrix(val,&d1,&d2,mat)<0) {
	sprintf(message,"error in coordinates: %s (%d %d)\n",val,d1,d2);
	comMessage(message);
	return -1;
      }
      if(ele==GEOM_ELE_POINT || ele==GEOM_ELE_LABEL) {
	if(d2!=1) {
	  comMessage("Point requires one coordinate\n");
	  return -1;
	}
	for(k=0;k<3;k++) p[k]=(float)mat[k];
      } else if(ele==GEOM_ELE_LINE) {
	if(d2!=2) {
	  comMessage("Line requires two coordinates\n");
	  return -1;
	}
	for(k=0;k<6;k++) p[k]=(float)mat[k];
      } else if(ele==GEOM_ELE_TRI) {
	if(d2!=3) {
	  comMessage("Triangle requires three coordinates\n");
	  return -1;
	}
	for(k=0;k<9;k++) p[k]=(float)mat[k];
      } else if(ele==GEOM_ELE_RECT) {
	if(d2!=4) {
	  comMessage("Rectangle requires four coordinates\n");
	  return -1;
	}
	for(k=0;k<12;k++) p[k]=(float)mat[k];
      }
    } else if(!strcmp(prop,"c")) {
      if((comGetColor(val,c+0,c+1,c+2)!=0)) {
	sprintf(message,"unknown color %s\n",val);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"r")) {
      r=atof(val);
    } else if(!strcmp(prop,"t")) {
      t=atof(val);
    } else if(!strcmp(prop,"s")) {
      strncpy(lab,val,127);
      lab[127]='\0';
    } else {
      sprintf(message,"unknown property %s\n",prop);
      comMessage(message);
      return -1;
    }
  }

  if(!pflag) {
    comMessage("coordinates missing\n");
    return -1;
  }

  /* add element to current object */
  switch(ele) {
  case GEOM_ELE_POINT:
    npoint=&obj->point[obj->point_count++];

    if(obj->point_count>=obj->point_max) {
      obj->point_max+=500;
      obj->point=Crecalloc(obj->point,obj->point_max,sizeof(geomPoint));
      npoint=&obj->point[obj->point_count];
    }
    npoint->ps=1.0;
    npoint->c[0]=c[0];
    npoint->c[1]=c[1];
    npoint->c[2]=c[2];
    npoint->c[3]=t;
    for(i=0;i<3;i++)
      npoint->v[i]=p[i];
    npoint->r=r;
    break;
  case GEOM_ELE_LINE:
    nline=&obj->line[obj->line_count++];

    if(obj->line_count>=obj->line_max) {
      obj->line_max+=500;
      obj->line=Crecalloc(obj->line,obj->line_max,sizeof(geomLine));
      nline=&obj->line[obj->line_count];
    }
    nline->lw=1.0;
    nline->c[0]=c[0];
    nline->c[1]=c[1];
    nline->c[2]=c[2];
    nline->c[3]=t;
    
    for(i=0;i<3;i++) {
      nline->v1[i]=p[0*3+i];
      nline->v2[i]=p[1*3+i];
    }

    vdiff[0]=(double)(nline->v2[0]-nline->v1[0]);
    vdiff[1]=(double)(nline->v2[1]-nline->v1[1]);
    vdiff[2]=(double)(nline->v2[2]-nline->v1[2]);
    
    length=sqrt(vdiff[0]*vdiff[0]+vdiff[1]*vdiff[1]+vdiff[2]*vdiff[2]);
    nline->length=length;
    
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
    
    matMakeRotMat(angle,axis[0],axis[1],axis[2],mat);
    for(i=0;i<16;i++)
      nline->rmat[i]=(float)mat[i];

    nline->r=r;


    break;
  case GEOM_ELE_TRI:
    ntri=&obj->tri[obj->tri_count++];

    if(obj->tri_count>=obj->tri_max) {
      obj->tri_max+=500;
      obj->tri=Crecalloc(obj->tri,obj->tri_max,sizeof(geomTri));
      ntri=&obj->tri[obj->tri_count];
    }
    ntri->lw=1.0;
    ntri->fill=0;
    ntri->c[0]=c[0];
    ntri->c[1]=c[1];
    ntri->c[2]=c[2];
    ntri->c[3]=t;
    
    for(i=0;i<3;i++) {
      ntri->v1[i]=p[0*3+i];
      ntri->v2[i]=p[1*3+i];
      ntri->v3[i]=p[2*3+i];
    }
    break;
  case GEOM_ELE_RECT:
    nrect=&obj->rect[obj->rect_count++];

    if(obj->rect_count>=obj->rect_max) {
      obj->rect_max+=500;
      obj->rect=Crecalloc(obj->rect,obj->rect_max,sizeof(geomRect));
      nrect=&obj->rect[obj->rect_count];
    }
    nrect->lw=1.0;
    nrect->fill=0;
    nrect->c[0]=c[0];
    nrect->c[1]=c[1];
    nrect->c[2]=c[2];
    nrect->c[3]=t;
    
    for(i=0;i<3;i++) {
      nrect->v1[i]=p[0*3+i];
      nrect->v2[i]=p[1*3+i];
      nrect->v3[i]=p[2*3+i];
      nrect->v4[i]=p[3*3+i];
    }
    break;
  case GEOM_ELE_LABEL:
    if(strlen(lab)==0) {
      comMessage("label requires more than emtpy string\n");
      return -1;
    }
    nlabel=&obj->label[obj->label_count++];
    strcpy(nlabel->s,lab);
    nlabel->p[0]=p[0];
    nlabel->p[1]=p[1];
    nlabel->p[2]=p[2];
    nlabel->c[0]=c[0];
    nlabel->c[1]=c[1];
    nlabel->c[2]=c[2];
    break;
  }

  return 0;
}

/****************************************
   delete elements

   .geom.geom delete w1 w2 w3 w4 w5...

   where each expression is the name of
   an element

******************************************/

int geomObjDel(geomObj *obj,int wc, char **wl)
{
  geomNameList list[128];
  int i,j,k,l,m,n,c;

  for(i=0;i<wc;i++) {
    c=geomGenNameList(list,wl[i]);
    if(c==-1)
      return -1;

    for(j=0;j<c;j++) {
      if(list[j].t==GEOM_TYPE_POINT ||
	 list[j].t==GEOM_TYPE_ALL) {
	if(list[j].n1==-1) {
	  /* delete all */
	  obj->point_count=0;
	} else if(list[j].n2>list[j].n1) {
	  /* delete range */
	  if(list[j].n1<obj->point_count) {
	    m=list[j].n1;
	    if(list[j].n2>=obj->point_count)
	      n=obj->point_count-1;
	    else
	      n=list[j].n2;
	    for(k=m,l=n+1;l<obj->point_count;k++,l++)
	      memcpy(&obj->point[k],&obj->point[l],sizeof(geomPoint));

	    obj->point_count-=(n-m+1);
	  }
	} else {
	  /* delete a single entry */
	  for(l=list[j].n1+1;l<obj->point_count;l++)
	    memcpy(&obj->point[l-1],&obj->point[l],sizeof(geomPoint));
	  obj->point_count--;
	}
      }
      if(list[j].t==GEOM_TYPE_LINE ||
	 list[j].t==GEOM_TYPE_ALL) {
	if(list[j].n1==-1) {
	  /* delete all */
	  obj->line_count=0;
	} else if(list[j].n2>list[j].n1) {
	  /* delete range */
	  if(list[j].n1<obj->line_count) {
	    m=list[j].n1;
	    if(list[j].n2>=obj->line_count)
	      n=obj->line_count-1;
	    else
	      n=list[j].n2;
	    for(k=m,l=n+1;l<obj->line_count;k++,l++)
	      memcpy(&obj->line[k],&obj->line[l],sizeof(geomLine));

	    obj->line_count-=(n-m+1);
	  }
	} else {
	  /* delete a single entry */
	  for(l=list[j].n1+1;l<obj->line_count;l++)
	    memcpy(&obj->line[l-1],&obj->line[l],sizeof(geomLine));
	  obj->line_count--;
	}
      }
      if(list[j].t==GEOM_TYPE_TRI ||
	 list[j].t==GEOM_TYPE_ALL) {
	if(list[j].n1==-1) {
	  /* delete all */
	  obj->tri_count=0;
	} else if(list[j].n2>list[j].n1) {
	  /* delete range */
	  if(list[j].n1<obj->tri_count) {
	    m=list[j].n1;
	    if(list[j].n2>=obj->tri_count)
	      n=obj->tri_count-1;
	    else
	      n=list[j].n2;
	    for(k=m,l=n+1;l<obj->tri_count;k++,l++)
	      memcpy(&obj->tri[k],&obj->tri[l],sizeof(geomTri));

	    obj->tri_count-=(n-m+1);
	  }
	} else {
	  /* delete a single entry */
	  for(l=list[j].n1+1;l<obj->tri_count;l++)
	    memcpy(&obj->tri[l-1],&obj->tri[l],sizeof(geomTri));
	  obj->tri_count--;
	}
      }
      if(list[j].t==GEOM_TYPE_RECT ||
	 list[j].t==GEOM_TYPE_ALL) {
	if(list[j].n1==-1) {
	  /* delete all */
	  obj->rect_count=0;
	} else if(list[j].n2>list[j].n1) {
	  /* delete range */
	  if(list[j].n1<obj->rect_count) {
	    m=list[j].n1;
	    if(list[j].n2>=obj->rect_count)
	      n=obj->rect_count-1;
	    else
	      n=list[j].n2;
	    for(k=m,l=n+1;l<obj->rect_count;k++,l++)
	      memcpy(&obj->rect[k],&obj->rect[l],sizeof(geomRect));

	    obj->rect_count-=(n-m+1);
	  }
	} else {
	  /* delete a single entry */
	  for(l=list[j].n1+1;l<obj->rect_count;l++)
	    memcpy(&obj->rect[l-1],&obj->rect[l],sizeof(geomRect));
	  obj->rect_count--;
	}
      }
    }

  }  

  return 0;
}

int geomObjList(geomObj *obj, int wc, char **wl)
{
  int i,f;
  char message[256];

  for(i=0;i<obj->point_count;i++) {
    sprintf(message,"p%d: p={%g,%g,%g}\n",
	    i,obj->point[i].v[0],obj->point[i].v[1],obj->point[i].v[2]);
    comMessage(message);
    f++;
  }
  for(i=0;i<obj->line_count;i++) {
    sprintf(message,"l%d: p={{%g,%g,%g},{%g,%g,%g}}\n",
	    i,
	    obj->line[i].v1[0],obj->line[i].v1[1],obj->line[i].v1[2],
	    obj->line[i].v2[0],obj->line[i].v2[1],obj->line[i].v2[2]);
    comMessage(message);
    f++;
  }
  for(i=0;i<obj->tri_count;i++) {
    sprintf(message,"tri #%d: p={{%g,%g,%g},{%g,%g,%g},{%g,%g,%g}}\n",
	    i,
	    obj->tri[i].v1[0],obj->tri[i].v1[1],obj->tri[i].v1[2],
	    obj->tri[i].v2[0],obj->tri[i].v2[1],obj->tri[i].v2[2],
	    obj->tri[i].v3[0],obj->tri[i].v3[1],obj->tri[i].v3[2]);
    comMessage(message);
    f++;
  }
  for(i=0;i<obj->rect_count;i++) {
    sprintf(message,"rect #%d: p={{%g,%g,%g},{%g,%g,%g},{%g,%g,%g},{%g,%g,%g}}\n",
	    i,
	    obj->rect[i].v1[0],obj->rect[i].v1[1],obj->rect[i].v1[2],
	    obj->rect[i].v2[0],obj->rect[i].v2[1],obj->rect[i].v2[2],
	    obj->rect[i].v3[0],obj->rect[i].v3[1],obj->rect[i].v3[2],
	    obj->rect[i].v4[0],obj->rect[i].v4[1],obj->rect[i].v4[2]);
    comMessage(message);
    f++;
  }
  if(!f) 
    comMessage("no elements\n");

  return 0;
}

int geomObjSet(geomObj *obj, struct DBM_SET *s, int flag)
{
  int i,j,k;
  geomPoint *point;
  geomLine *line;
  geomTri *tri;
  geomRect *rect;
  char message[256];
  char **sl;
  int sc;
  char sel[2048];
  char* selp[2048];

  /* no precreation properties */
  if(flag==0)
    return 0;

  if(s->range_flag) {
    sprintf(message,"ranges not supported for geometric objects\n");
    comMessage(message);
    return -1;
  }

  strcpy(sel,s->sel_string);

  dbmSplit(sel,' ',&sc,&sl);

  if(clStrcmp(sel,"*")) {
    for(j=0;j<s->ec;j++) {
      switch(s->e[j].id) {
      case GEOM_COLOR:
	obj->r=s->e[j].value.v[0][0];
	obj->g=s->e[j].value.v[0][1];
	obj->b=s->e[j].value.v[0][2];
	break;
      }
    }
  }

  for(k=0;k<sc;k++)
    selp[k]=sl[k];
  
  for(k=0;k<sc;k++) {
      for(i=0;i<obj->point_count;i++) {
	
	if(geomEleMatch(selp[k],GEOM_TYPE_POINT,i)) {
	  point=&obj->point[i];
	  
	  for(j=0;j<s->ec;j++) {
	    switch(s->e[j].id) {
	    case GEOM_COLOR:
	      point->c[0]=s->e[j].value.v[0][0];
	      point->c[1]=s->e[j].value.v[0][1];
	      point->c[2]=s->e[j].value.v[0][2];
	      break;
	    case GEOM_TRANSPARENCY:
	      point->c[3]=s->e[j].value.f[0];
	      break;
	    case GEOM_RADIUS:
	      point->r=s->e[j].value.f[0];
	      break;
	    }
	  }
	}
      }
      
      for(i=0;i<obj->line_count;i++) {
	if(geomEleMatch(selp[k],GEOM_TYPE_LINE,i)) {
	  line=&obj->line[i];
	  
	  for(j=0;j<s->ec;j++) {
	    switch(s->e[j].id) {
	    case GEOM_COLOR:
	      line->c[0]=s->e[j].value.v[0][0];
	      line->c[1]=s->e[j].value.v[0][1];
	      line->c[2]=s->e[j].value.v[0][2];
	      break;
	    case GEOM_TRANSPARENCY:
	      line->c[3]=s->e[j].value.f[0];
	      break;
	    case GEOM_RADIUS:
	      line->r=s->e[j].value.f[0];
	      break;
	    }
	  }
	}
      }
      for(i=0;i<obj->line_count;i++) {
	if(geomEleMatch(selp[k],GEOM_TYPE_TRI,i)) {
	  tri=&obj->tri[i];
	  
	  for(j=0;j<s->ec;j++) {
	    switch(s->e[j].id) {
	    case GEOM_COLOR:
	      tri->c[0]=s->e[j].value.v[0][0];
	      tri->c[1]=s->e[j].value.v[0][1];
	      tri->c[2]=s->e[j].value.v[0][2];
	      break;
	    case GEOM_TRANSPARENCY:
	      tri->c[3]=s->e[j].value.f[0];
	      break;
	    case GEOM_FILL:
	      tri->fill=s->e[j].value.i[0];
	      break;
	    }
	  }
	}
      }
      for(i=0;i<obj->rect_count;i++) {
	if(geomEleMatch(selp[k],GEOM_TYPE_RECT,i)) {
	  rect=&obj->rect[i];
	  
	  for(j=0;j<s->ec;j++) {
	    switch(s->e[j].id) {
	    case GEOM_COLOR:
	      rect->c[0]=s->e[j].value.v[0][0];
	      rect->c[1]=s->e[j].value.v[0][1];
	      rect->c[2]=s->e[j].value.v[0][2];
	      break;
	    case GEOM_TRANSPARENCY:
	      rect->c[3]=s->e[j].value.f[0];
	      break;
	    case GEOM_FILL:
	      rect->fill=s->e[j].value.i[0];
	      break;
	    }
	  }
	}
      }
  }
  return 0;
}

int geomEleMatch(char *oexpr,int type, int n)
{
  int i,c;
  geomNameList list[1024];
  char expr[2048];

  strcpy(expr,oexpr);
	   
  c=geomGenNameList(list, expr);
  if(c==-1)
    return 0;

  for(i=0;i<c;i++) {
    if(list[i].t==type || 
       list[i].t==GEOM_TYPE_ALL) {
      if(list[i].n1==-1) {
	return 1;
      } else {
	if(n>=list[i].n1 && n<=list[i].n2)
	  return 1;
      }    
    } 
  }
  
  return 0;
}

int geomSmooth(geomObj *obj)
{
  cgfxSplinePoint *point_list;
  cgfxPoint *spoint_list;
  cgfxVA va;
  int i;
  int detail;
  float n1[4],n2[4],n3[4],n4[4];

  n4[0]=1.0; n4[1]=0.0; n4[2]=0.0;

  if(obj->va.count>0)
    Cfree(obj->va.p);
  obj->va.count=0;
  obj->va.max=0;
  obj->va.p=NULL;

  if(obj->point_count==0)
    return -1;

  point_list=Ccalloc(obj->point_count,sizeof(cgfxSplinePoint));

  detail=obj->render.detail1;
  if(detail<1)
    detail=1;

  for(i=0;i<obj->point_count;i++) {
    point_list[i].v[0]=obj->point[i].v[0];
    point_list[i].v[1]=obj->point[i].v[1];
    point_list[i].v[2]=obj->point[i].v[2];
    point_list[i].colp[0]=obj->point[i].c;
    point_list[i].colp[1]=obj->point[i].c;
    point_list[i].colp[2]=obj->point[i].c;
    obj->point[i].c[3]=obj->render.transparency;
    point_list[i].rad=obj->render.bond_width;
    point_list[i].id=CGFX_COIL;
  }

  for(i=0;i<obj->point_count;i++) {
    if(i==0) {
      n1[0]=point_list[0].v[0]-point_list[1].v[0];
      n1[1]=point_list[0].v[1]-point_list[1].v[1];
      n1[2]=point_list[0].v[2]-point_list[1].v[2];
      n2[0]=point_list[2].v[0]-point_list[1].v[0];
      n2[1]=point_list[2].v[1]-point_list[1].v[1];
      n2[2]=point_list[2].v[2]-point_list[1].v[2];
    } else if(i==obj->point_count-1) {
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
    fprintf(stderr," %.2f %.2f %.2f\n %.2f %.2f %.2f\n %.2f %.2f %.2f\n\n",
	    point_list[i].v[0],point_list[i].v[1],point_list[i].v[2],
	    point_list[i].d[0],point_list[i].d[1],point_list[i].d[2],
	    point_list[i].n[0],point_list[i].n[1],point_list[i].n[2]);
    */
    // take as new reference
    n4[0]=n3[0];
    n4[1]=n3[1];
    n4[2]=n3[2];
    matfNormalize(n4,n4);
  }
  
  bsplineGenerate(point_list, &spoint_list, obj->point_count,
		  detail, CGFX_INTPOL_COL);

  cgfxGenHSC(&va, point_list, obj->point_count, &obj->render);
  cgfxAppend(&obj->va,&va);
  Cfree(va.p);
  /*
  for(i=0;i<obj->va.count;i++) {
    fprintf(stderr,"%.2f %.2f %.2f  %.2f %.2f %.2f  %.2f %.2f %.2f\n",
	    obj->va.p[i].v[0],obj->va.p[i].v[1],obj->va.p[i].v[2],
	    obj->va.p[i].n[0],obj->va.p[i].n[1],obj->va.p[i].n[2],
	    obj->va.p[i].c[0],obj->va.p[i].c[1],obj->va.p[i].c[2]);
  }
  */
  return 0;
}
