#include <string.h>

#include <GL/gl.h>

#ifdef LINUX
#include <tiff.h>
#include <tiffio.h>
#endif
#ifdef SGI
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif
#ifdef DEC
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#endif


#include "Cmalloc.h"
#include "com.h"
#include "mat.h"

#include "grid.h"

char grid_return[256];

int gridNewNode(dbmGridNode *node)
{
  int i;
  /* initialize the new node with default values */
  node->obj_count=0;
  node->obj_max=64;
  node->obj=Ccalloc(node->obj_max,sizeof(gridObj *));

  for(i=0;i<node->obj_max;i++)
    node->obj[i]=NULL;

  return 0;
}

int gridReadNode(int fd, char *fn, dbmGridNode *node)
{
  TIFF *t;
  uint32 w,h;
  uint16 b;
  uint32 *data;
  int i,j,p1,p2;
  char message[256];
  t=TIFFFdOpen(fd,fn,"r");
  if(t==NULL) {
    sprintf(message,"\nreadGrid: error opening file");
    comMessage(message);
    return -1;
  }
  TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&w);
  TIFFGetField(t,TIFFTAG_IMAGEWIDTH,&h);
  TIFFGetField(t,TIFFTAG_BITSPERSAMPLE,&b);

  if((w*h)>(4096*4096)) {
    sprintf(message,"\nsize exceeded or invalid file format");
    comMessage(message);
    return -1;
  }

  if((data=Ccalloc(w*h,sizeof(uint32)))==NULL) {
    sprintf(message,"\nMemory allocation error #1");
    comMessage(message);
    return -1;
  }
  TIFFReadRGBAImage(t,w,h,data,-1);   

  /* phantom points on the edge */
  node->field.width=w+2; 
  node->field.height=h+2;
  node->field.scale_x=1.0;
  node->field.scale_y=1.0;
  node->field.scale_z=1.0;
  node->field.offset_x=(float)w/2.0;
  node->field.offset_y=(float)h/2.0;
  node->field.offset_z=0;

  node->field.data=Ccalloc((w+2)*(h+2),sizeof(unsigned char));

  if(node->field.data==NULL) {
    sprintf(message,"\nMemory allocation error #2");
    comMessage(message);
    Cfree(data);
    TIFFClose(t);
    return -1;
  }
  /*
    transfer red value to field data
  */
  for(j=0;j<h;j++) {
    for(i=0;i<w;i++) {
      p1=j*w+i;
      p2=(j+1)*(w+2)+(i+1);
      node->field.data[p2]=TIFFGetR(data[p1]);
    }
  }

  /* fill phantom edges */
  for(i=0;i<w;i++) {
    p1=(0)*(w+2)+i+1;
    p2=(1)*(w+2)+i+1;
    node->field.data[p2]=node->field.data[p1];
    p1=(h+0)*(w+2)+i+1;
    p2=(h+1)*(w+2)+i+1;
    node->field.data[p2]=node->field.data[p1];
  }
  for(j=0;j<h;j++) {
    p1=(j+1)*(w+2)+1;
    p2=(j+1)*(w+2)+0;
    node->field.data[p2]=node->field.data[p1];
    p1=(j+1)*(w+2)+w+0;
    p2=(j+1)*(w+2)+w+1;
    node->field.data[p2]=node->field.data[p1];
  }

  Cfree(data);
  
  TIFFClose(t);

  return 0;
}

