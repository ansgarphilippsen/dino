#include "pov.h"
#include "pov_macro.h"
#include "gfx.h"
#include "mat.h"

extern struct DBM dbm;
extern struct GFX gfx;

GLdouble write_pov_mm[16],write_pov_pm[16],write_pov_rm[16];
GLint write_pov_vp[4];

int write_pov_flag, write_pov_mode;

/*
  two files are written: file.pov and file.inc

  the first contains the camera and light position
  as well as the variables for the texture parameters
  for all following objects
  the second contains all objects
*/

int writePOV(FILE *fpov, FILE *finc,char *fpovn, int flag, int mode)
{
  int i,j,k;
  FILE *f;
  int tri_flag=1,cyl_flag=1;
  float lim[6];

  write_pov_flag=flag;
  write_pov_mode=mode;
  
  lim[0]=lim[1]=lim[2]=-1e9;
  lim[3]=lim[4]=lim[5]=1e9;

  glGetDoublev(GL_MODELVIEW_MATRIX,write_pov_mm);
  glGetDoublev(GL_PROJECTION_MATRIX,write_pov_pm);
  glGetIntegerv(GL_VIEWPORT,write_pov_vp);

  memcpy(write_pov_rm,write_pov_mm,16*sizeof(GLdouble));
  write_pov_rm[12]=0.0;
  write_pov_rm[13]=0.0;
  write_pov_rm[14]=0.0;
  write_pov_rm[15]=1.0;

  fprintf(fpov,"/*\n  POVRAY output from DINO\n\n");
  fprintf(fpov,"  This is the main file containing the user\n");
  fprintf(fpov,"  defineable parameters\n*/\n\n");

  writePOVScene(fpov);

  if(write_pov_mode==WRITE_POV_MEGA) {
    fprintf(finc,"#version unofficial MegaPov 0.5;\n");
  }

  if(write_pov_mode==WRITE_POV_SMOOTH)
    fprintf(finc,pov_tri_macro);
  fprintf(finc,pov_cyl_macro);
  
  for(k=1;k>=0;k--) {
    if(k==0)
      f=fpov;
    else
      f=finc;

    for(i=0;i<dbm.nodec_max;i++) {
      switch(dbm.node[i].common.type) {
      case DBM_NODE_STRUCT:
	for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	  if(dbm.node[i].structNode.obj_flag[j]!=0)
	    if(dbm.node[i].structNode.obj[j].render.show)
	      writePOVStructObj(f,&dbm.node[i].structNode.obj[j],k,lim);
	break;
      case DBM_NODE_SCAL:
	for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	  if(dbm.node[i].scalNode.obj[j]!=NULL)
	    if(dbm.node[i].scalNode.obj[j]->render.show)
	      writePOVScalObj(f,dbm.node[i].scalNode.obj[j],k,lim);
	break;
      case DBM_NODE_SURF:
	for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	  if(dbm.node[i].surfNode.obj_flag[j]!=0)
	    if(dbm.node[i].surfNode.obj[j].render.show)
	      writePOVSurfObj(f,&dbm.node[i].surfNode.obj[j],k,lim);
	break;
      case DBM_NODE_GRID:
	for(j=0;j<dbm.node[i].gridNode.obj_max;j++)
	  if(dbm.node[i].gridNode.obj_flag[j]!=0)
	    if(dbm.node[i].gridNode.obj[j].render.show)
	      writePOVGridObj(f,&dbm.node[i].gridNode.obj[j],k,lim);
	break;
      case DBM_NODE_GEOM:
	for(j=0;j<dbm.node[i].geomNode.obj_max;j++)
	  if(dbm.node[i].geomNode.obj[j]!=NULL)
	    if(dbm.node[i].geomNode.obj[j]->render.show)
	      writePOVGeomObj(f,dbm.node[i].geomNode.obj[j],k,lim);
      }
      
    }

    if(k==0) {
      fprintf(f,"\n#declare _Scene = #include \"%s\"\n",fpovn);
    }

    if(k==1) {
      fprintf(fpov,"\n// bounding box\n");
      fprintf(fpov,"// %.3f %.3f %.3f  %.3f %.3f %.3f\n\n",
	      lim[3]-2.0,lim[4]-2.0,lim[5]-2.0,
	      lim[0]+2.0,lim[1]+2.0,lim[2]+2.0);
    }
    
  }

  fprintf(fpov,"object {\n    _Scene\n}\n");
  
  return 0;
}


int writePOVScene(FILE *f)
{
  double cpos[4];

  fprintf(f,"#version 3.1;\n");

  fprintf(f,"\nbackground {color rgb <%.3f,%.3f,%.3f>}\n\n",
	  gfx.r,gfx.g,gfx.b);

  fprintf(f,"camera {\nperspective\nlocation <0,0,0>\ndirection <0,0,-1>\nup <0,1,0>\nright <1,0,0>\n");
  fprintf(f,"angle %.4f\n",gfx.fovy);
  fprintf(f,"translate <0,0,%.4f>\n",-gfx.transform.tra[2]);
  fprintf(f,"/*\n  for stereo pictures, uncomment the following two lines and\n");
  fprintf(f,"  write out two pictures, one with the x-component of the\n");
  fprintf(f,"  translate statement positive, the other negative (this is\n");
  fprintf(f,"  IOC (interoccular distance) divided by two)\n*/\n");
  fprintf(f,"//translate <2.0,0,0>\n//look_at <0,0,0>\n");
  fprintf(f,"}\n");
  fprintf(f,"\n\n");

  fprintf(f,"light_source {<0,0,%.4f> color rgb <1,1,1>}\n",
	  -gfx.transform.tra[2]);
  fprintf(f,"light_source {<0,0,0> color rgb 0.1 shadowless}\n");

  if(gfx.fog) {
    fprintf(f,"\n// depth cueing\n");
    fprintf(f,"plane {z, %.3f texture {pigment {color rgbft <0,0,0,1,1>}}\n",
	    -gfx.transform.tra[2]-gfx.transform.slabn);
    fprintf(f,"hollow interior {fade_power 1 fade_distance %.3f}\n}\n",
	    (gfx.transform.slabf-gfx.transform.slabn)*0.5);
  }

  return 0;
}

