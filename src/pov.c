#include <math.h>
#include <string.h>

#include "pov.h"
#include "pov_macro.h"
#include "gfx.h"
#include "mat.h"

extern struct DBM dbm;
extern struct GFX gfx;

GLdouble write_pov_mm[16],write_pov_pm[16],write_pov_rm[16];
GLint write_pov_vp[4];

int write_pov_flag, write_pov_mode, write_pov_ver;


static int dlim(float *v1, float *v2) {
  static float epsilon= 1e-5;
  return (fabs(v1[0]-v2[0])<epsilon && fabs(v1[1]-v2[1])<epsilon && fabs(v1[2]-v2[2])<epsilon);
}

/*
  Utility funtions for basic POV elements
*/

static int writePOVTransform(transMat *m, float *v1, float *v2,int f)
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

static void writePOVCheckLim(float *v, float *lim)
{
  if(v[0]>lim[0]) lim[0]=v[0];
  if(v[1]>lim[1]) lim[1]=v[1];
  if(v[2]>lim[2]) lim[2]=v[2];
  if(v[0]<lim[3]) lim[3]=v[0];
  if(v[1]<lim[4]) lim[4]=v[1];
  if(v[2]<lim[5]) lim[5]=v[2];
}

static void writePOVCheckName(char *name)
{
  int c,l;
  l=strlen(name);
  for(c=0;c<l;c++)
    if(name[c]=='-')
      name[c]='_';
}

static int writePOVGetColorCode(float r, float g, float b)
{
  int i,j,k;

  if(r<0.0) r=0.0;
  if(r>1.0) r=1.0;
  if(g<0.0) g=0.0;
  if(g>1.0) g=1.0;
  if(b<0.0) b=0.0;
  if(b>1.0) b=1.0;

  i=(int)(r*7.0);
  j=(int)(g*7.0);
  k=(int)(b*7.0);

  return i*100+j*10+k;
}

static char write_pov_mega_tex_buf[256];

static const char * writePOVTriSmoothTex(const char *o,float r, float g, float b) 
{
  char *buf=write_pov_mega_tex_buf;
  sprintf(buf,"%s_tex%03d",o,writePOVGetColorCode(r,g,b));
  return buf;
}

static void writePOVGenTriSmoothTex(FILE *f, const char *o, const char *tx, const char *tp, const char *fi) 
{
  int i,j,k;
  float r,g,b;

  for(i=0;i<8;i++) {
    r=(float)i/7.0;
    for(j=0;j<8;j++) {
      g=(float)j/7.0;
      for(k=0;k<8;k++) {
	b=(float)k/7.0;

	fprintf(f,"#declare %s = texture {%s pigment {color rgbft <%.2f,%.2f,%.2f,%s,%s>}}\n",
		writePOVTriSmoothTex(o,r,g,b),tx,r,g,b,fi,tp);
		
      }
    }
  }

  
}


static void writePOVFinish(FILE *f, float am, float di, float br, float sp, float ro) 
{
  fprintf(f,"  finish {\n");
  fprintf(f,"   ambient %.2f\n",am);
  fprintf(f,"   diffuse %.2f  brilliance %.2f\n",di,br);
  fprintf(f,"   specular %.2f  roughness %.4f\n",sp,ro);
  fprintf(f,"  }\n");
}

