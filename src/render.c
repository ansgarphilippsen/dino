#include <stdio.h>
#include <stdlib.h>
#ifndef OSX
#include <malloc.h>
#endif
#include <string.h>

#include "dino.h"
#include "render.h"
#include "dbm.h"
#include "com.h"
#include "cgfx.h"
#include "mat.h"

extern int gfx_flags;

void renderDefault(struct RENDER *r)
{
  struct RENDER_MATERIAL render_default_mat= {
    {0,0,0,1},
    {1,1,1,1},
    {0,0,0,1},
    {0,0,0,0},
    1
  };

  r->show=1;
  r->mode=RENDER_SIMPLE;
  r->detail=3;
  r->detail1=3;
  r->detail2=3;
  r->nice=1;
  r->dbl_light=0;
  r->face_reverse=0;
  r->line_width=1.0;
  r->point_size=1.0;
  r->bond_width=0.2;
  r->sphere_radius=0.2;
  r->helix_width=0.1;
  r->helix_thickness=0.1;
  r->strand_width=0.1;
  r->strand_thickness=0.1;
  r->arrow_thickness=1.0;
  r->tube_ratio=1.0;
  r->tube_width=0.2;
  r->sugar_thickness=0.1;
  r->base_thickness=0.1;
  r->transparency=1.0;
  r->cgfx_flag=0;
  r->strand_method=0;
  r->helix_method=0;
  r->na_method=0;
  r->cull=0;
  r->stipple_flag=0;
  r->polyf=0.0;
  r->polyu=0.0;
  r->solid=0;

  memcpy(&r->mat,&render_default_mat,sizeof(struct RENDER_MATERIAL));
}


