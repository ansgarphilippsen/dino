#include <stdio.h>
#include <math.h>

#include "com.h"
#include "gfx.h"
#include "Cmalloc.h"
#include "writePS.h"

static char *gouraudtriangleEPS[] =
{
  "/bd{bind def}bind def /triangle { aload pop   setrgbcolor  aload pop 5 3",
  "roll 4 2 roll 3 2 roll exch moveto lineto lineto closepath fill } bd",
  "/computediff1 { 2 copy sub abs threshold ge {pop pop pop true} { exch 2",
  "index sub abs threshold ge { pop pop true} { sub abs threshold ge } ifelse",
  "} ifelse } bd /computediff3 { 3 copy 0 get 3 1 roll 0 get 3 1 roll 0 get",
  "computediff1 {true} { 3 copy 1 get 3 1 roll 1 get 3 1 roll 1 get",
  "computediff1 {true} { 3 copy 2 get 3 1 roll  2 get 3 1 roll 2 get",
  "computediff1 } ifelse } ifelse } bd /middlecolor { aload pop 4 -1 roll",
  "aload pop 4 -1 roll add 2 div 5 1 roll 3 -1 roll add 2 div 3 1 roll add 2",
  "div 3 1 roll exch 3 array astore } bd /gouraudtriangle { computediff3 { 4",
  "-1 roll aload 7 1 roll 6 -1 roll pop 3 -1 roll pop add 2 div 3 1 roll add",
  "2 div exch 3 -1 roll aload 7 1 roll exch pop 4 -1 roll pop add 2 div 3 1",
  "roll add 2 div exch 3 -1 roll aload 7 1 roll pop 3 -1 roll pop add 2 div 3",
  "1 roll add 2 div exch 7 3 roll 10 -3 roll dup 3 index middlecolor 4 1 roll",
  "2 copy middlecolor 4 1 roll 3 copy pop middlecolor 4 1 roll 13 -1 roll",
  "aload pop 17 index 6 index 15 index 19 index 6 index 17 index 6 array",
  "astore 10 index 10 index 14 index gouraudtriangle 17 index 5 index 17",
  "index 19 index 5 index 19 index 6 array astore 10 index 9 index 13 index",
  "gouraudtriangle 13 index 16 index 5 index 15 index 18 index 5 index 6",
  "array astore 12 index 12 index 9 index gouraudtriangle 17 index 16 index",
  "15 index 19 index 18 index 17 index 6 array astore 10 index 12 index 14",
  "index gouraudtriangle 18 {pop} repeat } { aload pop 5 3 roll aload pop 7 3",
  "roll aload pop 9 3 roll 4 index 6 index 4 index add add 3 div 10 1 roll 7",
  "index 5 index 3 index add add 3 div 10 1 roll 6 index 4 index 2 index add",
  "add 3 div 10 1 roll 9 {pop} repeat 3 array astore triangle } ifelse } bd",
  NULL
};

#define EPS_GOURAUD_THRESHOLD 0.1

int writeGenScene(GLfloat **buf)
{
  GLint fbuf_size;
  GLfloat *fbuf;
  int ret,failure;

  fbuf_size=1000000;

  fbuf=Ccalloc(fbuf_size,sizeof(GLfloat));
  
  failure=0;
  do {
    glFeedbackBuffer(fbuf_size, GL_3D_COLOR,fbuf);
    
    glRenderMode(GL_FEEDBACK);
    
    gfxRedraw();
    
    ret=glRenderMode(GL_RENDER);
    if(ret<0) {
      failure++;
      Cfree(fbuf);
      fbuf_size+=100000;
      fbuf=Ccalloc(fbuf_size,sizeof(GLfloat));
    } else {
      fprintf(stderr,"%d entries in feedback buffer\n",ret);
    }
  } while(ret<0 && failure<20);

  if(ret>=0) {
    (*buf)=fbuf;
  } else {
    (*buf)=NULL;
    Cfree(fbuf);
    ret=-1;
    comMessage("Error during feedback buffer generation\n");
  }

  comRedraw();

  return ret;
}

/* 
   modified from "rendereps"
   Copyright (c) Mark J. Kilgard, 1997. 
*/

GLfloat write_point_size;

