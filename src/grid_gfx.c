#include "gl_include.h"

#include "grid_db.h"
#include "dbm.h"

int gridDraw(dbmGridNode *node, int f)
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
	    gridDrawObj(&node->obj[j]);
  } else {
    for(j=0;j<node->obj_max;j++)
      if(node->obj_flag[j]!=0)
	if(node->obj[j].render.show)
	  if(node->obj[j].render.transparency<1.0)
	    gridDrawObj(&node->obj[j]);
  }

  glPopMatrix();

  return 0;
}

int gridDrawObj(gridObj *obj)
{
  int i;
  float t=obj->render.transparency;
  gridTexture *tex;

  if(obj->type==GRID_SURFACE) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);
    
    glColor3f(0.5,0.5,0.5);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(obj->render.polyf,obj->render.polyu);
    
    if(t<1.0) {
      glPushMatrix();
      glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
      
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_BLEND);
      glDisable(GL_FOG);
      glDisable(GL_NORMALIZE);
      glDisable(GL_STENCIL_TEST);
      glDepthMask(GL_TRUE);
#ifdef LINUX
      // bug in NVIDIA driver
      glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#else
      glDrawBuffer(GL_NONE);
#endif
      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->facec;i++) {
	glNormal3fv(obj->face[i].n1);
	glColor4f(obj->face[i].c1[0],
		  obj->face[i].c1[1],
		  obj->face[i].c1[2],
		  t);
	glVertex3fv(obj->face[i].v1);
	
	glNormal3fv(obj->face[i].n2);
	glColor4f(obj->face[i].c2[0],
		  obj->face[i].c2[1],
		  obj->face[i].c2[2],
		  t);
	glVertex3fv(obj->face[i].v2);
	
	glNormal3fv(obj->face[i].n3);
	glColor4f(obj->face[i].c3[0],
		  obj->face[i].c3[1],
		  obj->face[i].c3[2],
		  t);
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();
#ifdef LINUX
      // bug in NVIDIA driver
      glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif      
      
      glPopAttrib();
      glPopMatrix();
    }
    
    if(obj->map==-1) {
      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->facec;i++) {
	glNormal3fv(obj->face[i].n1);
	glColor4f(obj->face[i].c1[0],
		  obj->face[i].c1[1],
		  obj->face[i].c1[2],
		  t);
	glVertex3fv(obj->face[i].v1);
	
	glNormal3fv(obj->face[i].n2);
	glColor4f(obj->face[i].c2[0],
		  obj->face[i].c2[1],
		  obj->face[i].c2[2],
		  t);
	glVertex3fv(obj->face[i].v2);
	
	glNormal3fv(obj->face[i].n3);
	glColor4f(obj->face[i].c3[0],
		  obj->face[i].c3[1],
		  obj->face[i].c3[2],
		  t);
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();
    } else {
      tex=&obj->node->texture[obj->map];

      glEnable(GL_TEXTURE_2D);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      /*
      glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,
		   tex->width,tex->height, 0,
		   GL_RGBA, GL_BYTE, tex->data);
      */      
      glBindTexture(GL_TEXTURE_2D, obj->texname);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      
      // TODO vertex array

      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->facec;i++) {
	glTexCoord2fv(obj->face[i].t1);
	glColor4f(obj->face[i].c1[0],
		  obj->face[i].c1[1],
		  obj->face[i].c1[2],
		  t);
	glNormal3fv(obj->face[i].n1);
	glVertex3fv(obj->face[i].v1);
	
	glTexCoord2fv(obj->face[i].t2);
	glColor4f(obj->face[i].c2[0],
		  obj->face[i].c2[1],
		  obj->face[i].c2[2],
		  t);
	glNormal3fv(obj->face[i].n2);
	glVertex3fv(obj->face[i].v2);
	
	glTexCoord2fv(obj->face[i].t3);
	glColor4f(obj->face[i].c3[0],
		  obj->face[i].c3[1],
		  obj->face[i].c3[2],
		  t);
	glNormal3fv(obj->face[i].n3);
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();

      glDisable(GL_TEXTURE_2D);
      
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
  } else {
    glPushMatrix();
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glLineWidth(obj->render.line_width);

    glBegin(GL_LINES);
    for(i=0;i<obj->vertc;i++) {
      glColor4f(obj->vert[i].c[0],obj->vert[i].c[1],obj->vert[i].c[2],t);
      glVertex3fv(obj->vert[i].v);
    }
    glEnd();

    glPopAttrib();
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
  }
  return 0;
}