int writePOVStructObj(FILE *f, structObj *obj, int k,float *lim)
{
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float lw;
  int i;
  float def_amb,def_diff,def_spec,def_rough;
  char tex_name[128],def_name[64],fi_name[64],tp_name[64],tex2_name[128];

  def_amb=0.1;
  def_diff=0.6;
  def_spec=0.3;
  def_rough=0.01;

  lw=obj->render.line_width*0.05;
  t=1.0;

  sprintf(tex_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);

  if(k==0) {
    fprintf(f,"// material, transparency and filter definition for object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = \ntexture {\n",tex_name);
    if(write_pov_mode==WRITE_POV_NOCOLOR)
      fprintf(f,"pigment {color rgb 1}\n");
    fprintf(f,"finish {ambient %.2f diffuse %.2f specular %.2f roughness %.2f}\n}\n",
	    def_amb,def_diff,def_spec,def_rough);
    fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.4f;\n\n",fi_name,0.0);
    return 0;
  }

  fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);

  switch(obj->render.mode) {
  case RENDER_SLINE:
    fprintf(f,"union {\n");
    for(i=0;i<obj->va.count;i+=2) {
      v1[0]=obj->va.p[i].v[0];
      v1[1]=obj->va.p[i].v[1];
      v1[2]=obj->va.p[i].v[2];
      v2[0]=obj->va.p[i+1].v[0];
      v2[1]=obj->va.p[i+1].v[1];
      v2[2]=obj->va.p[i+1].v[2];
      if(!writePOVTransform(&obj->node->transform,v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,v2,v2,1)) continue;

      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v2,lim);

      c1=obj->va.p[i].c;
      c2=obj->va.p[i+1].c;

      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f \n",
		v1[0],v1[1],v1[2],
		v2[0],v2[1],v2[2],
		lw);
	fprintf(f," material {texture {%s}}}\n",tex_name);
      } else {
	fprintf(f,"color_cylinder (<%.4f,%.4f,%.4f>,<%.3f,%.3f,%.3f>,<%.4f,%.4f,%.4f>,<%.3f,%.3f,%.3f>,%.4f)\n",
		v1[0],v1[1],v1[2],c1[0],c1[1],c1[2],
		v2[0],v2[1],v2[2],c2[0],c2[1],c2[2],
		lw);
      }      
    }
    break;
  case RENDER_TUBE:
  case RENDER_HSC:
      if(write_pov_mode==WRITE_POV_NEW ||
	 write_pov_mode==WRITE_POV_NOCOLOR ||
	 write_pov_mode==WRITE_POV_MEGA) {
	fprintf(f,"mesh {\n");
      } else {
	fprintf(f,"union {\n");
      }
    if(write_pov_mode==WRITE_POV_SMOOTH) {
      fprintf(f,"#declare triangle_base_texture = material {texture{%s}}\n",tex_name);
    }
    for(i=0;i<obj->va.count;i+=3) {
      v1[0]=obj->va.p[i+0].v[0];
      v1[1]=obj->va.p[i+0].v[1];
      v1[2]=obj->va.p[i+0].v[2];
      n1[0]=obj->va.p[i+0].n[0];
      n1[1]=obj->va.p[i+0].n[1];
      n1[2]=obj->va.p[i+0].n[2];
      c1=obj->va.p[i+0].c;
      v2[0]=obj->va.p[i+1].v[0];
      v2[1]=obj->va.p[i+1].v[1];
      v2[2]=obj->va.p[i+1].v[2];
      n2[0]=obj->va.p[i+1].n[0];
      n2[1]=obj->va.p[i+1].n[1];
      n2[2]=obj->va.p[i+1].n[2];
      c2=obj->va.p[i+1].c;
      v3[0]=obj->va.p[i+2].v[0];
      v3[1]=obj->va.p[i+2].v[1];
      v3[2]=obj->va.p[i+2].v[2];
      n3[0]=obj->va.p[i+2].n[0];
      n3[1]=obj->va.p[i+2].n[1];
      n3[2]=obj->va.p[i+2].n[2];
      c3=obj->va.p[i+2].c;

      if(!writePOVTransform(&obj->node->transform,v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,v2,v2,1)) continue;
      if(!writePOVTransform(&obj->node->transform,v3,v3,1)) continue;

      if(v1[0]==v2[0] && v2[0]==v3[0])
	if(v1[1]==v2[1] && v2[1]==v3[1])
	  if(v1[2]==v2[2] && v2[2]==v3[2])
	    continue;

      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v2,lim);
      writePOVCheckLim(v3,lim);

      writePOVTransform(&obj->node->transform,n1,n1,0);
      writePOVTransform(&obj->node->transform,n2,n2,0);
      writePOVTransform(&obj->node->transform,n3,n3,0);
      if(write_pov_mode==WRITE_POV_DEFAULT) {
	// fast triangles
	fprintf(f,"smooth_triangle {");

	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f," material {texture{%s}} pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,
		(c1[0]+c1[0]+c2[0])/3.0,
		(c1[1]+c1[1]+c2[1])/3.0,
		(c1[2]+c1[2]+c2[2])/3.0,
		fi_name,tp_name);
      } else if(write_pov_mode==WRITE_POV_NOCOLOR) {
	if( (v1[0]==v2[0] && v1[1]==v2[1] && v1[2]==v2[2]) ||
	    (v3[0]==v2[0] && v3[1]==v2[1] && v3[2]==v2[2]) ||
	    (v1[0]==v3[0] && v1[1]==v3[1] && v1[2]==v3[2])) {
	} else {
	fprintf(f,"smooth_triangle {");

	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f," texture {%s}} \n",tex_name);
	}
      } else if(write_pov_mode==WRITE_POV_MEGA) {
	// format of megapov
	fprintf(f,"#local tex1 = texture { %s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,c1[0],c1[1],c1[2],fi_name,tp_name);
	fprintf(f,"#local tex2 = texture { %s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,c2[0],c2[1],c2[2],fi_name,tp_name);
	fprintf(f,"#local tex3 = texture { %s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,c3[0],c3[1],c3[2],fi_name,tp_name);
	fprintf(f,"smooth_triangle {");

	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f,"\ntexture_list { tex1 tex2 tex3}\n");
	fprintf(f,"}\n");
      } else {
	if(write_pov_mode==WRITE_POV_NEW) {
	  fprintf(f,"smooth_color_triangle {");
	} else {
	  fprintf(f,"colored_smooth_triangle (");
	}
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2],c1[0],c1[1],c1[2],
		fi_name,tp_name);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2],c2[0],c2[1],c2[2],
		fi_name,tp_name);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2],c3[0],c3[1],c3[2],
		fi_name,tp_name);
	if(write_pov_mode==WRITE_POV_NEW) 
	  fprintf(f,"}\n");
	else
	  fprintf(f,")\n");
	
      }
    }
    if(write_pov_mode==WRITE_POV_NEW) {
      fprintf(f,"material {texture{%s}}\n",tex_name);
    }
    
    break;
  case RENDER_SIMPLE:
    fprintf(f,"union {\n");

    for(i=0;i<obj->bond_count;i++) {
      v1[0]=obj->bond[i].atom1->p->x;
      v1[1]=obj->bond[i].atom1->p->y;
      v1[2]=obj->bond[i].atom1->p->z;
      v3[0]=obj->bond[i].atom2->p->x;
      v3[1]=obj->bond[i].atom2->p->y;
      v3[2]=obj->bond[i].atom2->p->z;
      if(!writePOVTransform(&obj->node->transform,v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,v3,v3,1)) continue;

      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v3,lim);

      v2[0]=v1[0]+(v3[0]-v1[0])*0.5;
      v2[1]=v1[1]+(v3[1]-v1[1])*0.5;
      v2[2]=v1[2]+(v3[2]-v1[2])*0.5;

      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f",
		v1[0],v1[1],v1[2],v2[0],v2[1],v2[2],lw);
	fprintf(f," material {texture {%s}}}\n",tex_name);
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f",
		v2[0],v2[1],v2[2],v3[0],v3[1],v3[2],lw);
	fprintf(f," material {texture {%s}}}\n",tex_name);
      } else {
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f ",
		v1[0],v1[1],v1[2],v2[0],v2[1],v2[2],lw);
	fprintf(f,"texture {pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}} }\n",
		obj->bond[i].prop1->r,
		obj->bond[i].prop1->g,
		obj->bond[i].prop1->b,
		fi_name,tp_name);
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f ",
		v2[0],v2[1],v2[2],v3[0],v3[1],v3[2],lw);
	fprintf(f,"texture {pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}} }\n",
		obj->bond[i].prop2->r,
		obj->bond[i].prop2->g,
		obj->bond[i].prop2->b,
		fi_name,tp_name);
      }
    }
    break;
  case RENDER_CPK:
    fprintf(f,"union {\n");
    for(i=0;i<obj->atom_count;i++) {
      v1[0]=obj->atom[i].ap->p->x;
      v1[1]=obj->atom[i].ap->p->y;
      v1[2]=obj->atom[i].ap->p->z;
      if(!writePOVTransform(&obj->node->transform,v1,v2,1)) continue;
      writePOVCheckLim(v2,lim);
      fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%.4f ",
	      v2[0],v2[1],v2[2],  obj->atom[i].prop.radius);
      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f,"material { texture{%s} } }\n",tex_name);
      } else {
	fprintf(f,"material { texture {%s} } pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,
		obj->atom[i].prop.r,
		obj->atom[i].prop.g,
		obj->atom[i].prop.b,
		fi_name,tp_name);
      }
    }
    break;
  case RENDER_CUSTOM:
    fprintf(f,"union {\n");
    for(i=0;i<obj->bond_count;i++) {
      v1[0]=obj->bond[i].atom1->p->x;
      v1[1]=obj->bond[i].atom1->p->y;
      v1[2]=obj->bond[i].atom1->p->z;
      v3[0]=obj->bond[i].atom2->p->x;
      v3[1]=obj->bond[i].atom2->p->y;
      v3[2]=obj->bond[i].atom2->p->z;
      if(!writePOVTransform(&obj->node->transform,v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,v3,v3,1)) continue;

      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v3,lim);

      v2[0]=v1[0]+(v3[0]-v1[0])*0.5;
      v2[1]=v1[1]+(v3[1]-v1[1])*0.5;
      v2[2]=v1[2]+(v3[2]-v1[2])*0.5;

      fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f ",
	      v1[0],v1[1],v1[2],v2[0],v2[1],v2[2],obj->render.bond_width);
      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f," material {texture {%s}}}\n",tex_name);
      } else {
	fprintf(f," material { texture{%s} } pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,
		obj->bond[i].prop1->r,
		obj->bond[i].prop1->g,
		obj->bond[i].prop1->b,
		fi_name,tp_name);
      }
      fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f ",
	      v2[0],v2[1],v2[2],v3[0],v3[1],v3[2],obj->render.bond_width);
      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f," material {texture {%s}}}\n",tex_name);
      } else {
	fprintf(f," material { texture{%s} } pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,
		obj->bond[i].prop2->r,
		obj->bond[i].prop2->g,
		obj->bond[i].prop2->b,
		tp_name,fi_name);
      }
    }
    for(i=0;i<obj->atom_count;i++) {
      v1[0]=obj->atom[i].ap->p->x;
      v1[1]=obj->atom[i].ap->p->y;
      v1[2]=obj->atom[i].ap->p->z;
      if(!writePOVTransform(&obj->node->transform,v1,v2,1)) continue;
      writePOVCheckLim(v2,lim);
      fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%.4f ",
	      v2[0],v2[1],v2[2],  obj->render.sphere_radius);
      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f," material {texture {%s}}}\n",tex_name);
      } else {
	fprintf(f,"material { texture{%s} } pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,
		obj->atom[i].prop.r,
		obj->atom[i].prop.g,
		obj->atom[i].prop.b,
		fi_name,tp_name);
      }
    }
    break;
  }

  fprintf(f,"}\n");

  return 0;
}

