#include <stdio.h>

#include <GL/glu.h>
#include <GL/gl.h>

#include "struct_db.h"
#include "struct_obj.h"
#include "gfx.h"
#include "glf.h"
#include "cgfx.h"
#include "com.h"
#include "transform.h"
#include "build.h"
#include "cl.h"

extern struct GFX gfx;

int structDraw(dbmStructNode *node, int f)
{
  int j;
  structObj *obj;
  transMat *trans;

  // apply ds transform

  /* draw cell if requested */
  if(node->show_cell && node->xtal!=NULL) {
    gfxDrawCell(node->xtal);
  }

  /*
    special BD Trj objects
  */

  if(node->trj_flag && node->trj.type==STRUCT_TRJ_BD && f==0) {
    for(j=0;j<node->obj_max;j++)
      if(node->obj_flag[j]!=0)
	if(node->obj[j].render.show) {
	  obj=&node->obj[j];
	  structDrawBDObj(node,obj);
	}
  } else {
    for(j=0;j<node->obj_max;j++)
      if(node->obj_flag[j]!=0)
	if(node->obj[j].render.show) {
	  glPushMatrix();
	  if(node->obj[j].build!=NULL) {
	    obj=&node->obj[j];
	    trans=&obj->build->trans;
	    
	    glTranslated(trans->cen[0],
			 trans->cen[1],
			 trans->cen[2]);
	    glTranslated(trans->tra[0],
			 trans->tra[1],
			 trans->tra[2]);
	    glMultMatrixd(trans->rot);
	    glTranslated(-trans->cen[0],
			 -trans->cen[1],
			 -trans->cen[2]);
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
	  
	  if((f==0 && node->obj[j].render.transparency==1.0) ||
	     (f!=0 && node->obj[j].render.transparency<1.0)) {
	    structDrawObj(&node->obj[j]);
	  }
	  glPopMatrix();
	}
  }

  return 0;
}

int structDrawObj(structObj *obj)
{
  int i,j;
  float p0[3],p1[3],p2[3];
  float bl2;
  int detail;
  float bond_width,sphere_radius;
  char label[256];
  float sd=0.2;
  int cyl_type;

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);

  detail=obj->render.detail1;
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

      glColor4f(obj->bond[i].prop1->r,
		obj->bond[i].prop1->g,
		obj->bond[i].prop1->b,
		obj->render.transparency);
      glVertex3fv(p0);
      glVertex3fv(p1);

      glColor4f(obj->bond[i].prop2->r,
		obj->bond[i].prop2->g,
		obj->bond[i].prop2->b,
		obj->render.transparency);
      glVertex3fv(p1);
      glVertex3fv(p2);
    }
    
    for(i=0;i<obj->s_bond_count;i++) {
      p0[0]=obj->s_bond[i].atom->p->x;
      p0[1]=obj->s_bond[i].atom->p->y;
      p0[2]=obj->s_bond[i].atom->p->z;
      glColor4f(obj->s_bond[i].prop->r,
		obj->s_bond[i].prop->g,
		obj->s_bond[i].prop->b,
		obj->render.transparency);
      glVertex3f(p0[0]-sd,p0[1],p0[2]);
      glVertex3f(p0[0]+sd,p0[1],p0[2]);
      glVertex3f(p0[0],p0[1]-sd,p0[2]);
      glVertex3f(p0[0],p0[1]+sd,p0[2]);
      glVertex3f(p0[0],p0[1],p0[2]-sd);
      glVertex3f(p0[0],p0[1],p0[2]+sd);
    }
    glEnd();

    break;
#ifdef CPK_NEW
    
  case RENDER_CPK:

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
  case RENDER_CPK:
#endif
  case RENDER_CUSTOM:
#ifdef USE_DLIST
    if(gfx.use_dlist_flag) {
      if(obj->va_list_flag) {
	glCallList(obj->va_list);
      } else {
	glNewList(obj->va_list, GL_COMPILE_AND_EXECUTE);
	guiMessage("Compiling Display List...");
      }
    }
    if(!gfx.use_dlist_flag || (gfx.use_dlist_flag && !obj->va_list_flag)) {
#endif
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);

    //glEnable(GL_VERTEX_ARRAY);
    //glEnable(GL_NORMAL_ARRAY);
    //glEnable(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].v[0]);
    glNormalPointer(GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].n[0]);
    glColorPointer(4,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].c[0]);
    
#ifdef SUN
     glBegin(GL_TRIANGLES);
     for(i=0;i<obj->va.count;i++) {
	glArrayElement(i);
     }
     glEnd();
#else
    glDrawArrays(GL_TRIANGLES,0,obj->va.count);
#endif

    //glDisable(GL_VERTEX_ARRAY);
    //glDisable(GL_NORMAL_ARRAY);
    //glDisable(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
#ifdef USE_DLIST
    }
    if(gfx.use_dlist_flag && !obj->va_list_flag) {
      obj->va_list_flag=1;
      guiMessage("");
      glEndList();
    }
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

      clStrcpy(label,"");
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

  glPopAttrib();

  
  return 0;
}

int structDrawBDObj(dbmStructNode *node, structObj *obj)
{
  int i,f,a;
  int index;
  float x,y,z;
  float sd=0.2;
  int detail;
  float bond_width,sphere_radius;

  glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT);

  f=node->frame;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);

  detail=obj->render.detail1;
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

  for(i=node->trj.frame[f].start;i<node->trj.frame[f].end;i++) {
    index=node->trj.entry[i].index;
    x=node->trj.entry[i].x;
    y=node->trj.entry[i].y;
    z=node->trj.entry[i].z;

    // set render modes
    switch(obj->render.mode) {
    case RENDER_SIMPLE:
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glBegin(GL_LINES);
      break;
    case RENDER_CPK:
    case RENDER_CUSTOM:
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glEnable(GL_CULL_FACE);
      glEnable(GL_NORMALIZE);
      break;
    }
    for(a=0;a<obj->atom_count;a++) {
      if(obj->atom[a].ap->anum==index) {

	switch(obj->render.mode) {
	case RENDER_SIMPLE:
	  glColor3f(obj->atom[a].prop.r,
		    obj->atom[a].prop.g,
		    obj->atom[a].prop.b);
	  glVertex3f(x-sd,y,z); glVertex3d(x+sd,y,z);
	  glVertex3f(x,y-sd,z); glVertex3d(x,y+sd,z);
	  glVertex3f(x,y,z-sd); glVertex3d(x,y,z+sd);
	  break;
	case RENDER_CPK:
	  glPushMatrix();
	  glColor3f(obj->atom[a].prop.r,
		    obj->atom[a].prop.g,
		    obj->atom[a].prop.b);
	  glTranslatef(x,y,z);
	  glScalef(obj->atom[a].prop.radius,
		   obj->atom[a].prop.radius,
		   obj->atom[a].prop.radius);

	  glCallList(obj->sphere_list);

	  glPopMatrix();
	  
	  break;
	case RENDER_CUSTOM:
	  glPushMatrix();
	  glColor3f(obj->atom[a].prop.r,
		    obj->atom[a].prop.g,
		    obj->atom[a].prop.b);
	  glTranslatef(x,y,z);
	  glScalef(sphere_radius,
		   sphere_radius,
		   sphere_radius);

	  glCallList(obj->sphere_list);

	  glPopMatrix();
	  break;
	}
      }
    }
  }

  switch(obj->render.mode) {
  case RENDER_SIMPLE:
    glEnd();
    break;
  }

  glPopAttrib();

  return 0;
}