static int writePOVTriangle(FILE *f, transMat *transform,int mode, float ov1[3],float on1[3],float c1[3],float ov2[3], float on2[3], float c2[3],float ov3[3], float on3[3], float c3[3], const char *obj, float *lim) 
{
  float v1[4],v2[4],v3[4];
  float n1[4],n2[4],n3[4];

  char mat[64],tex[64],tp[64],fi[64];
  char tex1[64],tex2[64],tex3[64];

  sprintf(mat,"%s_mat",obj);
  sprintf(tex,"%s_tex",obj);
  sprintf(tp,"%s_tp",obj);
  sprintf(fi,"%s_fi",obj);

  if(!writePOVTransform(transform,ov1,v1,1)) return 0;
  if(!writePOVTransform(transform,ov2,v2,1)) return 0;
  if(!writePOVTransform(transform,ov3,v3,1)) return 0;
  
  if(v1[0]==v2[0] && v2[0]==v3[0])
    if(v1[1]==v2[1] && v2[1]==v3[1])
      if(v1[2]==v2[2] && v2[2]==v3[2])
	return 0;

  if(dlim(v1,v2) || dlim(v1,v3) || dlim (v2,v3)) {
    // degenerate triangle
    return 0;
  }

    /*
  if((v1[0]==v2[0] && v1[1]==v2[1] && v1[2]==v2[2]) ||
     (v3[0]==v2[0] && v3[1]==v2[1] && v3[2]==v2[2]) ||
     (v1[0]==v3[0] && v1[1]==v3[1] && v1[2]==v3[2]))
    return 0;
    */

  writePOVCheckLim(v1,lim);
  writePOVCheckLim(v2,lim);
  writePOVCheckLim(v3,lim);
  
  writePOVTransform(transform,on1,n1,0);
  writePOVTransform(transform,on2,n2,0);
  writePOVTransform(transform,on3,n3,0);
  
  switch(mode) {
  case WRITE_POV_DEFAULT:
    if(write_pov_ver==WRITE_POV_V31) {
      // old v3.1 
      fprintf(f,"smooth_triangle {");
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
      fprintf(f," material {%s} pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}",
	      mat,
	      (c1[0]+c1[0]+c2[0])/3.0,
	      (c1[1]+c1[1]+c2[1])/3.0,
	      (c1[2]+c1[2]+c2[2])/3.0,
	      fi,tp);
      fprintf(f,"}\n");
    } else if(write_pov_ver==WRITE_POV_V35) {
      // new v3.5
      fprintf(f,"smooth_triangle {");
      
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
      
      strncpy(tex1,writePOVTriSmoothTex(obj,c1[0],c1[1],c1[2]),63);
      strncpy(tex2,writePOVTriSmoothTex(obj,c2[0],c2[1],c2[2]),63);
      strncpy(tex3,writePOVTriSmoothTex(obj,c3[0],c3[1],c3[2]),63);
      fprintf(f,"texture_list { %s %s %s}\n\n",tex1,tex2,tex3);
      fprintf(f,"}\n");
    }
    break;
  case WRITE_POV_PATCH:
    fprintf(f,"smooth_color_triangle {");
    fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
	    v1[0],v1[1],v1[2],n1[0],n1[1],n1[2],c1[0],c1[1],c1[2],
	    fi,tp);
    fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
	    v2[0],v2[1],v2[2],n2[0],n2[1],n2[2],c2[0],c2[1],c2[2],
	    fi,tp);
    fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>",
	    v3[0],v3[1],v3[2],n3[0],n3[1],n3[2],c3[0],c3[1],c3[2],
	    fi,tp);
    fprintf(f,"}\n");
    break;
  case WRITE_POV_SMOOTH:
    // use smooth macro
    // version does not matter
    if(c1[0]==c2[0] && c1[1]==c2[1] && c1[2]==c2[2] &&
       c2[0]==c3[0] && c2[1]==c3[1] && c2[2]==c3[2]) {
      fprintf(f,"smooth_triangle {");
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
      fprintf(f," material {%s} pigment {color rgbft<%.3f,%.3f,%.3f,%s,%s>}",
	      mat,
	      c1[0],c1[1],c1[2],fi,tp);
      fprintf(f,"}\n");
    } else {
      // this is the macro definition
      fprintf(f,"colored_smooth_triangle (");
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
	      v1[0],v1[1],v1[2],n1[0],n1[1],n1[2],c1[0],c1[1],c1[2],
	      fi,tp);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>,",
	      v2[0],v2[1],v2[2],n2[0],n2[1],n2[2],c2[0],c2[1],c2[2],
	      fi,tp);
      fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,rgbft<%.4f,%.4f,%.4f,%s,%s>",
	      v3[0],v3[1],v3[2],n3[0],n3[1],n3[2],c3[0],c3[1],c3[2],
	      fi,tp);
      fprintf(f,")\n");
    }
    break;
  case WRITE_POV_NOCOLOR:
    // version does not matter
    fprintf(f,"smooth_triangle {");
    fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	    v1[0],v1[1],v1[2],n1[0],n1[1],n1[2]);
    fprintf(f,"<%f,%f,%f>,<%f,%f,%f>,",
	    v2[0],v2[1],v2[2],n2[0],n2[1],n2[2]);
    fprintf(f,"<%f,%f,%f>,<%f,%f,%f>",
	    v3[0],v3[1],v3[2],n3[0],n3[1],n3[2]);
    fprintf(f,"}\n");
    break;
  }
  return 1;
} 

static int writePOVCylinder(FILE *f, transMat *transform, int mode, float ov1[3], float c1[3], float ov2[3], float c2[3], char *rad, const char *mat, const char *tp, const char *fi, float *lim)
{
  float v1[4],v2[4];
  float epsilon = 1e-4;

  if(!writePOVTransform(transform,ov1,v1,1)) return 0;
  if(!writePOVTransform(transform,ov2,v2,1)) return 0;
  
  writePOVCheckLim(v1,lim);
  writePOVCheckLim(v2,lim);

  // check for deprecated cylinder
  if(dlim(v1,v2)) {
    return 0;
  }

  if(mode==WRITE_POV_NOCOLOR) {
    fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%s",
	    v1[0],v1[1],v1[2],
	    v2[0],v2[1],v2[2],
	    rad);
    fprintf(f," material {%s}",mat);
    fprintf(f,"}\n");
  } else {
    // use macro color_cylinder
    fprintf(f,"color_cylinder (<%.4f,%.4f,%.4f>,<%.3f,%.3f,%.3f>,<%.4f,%.4f,%.4f>,<%.3f,%.3f,%.3f>,%s)\n",
	    v1[0],v1[1],v1[2],c1[0],c1[1],c1[2],
	    v2[0],v2[1],v2[2],c2[0],c2[1],c2[2],
	    rad);
  }
  return 1;
}

