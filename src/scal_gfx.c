#include <stdio.h>
#include <math.h>

#include <GL/glu.h>
#include <GL/gl.h>

#include "scal_db.h"
#include "gfx.h"
#include "dbm.h"
#include "mat.h"

extern struct GFX gfx;

#define OLD_SCAL_CONTOUR_DRAWING

int scalDraw(dbmScalNode *node, int f)
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
      if(node->obj[j]!=NULL)
	if(node->obj[j]->render.show)
	  if(node->obj[j]->render.transparency==1.0)
	    scalDrawObj(node->obj[j]);
  } else {
    for(j=0;j<node->obj_max;j++)
      if(node->obj[j]!=NULL)
	if(node->obj[j]->render.show)
	  if(node->obj[j]->render.transparency<1.0)
	    scalDrawObj(node->obj[j]);
  }

  glPopMatrix();

  return 0;
}

int scalDrawObj(scalObj *obj)
{
  int i,detail;
  int corner[][3]={
    {0,0,0},{1,0,0},{0,1,0},{1,1,0},
    {0,0,1},{1,0,1},{0,1,1},{1,1,1}
  };
  int edge[][2]={
    {0,1},{0,2},{2,3},{1,3},
    {0,4},{2,6},{3,7},{1,5},
    {4,5},{4,6},{6,7},{5,7}
  };

  detail=obj->render.detail1;

//  fprintf(stderr," .%s.%s\n",obj->node->name, obj->name);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obj->render.mat.amb);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obj->render.mat.spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, obj->render.mat.emm);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->render.mat.shin);

  glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
  
  glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);

  if(obj->render.nice) {
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
  } else {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
  }

  glPushMatrix();

  switch(obj->type) {
  case SCAL_CONTOUR:
    if(obj->render.mode==RENDER_POINT) {
      // RENDER DOTS ONLY
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);

      glPointSize(obj->render.point_size);

      glBegin(GL_POINTS);
      
      for(i=0;i<obj->point_count;i++) {
#ifdef CONTOUR_COLOR
	// TODO
#endif
	glVertex3fv(obj->point[i].v);
      }
      glEnd();
    } else if(obj->render.mode==RENDER_LINE) {
#ifndef OLD_SCAL_CONTOUR_DRAWING
      glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT );
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glEnable(GL_BLEND);
      glLineWidth(obj->render.line_width);

#ifndef CONTOUR_COLOR
      glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
#endif
      if(obj->render.transparency<1.0) {
	glDisable(GL_BLEND);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	glBegin(GL_LINES);
	for(i=0;i<obj->line_count;i++) {
	  glVertex3fv(obj->point[obj->line[i].pi0].v);
	  glVertex3fv(obj->point[obj->line[i].pi1].v);
	}
	glEnd();
	
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
      }
      
      glBegin(GL_LINES);
      for(i=0;i<obj->line_count;i++) {
#ifdef CONTOUR_COLOR
	glColor4fv(obj->point[obj->line[i].pi0].c);
#endif
	glVertex3fv(obj->point[obj->line[i].pi0].v);
#ifdef CONTOUR_COLOR
	glColor4fv(obj->point[obj->line[i].pi1].c);
#endif
	glVertex3fv(obj->point[obj->line[i].pi1].v);
      }
      glEnd();

      glPopAttrib();
#else
      // RENDER IN CHICKEN WIRE MODE
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);

      glLineWidth(obj->render.line_width);

#ifndef CONTOUR_COLOR
      glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
#endif

      /*
      glEnable(GL_VERTEX_ARRAY);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3,GL_FLOAT,
		      sizeof(struct SCAL_POINT), &obj->point[0].v[0]);
      */

      glDepthMask(GL_FALSE);


      glBegin(GL_LINES);
      if(gfx.transform.rot[10]>0.0) {
	for(i=0;i<obj->line_count;i++) {
#ifdef CONTOUR_COLOR
	  glColor4fv(obj->point[obj->line[i].pi0].c);
#endif
	  glVertex3fv(obj->point[obj->line[i].pi0].v);
#ifdef CONTOUR_COLOR
	  glColor4fv(obj->point[obj->line[i].pi1].c);
#endif
	  glVertex3fv(obj->point[obj->line[i].pi1].v);
	  //glArrayElement(obj->line[i].pi0);
	  //glArrayElement(obj->line[i].pi1);
	}
      } else {
	for(i=obj->line_count-1;i>=0;i--) {
#ifdef CONTOUR_COLOR
	  glColor4fv(obj->point[obj->line[i].pi0].c);
#endif
	  glVertex3fv(obj->point[obj->line[i].pi0].v);
#ifdef CONTOUR_COLOR
	  glColor4fv(obj->point[obj->line[i].pi1].c);
#endif
	  glVertex3fv(obj->point[obj->line[i].pi1].v);
	  //glArrayElement(obj->line[i].pi0);
	  //glArrayElement(obj->line[i].pi1);
	}
      }
      glEnd();

      glDepthMask(GL_TRUE);

      /* update the depthbuffer */

      glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);

      glDisable(GL_BLEND);
      glDisable(GL_FOG);
