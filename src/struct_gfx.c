#include <stdio.h>

#ifdef USE_MESA
#include <MesaGL/glx.h>
#include <MesaGL/glu.h>
#include <MesaGL/gl.h>
#else
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include "struct_db.h"
#include "gfx.h"
#include "glf.h"
#include "cgfx.h"
#include "com.h"

extern struct GFX gfx;

#define CUSTOM_NEW

int structDraw(dbmStructNode *node, int f)
{
  int j;
  structObj *obj;
  //double c[3];

  glPushMatrix();

  // apply ds transform

  //comGetCurrentCenter(c);

  /* draw cell if requested */
  if(node->show_cell && node->xtal!=NULL) {
    gfxDrawCell(node->xtal);
  }

  /*
    first draw objects with transform
  */

  for(j=0;j<node->obj_max;j++)
    if(node->obj_flag[j]!=0)
      if(node->obj[j].render.show)
	if(node->obj[j].transform_flag) {
	  obj=&node->obj[j];
	  /*
	    apply obj transform BEFORE dataset transform
	  */

	  glPushMatrix();
	  glTranslated(obj->transform.cen[0],
		       obj->transform.cen[1],
		       obj->transform.cen[2]);
	  
	  glTranslated(obj->transform.tra[0],
		       obj->transform.tra[1],
		       obj->transform.tra[2]);
	  
	  glMultMatrixd(obj->transform.rot);
	  
	  glTranslated(-obj->transform.cen[0],
		       -obj->transform.cen[1],
		       -obj->transform.cen[2]);
	  
	  glTranslated(node->transform.cen[0],
		       node->transform.cen[1],
		       node->transform.cen[2]);
	  
	  glTranslated(node->transform.tra[0],
		       node->transform.tra[1],
		       node->transform.tra[2]);
	  
	  glMultMatrixd(node->transform.rot);
	  
	  glTranslated(-node->transform.cen[0],
		       -node->transform.cen[1],
		       -node->transform.cen[2]);

	  if((f==0 && node->obj[j].render.transparency==1.0) ||
	     (f!=0 && node->obj[j].render.transparency<1.0)) {
	    structDrawObj(&node->obj[j]);
	  }

	  glPopMatrix();
	  
	}
  
  
  glTranslated(node->transform.cen[0],
	       node->transform.cen[1],
	       node->transform.cen[2]);

  glTranslated(node->transform.tra[0],
	       node->transform.tra[1],
	       node->transform.tra[2]);

  glMultMatrixd(node->transform.rot);

  glTranslated(-node->transform.cen[0],
	       -node->transform.cen[1],
	       -node->transform.cen[2]);

  if(f==0) {
    for(j=0;j<node->obj_max;j++)
      if(node->obj_flag[j]!=0)
	if(node->obj[j].render.show)
	  if(!node->obj[j].transform_flag)
	    if(node->obj[j].render.transparency==1.0)
	      structDrawObj(&node->obj[j]);
  } else {
    for(j=0;j<node->obj_max;j++)
      if(node->obj_flag[j]!=0)
	if(node->obj[j].render.show)
	  if(!node->obj[j].transform_flag)
	    if(node->obj[j].render.transparency<1.0)
	      structDrawObj(&node->obj[j]);
  }

  glPopMatrix();

  return 0;
}

int structDrawObj(structObj *obj)
{
  int i,j;
  float p0[3],p1[3],p2[3];
  float bl2;
  int detail,cf;
  float bond_width,sphere_radius;
  char label[256];
  float sd=0.2;
  int cyl_type;


  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);

  detail=obj->render.detail;
  bond_width=obj->render.bond_width;
  sphere_radius=obj->render.sphere_radius;
  
  if(obj->render.nice) {
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DITHER);
  } else {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_DITHER);
  }

  glLineWidth(obj->render.line_width);
  glPointSize(obj->render.point_size);
  if(obj->render.stipple_flag) {
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(10,0xAAAA);
  }


  if(obj->r==-1.0) {
    cf=1;
  } else {
    cf=0;
    glColor3d(obj->r, obj->g, obj->b);
  }

  switch(obj->render.mode) {
  case RENDER_SLINE:
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3,GL_FLOAT,sizeof(cgfxVAField),obj->va.p[0].v);
    glColorPointer(3,GL_FLOAT,sizeof(cgfxVAField),obj->va.p[0].c);
    