static int writePOVSphere(FILE *f, transMat *transform, int mode, float v1[3],float c1[3], char *rad, const char *mat, const char *tp, const char *fi, float *lim, float frad)
{
  float v2[4];
  if(!writePOVTransform(transform,v1,v2,1)) return 0;
  writePOVCheckLim(v2,lim);

  if(frad>0.0) {
    fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s*%f",
	    v2[0],v2[1],v2[2], rad, frad);
  } else {
    fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s ",
	    v2[0],v2[1],v2[2], rad);
  }
  if(write_pov_mode==WRITE_POV_NOCOLOR) {
    fprintf(f,"material {%s}",mat);
  } else {
    fprintf(f,"material {%s} pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}",
	    mat,
	    c1[0],c1[1],c1[2],
	    fi,tp);
  }
  fprintf(f,"}\n");

  return 1;
}

// vertex array output
static int write_pov_va(FILE *f, cgfxVA *va, int mode, char *mat_name, char *obj_name,transMat *transform, float *lim)
{
  int i;
  if(mode==WRITE_VA_TRIS) {
    // prepare the output
    if(write_pov_mode==WRITE_POV_DEFAULT) {
      if(write_pov_ver==WRITE_POV_V31) {
	fprintf(f,"union {\n");
      } else if(write_pov_ver==WRITE_POV_V35) {
	// probably this should be mesh2
	fprintf(f,"mesh {\n");
      }
    } else if(write_pov_mode==WRITE_POV_PATCH) {
      if(write_pov_ver==WRITE_POV_V31) {
	fprintf(f,"mesh {\n");
      } else if(write_pov_ver==WRITE_POV_V35) {
	// TODO PATCHED v35
      }
    } else if(write_pov_mode==WRITE_POV_SMOOTH) {
      // using a macro, so version doesn't matter
      fprintf(f,"union {\n");
      fprintf(f,"#declare triangle_base_texture = material {%s}\n",mat_name);
    } else {
      // no-color version
      fprintf(f,"mesh {\n");
    }

    for(i=0;i<va->count;i+=3) {
      writePOVTriangle(f,transform,write_pov_mode,
		       va->p[i+0].v,va->p[i+0].n,va->p[i+0].c,
		       va->p[i+1].v,va->p[i+1].n,va->p[i+1].c,
		       va->p[i+2].v,va->p[i+2].n,va->p[i+2].c,
		       obj_name, lim);
    }

    if(write_pov_mode==WRITE_POV_PATCH ||
       write_pov_mode==WRITE_POV_NOCOLOR) {
      fprintf(f,"material {%s}\n",mat_name);
    }
    fprintf(f,"}\n");
  }
  return 1;
}

/*
  output funtions for the 5 datasets
*/

static int writePOVStructObj(FILE *f, structObj *obj, int k,float *lim)
{
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float lw;
  int i;
  float def_amb,def_diff,def_bri,def_spec,def_rough;
  char mat_name[128],tex_name[64],def_name[64],fi_name[64],tp_name[64];
  char lw_name[128],ps_name[128];
  char obj_name[128];

  def_amb=0.1;
  def_diff=0.7;
  def_bri=1.0;
  def_spec=0.3;
  def_rough=0.005;

  lw=obj->render.line_width*0.05;
  t=obj->render.transparency;

  sprintf(mat_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(mat_name);
  sprintf(tex_name,"_%s_%s_tex",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(obj_name,"_%s_%s",obj->node->name,obj->name);
  writePOVCheckName(obj_name);

  if(k==0) {
    fprintf(f,"// object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = 1;\n",obj_name);
    fprintf(f,"#declare %s = texture {\n",tex_name);
    if(write_pov_mode==WRITE_POV_NOCOLOR) {
      fprintf(f,"  pigment {color rgb 1}\n");
    } else {
      fprintf(f,"  pigment {color rgbft <0,0,0,1,1>}\n");
    }
    fprintf(f,"  normal {granite 0.0 scale 1.0}\n");
    writePOVFinish(f,def_amb,def_diff,def_bri,def_spec,def_rough);
    fprintf(f," }\n");
    fprintf(f,"#declare %s = material {texture {%s}}\n",mat_name,tex_name);
    fprintf(f,"#declare %s = %.2f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.2f;\n\n",fi_name,0.0);
    return 0;
  }

  fprintf(f,"#if (%s)\n",obj_name);

  if(write_pov_ver==WRITE_POV_V35 && write_pov_mode==WRITE_POV_DEFAULT) {
    // prepare the smoothing textures to use
    writePOVGenTriSmoothTex(f,obj_name,tex_name,tp_name,fi_name);
  }

  switch(obj->render.mode) {
  case RENDER_SLINE:
    fprintf(f,"union {\n");
    for(i=0;i<obj->va.count;i+=2) {
      sprintf(lw_name,"%.4f",lw);
      writePOVCylinder(f,&obj->node->transform,write_pov_mode,
		       obj->va.p[i].v,obj->va.p[i].c,
		       obj->va.p[i+1].v,obj->va.p[i+1].c,
		       lw_name,
		       mat_name,tp_name,fi_name,lim);

    }
    fprintf(f,"}\n");
    break;
  case RENDER_TUBE:
  case RENDER_HSC:

    write_pov_va(f,&obj->va,WRITE_VA_TRIS, mat_name,obj_name,&obj->node->transform,lim);

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
	fprintf(f," material {%s}}\n",mat_name);
	fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f",
		v2[0],v2[1],v2[2],v3[0],v3[1],v3[2],lw);
	fprintf(f," material {%s}}\n",mat_name);
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
    fprintf(f,"}\n");
    break;
  case RENDER_CPK:
    fprintf(f,"union {\n");
    for(i=0;i<obj->atom_count;i++) {
      v1[0]=obj->atom[i].ap->p->x;
      v1[1]=obj->atom[i].ap->p->y;
      v1[2]=obj->atom[i].ap->p->z;

      n1[0]=obj->atom[i].prop.r;
      n1[1]=obj->atom[i].prop.g;
      n1[2]=obj->atom[i].prop.b;
      sprintf(ps_name,"%.4f",obj->atom[i].prop.radius);
      writePOVSphere(f,&obj->node->transform,write_pov_mode,v1,n1,
		     ps_name,
		     mat_name,tp_name,fi_name,lim,1.0);
    }
    fprintf(f,"}\n");
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
	fprintf(f," material {%s}}\n",mat_name);
      } else {
	fprintf(f," material {%s} pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		mat_name,
		obj->bond[i].prop1->r,
		obj->bond[i].prop1->g,
		obj->bond[i].prop1->b,
		fi_name,tp_name);
      }
      fprintf(f,"cylinder {<%.4f,%.4f,%.4f>,<%.4f,%.4f,%.4f>,%.4f ",
	      v2[0],v2[1],v2[2],v3[0],v3[1],v3[2],obj->render.bond_width);
      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f," material {%s}}\n",mat_name);
      } else {
	fprintf(f," material {%s} pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		mat_name,
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
	fprintf(f," material {%s}}\n",mat_name);
      } else {
	fprintf(f,"material {%s} pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
		mat_name,
		obj->atom[i].prop.r,
		obj->atom[i].prop.g,
		obj->atom[i].prop.b,
		fi_name,tp_name);
      }
    }
    fprintf(f,"}\n");
    break;
  }


  fprintf(f,"#end\n");

  return 0;
}