#ifdef LINUX
      glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#else
      glDrawBuffer(GL_NONE);
#endif
      glBegin(GL_LINES);
      for(i=0;i<obj->line_count;i++) {
#ifdef CONTOUR_COLOR
	glColor4fv(obj->point[obj->line[i].pi0].c);
#endif
	glVertex3fv(obj->point[obj->line[i].pi0].v);
#ifdef CONTOUR_COLOR
	glColor4fv(obj->point[obj->line[i].pi1].c);
#endif
	glVertex3fv(obj->point[obj->line[i].pi1].v);
	//glArrayElement(obj->line[i].pi0);
	//glArrayElement(obj->line[i].pi1);
      }
      glEnd();
#ifdef LINUX
      glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif      
      glPopAttrib();

      /*
      glDisable(GL_VERTEX_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      */

      glEnable(GL_LIGHTING);
#endif
    } else if(obj->render.mode==RENDER_SURFACE) {

      // RENDER WITH FULL SURFACE MODE

#ifndef CONTOUR_COLOR
      glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
#endif

      // TEMPORARY
      //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
      //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glDisable(GL_CULL_FACE);

      if(obj->render.transparency<1.0) {
	glPushMatrix();
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_NORMALIZE);

	glDepthMask(GL_TRUE);
#ifdef LINUX
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#else
	glDrawBuffer(GL_NONE);
#endif
	glBegin(GL_TRIANGLES);
	for(i=0;i<obj->face_count;i++) {
	  glNormal3fv(obj->face[i].n1);
	  glVertex3fv(obj->face[i].v1);
	  glNormal3fv(obj->face[i].n2);
	  glVertex3fv(obj->face[i].v2);
	  glNormal3fv(obj->face[i].n3);
	  glVertex3fv(obj->face[i].v3);
	}
	glEnd();
#ifdef LINUX
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif      
	glPopAttrib();
	glPopMatrix();
      }

      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->face_count;i++) {
	glNormal3fv(obj->face[i].n1);
#ifdef CONTOUR_COLOR
	glColor4fv(obj->face[i].c1);
#endif
	glVertex3fv(obj->face[i].v1);
	glNormal3fv(obj->face[i].n2);
#ifdef CONTOUR_COLOR
	glColor4fv(obj->face[i].c2);
#endif
	glVertex3fv(obj->face[i].v2);
	glNormal3fv(obj->face[i].n3);
#ifdef CONTOUR_COLOR
	glColor4fv(obj->face[i].c3);
#endif
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();


      glEnable(GL_CULL_FACE);
      glEnable(GL_LIGHTING);
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    } else {
    }
    break;
  case SCAL_GRID:
    if(obj->render.mode==RENDER_OFF) {
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      
      glPointSize(obj->render.point_size);
      
      glBegin(GL_POINTS);
      for(i=0;i<obj->point_count;i++) {
	glColor4fv(obj->point[i].c);
	glVertex3fv(obj->point[i].v);
      }	
      glEnd();
    } else {
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glEnable(GL_CULL_FACE);

      for(i=0;i<obj->point_count;i++) {
	glPushMatrix();
	glColor4f(obj->point[i].c[0],
		  obj->point[i].c[1],
		  obj->point[i].c[2],
		  obj->render.transparency);
	glTranslatef(obj->point[i].v[0],
		     obj->point[i].v[1],
		     obj->point[i].v[2]);
	cgfxSphere(obj->point[i].rad,detail);
	glPopMatrix();
      }

    }
    break;
  case SCAL_GRAD:
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);


    glBegin(GL_LINES);
    for(i=0;i<obj->vect_count;i++) {
      glColor3fv(obj->vect[i].c);
      glVertex3fv(obj->vect[i].v1);
      glVertex3fv(obj->vect[i].v2);
    }
    glEnd();
    break;
  case SCAL_SLAB:
    glEnable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBindTexture(GL_TEXTURE_2D, obj->slab.texname);

//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);
    glColor4f(1.0,1.0,1.0,1.0);
    glTexCoord2f(0.0,0.0);
    glVertex3dv(obj->slab.bound[0]);
    glTexCoord2f(1.0,0.0);
    glVertex3dv(obj->slab.bound[1]);
    glTexCoord2f(1.0,1.0);
    glVertex3dv(obj->slab.bound[2]);
    glTexCoord2f(0.0,1.0);
    glVertex3dv(obj->slab.bound[3]);
    glEnd();

    glEnable(GL_LIGHTING);

    glDisable(GL_TEXTURE_2D);