int writePS(FILE *f)
{
  GLfloat *buf;
  int n;

  n=writeGenScene(&buf);

  if(buf==NULL)
    return -1;
  
  writePSHeader(f);
  writePSObj(f, buf, n);

  /*
  i=0;
  while(i<n) {
    token=buf[i++];
    if(token==GL_POINT_TOKEN) {
      v[0]=buf[i++];
      v[1]=buf[i++];
      v[2]=buf[i++];
      c[0]=buf[i++];
      c[1]=buf[i++];
      c[2]=buf[i++];
      c[3]=buf[i++];
      fprintf(stderr,"POINT: %.3f %.3f %.3f   %.3f %.3f %.3f %.3f\n",
	      v[0],v[1],v[2],c[0],c[1],c[2],c[3]);
    } else if(token==GL_LINE_TOKEN ||
	      token==GL_LINE_RESET_TOKEN) {
      v[0]=buf[i++];
      v[1]=buf[i++];
      v[2]=buf[i++];
      c[0]=buf[i++];
      c[1]=buf[i++];
      c[2]=buf[i++];
      c[3]=buf[i++];
      fprintf(stderr,"LINE: %.3f %.3f %.3f   %.3f %.3f %.3f %.3f\n",
	      v[0],v[1],v[2],c[0],c[1],c[2],c[3]);

      v[0]=buf[i++];
      v[1]=buf[i++];
      v[2]=buf[i++];
      c[0]=buf[i++];
      c[1]=buf[i++];
      c[2]=buf[i++];
      c[3]=buf[i++];
      fprintf(stderr,"      %.3f %.3f %.3f   %.3f %.3f %.3f %.3f\n",
	      v[0],v[1],v[2],c[0],c[1],c[2],c[3]);
    } else if(token==GL_POLYGON_TOKEN) {
      pc=(int)buf[i++];
      iz=0;
      fprintf(stderr,"POLY:\n");
      while(iz<pc) {
	v[0]=buf[i++];
	v[1]=buf[i++];
	v[2]=buf[i++];
	c[0]=buf[i++];
	c[1]=buf[i++];
	c[2]=buf[i++];
	c[3]=buf[i++];
	iz++;
      fprintf(stderr,"%4d: %.3f %.3f %.3f   %.3f %.3f %.3f %.3f\n",
	      iz,v[0],v[1],v[2],c[0],c[1],c[2],c[3]);
      }
    } else if(token==GL_BITMAP_TOKEN) {
      v[0]=buf[i++];
      v[1]=buf[i++];
      v[2]=buf[i++];
      c[0]=buf[i++];
      c[1]=buf[i++];
      c[2]=buf[i++];
      c[3]=buf[i++];
      fprintf(stderr,"BITMAP: %.3f %.3f %.3f   %.3f %.3f %.3f %.3f\n",
	      v[0],v[1],v[2],c[0],c[1],c[2],c[3]);
    } else if (token==GL_DRAW_PIXEL_TOKEN ||
	       token==GL_COPY_PIXEL_TOKEN) {
      v[0]=buf[i++];
      v[1]=buf[i++];
      v[2]=buf[i++];
      c[0]=buf[i++];
      c[1]=buf[i++];
      c[2]=buf[i++];
      c[3]=buf[i++];
      fprintf(stderr,"PIXEL: %.3f %.3f %.3f   %.3f %.3f %.3f %.3f\n",
	      v[0],v[1],v[2],c[0],c[1],c[2],c[3]);
    } else if (token==GL_PASS_THROUGH_TOKEN) {
      i++;
    }
  }
  */
  Cfree(buf);
  return 0;
}

int writePSHeader(FILE *f)
{
  GLfloat clear_color[4], viewport[4];
  GLfloat line_width;
  int i;

  glGetFloatv(GL_VIEWPORT, viewport);
  glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
  glGetFloatv(GL_LINE_WIDTH, &line_width);
  glGetFloatv(GL_POINT_SIZE, &write_point_size);

  fputs("%!PS-Adobe-2.0 EPSF-2.0\n", f);

  fprintf(f,"%%%%Created by dino\n");
  fprintf(f, "%%%%Bounding box: %g %g %g %g\n",
	  viewport[0], viewport[1], viewport[2], viewport[3]);

  
  fputs("\n", f);
  fputs("gsave\n", f);
  fputs("\n", f);

  /* 
     Output Frederic Delhoume's "gouraudtriangle" PostScript
     fragment.
  */
  fputs("% the gouraudtriangle PostScript fragement below is free\n", f);
  fputs("% written by Frederic Delhoume (delhoume@ilog.fr)\n", f);
  fprintf(f, "/threshold %g def\n", EPS_GOURAUD_THRESHOLD);
  for (i = 0; gouraudtriangleEPS[i]; i++) {
    fprintf(f, "%s\n", gouraudtriangleEPS[i]);
  }
  
  fprintf(f, "%g setlinewidth\n\n", line_width);

  fprintf(f, "%g %g %g setrgbcolor\n",
	  clear_color[0], clear_color[1], clear_color[2]);
  fprintf(f, "%g %g %g %g rectfill\n\n",
	  viewport[0], viewport[1], viewport[2], viewport[3]);
  return 0;
}