int renderSet(struct RENDER *render, int owc, char **owl)
{
  char expr[2048];
  char prop[256];
  char op[256];
  char val[256];
  char message[256];
  char **wl;
  int wc;
  int c;
  int oldi;
  float col[3];

  if(owc<=0)
    return 0;

  memset(expr,0,sizeof(expr));
  for(c=0;c<owc;c++)
    strcat(expr,owl[c]);
  
  dbmSplit(expr,',',&wc,&wl);
  
  for(c=0;c<wc;c++) {
    dbmSplitPOV(wl[c],prop,op,val);

    if(!strcmp(prop,"simple")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_SIMPLE;
    } else  if(!strcmp(prop,"cpk")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_CPK;
    } else if(!strcmp(prop,"custom")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_CUSTOM;
    } else if(!strcmp(prop,"helix")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_HELIX;
    } else if(!strcmp(prop,"strand")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_STRAND;
    } else if(!strcmp(prop,"strand2")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_STRAND2;
    } else if(!strcmp(prop,"tube")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_TUBE;
    } else if(!strcmp(prop,"hsc")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_HSC;
    } else if(!strcmp(prop,"sline")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_SLINE;
    } else if(!strcmp(prop,"cyl")) {
      if(strlen(op)!=0) {
	sprintf(message,"render:: error in expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_CYLINDER;
    } else if(!strcmp(prop,"fill")){
      /********************
          render fill
       ********************/
      if(strlen(op)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_SURFACE;
      comRedraw();
    } else if(!strcmp(prop,"lines") ||
	      !strcmp(prop,"line")){
      /********************
          render lines
       ********************/
      if(strlen(op)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_LINE;
      comRedraw();
    } else if(!strcmp(prop,"dots")){
      /********************
          render dots
       ********************/
      if(strlen(op)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_POINT;
    } else if(!strcmp(prop,"off")){
      if(strlen(op)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_OFF;
    } else if(!strcmp(prop,"on")){
      if(strlen(op)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)>0) {
      	sprintf(message,"expression syntax error\n");
	comMessage(message);
	return -1;
      }
      render->mode=RENDER_ON;
    } else if(!strcmp(prop,"nice")) {
      /********************
	      nice
      ********************/
      if(!strcmp(op,"!")) {
	render->nice=0;
      } else if(strlen(op)==0) {
	render->nice=1;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->nice=0;
	} else {
	  render->nice=1;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"fast")) {
      /********************
	      fast
      ********************/
      if(!strcmp(op,"!")) {
	render->nice=1;
      } else if(strlen(op)==0) {
	render->nice=0;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->nice=1;
	} else {
	  render->nice=0;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"light2")) {
      /********************
	      light2
      ********************/
      if(!strcmp(op,"!")) {
	render->dbl_light=0;
      } else if(strlen(op)==0) {
	render->dbl_light=1;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->dbl_light=0;
	} else {
	  render->dbl_light=1;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"light1")) {
      /********************
	      light1
      ********************/
      if(!strcmp(op,"!")) {
	render->dbl_light=1;
      } else if(strlen(op)==0) {
	render->dbl_light=0;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->dbl_light=1;
	} else {
	  render->dbl_light=0;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"rev")) {
      /********************
	      rev
      ********************/
      if(!strcmp(op,"!")) {
	render->face_reverse=0;
      } else if(strlen(op)==0) {
	render->face_reverse=1;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->face_reverse=0;
	} else {
	  render->face_reverse=1;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"cull")) {
      /*******************
	      cull
      ********************/
      if(!strcmp(op,"!")) {
	render->cull=0;
      } else if(strlen(op)==0) {
	render->cull=1;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->cull=0;
	} else {
	  render->cull=1;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"detail")) {
      /********************
	     detail
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      oldi=render->detail;
      if(!strcmp(op,"=")) {
	render->detail=atoi(val);
      } else if(!strcmp(op,"+=")) {
	render->detail+=atoi(val);
      } else if(!strcmp(op,"-=")) {
	render->detail-=atoi(val);
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
      if(render->detail<1 || render->detail>90) {
	sprintf(message,"value out of range (1-90)\n");
	comMessage(message);
	render->detail=oldi;
	return -1;
      }
      render->detail1=render->detail;
      // DEPRECATED
      //render->detail2=render->detail;
    } else if(!strcmp(prop,"detail1")) {
      /********************
	     detail1
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      oldi=render->detail1;
      if(!strcmp(op,"=")) {
	render->detail1=atoi(val);
      } else if(!strcmp(op,"+=")) {
	render->detail1+=atoi(val);
      } else if(!strcmp(op,"-=")) {
	render->detail1-=atoi(val);
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
      if(render->detail1<1 || render->detail1>90) {
	sprintf(message,"value out of range (1-90)\n");
	comMessage(message);
	render->detail1=oldi;
	return -1;
      }
    } else if(!strcmp(prop,"detail2")) {
      /********************
	     detail2
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      oldi=render->detail2;
      if(!strcmp(op,"=")) {
	render->detail2=atoi(val);
      } else if(!strcmp(op,"+=")) {
	render->detail2+=atoi(val);
      } else if(!strcmp(op,"-=")) {
	render->detail2-=atoi(val);
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
      if(render->detail2<1 || render->detail2>100) {
	sprintf(message,"value out of range (1-100)\n");
	comMessage(message);
	render->detail2=oldi;
	return -1;
      }
    } else if(!strcmp(prop,"lw")) {
      /********************
	     lw
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->line_width=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->line_width=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->line_width=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      if(render->line_width<0.0)
	render->line_width=0.0;
     } else if(!strcmp(prop,"bw")) {
      /********************
	     bw
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->bond_width=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->bond_width=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->bond_width=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      /*
      if(render->bond_width<0.1)
	render->bond_width=0.1;
	*/
      if(render->sphere_radius<render->bond_width)
	render->sphere_radius=render->bond_width;

     } else if(!strcmp(prop,"tuber")) {
      /********************
	     tuber
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->tube_ratio=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->tube_ratio=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->tube_ratio=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
     } else if(!strcmp(prop,"tubew")) {
      /********************
	     tubew
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->tube_width=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->tube_width=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->tube_width=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"sr")) {
      /********************
	     sr
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->sphere_radius=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->sphere_radius=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->sphere_radius=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      /*
      if(render->sphere_radius<0.1)
	render->sphere_radius=0.1;
	*/
    } else if(!strcmp(prop,"ps")) {
      /********************
	       ps
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->point_size=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->point_size=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->point_size=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      if(render->point_size<0.1)
	render->point_size=0.1;
    } else if(!strcmp(prop,"helixw")) {
      /********************
	   helixwidth
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->helix_width=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->helix_width+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->helix_width-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"helixt")) {
      /********************
         helixthickness
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->helix_thickness=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->helix_thickness+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->helix_thickness-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"strandw")) {
      /********************
	  strandwidth
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->strand_width=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->strand_width+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->strand_width-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"strandt")) {
      /********************
	  strandthickness
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->strand_thickness=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->strand_thickness+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->strand_thickness-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      if(render->strand_thickness<0.01) {
	render->strand_thickness=0.01;
      }
    } else if(!strcmp(prop,"arrowt")) {
      /********************
	  arrowthickness
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->arrow_thickness=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->arrow_thickness+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->arrow_thickness-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      if(render->arrow_thickness<0.01) {
	render->arrow_thickness=0.01;
      }
    } else if(!strcmp(prop,"baset")) {
      /********************
	  base thickness
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->base_thickness=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->base_thickness+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->base_thickness-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      if(render->base_thickness<0.01) {
	render->base_thickness=0.01;
      }
    } else if(!strcmp(prop,"sugart")) {
      /********************
	  sugar thickness
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->sugar_thickness=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->sugar_thickness+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->sugar_thickness-=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
      if(render->sugar_thickness<0.01) {
	render->sugar_thickness=0.01;
      }
    } else if(!strcmp(prop,"t")){
      /********************
	      render t
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->transparency=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->transparency+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->transparency-=atof(val);
      } else {
	sprintf(message,"invalid operator: %s\n",op);
	comMessage(message);
	return -1;
      }
      if(render->transparency<0.0)
	render->transparency=0.0;
      if(render->transparency>1.0)
	render->transparency=1.0;
    } else if(!strcmp(prop,"polyf")){
      /********************
	      polygon offset f
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->polyf=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->polyf+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->polyf-=atof(val);
      } else {
	sprintf(message,"invalid operator: %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"polyu")){
      /********************
	      polygon offset units
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->polyu=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->polyu+=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->polyu-=atof(val);
      } else {
	sprintf(message,"invalid operator: %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"intpol")) {
      /********************
	      intpol
      ********************/
      if(!strcmp(op,"!")) {
	render->cgfx_flag&=(~CGFX_INTPOL_COL);
      } else if(strlen(op)==0) {
	render->cgfx_flag|=CGFX_INTPOL_COL;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->cgfx_flag&=(~CGFX_INTPOL_COL);
	} else {
	  render->cgfx_flag|=CGFX_INTPOL_COL;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"cap")) {
      /********************
	      hsc cap
      ********************/
      if(!strcmp(op,"!")) {
	render->cgfx_flag&=(~CGFX_HSC_CAP);
      } else if(strlen(op)==0) {
	render->cgfx_flag|=CGFX_HSC_CAP;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->cgfx_flag&=(~CGFX_HSC_CAP);
	} else {
	  render->cgfx_flag|=CGFX_HSC_CAP;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"userad")) {
      /********************
	      userad
      ********************/
      if(!strcmp(op,"!")) {
	render->cgfx_flag&=(~CGFX_USE_RAD);
      } else if(strlen(op)==0) {
	render->cgfx_flag|=CGFX_USE_RAD;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->cgfx_flag&=(~CGFX_USE_RAD);
	} else {
	  render->cgfx_flag|=CGFX_USE_RAD;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"strandm")) {
      /********************
	  strand method
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	if(atoi(val)==0 || atoi(val)==1) {
	  render->strand_method=atoi(val);
	} else {
	  sprintf(message,"invalid strand method %d\n", atoi(val));
	  comMessage(message);
	  return -1;
	}
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"nam")) {
      /********************
	  NA method
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	if(atoi(val)==0 || atoi(val)==1) {
	  render->na_method=atoi(val);
	} else {
	  sprintf(message,"invalid NA method %d\n", atoi(val));
	  comMessage(message);
	  return -1;
	}
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"helixm")) {
      /********************
	  helix method
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	if(atoi(val)==0 || atoi(val)==1) {
	  render->helix_method=atoi(val);
	} else {
	  sprintf(message,"invalid strand method %d\n", atoi(val));
	  comMessage(message);
	  return -1;
	}
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"solid")) {
      if(gfx_flags & DINO_FLAG_NOSTENCIL) {
	comMessage("Solid rendering not possible on this display: missing stencil buffer\n");
      } else {
	/********************
			     solid
	********************/
	if(!strcmp(op,"!")) {
	  render->solid=0;
	} else if(strlen(op)==0) {
	  render->solid=1;
	} else if(!strcmp(op,"=")) {
	  if(!strcmp(val,"0") ||
	     !strcmp(val,"false") ||
	     !strcmp(val,"off")) {
	    render->solid=0;
	  } else {
	    render->solid=1;
	  }
	} else {
	  sprintf(message,"invalid operator %s\n",op);
	  comMessage(message);
	  return -1;
	}
      }
    } else if(!strcmp(prop,"solidc")) {
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(strcmp(op,"=")) {
	sprintf(message,"invalid operator: %s\n",op);
	comMessage(message);
	return -1;
      }
      if(comGetColor(val, &col[0], &col[1], &col[2])!=0) {
	sprintf(message,"unknown color: %s\n", val);
	comMessage(message);
	return -1;
      }
      render->solidc[0]=col[0];
      render->solidc[1]=col[1];
      render->solidc[2]=col[2];
    } else if(!strcmp(prop,"stipple")) {
      /********************
	      stipple
      ********************/
      if(!strcmp(op,"!")) {
	render->stipple_flag=0;
      } else if(strlen(op)==0) {
	render->stipple_flag=1;
      } else if(!strcmp(op,"=")) {
	if(!strcmp(val,"0") ||
	   !strcmp(val,"false") ||
	   !strcmp(val,"off")) {
	  render->stipple_flag=0;
	} else {
	  render->stipple_flag=1;
	}
      } else {
	sprintf(message,"invalid operator %s\n",op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"stipplei")) {
      /********************
	       stipplei
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->stipplei=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->stipplei=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->stipplei=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"stippleo")) {
      /********************
	       stipplei
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->stippleo=atof(val);
      } else if(!strcmp(op,"+=")) {
	render->stippleo=atof(val);
      } else if(!strcmp(op,"-=")) {
	render->stippleo=atof(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"stipplef")) {
      /********************
	       stipplef
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->stipple_factor=atoi(val);
      } else if(!strcmp(op,"+=")) {
	render->stipple_factor=atoi(val);
      } else if(!strcmp(op,"-=")) {
	render->stipple_factor=atoi(val);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else if(!strcmp(prop,"stipplep")) {
      /********************
	       stipplep
      ********************/
      if(strlen(op)==0) {
	sprintf(message,"missing operator\n");
	comMessage(message);
	return -1;
      }
      if(strlen(val)==0) {
	sprintf(message,"missing value\n");
	comMessage(message);
	return -1;
      }
      if(!strcmp(op,"=")) {
	render->stipple_pattern=strtol(val,NULL,0);
      } else {
	sprintf(message,"invalid operator %s\n", op);
	comMessage(message);
	return -1;
      }
    } else {
      sprintf(message,"render:: unknown property %s\n",prop);
      comMessage(message);
      return -1;
    }
  }

  return 0;
}

int renderMaterialSet(struct RENDER_MATERIAL *mat, int owc, char **owl)
{
  char expr[2048];
  char prop[256];
  char op[256];
  char val[256];
  char message[256];
  char **wl;
  int wc;
  int c;
  float v[3];

  if(owc<=0)
    return 0;

  memset(expr,0,sizeof(expr));
  for(c=0;c<owc;c++)
    strcat(expr,owl[c]);
  
  dbmSplit(expr,',',&wc,&wl);
  
  for(c=0;c<wc;c++) {
    dbmSplitPOV(wl[c],prop,op,val);
    if(strlen(op)==0) {
      comMessage("error:material: missing operator for \n");
      comMessage(prop);
      return -1;
    }
    if(strcmp(op,"=")) {
      comMessage("error: material: expected operator =\n");
      return -1;
    }

    if(!strcmp(prop,"amb")) {
      if(val[0]!='{') {
	mat->amb[0]=atof(val);
	mat->amb[1]=atof(val);
	mat->amb[2]=atof(val);
      } else {
	if(matExtract1Df(val,3,v)!=0) {
	  comMessage("error in vector \n");
	  comMessage(val);
	  return -1;
	}
	mat->amb[0]=v[0];
	mat->amb[1]=v[1];
	mat->amb[2]=v[2];
      }
    } else if(!strcmp(prop,"diff")) {
      if(val[0]!='{') {
	mat->diff[0]=atof(val);
	mat->diff[1]=atof(val);
	mat->diff[2]=atof(val);
      } else {
	if(matExtract1Df(val,3,v)!=0) {
	  comMessage("error in vector \n");
	  comMessage(val);
	  return -1;
	}
	mat->diff[0]=v[0];
	mat->diff[1]=v[1];
	mat->diff[2]=v[2];
      }
    } else if(!strcmp(prop,"amb_diff")) {
      if(val[0]!='{') {
	mat->amb[0]=atof(val);
	mat->amb[1]=atof(val);
	mat->amb[2]=atof(val);
	mat->diff[0]=atof(val);
	mat->diff[1]=atof(val);
	mat->diff[2]=atof(val);
      } else {
	if(matExtract1Df(val,3,v)!=0) {
	  comMessage("error in vector \n");
	  comMessage(val);
	  return -1;
	}
	mat->amb[0]=v[0];
	mat->amb[1]=v[1];
	mat->amb[2]=v[2];
	mat->diff[0]=v[0];
	mat->diff[1]=v[1];
	mat->diff[2]=v[2];
	fprintf(stderr,"%f %f %f\n",v[0],v[1],v[2]);
      }
    } else if(!strcmp(prop,"spec")) {
      if(val[0]!='{') {
	mat->spec[0]=atof(val);
	mat->spec[1]=atof(val);
	mat->spec[2]=atof(val);
      } else {
	if(matExtract1Df(val,3,v)!=0) {
	  comMessage("error in vector \n");
	  comMessage(val);
	  return -1;
	}
	mat->spec[0]=v[0];
	mat->spec[1]=v[1];
	mat->spec[2]=v[2];
      }
    } else if(!strcmp(prop,"emm")) {
      if(val[0]!='{') {
	mat->emm[0]=atof(val);
	mat->emm[1]=atof(val);
	mat->emm[2]=atof(val);
      } else {
	if(matExtract1Df(val,3,v)!=0) {
	  comMessage("error in vector \n");
	  comMessage(val);
	  return -1;
	}
	mat->emm[0]=v[0];
	mat->emm[1]=v[1];
	mat->emm[2]=v[2];
      }
    } else if(!strcmp(prop,"shin")) {
      mat->shin=atof(val);
    } else {
      comMessage("error: material: unknown property \n");
      comMessage(prop);
      return -1;
    }
  }
  return 0;
}

static char mat_buf[1024];

const char *renderGetMaterial(struct RENDER_MATERIAL *mat)
{
  sprintf(mat_buf,"Current material setting:\namb: {%.3f,%.3f,%.3f}\nspec: {%3f,%3f,%3f}\nshin: %f\nemm: {%.3f,%.3f,%.3f}\n",
	  mat->amb[0],
	  mat->amb[1],
	  mat->amb[2],
	  mat->spec[0],
	  mat->spec[1],
	  mat->spec[2],
	  mat->shin,
	  mat->emm[0],
	  mat->emm[1],
	  mat->emm[2]);
  return mat_buf;
}