/*     glBegin(GL_LINES);
    glColor3f(1.0,0.0,1.0);
    glVertex3d(obj->slab.center[0],
	       obj->slab.center[1],
	       obj->slab.center[2]);
    glVertex3d(obj->slab.center[0]+obj->slab.dir[0]*10.0,
	       obj->slab.center[1]+obj->slab.dir[1]*10.0,
	       obj->slab.center[2]+obj->slab.dir[2]*10.0);

    glColor3f(1.0,1.0,1.0);
    for(i=0;i<12;i++) {
      glVertex3d(obj->slab.corner[corner[edge[i][0]][0]][0],
		 obj->slab.corner[corner[edge[i][0]][1]][1],
		 obj->slab.corner[corner[edge[i][0]][2]][2]);
      glVertex3d(obj->slab.corner[corner[edge[i][1]][0]][0],
		 obj->slab.corner[corner[edge[i][1]][1]][1],
		 obj->slab.corner[corner[edge[i][1]][2]][2]);
    }
    glEnd();

    glBegin(GL_LINES);
    for(i=0;i<obj->slab.linec;i++) {
      glColor3f(1.0,1.0,0.0);
      glVertex3fv(obj->slab.point[obj->slab.line[i][0]]);
      glVertex3fv(obj->slab.point[obj->slab.line[i][1]]);
    }

    glEnd();
 */


    break;
#ifdef VR
  case SCAL_VR:
    scalDrawVR(obj);
    break;
#endif
#ifdef BONO
  case SCAL_BONO:

    
    switch(obj->render.mode) {
    case RENDER_POINT:
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->face_count;i++) {
	glNormal3fv(obj->face[i].n);
	glVertex3fv(obj->face[i].v1);
	glVertex3fv(obj->face[i].v2);
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();
      break;
    case RENDER_LINE:
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->face_count;i++) {
	glNormal3fv(obj->face[i].n);
	glVertex3fv(obj->face[i].v1);
	glVertex3fv(obj->face[i].v2);
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();
      break;
    case RENDER_SURFACE:
      glEnable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glColor4f(obj->r, obj->g, obj->b,obj->render.transparency);
      glBegin(GL_TRIANGLES);
      for(i=0;i<obj->face_count;i++) {
	glNormal3fv(obj->face[i].n);
	glVertex3fv(obj->face[i].v1);
	glVertex3fv(obj->face[i].v2);
	glVertex3fv(obj->face[i].v3);
      }
      glEnd();
      
      
      break;
    }

    
    /*
      glColor3f(1.0,1.0,0.0);
      glBegin(GL_POINTS);
      for(i=0;i<obj->point_count;i++) {
      glVertex3fv(obj->point[i].v);
      }
      glEnd();
    */
    
    break;
#endif
  }

  glPopAttrib();

  glPopMatrix();

  return 0;
}

#ifdef VR

int scalDrawVR(scalObj *obj)
{
  float mid,tid,col[4];
  int width;

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);

  col[0]=obj->r; col[1]=obj->g; col[2]=obj->b; col[3]=1.0;

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);

  glTexImage3DEXT(GL_TEXTURE_3D_EXT,0,
		  GL_LUMINANCE,
		  obj->vr.usize,obj->vr.vsize,obj->vr.wsize,
		  0,GL_LUMINANCE, GL_FLOAT,obj->field->data);
  
  mid=(obj->vr.xyz2[2]-obj->vr.xyz1[2])*0.5+obj->vr.xyz1[2];
  tid=(obj->vr.tex2[2]-obj->vr.tex1[2])*0.5+obj->vr.tex1[2];
  
  glBegin(GL_QUADS);

  glColor3f(1.0,1.0,1.0);

  glTexCoord3f(obj->vr.tex1[0],obj->vr.tex1[1],tid);
  glVertex3f(obj->vr.xyz1[0],obj->vr.xyz1[1],mid);

  glTexCoord3f(obj->vr.tex2[0],obj->vr.tex1[1],tid);
  glVertex3f(obj->vr.xyz2[0],obj->vr.xyz1[1],mid);

  glTexCoord3f(obj->vr.tex2[0],obj->vr.tex2[1],tid);
  glVertex3f(obj->vr.xyz2[0],obj->vr.xyz2[1],mid);

  glTexCoord3f(obj->vr.tex1[0],obj->vr.tex2[1],tid);
  glVertex3f(obj->vr.xyz1[0],obj->vr.xyz2[1],mid);

  glEnd();

  return 0;
}
#endif