int writePSObj(FILE *f, GLfloat *buf, int n)
{
  struct FEEDBACK_INDEX *indx;
  struct FEEDBACK_3D_COLOR *vertex;
  int i,j,k,indx_count,token;
  GLfloat dsum;
  
  /* count the entries */
  i=0;
  indx_count=0;
  while(i<n) {
    token=buf[i++];
    switch(token) {
    case GL_POINT_TOKEN:
      i+=7;
      indx_count++;
      break;
    case GL_LINE_TOKEN:
    case GL_LINE_RESET_TOKEN:
      i+=14;
      indx_count++;
      break;
    case GL_POLYGON_TOKEN:
      j=buf[i++];
      i+=(7*j);
      indx_count++;
      break;
      /*
    case GL_BITMAP_TOKEN:
    case GL_DRAW_PIXEL_TOKEN:
    case GL_COPY_PIXEL_TOKEN:
      i+=7;
      break;
      */
    default:
      fprintf(stderr,"Error: unknown token\n");
      i=n;
      break;
    }
  }
  fprintf(stderr,"%d elements found\n",indx_count);

  if(indx_count==0)
    return 0;

  indx=Ccalloc(indx_count,sizeof(struct FEEDBACK_INDEX));

  i=0;
  indx_count=0;
  while(i<n) {
    indx[indx_count].entry=buf+i;
    token=buf[i++];
    switch(token) {
    case GL_POINT_TOKEN:
      vertex=(struct FEEDBACK_3D_COLOR *)(buf+i);
      i+=7;
      indx[indx_count++].depth=vertex[0].z;
      break;
    case GL_LINE_TOKEN:
    case GL_LINE_RESET_TOKEN:
      vertex=(struct FEEDBACK_3D_COLOR *)(buf+i);
      i+=14;
      indx[indx_count++].depth=(vertex[0].z+vertex[1].z)*0.5;
      break;
    case GL_POLYGON_TOKEN:
      j=buf[i++];
      vertex=(struct FEEDBACK_3D_COLOR *)(buf+i);
      dsum=vertex[0].z;
      for(k=1;k<j;k++)
	dsum+=vertex[k].z;
      indx[indx_count++].depth=dsum/(GLfloat)j;
      i+=(7*j);
      break;
    case GL_BITMAP_TOKEN:
    case GL_DRAW_PIXEL_TOKEN:
    case GL_COPY_PIXEL_TOKEN:
      i+=7;
      break;
    default:
      fprintf(stderr,"Error: unknown token\n");
      i=n;
      break;
    }
  }

  qsort(indx,indx_count,sizeof(struct FEEDBACK_INDEX),writeCompare);

  for(i=0;i<indx_count;i++)
    writePSPrimitive(f,indx[i].entry);

  /*
  for(i=0;i<indx_count;i++) {
    switch((int)indx[i].entry[0]) {
    case GL_POINT_TOKEN:
      fprintf(stderr,"point\n");
      break;
    case GL_LINE_TOKEN:
    case GL_LINE_RESET_TOKEN:
      fprintf(stderr,"line\n");
      break;
    case GL_POLYGON_TOKEN:
      fprintf(stderr,"polygon\n");
      break;
    case GL_BITMAP_TOKEN:
    case GL_DRAW_PIXEL_TOKEN:
    case GL_COPY_PIXEL_TOKEN:
      break;
    }
    fprintf(stderr,"  %g",indx[i].depth);
  }
  */

  return 0;
}