int writePOVSurfObj(FILE *f, surfObj *obj, int k,float *lim)
{
  int i,p1,p2,p3;
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float def_amb,def_diff,def_spec,def_rough,def_bri;
  char mat_name[64],tex_name[64],tp_name[64],fi_name[64];

  sprintf(tex_name,"_%s_%s_tex",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);

  def_amb=0.10;
  def_diff=0.78;
  def_spec=0.02;
  def_rough=0.01;
  def_bri=0.7;

  if(k==0) {
    fprintf(f,"// material, transparency and filter definition for object .%s.%s\n",obj->node->name, obj->name);
    
    fprintf(f,"#declare %s = \ntexture {\n",tex_name);
    if(write_pov_mode==WRITE_POV_NOCOLOR)
      fprintf(f,"pigment {color rgb 1}\n");
    fprintf(f,"finish {ambient %.2f diffuse %.2f specular %.2f roughness %.2f brilliance %.2f}\n}\n",
	    def_amb,def_diff,def_spec,def_rough,def_bri);
    fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.4f;\n\n",fi_name,0.0);
    return 0;
  }

  fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);
  if(write_pov_mode==WRITE_POV_NEW ||
     write_pov_mode==WRITE_POV_NOCOLOR ||
     write_pov_mode==WRITE_POV_MEGA) {
    fprintf(f,"mesh{\n");
  } else {
    fprintf(f,"union {\n");
  }
  if(write_pov_mode==WRITE_POV_SMOOTH)
    fprintf(f,"#declare triangle_base_texture = material{texture{%s}}\n",tex_name);

  for(i=0;i<obj->facec;i++) {
    p1=obj->face[i*3+0];
    p2=obj->face[i*3+1];
    p3=obj->face[i*3+2];

    if(!writePOVTransform(&obj->node->transform,obj->vert[p1].p,v1,1)) continue;
    if(!writePOVTransform(&obj->node->transform,obj->vert[p2].p,v2,1)) continue;
    if(!writePOVTransform(&obj->node->transform,obj->vert[p3].p,v3,1)) continue;

    if(v1[0]==v2[0] && v2[0]==v3[0])
      if(v1[1]==v2[1] && v2[1]==v3[1])
	if(v1[2]==v2[2] && v2[2]==v3[2])
	  continue;

    writePOVCheckLim(v1,lim);
    writePOVCheckLim(v2,lim);
    writePOVCheckLim(v3,lim);
    
    writePOVTransform(&obj->node->transform,obj->vert[p1].n,n1,0);
    writePOVTransform(&obj->node->transform,obj->vert[p2].n,n2,0);
    writePOVTransform(&obj->node->transform,obj->vert[p3].n,n3,0);

    c1=obj->vert[p1].c;
    c2=obj->vert[p2].c;
    c3=obj->vert[p3].c;

    if(write_pov_mode==WRITE_POV_DEFAULT) {
      fprintf(f,"smooth_triangle {");
      
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
      fprintf(f," pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
	      (c1[0]+c1[0]+c2[0])/3.0,
	      (c1[1]+c1[1]+c2[1])/3.0,
	      (c1[2]+c1[2]+c2[2])/3.0,
	      fi_name,tp_name);
    } else if(write_pov_mode==WRITE_POV_NOCOLOR) {
      if( (v1[0]==v2[0] && v1[1]==v2[1] && v1[2]==v2[2]) ||
	  (v3[0]==v2[0] && v3[1]==v2[1] && v3[2]==v2[2]) ||
	  (v1[0]==v3[0] && v1[1]==v3[1] && v1[2]==v3[2])) {
      } else {
	fprintf(f,"smooth_triangle {");
	
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f," texture {%s}}\n", tex_name);
      }
    } else if(write_pov_mode==WRITE_POV_MEGA) {
      // format of megapov
      fprintf(f,"#local tex1 = texture {%s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
	      tex_name,c1[0],c1[1],c1[2],fi_name,tp_name);
      fprintf(f,"#local tex2 = texture {%s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
	      tex_name,c2[0],c2[1],c2[2],fi_name,tp_name);
      fprintf(f,"#local tex3 = texture {%s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
	      tex_name,c3[0],c3[1],c3[2],fi_name,tp_name);
      fprintf(f,"smooth_triangle {");
      
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
      fprintf(f,"\ntexture_list { tex1 tex2 tex3}\n");
      fprintf(f,"}\n");
    } else {
      if(write_pov_mode==WRITE_POV_NEW) {
	fprintf(f,"smooth_color_triangle {");
      } else {
	fprintf(f,"colored_smooth_triangle (");
      }
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2],c1[0],c1[1],c1[2],
	      fi_name,tp_name);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2],c2[0],c2[1],c2[2],
	      fi_name,tp_name);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2],c3[0],c3[1],c3[2],
	      fi_name,tp_name);
      if(write_pov_mode==WRITE_POV_NEW) 
	fprintf(f,"}\n");
      else
	fprintf(f,")\n");
      
    }
    
  }
  if(write_pov_mode==WRITE_POV_DEFAULT ||
     write_pov_mode==WRITE_POV_NEW) {
        fprintf(f,"material {texture {%s}}\n",tex_name);
  }


  fprintf(f,"}");


  return 0;
}

