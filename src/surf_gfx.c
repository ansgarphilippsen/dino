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

#include "surf_db.h"

#ifdef RENDER_SOLID
#include <math.h>
#include "gfx.h"
extern struct GFX gfx;
#endif

int surfDraw(dbmSurfNode *node, int f)
{
  int j;

  glPushMatrix();

  // apply ds transform


       
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
	  if(node->obj[j].render.transparency==1.0)
	    surfDrawObj(&node->obj[j]);
  } else {
    for(j=0;j<node->obj_max;j++)
      if(node->obj_flag[j]!=0)
	if(node->obj[j].render.show)
	  if(node->obj[j].render.transparency<1.0)
	    surfDrawObj(&node->obj[j]);
  }

  glPopMatrix();

  return 0;
}

int surfDrawObj(surfObj *obj)
{
  int f;
  int v1,v2,v3;
  float t=obj->render.transparency;
  float dummy;
  int tfc;
#ifdef RENDER_SOLID
  float rw,rh,rz;
#endif

  /*
    determined by color
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, obj->render.mat.diff);
  */
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);


  glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT);
  
  if(obj->render.dbl_light)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  else
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  /*
  if(obj->render.face_reverse)
    glFrontFace(GL_CW);
  else
    glFrontFace(GL_CCW);
    */    

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

//  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);

  // activate and specify arrays
  glInterleavedArrays(GL_C4F_N3F_V3F,sizeof(struct SURF_OBJ_V),obj->vert);

  if(obj->render.mode==RENDER_POINT) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
  } else if(obj->render.mode==RENDER_LINE) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#ifdef RENDER_SOLID
    if(obj->render.cull && !(obj->render.solid && t==1.0))
        glEnable(GL_CULL_FACE);
#else
    if(obj->render.cull)
        glEnable(GL_CULL_FACE);
#endif
  }

  if(t<1.0) {
    glPushMatrix();
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_BLEND);
    glDisable(GL_FOG);
    glDepthMask(GL_TRUE);
#ifdef LINUX
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#else
    glDrawBuffer(GL_NONE);
#endif
    glDrawElements(GL_TRIANGLES,obj->facec*3,GL_UNSIGNED_INT,obj->face);
#ifdef LINUX
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif    

    glPopAttrib();
    glPopMatrix();
    
  }
  glPushMatrix();

  glDepthMask(GL_TRUE);

#ifdef RENDER_SOLID
  if(obj->render.solid && t==1.0) {
    glClearStencil(0x0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS,0x0,0x1);
    glStencilOp(GL_INVERT,GL_INVERT,GL_INVERT);
  }
#endif

  glDrawElements(GL_TRIANGLES,obj->facec*3,GL_UNSIGNED_INT,obj->face);

  
  glDisableClientState(GL_VERTEX_ARRAY); 
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDisable(GL_CULL_FACE);

#ifdef RENDER_SOLID
  if(obj->render.solid && t==1.0) {
    glStencilFunc(GL_NOTEQUAL,0x0,0x1);
    glPushMatrix();
    glLoadIdentity();

    rh=2.0*fabs(tan(gfx.fovy)*gfx.transform.slabn);
    rw=rh*gfx.aspect;
    rz=-gfx.transform.slabn-0.1;

    /*
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0,1,0,1);
    */

    //    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv(obj->render.solidc);
    glNormal3f(0,0,-1);
    glVertex3f(-rw,-rh,rz);
    glVertex3f(rw,-rh,rz);
    glVertex3f(-rw,rh,rz);
    glVertex3f(rw,rh,rz);
    glEnd();

    /*
    glTranslated(0.0,0.0,rz);
    glRectf(-rw,-rh,rw,rh);
    */

    /*
    glRectd(0,0,1,1);
    */

    //    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopMatrix();
    glDisable(GL_STENCIL_TEST);
  }
#endif
  glPopMatrix();

  glPopAttrib();

  return 0;
}

int surfPrepObj(surfObj *obj)
{
  int f;
  glNewList(obj->list,GL_COMPILE);
  glBegin(GL_TRIANGLES);
  for(f=0;f<obj->facec*3;f++) {
    glColor3fv(obj->vert[obj->face[f]].c);
    glNormal3fv(obj->vert[obj->face[f]].n);
    glVertex3fv(obj->vert[obj->face[f]].p);
  }
  glEnd();
  glEndList();
  return 0;
}