int writePSPrimitive(FILE *file, GLfloat *loc)
{
  int token;
  int nvertices, i;
  GLfloat red, green, blue;
  int smooth;
  GLfloat dx, dy, dr, dg, db, absR, absG, absB, colormax;
  int steps;
  struct FEEDBACK_3D_COLOR *vertex;
  GLfloat xstep, ystep, rstep, gstep, bstep;
  GLfloat xnext, ynext, rnext, gnext, bnext, distance;

  token = *loc;
  loc++;
  switch (token) {
  case GL_LINE_RESET_TOKEN:
  case GL_LINE_TOKEN:
    vertex = (struct FEEDBACK_3D_COLOR *) loc;

    dr = vertex[1].red - vertex[0].red;
    dg = vertex[1].green - vertex[0].green;
    db = vertex[1].blue - vertex[0].blue;

    if (dr != 0 || dg != 0 || db != 0) {
      /* Smooth shaded line. */
      dx = vertex[1].x - vertex[0].x;
      dy = vertex[1].y - vertex[0].y;
      distance = sqrt(dx * dx + dy * dy);

      absR = fabs(dr);
      absG = fabs(dg);
      absB = fabs(db);

#define Max(a,b) (((a)>(b))?(a):(b))

#define EPS_SMOOTH_LINE_FACTOR 0.06  /* Lower for better smooth 

                                        lines. */

      colormax = Max(absR, Max(absG, absB));
      steps = Max(1.0, colormax * distance * EPS_SMOOTH_LINE_FACTOR);

      xstep = dx / steps;
      ystep = dy / steps;

      rstep = dr / steps;
      gstep = dg / steps;
      bstep = db / steps;

      xnext = vertex[0].x;
      ynext = vertex[0].y;
      rnext = vertex[0].red;
      gnext = vertex[0].green;
      bnext = vertex[0].blue;

      /* Back up half a step; we want the end points to be
         exactly the their endpoint colors. */
      xnext -= xstep / 2.0;
      ynext -= ystep / 2.0;
      rnext -= rstep / 2.0;
      gnext -= gstep / 2.0;
      bnext -= bstep / 2.0;
    } else {
      /* Single color line. */
      steps = 0;
    }

    fprintf(file, "%g %g %g setrgbcolor\n",
      vertex[0].red, vertex[0].green, vertex[0].blue);
//    fprintf(file,"%g line width\n",
    fprintf(file, "%g %g moveto\n", vertex[0].x, vertex[0].y);

    for (i = 0; i < steps; i++) {
      xnext += xstep;
      ynext += ystep;
      rnext += rstep;
      gnext += gstep;
      bnext += bstep;
      fprintf(file, "%g %g lineto stroke\n", xnext, ynext);
      fprintf(file, "%g %g %g setrgbcolor\n", rnext, gnext, bnext);
      fprintf(file, "%g %g moveto\n", xnext, ynext);
    }
    fprintf(file, "%g %g lineto stroke\n", vertex[1].x, vertex[1].y);

    loc += 14;          /* Each vertex element in the feedback
                           buffer is 7 GLfloats. */

    break;
  case GL_POLYGON_TOKEN:
    nvertices = *loc;
    loc++;

    vertex = (struct FEEDBACK_3D_COLOR *) loc;

    if (nvertices > 0) {
      red = vertex[0].red;
      green = vertex[0].green;
      blue = vertex[0].blue;
      smooth = 0;
      for (i = 1; i < nvertices; i++) {
        if (red != vertex[i].red || green != vertex[i].green || blue != vertex[i].blue) {
          smooth = 1;
          break;
        }
      }
      if (smooth) {
        /* Smooth shaded polygon; varying colors at vetices. */
        int triOffset;

        /* Break polygon into "nvertices-2" triangle fans. */
        for (i = 0; i < nvertices - 2; i++) {
          triOffset = i * 7;
          fprintf(file, "[%g %g %g %g %g %g]",
            vertex[0].x, vertex[i + 1].x, vertex[i + 2].x,
            vertex[0].y, vertex[i + 1].y, vertex[i + 2].y);
          fprintf(file, " [%g %g %g] [%g %g %g] [%g %g %g] gouraudtriangle\n",
            vertex[0].red, vertex[0].green, vertex[0].blue,
            vertex[i + 1].red, vertex[i + 1].green, vertex[i + 1].blue,
            vertex[i + 2].red, vertex[i + 2].green, vertex[i + 2].blue);
        }
      } else {
        /* Flat shaded polygon; all vertex colors the same. */
        fprintf(file, "newpath\n");
        fprintf(file, "%g %g %g setrgbcolor\n", red, green, blue);

        /* Draw a filled triangle. */
        fprintf(file, "%g %g moveto\n", vertex[0].x, vertex[0].y);
        for (i = 1; i < nvertices; i++) {
          fprintf(file, "%g %g lineto\n", vertex[i].x, vertex[i].y);
        }
        fprintf(file, "closepath fill\n\n");
      }
    }
    loc += nvertices * 7;  /* Each vertex element in the
                              feedback buffer is 7 GLfloats. */
    break;
  case GL_POINT_TOKEN:
    vertex = (struct FEEDBACK_3D_COLOR *) loc;
    fprintf(file, "%g %g %g setrgbcolor\n", vertex[0].red, vertex[0].green, vertex[0].blue);
    fprintf(file, "%g %g %g 0 360 arc fill\n\n", vertex[0].x, vertex[0].y, write_point_size / 2.0);
    loc += 7;           /* Each vertex element in the feedback
                           buffer is 7 GLfloats. */
    break;
  default:
    break;
  }
  return 0;
}

int writeCompare(const void *p1, const void *p2)
{
  struct FEEDBACK_INDEX *i1,*i2;

  i1=(struct FEEDBACK_INDEX *)p1;
  i2=(struct FEEDBACK_INDEX *)p2;

  if(i1->depth<i2->depth)
    return 1;
  else if(i1->depth>i2->depth)
    return -1;
  else
    return 0;
}