int writePOVScalObj(FILE *f, scalObj *obj, int k,float *lim)
{
  int i,p1,p2,p3;
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float def_amb,def_diff,def_spec,def_rough,def_bri;
  char tex_name[64],tp_name[64],lw_name[64],ps_name[64],fi_name[64];
  float lw,ps,rad;

  sprintf(tex_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(lw_name,"_%s_%s_lw",obj->node->name,obj->name);
  writePOVCheckName(lw_name);
  sprintf(ps_name,"_%s_%s_ps",obj->node->name,obj->name);
  writePOVCheckName(ps_name);

  def_amb=0.10;
  def_diff=0.78;
  def_spec=0.02;
  def_rough=0.05;
  def_bri=1.5;

  lw=obj->render.line_width*0.05;
  ps=obj->render.point_size*0.05;

  if(k==0) {
    fprintf(f,"// material, transparency, filter, linewidth and pointsize definition for object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = material {\ntexture {\n",tex_name);
    fprintf(f,"finish {ambient %.2f diffuse %.2f specular %.2f roughness %.2f brilliance %.2f}\n}\n}\n",
	    def_amb,def_diff,def_spec,def_rough,def_bri);
    fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.4f;\n\n",fi_name,0.0);

    if(obj->type==SCAL_CONTOUR && obj->render.mode==RENDER_LINE)
      fprintf(f,"#declare %s = %.4f;\n",lw_name,lw);
    if((obj->type==SCAL_CONTOUR && obj->render.mode==RENDER_POINT) ||
       (obj->type==SCAL_GRID))
      fprintf(f,"#declare %s = %.4f;\n",ps_name,ps);
    return 0;
  }




  switch(obj->type) {
  case SCAL_CONTOUR:
    switch(obj->render.mode) {
    case RENDER_POINT:
      fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);
      fprintf(f,"union {\n");
      for(i=0;i<obj->point_count;i++) {
	if(!writePOVTransform(&obj->node->transform,obj->point[i].v,v1,1)) continue;
	writePOVCheckLim(v1,lim);
	fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s}\n",
		v1[0],v1[1],v1[2],ps_name);
      }
      fprintf(f,"material {%s} pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}\n",
	      tex_name,obj->r,obj->g, obj->b, fi_name,tp_name);

      break;
    case RENDER_LINE:
      fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);
      fprintf(f,"union {\n");
      for(i=0;i<obj->line_count;i++) {
	if(!writePOVTransform(&obj->node->transform,obj->point[obj->line[i].pi0].v,v1,1)) continue;
	if(!writePOVTransform(&obj->node->transform,obj->point[obj->line[i].pi1].v,v2,1)) continue;
	writePOVCheckLim(v1,lim);
	writePOVCheckLim(v2,lim);

	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%s}\n",
		v1[0],v1[1],v1[2],v2[0],v2[1],v2[2],lw_name);
      }
      fprintf(f,"material {%s} pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}\n",
	      tex_name, obj->r,obj->g, obj->b, fi_name, tp_name);

      break;
    case RENDER_SURFACE:

      fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);
      fprintf(f,"mesh{\n");

      for(i=0;i<obj->face_count;i++) {
	if(!writePOVTransform(&obj->node->transform,obj->face[i].v1,v1,1)) continue;
	if(!writePOVTransform(&obj->node->transform,obj->face[i].v2,v2,1)) continue;
	if(!writePOVTransform(&obj->node->transform,obj->face[i].v3,v3,1)) continue;
	
	if(v1[0]==v2[0] && v2[0]==v3[0])
	  if(v1[1]==v2[1] && v2[1]==v3[1])
	    if(v1[2]==v2[2] && v2[2]==v3[2])
	      continue;
	
	writePOVCheckLim(v1,lim);
	writePOVCheckLim(v2,lim);
	writePOVCheckLim(v3,lim);
	
	writePOVTransform(&obj->node->transform,obj->face[i].n1,n1,0);
	writePOVTransform(&obj->node->transform,obj->face[i].n2,n2,0);
	writePOVTransform(&obj->node->transform,obj->face[i].n3,n3,0);
	
	fprintf(f,"smooth_triangle {");
	
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f,"}\n");
	  
      }
      fprintf(f,"material {%s} pigment {color rgbft<%.4f,%.4f,%.4f,%s,%s>}\n",
	      tex_name, obj->r,obj->g, obj->b, fi_name, tp_name);
      break;
    }
    break;
  case SCAL_GRID:
    fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);
    fprintf(f,"union {\n");
    for(i=0;i<obj->point_count;i++) {
      if(!writePOVTransform(&obj->node->transform,obj->point[i].v,v1,1)) continue;
      writePOVCheckLim(v1,lim);
      if(obj->render.mode==RENDER_ON) {
	rad=obj->point[i].rad;
	fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s ",
		v1[0],v1[1],v1[2], ps_name);
      } else {
	rad=ps;
	fprintf(f,"disc {<%.4f,%.4f,%.4f>, <0,0,1>, %s ",
		v1[0],v1[1],v1[2], ps_name);
      }
      fprintf(f,"material { %s } pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
	      tex_name,
	      obj->point[i].c[0],
	      obj->point[i].c[1],
	      obj->point[i].c[2],
	      tp_name,
	      fi_name);
      
    }
    break;
  }
  
  fprintf(f,"}\n");

  return 0;
}

