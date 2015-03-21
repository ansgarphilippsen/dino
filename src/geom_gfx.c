#include <stdio.h>
#include <math.h>

#include <OpenGL/gl.h>

#include "geom_db.h"
#include "cgfx.h"

int geomDraw(dbmGeomNode *node, int f)
{
  int i;
  if(f) {
    for(i=0;i<node->obj_max;i++) {
      if(node->obj[i]!=NULL) {
	if(node->obj[i]->render.show && node->obj[i]->render.transparency<1.0)
	  geomDrawObj(node->obj[i]);
	
      }
    }
  } else {
    for(i=0;i<node->obj_max;i++) {
      if(node->obj[i]!=NULL) {
	if(node->obj[i]->render.show && node->obj[i]->render.transparency==1.0)
	  geomDrawObj(node->obj[i]);
      }
    }
    
  }
  return 0;
}

int geomDrawObj(geomObj *obj)
{
  int i;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);

  glPushAttrib(GL_ENABLE_BIT);

  if(obj->render.mode==RENDER_OFF) {
  
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_COLOR_MATERIAL);

   if(obj->render.nice) {
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DITHER);
  } else {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_DITHER);
  }

  glPointSize(obj->render.point_size);
    glLineWidth(obj->render.line_width);

    glBegin(GL_POINTS);
    for(i=0;i<obj->point_count;i++) {
      glColor3fv(obj->point[i].c);
      glVertex3fv(obj->point[i].v);
    }
    glEnd();
    
    if(obj->render.stipple_flag) {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(obj->render.stipple_factor,obj->render.stipple_pattern);
    }
    
    for(i=0;i<obj->line_count;i++) {
      glBegin(GL_LINES);
      glColor4fv(obj->line[i].c);
      glVertex3fv(obj->line[i].v1);
      glVertex3fv(obj->line[i].v2);
      glEnd();
    }

    if(obj->render.stipple_flag) {
      glDisable(GL_LINE_STIPPLE);
    }

    /* first the line versions */

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for(i=0;i<obj->tri_count;i++) {
      glLineWidth(obj->tri[i].lw);
      if(!obj->tri[i].fill) {
	glColor4fv(obj->tri[i].c);
	glVertex3fv(obj->tri[i].v1);
	glVertex3fv(obj->tri[i].v2);
	glVertex3fv(obj->tri[i].v3);
      }
    }
    glEnd();
    
    glBegin(GL_QUADS);
    for(i=0;i<obj->rect_count;i++) {
      glLineWidth(obj->rect[i].lw);
      if(!obj->rect[i].fill) {
	glColor4fv(obj->rect[i].c);
	glVertex3fv(obj->rect[i].v1);
	glVertex3fv(obj->rect[i].v2);
	glVertex3fv(obj->rect[i].v3);
	glVertex3fv(obj->rect[i].v4);
      }
    }
    glEnd();
    
    /* now the filled and lit versions */
    // need normals for lighting !
    //    glEnable(GL_LIGHTING);
    //    glEnable(GL_COLOR_MATERIAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for(i=0;i<obj->tri_count;i++) {
      if(obj->tri[i].fill) {
	glColor4fv(obj->tri[i].c);
	glVertex3fv(obj->tri[i].v1);
	glVertex3fv(obj->tri[i].v2);
	glVertex3fv(obj->tri[i].v3);
      }
    }
    glEnd();
    
    glBegin(GL_QUADS);
    for(i=0;i<obj->rect_count;i++) {
      if(obj->rect[i].fill) {
	glColor4fv(obj->rect[i].c);
	glVertex3fv(obj->rect[i].v1);
	glVertex3fv(obj->rect[i].v2);
	glVertex3fv(obj->rect[i].v3);
	glVertex3fv(obj->rect[i].v4);
      }
    }
    glEnd();
    
  } else { // RENDER_ON
  
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glEnable(GL_COLOR_MATERIAL);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(cgfxVAField),obj->va.p[0].v);
    glNormalPointer(GL_FLOAT,sizeof(cgfxVAField),obj->va.p[0].n);
    glColorPointer(4,GL_FLOAT,sizeof(cgfxVAField),obj->va.p[0].c); 

#ifdef SUN
     glBegin(GL_TRIANGLES);
     for(i=0;i<obj->va.count;i++) {
	glArrayElement(i);
     }
     glEnd();
#else
     glDrawArrays(GL_TRIANGLES,0,obj->va.count);
#endif

     glDisableClientState(GL_VERTEX_ARRAY);
     glDisableClientState(GL_NORMAL_ARRAY);
     glDisableClientState(GL_COLOR_ARRAY);
  }
  glPopAttrib();

  glDisable(GL_LIGHTING);
  //glLogicOp(GL_XOR);

  for(i=0;i<obj->label_count;i++) {
    glColor3fv(obj->label[i].c);
    glRasterPos3fv(obj->label[i].p);
    glfDrawString(obj->label[i].s);
  }

  //glLogicOp(GL_COPY);
  glEnable(GL_LIGHTING);

  return 0;
}


int geomDrawCyl(geomObj *obj, geomLine *cyl)
{
  glPushMatrix();
  glTranslatef(cyl->v1[0],
	       cyl->v1[1],
	       cyl->v1[2]);

  glMultMatrixf(cyl->rmat);
  
  glColor4fv(cyl->c);
  cgfxCylinder(CGFX_CAP,cyl->length,cyl->r,obj->render.detail1);
  glPopMatrix();
  return 0;
}