int gridCommand(dbmGridNode *node, int wc, char **wl)
{
  char message[256];
  char *empty_com[]={"get","center"};

  if(wc<=0) {
    wc=2;
    wl[0]=empty_com[0];
    wl[1]=empty_com[1];
  }
  if(!strcmp(wl[0],"new")) {
    if(gridNew(node,wc-1,wl+1)!=0)
      return -1;
    sprintf(message,".%s.%s",node->name,wl[1]);
    strcpy(grid_return,message);
    wl[0]=grid_return;
    comRedraw();
  } else if(!strcmp(wl[0],"get")) {
    if(wc==1) {
      sprintf(message,"%s: missing parameter for get",node->name);
      comMessage(message);
     return -1;
    }
    if(!strcmp(wl[1],"center")) {
      sprintf(grid_return,"{%.3f,%.3f,%.3f}",
	      node->field.offset_x*node->field.scale_x,
	      node->field.offset_y*node->field.scale_y,
	      node->field.offset_z*node->field.scale_z);
      wl[0]=grid_return;
    } else {
      sprintf(message,"\n%s: unknown property %s",node->name,wl[1]);
      comMessage(message);
      return -1;
    }
  } else if(!strcmp(wl[0],"set")) {
    if(wc==1) {
      sprintf(message,"%s: missing parameter for get",node->name);
      comMessage(message);
      return -1;
    }
    gridSet(node,wc-1,wl+1);
  } else {
    sprintf(message,"\n%s:Unknown command %s",node->name,wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}

int gridNew(dbmGridNode *node, int wc, char **wl)
{
  char type[256];
  char name[256];
  char selection[2560];
  char set[2560];
  char message[256];
  int c;
  gridObj *obj;

  strcpy(name,node->name);
  strcpy(type,"default");
  strcpy(selection,"");
  strcpy(set,"");

  c=0;
  while(c<wc) {
    if(!strcmp(wl[c],"-name")) {
      if(c+1>=wc) {
	sprintf(message,"\n%s: missing name",node->name);
	comMessage(message);
	return -1;
      }
      c++;
      strcpy(name,wl[c]);
    } else {
      sprintf(message,"\n%s: unknown flag %s",node->name,wl[c]);
      comMessage(message);
      return -1;
    }
  }

  if((obj=gridNewObj(node,name))==NULL) {
    sprintf(message,"\n%s: memory allocation error in NewObj()",node->name);
    comMessage(message);
    return -1;
  }

  obj->type=0;

  gridObjRenew(obj,set,selection);

  return 0;
}

int gridObjRenew(gridObj *obj, char *set, char *sel)
{
  float v1[3],n1[3],n2[3],n3[3],n4[3],n5[3];
  float c1[3];
  float pn[4][3];
  float z;
  float f=10.0/256.0;
  unsigned char *data;
  int i,j,k,tc;
  int p[6];
  float x,y;
  struct GRID_VERT *vert;
  int vertc,vp;
  int w,h;
  float scalx,scaly,scalz;

  data=obj->field->data;
  w=obj->field->width;
  h=obj->field->height;

  scalx=obj->field->scale_x;
  scaly=obj->field->scale_z;
  scalz=obj->field->scale_z;

  vertc=w*h;
  vert=Ccalloc(vertc,sizeof(struct GRID_VERT));

  /*
    step 1: calculate the vertices
    including the phantom edges
  */

  for(j=0;j<h;j++) {
    for(i=0;i<w;i++) {
      vp=j*w+i;
      x=(float)i;
      y=(float)j;
      z=scalz*f*(float)data[vp];
      vert[vp].v[0]=x;
      vert[vp].v[1]=y;
      vert[vp].v[2]=z;
      vert[vp].c[0]=0.0;
      vert[vp].c[1]=1.0/256.0*(float)data[vp];
      vert[vp].c[2]=0.0;
    }
  }
  
  /* 
     step 2: calculate the normals
     with help of the phantom edges
  */
  n5[0]=0.0;
  n5[1]=0.0;
  n5[2]=1.0;
  for(j=1;j<(h-2);j++) {
    for(i=1;i<(w-2);i++) {
      p[0]=(j+0)*w+(i+0);
      p[1]=(j+0)*w+(i-1);
      p[2]=(j-1)*w+(i+0);
      p[3]=(j+0)*w+(i+1);
      p[4]=(j+1)*w+(i+0);
      p[5]=p[1];

      n1[0]=0.0;
      n1[1]=0.0;
      n1[2]=1.0;
      for(k=1;k<5;k++) {
	n2[0]=vert[p[k]].v[0]-vert[p[0]].v[0];
	n2[1]=vert[p[k]].v[1]-vert[p[0]].v[1];
	n2[2]=vert[p[k]].v[2]-vert[p[0]].v[2];
	n3[0]=vert[p[k+1]].v[0]-vert[p[0]].v[0];
	n3[1]=vert[p[k+1]].v[1]-vert[p[0]].v[1];
	n3[2]=vert[p[k+1]].v[2]-vert[p[0]].v[2];
	matfCalcCross(n2,n3,n4);
	matfNormalize(n4,n4);
	/*
	if(matfCalcDot(n4,n5)<0.0) {
	  fprintf(stderr,"x");
	  matfCalcCross(n3,n2,n4);
	}
	*/
	n1[0]+=n4[0];
	n1[1]+=n4[1];
	n1[2]+=n4[2];
	matfCopyVV(n4,pn[k-1]);
      }
      matfNormalize(n1,vert[p[0]].n);
    }
  }

  /*
    step 3: generate the tri's
    with the middle points
  */
  obj->tri_count=(h-2)*(w-2)*4;

  if(obj->tri!=NULL)
    Cfree(obj->tri);

  obj->tri=Ccalloc(obj->tri_count,sizeof(struct GRID_TRI));

  tc=0;

  for(j=2;j<(h-3);j++) {
    for(i=2;i<(w-3);i++) {
      p[0]=(j+0)*w+(i+0);
      p[1]=(j+0)*w+(i+1);
      p[2]=(j+1)*w+(i+0);
      p[3]=(j+1)*w+(i+1);
      p[4]=p[0];

      /* construct middle point */
      v1[0]=0.0;
      v1[1]=0.0;
      v1[2]=0.0;
      n1[0]=0.0;
      n1[1]=0.0;
      n1[2]=0.0;
      c1[0]=0.0;
      c1[1]=0.0;
      c1[2]=0.0;
      for(k=0;k<4;k++) {
	v1[0]+=vert[p[k]].v[0];
	v1[1]+=vert[p[k]].v[1];
	v1[2]+=vert[p[k]].v[2];
	n1[0]+=vert[p[k]].n[0];
	n1[1]+=vert[p[k]].n[1];
	n1[2]+=vert[p[k]].n[2];
	c1[0]+=vert[p[k]].c[0];
	c1[1]+=vert[p[k]].c[1];
	c1[2]+=vert[p[k]].c[2];
      }
      v1[0]/=4.0;
      v1[1]/=4.0;
      v1[2]/=4.0;
      n1[0]/=4.0;
      n1[1]/=4.0;
      n1[2]/=4.0; 
      matfNormalize(n1,n1);
      c1[0]/=4.0;
      c1[1]/=4.0;
      c1[2]/=4.0;

      /* middle point is now v1 and n1 and c1 */

      /* tri 0,1,4 */
      matfCopyVV(vert[p[0]].v,obj->tri[tc].v1);
      matfCopyVV(vert[p[0]].n,obj->tri[tc].n1);
      matfCopyVV(vert[p[0]].c,obj->tri[tc].c1);
      matfCopyVV(vert[p[1]].v,obj->tri[tc].v2);
      matfCopyVV(vert[p[1]].n,obj->tri[tc].n2);
      matfCopyVV(vert[p[1]].c,obj->tri[tc].c2);
      matfCopyVV(v1,obj->tri[tc].v3);
      matfCopyVV(n1,obj->tri[tc].n3);
      matfCopyVV(c1,obj->tri[tc].c3);
      tc++;
      /* tri 1,3,4 */
      matfCopyVV(vert[p[1]].v,obj->tri[tc].v1);
      matfCopyVV(vert[p[1]].n,obj->tri[tc].n1);
      matfCopyVV(vert[p[1]].c,obj->tri[tc].c1);
      matfCopyVV(vert[p[3]].v,obj->tri[tc].v2);
      matfCopyVV(vert[p[3]].n,obj->tri[tc].n2);
      matfCopyVV(vert[p[3]].c,obj->tri[tc].c2);
      matfCopyVV(v1,obj->tri[tc].v3);
      matfCopyVV(n1,obj->tri[tc].n3);
      matfCopyVV(c1,obj->tri[tc].c3);
      tc++;
      /* tri 3,2,4 */
      matfCopyVV(vert[p[3]].v,obj->tri[tc].v1);
      matfCopyVV(vert[p[3]].n,obj->tri[tc].n1);
      matfCopyVV(vert[p[3]].c,obj->tri[tc].c1);
      matfCopyVV(vert[p[2]].v,obj->tri[tc].v2);
      matfCopyVV(vert[p[2]].n,obj->tri[tc].n2);
      matfCopyVV(vert[p[2]].c,obj->tri[tc].c2);
      matfCopyVV(v1,obj->tri[tc].v3);
      matfCopyVV(n1,obj->tri[tc].n3);
      matfCopyVV(c1,obj->tri[tc].c3);
      tc++;
      /* tri 2,0,4 */
      matfCopyVV(vert[p[2]].v,obj->tri[tc].v1);
      matfCopyVV(vert[p[2]].n,obj->tri[tc].n1);
      matfCopyVV(vert[p[2]].c,obj->tri[tc].c1);
      matfCopyVV(vert[p[0]].v,obj->tri[tc].v2);
      matfCopyVV(vert[p[0]].n,obj->tri[tc].n2);
      matfCopyVV(vert[p[0]].c,obj->tri[tc].c2);
      matfCopyVV(v1,obj->tri[tc].v3);
      matfCopyVV(n1,obj->tri[tc].n3);
      matfCopyVV(c1,obj->tri[tc].c3);

      tc++;
    }
  }

  Cfree(vert);

  obj->tri_count=tc;
  return 0;
}


gridObj *gridNewObj(dbmGridNode *node, char *name)
{
  int i;
  gridObj *obj;

  gridDelObj(node,name);
  for(i=0;i<node->obj_max;i++)
    if(node->obj[i]==NULL) {
      node->obj[i]=Cmalloc(sizeof(gridObj));
      obj=node->obj[i];
      memset(obj,0,sizeof(gridObj));
      strcpy(obj->name,name);
      gridSetDefault(obj);
      obj->field=&node->field;
      obj->node=node;
      return obj;
    }
  /* catch and increase OBJ_MAX */

  return obj;
}

int gridDelObj(dbmGridNode *node, char *name)
{
  int i;
  
  for(i=0;i<node->obj_max;i++)
    if(node->obj[i]!=NULL) {
      if(!strcmp(node->obj[i]->name,name)) {
	Cfree(node->obj[i]);
	node->obj[i]=NULL;
      }
    }
  return 0;
}

int gridObjCommand(gridObj *obj, int wc, char **wl)
{
  return 0;
}

int gridSetDefault(gridObj *obj)
{
  obj->tri=NULL;
  obj->tri_count=0;
  obj->r=0;
  obj->g=0;
  obj->b=0;

  obj->render.show=1;
  obj->render.transparency=1.0;
  return 0;
}

int gridObjDraw(gridObj *obj)
{
  glColor3f(0.5,0.5,0.5);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glDisable(GL_CULL_FACE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  

#ifdef LINUX
  glEnable(GL_VERTEX_ARRAY);
  glEnable(GL_COLOR_ARRAY);
  glEnable(GL_NORMAL_ARRAY);

  glVertexPointer(3,GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri[0].v1);
  glNormalPointer(GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri[0].n1);
  glColorPointer(3,GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri[0].c1);

  glDrawArrays(GL_TRIANGLES,0,obj->tri_count*3);

  glDisable(GL_VERTEX_ARRAY);
  glDisable(GL_COLOR_ARRAY);
  glDisable(GL_NORMAL_ARRAY);
#endif
#ifdef SGI
  glEnable(GL_VERTEX_ARRAY_EXT);
  glEnable(GL_COLOR_ARRAY_EXT);
  glEnable(GL_NORMAL_ARRAY_EXT);

  glVertexPointerEXT(3,GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri_count*3,obj->tri[0].v1);
  glNormalPointerEXT(GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri_count*3,obj->tri[0].n1);
  glColorPointerEXT(3,GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri_count*3,obj->tri[0].c1);

  glDrawArraysEXT(GL_TRIANGLES,0,obj->tri_count*3);

  glDisable(GL_VERTEX_ARRAY_EXT);
  glDisable(GL_COLOR_ARRAY_EXT);
  glDisable(GL_NORMAL_ARRAY_EXT);
#endif
#ifdef DEC
  glEnable(GL_VERTEX_ARRAY);
  glEnable(GL_COLOR_ARRAY);
  glEnable(GL_NORMAL_ARRAY);

  glVertexPointer(3,GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri_count*3,obj->tri[0].v1);
  glNormalPointer(GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri_count*3,obj->tri[0].n1);
  glColorPointer(3,GL_FLOAT,
		     sizeof(struct GRID_TRI)/3,
		     obj->tri_count*3,obj->tri[0].c1);

  glDrawArrays(GL_TRIANGLES,0,obj->tri_count*3);

  glDisable(GL_VERTEX_ARRAY);
  glDisable(GL_COLOR_ARRAY);
  glDisable(GL_NORMAL_ARRAY);
#endif

  return 0;
}

static char grid_expr[2048];


int gridSet(dbmGridNode *node, int owc, char **owl)
{
  char *expr;
  char prop[256];
  char op[256];
  char val[256];
  char message[256];
  char **wl;
  int wc;
  int c,ret;
  float nv;

  wc=0;
  for(c=0;c<owc;c++)
    wc+=strlen(owl[c]);

  expr=grid_expr;
  strcpy(expr,"");

  for(c=0;c<owc;c++)
    strcat(expr,owl[c]);
  
  dbmSplit(expr,',',&wc,&wl);

  ret=0;
  
  for(c=0;c<wc;c++) {
    dbmSplitPOV(wl[c],prop,op,val);

    if(!strcmp(prop,"scalz")) {
      if(!strcmp(op,"=")) {
	nv=atof(val);
	node->field.scale_z=nv;
      } else {
	sprintf(message,"\n%s: invalid operator %s",node->name, op);
	comMessage(message);
	ret=-1;
	break;
      }
    } else {
      sprintf(message,"\n%s: unknown property %s",node->name, prop);
      comMessage(message);
      ret=-1;
      break;
    }
    
  }

  return ret;

}
