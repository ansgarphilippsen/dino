#include <stdio.h>

#include "vrml.h"
#include "dbm.h"

extern struct DBM dbm;
extern struct GFX gfx;

int writeVRML(FILE *f)
{
  int i,j;

  writeVRMLHeader(f);

  for(i=0;i<dbm.nodec_max;i++) {
    switch(dbm.node[i].common.type) {
    case DBM_NODE_STRUCT:
      for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	if(dbm.node[i].structNode.obj_flag[j]!=0)
	  if(dbm.node[i].structNode.obj[j].render.show)
	    writeVRMLStructObj(f,&dbm.node[i].structNode.obj[j]);
      break;
    case DBM_NODE_SCAL:
      for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	if(dbm.node[i].scalNode.obj[j]!=NULL)
	  if(dbm.node[i].scalNode.obj[j]->render.show)
	    writeVRMLScalObj(f,dbm.node[i].scalNode.obj[j]);
      break;
    case DBM_NODE_SURF:
      for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	if(dbm.node[i].surfNode.obj_flag[j]!=0)
	  if(dbm.node[i].surfNode.obj[j].render.show)
	    writeVRMLSurfObj(f,&dbm.node[i].surfNode.obj[j]);
    }
    
  }

  writeVRMLFooter(f);

  return 0;
}

int writeVRMLHeader(FILE *f)
{
  fprintf(f,"#VRML V2.0 utf8\n");
  fprintf(f,"Separator {\n");

  // lighting
  fprintf(f,"DirectionalLight{ direction 0 0 -1 }\n");
  fprintf(f,"PerspectiveCamera {\n");
  /*
  fprintf(f,"position %f %f %f\norientation %f %f %f\nfocalDistance %f\n",
	  0,0,0,
	  0,0,-1,
	  gfx.fovy);
  */	  
  fprintf(f,"}\n");

  return 0;
}

int writeVRMLFooter(FILE *f)
{
  fprintf(f,"}\n");
  return 0;
}

int writeVRMLStructObj(FILE *f,struct STRUCT_OBJ *obj)
{

  switch(obj->render.mode) {
  case RENDER_SLINE:
    break;
  case RENDER_HELIX:
  case RENDER_STRAND:
  case RENDER_TUBE:
    break;
  case RENDER_SIMPLE:
    break;
  case RENDER_CPK:
    break;
  case RENDER_CUSTOM:
    break;
  }
  return 0;
}

int writeVRMLScalObj(FILE *f,struct SCAL_OBJ *obj)
{
  return 0;
}

int writeVRMLSurfObj(FILE *f,surfObj *obj)
{
  int vc,fc;

  fprintf(f,"Transform {\n");
  fprintf(f,"translation 0 0 0\n");
  fprintf(f,"Shape {\n");

  fprintf(f,"appearance Appearance {\n");
  fprintf(f,"material Material {\n");
  fprintf(f,"diffuseColor %f %f %f\n",
	  obj->render.mat.diff[0],
	  obj->render.mat.diff[1],
	  obj->render.mat.diff[2]);
	  
  fprintf(f,"}\n"); // material
  fprintf(f,"}\n"); // appearance

  fprintf(f,"geometry IndexedFaceSet {\n");
  fprintf(f,"coord Coordinate {\n");
  
  fprintf(f,"point [\n");
  for(vc=0;vc<obj->vertc;vc++) {
    fprintf(f,"%f %f %f,\n",
	    obj->vert[vc].p[0],
	    obj->vert[vc].p[1],
	    obj->vert[vc].p[2]);
  }
  fprintf(f,"]\n"); // point

  fprintf(f,"}\n"); // coord

  fprintf(f,"coordIndex [\n");
  for(fc=0;fc<obj->facec;fc++) {
    fprintf(f,"%d %d %d -1\n",
	    obj->face[fc*3+0],
	    obj->face[fc*3+1],
	    obj->face[fc*3+2]);
  }
  fprintf(f,"]\n"); // coordIndex

  fprintf(f,"normal Normal {\nvector [\n");
  for(vc=0;vc<obj->vertc;vc++) {
    fprintf(f,"%f %f %f,\n",
	    obj->vert[vc].n[0],
	    obj->vert[vc].n[1],
	    obj->vert[vc].n[2]);
  }
  fprintf(f,"]\n}\n"); // normal

  fprintf(f,"color Color {\ncolor [\n");
  for(vc=0;vc<obj->vertc;vc++) {
    fprintf(f,"%f %f %f,\n",
	    obj->vert[vc].c[0],
	    obj->vert[vc].c[1],
	    obj->vert[vc].c[2]);
  }
  fprintf(f,"]\n}\n"); // color


  fprintf(f,"}\n"); // ifs
  fprintf(f,"}\n"); // Shape
  fprintf(f,"}\n"); // transform
  return 0;
}