int writePOVGridObj(FILE *f, gridObj *obj, int k,float *lim)
{
  int i;
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float def_amb,def_diff,def_spec,def_rough,def_bri;
  char tex_name[64],tp_name[64],fi_name[64];

  sprintf(tex_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);

  def_amb=0.10;
  def_diff=0.78;
  def_spec=0.02;
  def_rough=0.01;
  def_bri=0.7;

  if(k==0) {
    fprintf(f,"// material, transparency and filter definition for object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = texture {\n",tex_name);
    fprintf(f,"finish {ambient %.2f diffuse %.2f specular %.2f roughness %.2f brilliance %.2f}\n}\n",
	    def_amb,def_diff,def_spec,def_rough,def_bri);
    fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.4f;\n",fi_name,0.0);
    return 0;
  }


  fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);

  switch (obj->type) {
  case GRID_SURFACE:
    if(write_pov_mode==WRITE_POV_NEW || write_pov_mode==WRITE_POV_MEGA)
      fprintf(f,"mesh {\n");
    else
      fprintf(f,"union {\n");
    
    if(write_pov_mode==WRITE_POV_SMOOTH)
      fprintf(f,"#declare triangle_base_texture = material{texture{%s}}\n",tex_name);
    
    
    t=obj->render.transparency;
    
    for(i=0;i<obj->facec;i++) {
      
      if(!writePOVTransform(&obj->node->transform,obj->face[i].v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,obj->face[i].v2,v2,1)) continue;
      if(!writePOVTransform(&obj->node->transform,obj->face[i].v3,v3,1)) continue;
      
      if(v1[0]==v2[0] && v2[0]==v3[0])
	if(v1[1]==v2[1] && v2[1]==v3[1])
	  if(v1[2]==v2[2] && v2[2]==v3[2])
	    continue;
      
      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v2,lim);
      writePOVCheckLim(v3,lim);
      
      writePOVTransform(&obj->node->transform,obj->face[i].n1,n1,0);
      writePOVTransform(&obj->node->transform,obj->face[i].n2,n2,0);
      writePOVTransform(&obj->node->transform,obj->face[i].n3,n3,0);
      c1=obj->face[i].c1;
      c2=obj->face[i].c2;
      c3=obj->face[i].c3;
      
      if(write_pov_mode==WRITE_POV_DEFAULT) {
	fprintf(f,"smooth_triangle {");
	
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f," pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		(c1[0]+c1[0]+c2[0])/3.0,
		(c1[1]+c1[1]+c2[1])/3.0,
		(c1[2]+c1[2]+c2[2])/3.0,
		fi_name, tp_name);
      } else if(write_pov_mode==WRITE_POV_MEGA) {
	// format of megapov
	fprintf(f,"#local tex1 = texture {%s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,c1[0],c1[1],c1[2],fi_name,tp_name);
	fprintf(f,"#local tex2 = texture {%s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,c2[0],c2[1],c2[2],fi_name,tp_name);
	fprintf(f,"#local tex3 = texture {%s pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}}\n",
		tex_name,c3[0],c3[1],c3[2],fi_name,tp_name);
	fprintf(f,"smooth_triangle {");
	
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
	fprintf(f,"\ntexture_list { tex1 tex2 tex3}\n");
	fprintf(f,"}\n");
      } else {
	if(write_pov_mode==WRITE_POV_NEW) 
	  fprintf(f,"smooth_color_triangle {");
	else
	  fprintf(f,"colored_smooth_triangle (");
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
		v1[0],v1[1],v1[2],n1[0],n1[1],n1[2],c1[0],c1[1],c1[2],
		fi_name,tp_name);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
		v2[0],v2[1],v2[2],n2[0],n2[1],n2[2],c2[0],c2[1],c2[2],
		fi_name,tp_name);
	fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>",
		v3[0],v3[1],v3[2],n3[0],n3[1],n3[2],c3[0],c3[1],c3[2],
		fi_name,tp_name);
	if(write_pov_mode==WRITE_POV_NEW) 
	  fprintf(f,"}\n");
	else
	  fprintf(f,")\n");
	
      }
      
    }
    if(write_pov_mode==WRITE_POV_DEFAULT ||
       write_pov_mode==WRITE_POV_NEW) {
      fprintf(f,"material {texture{%s}}\n",tex_name);
    }
    fprintf(f,"}");
    break;
  case GRID_CONTOUR:
    fprintf(f,"// not supported yet\n");
    break;
  }
  return 0;
}

