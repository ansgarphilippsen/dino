/*
  marching cube algorithm using
  tetrahedrical subdivision of
  a unit cube, already converted
  to cube-lookup-table

  
*/
#include <stdio.h>

#include "Cmalloc.h"
#include "mat.h"

#include "scal_db.h"
#include "scal_obj.h"
#include "scal_mc_tet_table.h"
#include "scal_mc_tet.h"

static int work_cube(scalObj *obj, Select *sel, int u, int v, int w, int step);
static int calc_inters(float c[3], struct SCAL_FIELD *field, float level, float l1, float l2, int u1, int v1, int w1, int u2, int v2, int w2);
static void add_face(scalObj *obj, float c1[3], float c2[3], float c3[3]);

/*
  cube:
 

   7 __________ 6
    /.        /|
 3 / .     2 / |
  +---------+  |
  |  .      |  |
  |  .      |  |
  |  .......|..|
  | .4      | / 5
  |.        |/
  +---------+
 0           1

 v  w
 | /
 |/
 +---u

*/

static int cc_offsets[][3] = {
  {0,0,0},
  {1,0,0},
  {1,1,0},
  {0,1,0},
  {0,0,1},
  {1,0,1},
  {1,1,1},
  {0,1,1}
};


int scalMCT(scalObj *obj, Select *sel)
{
  int umin,umax,uc,usize,u;
  int vmin,vmax,vc,vsize,v;
  int wmin,wmax,wc,wsize,w;
  int step=1;

  // delete old gfx components
  if(obj->point_count>0)
    Cfree(obj->point);
  if(obj->line_count>0)
    Cfree(obj->line);
  if(obj->face_count>0)
    Cfree(obj->face);
  obj->point_count=0;
  obj->line_count=0;

  obj->face_max=1000;
  obj->face = Crecalloc(NULL,obj->face_max,sizeof(struct SCAL_FACE));
  obj->face_count=0;

  // set limits as given by user
  umin=obj->u_start;
  umax=obj->u_end;
  usize=umax-umin+1;
  vmin=obj->v_start;
  vmax=obj->v_end;
  vsize=vmax-vmin+1;
  wmin=obj->w_start;
  wmax=obj->w_end;
  wsize=wmax-wmin+1;

  for(w=wmin;w<wmax;w+=step) {
    for(v=vmin;v<vmax;v+=step) {
      for(u=umin;u<umax;u+=step) {
	work_cube(obj,sel,u,v,w,step);
      }
    }
  }

  fprintf(stderr,"faces: %d\n",obj->face_count);

  return 1;
}


static int work_cube(scalObj *obj, Select *sel, int u, int v, int w, int step)
{
  int i;
  float cval[8],level=obj->level;
  int pcode;
  int tri_num, tri_count;
  float coord[3][3];
  int cor1,cor2;

  pcode=0;
  for(i=0;i<8;i++) {
    // get values for corner
    cval[i] = scalReadField(obj->field,
			    u+cc_offsets[i][0]*step,
			    v+cc_offsets[i][1]*step,
			    w+cc_offsets[i][2]*step);

    // calculate the 8bit pattern code
    if(cval[i]>level) pcode += (1<<i);
  }
  
  //fprintf(stderr,"CUBE\npcode = %d\n",pcode);
  
  // get the amount of triangles as found in the lookup table
  tri_num = mc_edge_lookup_table[pcode].count;
  for(tri_count=0;tri_count<tri_num;tri_count++) {
    /*
      for each of the three corner pairs calculate 
      the intersection with the level
    */
    for(i=0;i<3;i++) {
      cor1=mc_edge_lookup_table[pcode].tab[tri_count][i*2+0];
      cor2=mc_edge_lookup_table[pcode].tab[tri_count][i*2+1];
      //fprintf(stderr,"(%d,%d) ",cor1,cor2);
      if(calc_inters(coord[i],
		     obj->field,
		     level,
		     cval[cor1],
		     cval[cor2],
		     u+cc_offsets[cor1][0]*step,
		     v+cc_offsets[cor1][1]*step,
		     w+cc_offsets[cor1][2]*step,
		     u+cc_offsets[cor2][0]*step,
		     v+cc_offsets[cor2][1]*step,
		     w+cc_offsets[cor2][2]*step) <0 ) {
	//return -1;
      }
      //fprintf(stderr," (%.3f,%.3f,%.3f)  ",coord[i][0],coord[i][1],coord[i][2]);
    }
    //fprintf(stderr,"\n");

    // add the three coordinates to the face entries
    add_face(obj, coord[0],coord[1],coord[2]);
  }

  return 0;
}

static int calc_inters(float c[3], struct SCAL_FIELD *field, float level, float l1, float l2, int u1, int v1, int w1, int u2, int v2, int w2)
{
  float d1 = (l2-l1);
  float d2 = (level-l2);
  float r;
  float uvw[3];

  //fprintf(stderr," (%d %d %d  %d %d %d) ",u1,v1,w1,u2,v2,w2);

  if(d1==0.0) {
    fprintf(stderr,"d1==0.0\n");
    return -1;
  }
    

  r=d2/d1;

  if(r<-1.0 || r>1.0) {
    fprintf(stderr,"r==%f\n",r);
    return -1;
  }

  uvw[0] = (float)u1 + r*(float)(u2-u1);
  uvw[1] = (float)v1 + r*(float)(v2-v1);
  uvw[2] = (float)w1 + r*(float)(w2-w1);

  //fprintf(stderr," (%.3f %.3f %.3f) ",uvw[0], uvw[1], uvw[2]);

  scalUVWtoXYZf(field, uvw, c);
  
  return 0;
}

static void add_face(scalObj *obj, float c1[3], float c2[3], float c3[3])
{
  struct SCAL_FACE *fp = &obj->face[obj->face_count];
  float v1[3],v2[3],n1[3];

  if(obj->face_count>=obj->face_max) {
    obj->face_max*=2;
    obj->face = Crecalloc(obj->face, obj->face_max, sizeof(struct SCAL_FACE));
  }

  fp->v1[0]=c1[0]; fp->v1[1]=c1[1]; fp->v1[2]=c1[2];
  fp->v2[0]=c2[0]; fp->v2[1]=c2[1]; fp->v2[2]=c2[2];
  fp->v3[0]=c3[0]; fp->v3[1]=c3[1]; fp->v3[2]=c3[2];

  v1[0]=c2[0]-c1[0]; v1[1]=c2[1]-c1[1]; v1[2]=c2[2]-c1[2];
  v2[0]=c3[0]-c1[0]; v2[1]=c3[1]-c1[1]; v2[2]=c3[2]-c1[2];
  matfCalcCross(v1,v2,n1);
  matfNormalize(n1,fp->n1);
  fp->n2[0]=fp->n1[0]; fp->n2[1]=fp->n1[1]; fp->n2[2]=fp->n1[2];
  fp->n3[0]=fp->n1[0]; fp->n3[1]=fp->n1[1]; fp->n3[2]=fp->n1[2];

  fp->c1[0]=1.0; fp->c1[1]=1.0; fp->c1[2]=0.0; fp->c1[3]=1.0;
  fp->c2[0]=1.0; fp->c2[1]=0.0; fp->c2[2]=1.0; fp->c2[3]=1.0;
  fp->c3[0]=0.0; fp->c3[1]=1.0; fp->c3[2]=1.0; fp->c3[3]=1.0;

  obj->face_count++;
}