#ifdef SUN
     glBegin(GL_LINES);
     for(i=0;i<obj->va.count;i++) {
	glArrayElement(i);
     }
     glEnd();
#else
    glDrawArrays(GL_LINES,0,obj->va.count);
#endif

    glDisable(GL_VERTEX_ARRAY);
    glDisable(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    break;
  case RENDER_TUBE:
  case RENDER_HELIX:
  case RENDER_STRAND:
  case RENDER_STRAND2:
  case RENDER_CYLINDER:
  case RENDER_HSC:
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    //    if(obj->render.transparency<1.0)
    glEnable(GL_CULL_FACE);

    if(cf)
      glColor4f(1.0,1.0,1.0,0.1);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

//    glEnable(GL_VERTEX_ARRAY);
//    glEnable(GL_NORMAL_ARRAY);
//    glEnable(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_BLEND);

    glVertexPointer(3,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].v[0]);
    glNormalPointer(GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].n[0]);
    if(obj->render.transparency<1.0)
      glColorPointer(4,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].c[0]);
    else
      glColorPointer(3,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].c[0]);
    
#ifdef SUN
     glBegin(GL_TRIANGLES);
     for(i=0;i<obj->va.count;i++) {
	glArrayElement(i);
     }
     glEnd();
#else
    glDrawArrays(GL_TRIANGLES,0,obj->va.count);
#endif

//    glDisable(GL_VERTEX_ARRAY);
//    glDisable(GL_NORMAL_ARRAY);
//    glDisable(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    break;
  case RENDER_SIMPLE:
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glBegin(GL_LINES);
    for(i=0;i<obj->bond_count;i++) {
      p0[0]=obj->bond[i].atom1->p->x;
      p0[1]=obj->bond[i].atom1->p->y;
      p0[2]=obj->bond[i].atom1->p->z;
      p2[0]=obj->bond[i].atom2->p->x;
      p2[1]=obj->bond[i].atom2->p->y;
      p2[2]=obj->bond[i].atom2->p->z;
      p1[0]=(p2[0]-p0[0])*0.50+p0[0];
      p1[1]=(p2[1]-p0[1])*0.50+p0[1];
      p1[2]=(p2[2]-p0[2])*0.50+p0[2];
      if(cf)
	glColor3f(obj->bond[i].prop1->r,
		  obj->bond[i].prop1->g,
		  obj->bond[i].prop1->b);
      glVertex3fv(p0);
      glVertex3fv(p1);
      if(cf)
	glColor3f(obj->bond[i].prop2->r,
		  obj->bond[i].prop2->g,
		  obj->bond[i].prop2->b);
      glVertex3fv(p1);
      glVertex3fv(p2);
    }
    for(i=0;i<obj->s_bond_count;i++) {
      p0[0]=obj->s_bond[i].atom->p->x;
      p0[1]=obj->s_bond[i].atom->p->y;
      p0[2]=obj->s_bond[i].atom->p->z;
      if(cf)
	glColor3f(obj->s_bond[i].prop->r,
		  obj->s_bond[i].prop->g,
		  obj->s_bond[i].prop->b);
      glVertex3f(p0[0]-sd,p0[1],p0[2]);
      glVertex3f(p0[0]+sd,p0[1],p0[2]);
      glVertex3f(p0[0],p0[1]-sd,p0[2]);
      glVertex3f(p0[0],p0[1]+sd,p0[2]);
      glVertex3f(p0[0],p0[1],p0[2]-sd);
      glVertex3f(p0[0],p0[1],p0[2]+sd);
    }
    glEnd();

    break;
  case RENDER_CPK:
#ifdef CPK_NEW
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    for(i=0;i<obj->atom_count;i++) {
      glPushMatrix();
      glColor3f(obj->atom[i].prop.r,obj->atom[i].prop.g,obj->atom[i].prop.b);
      glTranslatef(obj->atom[i].ap->p->x,
		   obj->atom[i].ap->p->y,
		   obj->atom[i].ap->p->z);
      glScalef(obj->atom[i].prop.radius,
	       obj->atom[i].prop.radius,
	       obj->atom[i].prop.radius);

      glCallList(obj->sphere_list);

      glPopMatrix();
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_NORMALIZE);

    break;