int writePOVGeomObj(FILE *f, geomObj *obj, int k,float *lim)
{
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float lw,ll;
  int i;
  float def_amb,def_diff,def_spec,def_rough;
  char tex_name[64],def_name[64],tp_name[64],fi_name[64],lw_name[64],ps_name[64];
  float stipple_add1[3],stipple_add2[3],stipple_dir[3],stipple_pos[3];

  def_amb=0.1;
  def_diff=0.6;
  def_spec=0.3;
  def_rough=0.01;

  sprintf(tex_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(tp_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(lw_name,"_%s_%s_lw",obj->node->name,obj->name);
  writePOVCheckName(lw_name);
  sprintf(ps_name,"_%s_%s_ps",obj->node->name,obj->name);
  writePOVCheckName(ps_name);


  if(k==0) {
    fprintf(f,"// material, transparency, filter, linewidth and pointsize definition for object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = material {\ntexture {\n",tex_name);
    fprintf(f,"finish {ambient %.2f diffuse %.2f specular %.2f roughness %.2f}\n}\n}\n",
	    def_amb,def_diff,def_spec,def_rough);

    fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.4f;\n",fi_name,0.0);
    fprintf(f,"#declare %s = %.4f;\n",lw_name,0.05);
    fprintf(f,"#declare %s = %.4f;\n",ps_name,0.05);

    return 0;
  }

  fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);

  fprintf(f,"union {\n");
  for(i=0;i<obj->point_count;i++) {
    if(!writePOVTransform(&obj->node->transform,obj->point[i].v,v1,1)) continue;
    writePOVCheckLim(v1,lim);
    
    if(obj->render.mode==RENDER_OFF) 
      fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s*%.3f\n",
	      v1[0],v1[1],v1[2],ps_name,obj->point[i].ps);
    else
      fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%.3f\n",
	      v1[0],v1[1],v1[2],obj->point[i].r);
    fprintf(f,"material {%s} pigment {color rgbft<%.3f,%.3f,%.3f,%s*%.3f,%s*%.3f>}",
	    tex_name, obj->point[i].c[0], 
	    obj->point[i].c[1],
	    obj->point[i].c[2], 
	    fi_name,1.0-obj->point[i].c[3],
	    tp_name,1.0-obj->point[i].c[3]);
    fprintf(f,"}\n");
  }
  for(i=0;i<obj->line_count;i++) {
    if(obj->render.stipple_flag) {
      fprintf(f,"union {\n");
      if(!writePOVTransform(&obj->node->transform,obj->line[i].v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,obj->line[i].v2,v2,1)) continue;
      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v2,lim);
      stipple_dir[0]=v2[0]-v1[0];
      stipple_dir[1]=v2[1]-v1[1];
      stipple_dir[2]=v2[2]-v1[2];
      ll=matfCalcLen(stipple_dir);
      if(ll==0.0)
	continue;
      stipple_dir[0]/=ll;
      stipple_dir[1]/=ll;
      stipple_dir[2]/=ll;
      stipple_add1[0]=stipple_dir[0]*obj->render.stipplei;
      stipple_add1[1]=stipple_dir[1]*obj->render.stipplei;
      stipple_add1[2]=stipple_dir[2]*obj->render.stipplei;
      stipple_add2[0]=stipple_dir[0]*obj->render.stippleo;
      stipple_add2[1]=stipple_dir[1]*obj->render.stippleo;
      stipple_add2[2]=stipple_dir[2]*obj->render.stippleo;
      stipple_pos[0]=v1[0];
      stipple_pos[1]=v1[1];
      stipple_pos[2]=v1[2];
     
      while(1) {
	if(obj->render.mode==RENDER_OFF) {
	  fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%s*%.3f}\n",
		  stipple_pos[0],
		  stipple_pos[1],
		  stipple_pos[2],
		  stipple_pos[0]+stipple_add1[0],
		  stipple_pos[1]+stipple_add1[1],
		  stipple_pos[2]+stipple_add1[2],
		  lw_name,obj->line[i].lw);
	  fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s*%.3f}\n",
		  stipple_pos[0],
		  stipple_pos[1],
		  stipple_pos[2],
		  lw_name,obj->line[i].lw);
	  fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s*%.3f}\n",
		  stipple_pos[0]+stipple_add1[0],
		  stipple_pos[1]+stipple_add1[1],
		  stipple_pos[2]+stipple_add1[2],
		  lw_name,obj->line[i].lw);
	} else {
	  fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.3f}\n",
		  stipple_pos[0],
		  stipple_pos[1],
		  stipple_pos[2],
		  stipple_pos[0]+stipple_add1[0],
		  stipple_pos[1]+stipple_add1[1],
		  stipple_pos[2]+stipple_add1[2],
		  obj->line[i].r);
	  fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%.3f}\n",
		  stipple_pos[0],
		  stipple_pos[1],
		  stipple_pos[2],
		  obj->line[i].r);
	  fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%.3f}\n",
		  stipple_pos[0]+stipple_add1[0],
		  stipple_pos[1]+stipple_add1[1],
		  stipple_pos[2]+stipple_add1[2],
		  obj->line[i].r);
	}
	stipple_pos[0]+=stipple_add1[0]+stipple_add2[0];
	stipple_pos[1]+=stipple_add1[1]+stipple_add2[1];
	stipple_pos[2]+=stipple_add1[2]+stipple_add2[2];
	stipple_dir[0]=stipple_pos[0]-v1[0];
	stipple_dir[1]=stipple_pos[1]-v1[1];
	stipple_dir[2]=stipple_pos[2]-v1[2];
	if(matfCalcLen(stipple_dir)>ll)
	  break;
      }
      
    } else {
      if(!writePOVTransform(&obj->node->transform,obj->line[i].v1,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,obj->line[i].v2,v2,1)) continue;
      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v2,lim);
      
      if(obj->render.mode==RENDER_OFF) {
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%s*%.3f\n",
		v1[0],v1[1],v1[2],v2[0],v2[1],v2[2],lw_name,obj->line[i].lw);
      } else {
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.3f\n",
		v1[0],v1[1],v1[2],v2[0],v2[1],v2[2],obj->line[i].r);
	
      }
    }
    fprintf(f,"material {%s} pigment {color rgbft<%.3f,%.3f,%.3f,%s*%.3f,%s*%.3f>}",
	    tex_name, obj->line[i].c[0], 
	    obj->line[i].c[1],
	    obj->line[i].c[2], 
	    fi_name,1.0-obj->line[i].c[3],
	    tp_name,1.0-obj->line[i].c[3]);
    fprintf(f,"}\n");
  }

  fprintf(f,"}\n");  

  return 0;
}