static int writePOVSurfObj(FILE *f, surfObj *obj, int k,float *lim)
{
  int i,p1,p2,p3;
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float def_amb,def_diff,def_spec,def_rough,def_bri;
  char mat_name[64],tex_name[64],tp_name[64],fi_name[64],obj_name[64];

  sprintf(mat_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(mat_name);
  sprintf(tex_name,"_%s_%s_tex",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(obj_name,"_%s_%s",obj->node->name,obj->name);
  writePOVCheckName(obj_name);

  def_amb=0.10;
  def_diff=0.70;
  def_spec=0.00;
  def_rough=0.01 ;
  def_bri=1.0;

  if(k==0) {
    fprintf(f,"// object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = 1;\n",obj_name);
    
    fprintf(f,"#declare %s = texture {\n",tex_name);
    if(write_pov_mode==WRITE_POV_NOCOLOR)
      fprintf(f,"  pigment {color rgb 1}\n");
    else
      fprintf(f,"  pigment {color rgbft <0,0,0,1,1>}\n");
    fprintf(f,"  normal {granite 0.0 scale 1.0}\n");
    writePOVFinish(f,def_amb,def_diff,def_bri,def_spec,def_rough);
    fprintf(f,"}\n");
    fprintf(f,"#declare %s = material { texture {%s}}\n",mat_name,tex_name);
    fprintf(f,"#declare %s = %.2f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.2f;\n\n",fi_name,0.0);
    return 0;
  }

  fprintf(f,"#if (%s)\n",obj_name);

  if(write_pov_ver==WRITE_POV_V35 && write_pov_mode==WRITE_POV_DEFAULT) {
    // prepare the smoothing textures to use
    writePOVGenTriSmoothTex(f,obj_name,tex_name,tp_name,fi_name);
  }

  // copied from write_pov_va BAD BAD STYLE
  // prepare the output
  if(write_pov_mode==WRITE_POV_DEFAULT) {
    if(write_pov_ver==WRITE_POV_V31) {
      fprintf(f,"union {\n");
    } else if(write_pov_ver==WRITE_POV_V35) {
      // probably this should be mesh2
      fprintf(f,"mesh {\n");
    }
  } else if(write_pov_mode==WRITE_POV_PATCH) {
    if(write_pov_ver==WRITE_POV_V31) {
      fprintf(f,"mesh {\n");
    } else if(write_pov_ver==WRITE_POV_V35) {
      // TODO PATCHED v35
    }
  } else if(write_pov_mode==WRITE_POV_SMOOTH) {
    // using a macro, so this doesn't matter
    fprintf(f,"union {\n");
    fprintf(f,"#declare triangle_base_texture = material {%s}\n",mat_name);
  } else {
    // no-color version
    fprintf(f,"mesh {\n");
  }
  

  for(i=0;i<obj->facec;i++) {
    p1=obj->face[i*3+0];
    p2=obj->face[i*3+1];
    p3=obj->face[i*3+2];

    writePOVTriangle(f,&obj->node->transform,write_pov_mode,
		     obj->vert[p1].p,obj->vert[p1].n,obj->vert[p1].c,
		     obj->vert[p2].p,obj->vert[p2].n,obj->vert[p2].c,
		     obj->vert[p3].p,obj->vert[p3].n,obj->vert[p3].c,
		     obj_name,lim);

  }

  if(write_pov_mode==WRITE_POV_PATCH ||
     write_pov_mode==WRITE_POV_NOCOLOR) {
    fprintf(f,"material {%s}\n",mat_name);
  }
  fprintf(f,"}\n");

  fprintf(f,"#end\n");

  return 0;
}

static int writePOVScalObj(FILE *f, scalObj *obj, int k,float *lim)
{
  int i,p1,p2,p3,m;
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float def_amb,def_diff,def_spec,def_rough,def_bri;
  char mat_name[64],tex_name[64];
  char tp_name[64],lw_name[64],ps_name[64],fi_name[64];
  char obj_name[64];
  char pstmp[64];
  float lw,ps,rad;

  sprintf(mat_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(mat_name);
  sprintf(tex_name,"_%s_%s_tex",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(lw_name,"_%s_%s_lw",obj->node->name,obj->name);
  writePOVCheckName(lw_name);
  sprintf(ps_name,"_%s_%s_ps",obj->node->name,obj->name);
  writePOVCheckName(ps_name);
  sprintf(obj_name,"_%s_%s",obj->node->name,obj->name);
  writePOVCheckName(obj_name);

  def_amb=0.2;
  def_diff=0.8;
  def_bri=1.0;
  def_spec=0.0;
  def_rough=0.05;

  lw=obj->render.line_width*0.05;
  ps=obj->render.point_size*0.05;

  if(k==0) {
    fprintf(f,"// object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = 1;\n",obj_name);
    fprintf(f,"#declare %s = texture {\n",tex_name);

#ifndef CONTOUR_COLOR
    if(obj->type==SCAL_CONTOUR) 
      fprintf(f,"  pigment {color rgbft <%.3f,%.3f,%.3f,%.3f,%.3f>}\n",
	      obj->r,obj->g,obj->b,0.0,1.0-obj->render.transparency);
    else
#endif
      fprintf(f,"  pigment {color rgbft <0,0,0,1,1>}\n");

    fprintf(f,"  normal {granite 0.0 scale 1}\n");
    writePOVFinish(f,def_amb,def_diff,def_bri,def_spec,def_rough);
    fprintf(f," }\n");
    fprintf(f,"#declare %s = material {texture {%s}}\n",mat_name,tex_name);
#ifndef CONTOUR_COLOR
    if(obj->type!=SCAL_CONTOUR) {
#endif
      fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
      fprintf(f,"#declare %s = %.4f;\n",fi_name,0.0);
#ifndef CONTOUR_COLOR
    }
#endif
    if(obj->type==SCAL_CONTOUR && obj->render.mode==RENDER_LINE)
      fprintf(f,"#declare %s = %.4f;\n",lw_name,lw);
    if((obj->type==SCAL_CONTOUR && obj->render.mode==RENDER_POINT) ||
       (obj->type==SCAL_GRID && obj->render.mode==RENDER_OFF)) {
      fprintf(f,"#declare %s = %.4f;\n",ps_name,ps);
    } else if(obj->type==SCAL_GRID && obj->render.mode==RENDER_ON) {
      fprintf(f,"#declare %s = %.4f;\n",ps_name,1.0);
    }


    return 0;
  }


  fprintf(f,"#if (%s)\n",obj_name);

  switch(obj->type) {
  case SCAL_CONTOUR:
    switch(obj->render.mode) {
    case RENDER_POINT:
      fprintf(f,"union {\n");
#ifndef CONTOUR_COLOR
      n1[0]=obj->r;
      n1[1]=obj->g;
      n1[2]=obj->b;
      m=WRITE_POV_NOCOLOR;
#else
      m=0;
#endif
      for(i=0;i<obj->point_count;i++) {
#ifdef CONTOUR_COLOR
	n1[0]=obj->point[i].c[0];
	n1[1]=obj->point[i].c[1];
	n1[2]=obj->point[i].c[2];
#endif
	writePOVSphere(f,&obj->node->transform,m,
		       obj->point[i].v,n1,ps_name,
		       mat_name,tp_name,fi_name,lim,1.0);
	
      }
      fprintf(f,"material {%s}\n",mat_name);
      fprintf(f,"}\n");
      break;

    case RENDER_LINE:
      fprintf(f,"union {\n");
#ifndef CONTOUR_COLOR
      n2[0]=n1[0]=obj->r;
      n2[1]=n1[1]=obj->g;
      n2[2]=n1[2]=obj->b;
      m=WRITE_POV_NOCOLOR;
#else
      m=0;
#endif
      for(i=0;i<obj->line_count;i++) {
#ifdef CONTOUR_COLOR
	n1[0]=obj->point[obj->line[i].pi1].c[0];
	n1[1]=obj->point[obj->line[i].pi1].c[1];
	n1[2]=obj->point[obj->line[i].pi1].c[2];
	n2[0]=obj->point[obj->line[i].pi0].c[0];
	n2[1]=obj->point[obj->line[i].pi0].c[1];
	n2[2]=obj->point[obj->line[i].pi0].c[2];
#endif
	writePOVCylinder(f,&obj->node->transform, m,
			 obj->point[obj->line[i].pi1].v,n1,
			 obj->point[obj->line[i].pi0].v,n2,
			 lw_name, mat_name, tp_name, fi_name, lim);

      } 
      fprintf(f,"material {%s}\n",mat_name);
      fprintf(f,"}\n");
      
      break;
    case RENDER_SURFACE:
#ifndef CONTOUR_COLOR
      n3[0]=n2[0]=n1[0]=obj->r;
      n3[1]=n2[1]=n1[1]=obj->g;
      n3[2]=n2[2]=n1[2]=obj->b;
      m=WRITE_POV_NOCOLOR;
      fprintf(f,"mesh{\n");
#else
      // copied from write_pov_va BAD BAD STYLE
      // prepare the output
      if(write_pov_mode==WRITE_POV_DEFAULT) {
	if(write_pov_ver==WRITE_POV_V31) {
	  fprintf(f,"union {\n");
	} else if(write_pov_ver==WRITE_POV_V35) {
	  // probably this should be mesh2
	  fprintf(f,"mesh {\n");
	}
      } else if(write_pov_mode==WRITE_POV_PATCH) {
	if(write_pov_ver==WRITE_POV_V31) {
	  fprintf(f,"mesh {\n");
	} else if(write_pov_ver==WRITE_POV_V35) {
	  // TODO PATCHED v35
	}
      } else if(write_pov_mode==WRITE_POV_SMOOTH) {
	// using a macro, so this doesn't matter
	fprintf(f,"union {\n");
	fprintf(f,"#declare triangle_base_texture = material {%s}\n",mat_name);
      } else {
	// no-color version
	fprintf(f,"mesh {\n");
      }

      m=write_pov_mode;
#endif
      for(i=0;i<obj->face_count;i++) {
#ifdef CONTOUR_COLOR
	n1[0]=obj->face[i].c1[0];
	n1[1]=obj->face[i].c1[1];
	n1[2]=obj->face[i].c1[2];
	n2[0]=obj->face[i].c2[0];
	n2[1]=obj->face[i].c2[1];
	n2[2]=obj->face[i].c2[2];
	n3[0]=obj->face[i].c3[0];
	n3[1]=obj->face[i].c3[1];
	n3[2]=obj->face[i].c3[2];
#endif
	writePOVTriangle(f,&obj->node->transform,m,
			 obj->face[i].v1,obj->face[i].n1,n1,
			 obj->face[i].v2,obj->face[i].n2,n2,
			 obj->face[i].v3,obj->face[i].n3,n3,
			 obj_name,lim);
      }
      fprintf(f,"material {%s}\n",mat_name);
      fprintf(f,"}\n");
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
	fprintf(f,"sphere {<%.4f,%.4f,%.4f>,%s*%f ",
		v1[0],v1[1],v1[2], ps_name, obj->point[i].rad);
      } else {
	rad=ps;
	fprintf(f,"disc {<%.4f,%.4f,%.4f>, <0,0,1>, %s ",
		v1[0],v1[1],v1[2], ps_name);
      }
      fprintf(f,"material { %s } pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}\n",
	      mat_name,
	      obj->point[i].c[0],
	      obj->point[i].c[1],
	      obj->point[i].c[2],
	      tp_name,
	      fi_name);
      
    }
    fprintf(f,"}\n");
    break;
  }
  

  fprintf(f,"#end\n");

  return 0;
}

static int writePOVGridObj(FILE *f, gridObj *obj, int k,float *lim)
{
  int i;
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float def_amb,def_diff,def_spec,def_rough,def_bri;
  char mat_name[64],tp_name[64],fi_name[64],obj_name[64],lw_name[64];
  char tex_name[64];
  float lw;

  lw=obj->render.line_width*0.05;

  sprintf(mat_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(mat_name);
  sprintf(tex_name,"_%s_%s_tex",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(lw_name,"_%s_%s_lw",obj->node->name,obj->name);
  writePOVCheckName(lw_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(obj_name,"_%s_%s",obj->node->name, obj->name);
  writePOVCheckName(obj_name);

  def_amb=0.1;
  def_diff=0.7;
  def_spec=0.2;
  def_rough=0.1;
  def_bri=1.0;

  if(k==0) {
    fprintf(f,"// object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = 1;\n",obj_name);
    fprintf(f,"#declare %s = texture {\n",tex_name);
    if(write_pov_mode==WRITE_POV_NOCOLOR)
      fprintf(f,"  pigment {color rgb 1}\n");
    else
      fprintf(f,"  pigment {color rgbft <0,0,0,1,1>}\n");
    fprintf(f,"  normal {granite 0.0 scale 1}\n");
    writePOVFinish(f,def_amb,def_diff,def_bri,def_spec,def_rough);
    fprintf(f," }\n");
    fprintf(f,"#declare %s = material {texture {%s}}\n",mat_name,tex_name);
    fprintf(f,"#declare %s = %.4f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.4f;\n",fi_name,0.0);
    if(obj->type==GRID_CONTOUR) {
      fprintf(f,"#declare %s = %.4f;\n",lw_name,lw);
    }
    return 0;
  }


  fprintf(f,"#if (%s)\n",obj_name);

  if(write_pov_ver==WRITE_POV_V35 && write_pov_mode==WRITE_POV_DEFAULT) {
    // prepare the smoothing textures to use
    writePOVGenTriSmoothTex(f,obj_name,tex_name,tp_name,fi_name);
  }

  switch (obj->type) {
  case GRID_SURFACE:

    // copied from write_pov_va BAD BAD STYLE
    // prepare the output
    if(write_pov_mode==WRITE_POV_DEFAULT) {
      if(write_pov_ver==WRITE_POV_V31) {
	fprintf(f,"union {\n");
      } else if(write_pov_ver==WRITE_POV_V35) {
	// probably this should be mesh2
	fprintf(f,"mesh {\n");
      }
    } else if(write_pov_mode==WRITE_POV_PATCH) {
      if(write_pov_ver==WRITE_POV_V31) {
	fprintf(f,"mesh {\n");
      } else if(write_pov_ver==WRITE_POV_V35) {
	// TODO PATCHED v35
      }
    } else if(write_pov_mode==WRITE_POV_SMOOTH) {
      // using a macro, so this doesn't matter
      fprintf(f,"union {\n");
      fprintf(f,"#declare triangle_base_texture = material {%s}\n",mat_name);
    } else {
      // no-color version
      fprintf(f,"mesh {\n");
    }

    for(i=0;i<obj->facec;i++) {
      writePOVTriangle(f,&obj->node->transform,write_pov_mode,
		       obj->face[i].v1,obj->face[i].n1,obj->face[i].c1,
		       obj->face[i].v2,obj->face[i].n2,obj->face[i].c2,
		       obj->face[i].v3,obj->face[i].n3,obj->face[i].c3,
		       obj_name,lim);
    }

    if(write_pov_mode==WRITE_POV_DEFAULT ||
       write_pov_mode==WRITE_POV_NOCOLOR ||
       write_pov_mode==WRITE_POV_PATCH) {
      fprintf(f,"material {%s}\n",mat_name);
    }
    fprintf(f,"}\n");

    break;
  case GRID_CONTOUR:
    fprintf(f,"// .%s.%s\n",obj->node->name,obj->name);
    fprintf(f,"union {\n");
    for(i=0;i<obj->vertc;i+=2) {
      
      if(!writePOVTransform(&obj->node->transform,obj->vert[i+0].v,v1,1)) continue;
      if(!writePOVTransform(&obj->node->transform,obj->vert[i+1].v,v2,1)) continue;
      writePOVCheckLim(v1,lim);
      writePOVCheckLim(v2,lim);
      
      if(v1[0]==v2[0] && v1[1]==v2[1] && v1[2]==v2[2])
	continue;
      fprintf(f,"cylinder {");
      fprintf(f,"<%.4f,%.4f,%.4f>,<%4f,%4f,%4f> %s ",
	      v1[0],v1[1],v1[2],
	      v2[0],v2[1],v2[2],
	      lw_name);
      if(write_pov_mode==WRITE_POV_NOCOLOR) {
	fprintf(f,"material {%s}",mat_name);
      } else {
	fprintf(f,"material {%s texture {pigment {color rgbft <%.3f,%.3f,%.3f,%s,%s>}}}\n",
		mat_name,
		(obj->vert[i+0].c[0]+obj->vert[i+1].c[0])*0.5,
		(obj->vert[i+0].c[1]+obj->vert[i+1].c[1])*0.5,
		(obj->vert[i+0].c[2]+obj->vert[i+1].c[2])*0.5,
		fi_name,tp_name);
      }
      fprintf(f,"}\n");
    }

    
    fprintf(f,"}\n");
    break;
  }

  fprintf(f,"#end\n");

  return 0;
}

static int writePOVGeomObj(FILE *f, geomObj *obj, int k,float *lim)
{
  float v1[4],v2[4],v3[4],n1[4],n2[4],n3[4],*c1,*c2,*c3,t;
  float lw,ll;
  int i;
  float def_amb,def_diff,def_bri,def_spec,def_rough;
  char tex_name[64],mat_name[64],obj_name[64];
  char def_name[64],tp_name[64],fi_name[64],lw_name[64],ps_name[64];
  float stipple_add1[3],stipple_add2[3],stipple_dir[3],stipple_pos[3];

  def_amb = 0.1;
  def_diff = 0.7;
  def_spec = 0.3;
  def_bri = 1.0;
  def_rough = 0.05;

  sprintf(mat_name,"_%s_%s_mat",obj->node->name,obj->name);
  writePOVCheckName(mat_name);
  sprintf(tex_name,"_%s_%s_tex",obj->node->name,obj->name);
  writePOVCheckName(tex_name);
  sprintf(obj_name,"_%s_%s",obj->node->name,obj->name);
  writePOVCheckName(obj_name);
  sprintf(tp_name,"_%s_%s_tp",obj->node->name,obj->name);
  writePOVCheckName(tp_name);
  sprintf(fi_name,"_%s_%s_fi",obj->node->name,obj->name);
  writePOVCheckName(fi_name);
  sprintf(lw_name,"_%s_%s_lw",obj->node->name,obj->name);
  writePOVCheckName(lw_name);
  sprintf(ps_name,"_%s_%s_ps",obj->node->name,obj->name);
  writePOVCheckName(ps_name);


  if(k==0) {
    fprintf(f,"// object .%s.%s\n",obj->node->name, obj->name);
    fprintf(f,"#declare %s = 1;\n",obj_name);
    fprintf(f,"#declare %s = texture {\n",tex_name);
    if(write_pov_mode==WRITE_POV_NOCOLOR)
      fprintf(f,"  pigment {color rgb 1}\n");
    else
      fprintf(f,"  pigment {color rgbft <0,0,0,1,1>}\n");
    fprintf(f,"  normal {granite 0.0 scale 1}\n");
    writePOVFinish(f,def_amb,def_diff,def_bri,def_spec,def_rough);
    fprintf(f," }\n");
    fprintf(f,"#declare %s = material {texture {%s}}\n",mat_name,tex_name);
    fprintf(f,"#declare %s = %.2f;\n",tp_name,1.0-obj->render.transparency);
    fprintf(f,"#declare %s = %.2f;\n",fi_name,0.0);
    fprintf(f,"#declare %s = %.3f;\n",lw_name,0.05*obj->render.line_width);
    fprintf(f,"#declare %s = %.3f;\n",ps_name,0.05*obj->render.point_size);

    return 0;
  }

  fprintf(f,"#if (%s)\n",obj_name);
  if(obj->render.mode==RENDER_TUBE) {

    if(write_pov_ver==WRITE_POV_V35) {
      writePOVGenTriSmoothTex(f,obj_name,tex_name,tp_name,fi_name);
    }

    write_pov_va(f,&obj->va,WRITE_VA_TRIS, mat_name,obj_name,&obj->node->transform,lim);

  } else {
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
	      mat_name, obj->point[i].c[0], 
	      obj->point[i].c[1],
	      obj->point[i].c[2], 
	      fi_name,1.0-obj->point[i].c[3],
	      tp_name,1.0-obj->point[i].c[3]);
      fprintf(f,"}\n");
    }
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
	    mat_name, obj->line[i].c[0], 
	    obj->line[i].c[1],
	    obj->line[i].c[2], 
	    fi_name,1.0-obj->line[i].c[3],
	    tp_name,1.0-obj->line[i].c[3]);
    fprintf(f,"}\n");
  }

  fprintf(f,"}\n");  

  fprintf(f,"#end\n");

  return 0;
}


static int writePOVScene(FILE *f)
{
  double cpos[4];

  if(write_pov_ver==WRITE_POV_V31) {
    fprintf(f,"#version 3.1;\n");
  } else if(write_pov_ver==WRITE_POV_V35) {
    fprintf(f,"#version 3.5;\n");
  }

  fprintf(f,"background {color rgb <%.3f,%.3f,%.3f>}\n\n\n",
	  gfx.r,gfx.g,gfx.b);

  fprintf(f,"camera {\n perspective\n location <0,0,0>\n direction <0,0,-1>\n up <0,1,0>\n right <1,0,0>\n");
  fprintf(f," angle %.4f\n",gfx.fovy);
  fprintf(f," translate <0,0,%.4f>\n",-gfx.transform.tra[2]);
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
    fprintf(f,"// depth effect\n\n");
    fprintf(f,"plane {z, %.3f texture {pigment {color rgbft <0,0,0,1,1>}}\n",
	    -gfx.transform.tra[2]-gfx.transform.slabn);
    fprintf(f,"hollow interior {fade_power 2 fade_distance %.3f}\n}\n",
	    (gfx.transform.slabf-gfx.transform.slabn)*0.5);
  } else {
    fprintf(f,"// uncomment for depth effect \n\n");
    fprintf(f,"//plane {z, %.3f texture {pigment {color rgbft <0,0,0,1,1>}}\n",
	    -gfx.transform.tra[2]-gfx.transform.slabn);
    fprintf(f,"//hollow interior {fade_power 2 fade_distance %.3f}\n//}\n",
	    (gfx.transform.slabf-gfx.transform.slabn)*0.5);

  }


  return 0;
}

/*
  main funtion called from outside
*/

int writePOV(FILE *fpov, FILE *finc,char *fpovn, int flag, int mode, int version)
{
  int i,j,k;
  FILE *f;
  int tri_flag=1,cyl_flag=1;
  float lim[6];

  write_pov_flag=flag;
  write_pov_mode=mode;
  write_pov_ver=version;
  
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
  fprintf(fpov,"  This is the main file containing\n");
  fprintf(fpov,"  the user editable parameters\n*/\n\n");

  writePOVScene(fpov);


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
      fprintf(f,"\n#declare _Scene = union{#include \"%s\"}\n",fpovn);
    }

    if(k==1) {
      fprintf(fpov,"// bounding box\n\n");
      fprintf(fpov,"// %.3f %.3f %.3f  %.3f %.3f %.3f\n\n",
	      lim[3]-1.0,lim[4]-1.0,lim[5]-1.0,
	      lim[0]+1.0,lim[1]+1.0,lim[2]+1.0);
    }
  }

  fprintf(fpov,"object {\n    _Scene\n}\n");
  
  return 0;
}