#else
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);


    for(i=0;i<obj->atom_count;i++) {
      glPushMatrix();
      glColor3f(obj->atom[i].prop.r,obj->atom[i].prop.g,obj->atom[i].prop.b);
      glTranslatef(obj->atom[i].ap->p->x,
		   obj->atom[i].ap->p->y,
		   obj->atom[i].ap->p->z);
      cgfxSphere(obj->atom[i].prop.radius,detail);
      glPopMatrix();
    }

    glDisable(GL_CULL_FACE);

    break;
#endif
  case RENDER_CUSTOM:
#ifdef CUSTOM_NEW
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_NORMAL_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].v[0]);
    glNormalPointer(GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].n[0]);
    glColorPointer(3,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].c[0]);
    
#ifdef SUN
     glBegin(GL_TRIANGLES);
     for(i=0;i<obj->va.count;i++) {
	glArrayElement(i);
     }
     glEnd();
#else
    glDrawArrays(GL_TRIANGLES,0,obj->va.count);
#endif

    glDisable(GL_VERTEX_ARRAY);
    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);

#else
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);

    if(sphere_radius!=bond_width)
      cyl_type=CGFX_BLUNT;
    else
      cyl_type=CGFX_SINGLE_ROUND;
    

    if(bond_width>0.0)
    for(i=0;i<obj->bond_count;i++) {
      glPushMatrix();
      bl2=obj->bond[i].length*0.5;
      glTranslatef(obj->bond[i].atom1->p->x,
		   obj->bond[i].atom1->p->y,
		   obj->bond[i].atom1->p->z);
      glMultMatrixf(obj->bond[i].rotmat);

      glTranslatef(0.0,0.0,bl2);
      if(cf)
	glColor3f(obj->bond[i].prop1->r,
		  obj->bond[i].prop1->g,
		  obj->bond[i].prop1->b);
      cgfxCylinder(cyl_type,-bl2,bond_width,detail);
      if(cf)
	glColor3f(obj->bond[i].prop2->r,
		  obj->bond[i].prop2->g,
		  obj->bond[i].prop2->b);
      cgfxCylinder(cyl_type,bl2,bond_width,detail);

      glPopMatrix();
    }

    if(sphere_radius>0.0 && sphere_radius != bond_width)
      for(i=0;i<obj->atom_count;i++) {
	glPushMatrix();
	if(cf)
	  glColor3f(obj->atom[i].prop.r,obj->atom[i].prop.g,obj->atom[i].prop.b);
	glTranslatef(obj->atom[i].ap->p->x,
		     obj->atom[i].ap->p->y,
		     obj->atom[i].ap->p->z);

	cgfxSphere(sphere_radius,detail);
	glPopMatrix();
      }

    for(i=0;i<obj->s_bond_count;i++) {
      glPushMatrix();
      if(cf)
	glColor3f(obj->s_bond[i].prop->r,
		  obj->s_bond[i].prop->g,
		  obj->s_bond[i].prop->b);
      glTranslatef(obj->s_bond[i].atom->p->x,
		   obj->s_bond[i].atom->p->y,
		   obj->s_bond[i].atom->p->z);
      cgfxSphere(sphere_radius,detail);
      glPopMatrix();
    }


    glDisable(GL_CULL_FACE);
#endif
  }

  if(obj->render.stipple_flag) {
    glDisable(GL_LINE_STIPPLE);
  }  

  glDisable(GL_LIGHTING);
  glColor3f(1.0,1.0,1.0);
  for(i=0;i<obj->atom_count;i++) {
    if(obj->atom[i].label) {

      glRasterPos3f(obj->atom[i].ap->p->x+0.1,
		    obj->atom[i].ap->p->y+0.1,
		    obj->atom[i].ap->p->z+0.1);

      strcpy(label,"");
      if(obj->node->model_flag)
	sprintf(label,"%s%d.",label,obj->atom[i].ap->model->num);
      if(obj->node->chain_flag)
	sprintf(label,"%s%s.",label,obj->atom[i].ap->chain->name);
      if(obj->node->residue_flag)
	sprintf(label,"%s%s%d.",label,
		obj->atom[i].ap->residue->name,
		obj->atom[i].ap->residue->num);
      sprintf(label,"%s%s",label,obj->atom[i].ap->name);

      glfDrawString(label);
    }
  }
  glEnable(GL_LIGHTING);

  
  return 0;

}