/*
  TODO
  NORMALS NEED DIFFERENT TRANSFORMATION  !!!
*/

int writePOVTransform(transMat *m, float *v1, float *v2,int f)
{
  GLdouble x,y,z,px,py,pz;
  float tmp[3];
  double in[4],out[4];

  tmp[0]=v1[0];
  tmp[1]=v1[1];
  tmp[2]=v1[2];
  
  if(f) {
    transApplyf(m,tmp);
  } else {
    transApplyRotf(m,tmp);
  } 
   
  x=(GLdouble)tmp[0];
  y=(GLdouble)tmp[1];
  z=(GLdouble)tmp[2];
  if(f) {
    
    gluProject(x,y,z,write_pov_mm,write_pov_pm,write_pov_vp,&px,&py,&pz);
    

    if(pz<0.0)
      return 0;
    else if(pz>1.0)
      return 0;

    /****
    else if(px<write_pov_vp[0])
      return 0;
    else if(px>(write_pov_vp[2]-write_pov_vp[0]))
      return 0;
    else if(py<write_pov_vp[1])
      return 0;
    else if(py>(write_pov_vp[3]-write_pov_vp[1]))
      return 0;
    ****/
  }
  
  in[0]=x;
  in[1]=y;
  in[2]=z;
  in[3]=1.0;
  
  if(f) {
    matMultVM(in,write_pov_mm,out);
  } else {
    matMultVM(in,write_pov_rm,out);
    matNormalize(out,out);
  }
  /*
  fprintf(stderr,"\n%.3f %.3f %3f %.3f - %.3f %.3f %.3f %.3f",
	  in[0],in[1],in[2],in[3],out[0],out[1],out[2],out[3]);
  */

  if(f)
    if(out[3]==0.0)
      return 0;
  
  v2[0]=(float)(out[0]);
  v2[1]=(float)(out[1]);
  v2[2]=(float)(out[2]);
  if(f)
    v2[2]-=gfx.transform.tra[2];
  
  return 1;
}

int writePOVCheckLim(float *v, float *lim)
{
  if(v[0]>lim[0])
    lim[0]=v[0];
  if(v[1]>lim[1])
    lim[1]=v[1];
  if(v[2]>lim[2])
    lim[2]=v[2];

  if(v[0]<lim[3])
    lim[3]=v[0];
  if(v[1]<lim[4])
    lim[4]=v[1];
  if(v[2]<lim[5])
    lim[5]=v[2];

  return 0;
}

int writePOVCheckName(char *name)
{
  int c,l;

  l=strlen(name);

  for(c=0;c<l;c++)
    if(name[c]=='-')
      name[c]='_';

  return 0;
}

