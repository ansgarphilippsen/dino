/*
  isocontouring algorithm

  approx the fifth attempt :)
*/

#include <stdio.h>
#include <math.h>

#include "mat.h"
#include "Cmalloc.h"

#include "scal_db.h"
#include "scal_obj.h"
#include "scal_iso_table.h"
#include "scal_iso.h"

static int iso(scalObj *obj);
static int add_point(scalObj *obj, float xyz[3]);
static void add_face(scalObj *obj, int i1, int i2, int i3);
static int gen_coord(scalObj *obj, float l1, float l2, int u1, int v1, int w1, int u2, int v2, int w2);
static void gen_faces(scalObj *obj);

int scalIso(scalObj *obj, Select *sel)
{
  // selection is currently ignored
  return iso(obj);
}


/*
  unit cube definition:
 

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

/*
 each pairwise combination of corners that
 forms a potential vertex can be encoded with
 a unique number from 0 to 27 (vid)
*/

static int corner2vid[8][8] = {
  //0   1   2   3   4   5   6   7  c2 / c1
  { -1,  0,  1,  2,  3,  4,  5,  6},  // 0
  {  0, -1,  7,  8,  9, 10, 11, 12},  // 1
  {  1,  7, -1, 13, 14, 15, 16, 17},  // 2
  {  2,  8, 13, -1, 18, 19, 20, 21},  // 3
  {  3,  9, 14, 18, -1, 22, 23, 24},  // 4
  {  4, 10, 15, 19, 22, -1, 25, 26},  // 5
  {  5, 11, 16, 20, 23, 25, -1, 27},  // 6
  {  6, 12, 17, 21, 24, 26, 27, -1}   // 7
};

/*
  neighbourhood of vertices, encoded
  with unique vid:

  u+1:
  1-2 = 0-3   7 =  2
  1-5 = 0-4  10 =  3
  1-6 = 0-7  11 =  6
  2-5 = 3-4  15 = 18
  2-6 = 3-7  16 = 21
  5-6 = 4-7  25 = 24
  
  v+1:
  2-3 = 1-0  13 =  0
  2-6 = 1-5  16 = 10
  2-7 = 1-4  17 =  9
  3-6 = 0-5  20 =  4
  3-7 = 0-4  21 =  3
  6-7 = 5-4  27 = 22
  
  w+1:
  4-5 = 0-1  22 =  0
  4-6 = 0-2  23 =  1
  4-7 = 0-3  24 =  3
  5-6 = 1-2  25 =  7
  5-7 = 1-3  26 =  8
  6-7 = 2-3  27 = 13
  
*/

static int vid_neigh_u[28] = {
  -1,-1,-1,-1,-1,-1, 2,-1,-1, 3, 6,-1,-1,-1,
  -1,18,21,-1,-1,-1,-1,-1,-1,-1,-1,24,-1,-1
};

static int vid_neigh_v[28] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,
  -1,-1,10, 9,-1,-1, 4, 3,-1,-1,-1,-1,-1,22
  
};

static int vid_neigh_w[28] = {
  -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
  -1,-1,-1,-1,-1,-1,-1,-1, 0, 1, 3, 7, 8,13
};


static int iso(scalObj *obj) 
{
  int i;
  int ustart,vstart,wstart;
  int usize,vsize,wsize;
  int uc,vc,wc;
  int *cip,*nip;
  int pcode;
  float cval[8],level=obj->level;
  int step=obj->step;
  int tri_num,tri_count;
  int tri_index[3],cor1,cor2,vert_id,coord_id;
  struct SCAL_ISO_CUBE *layer, *ccube, *ncube;

  // delete old gfx components
  if(obj->point_count>0)
    Cfree(obj->point);
  if(obj->line_count>0)
    Cfree(obj->line);
  if(obj->face_count>0)
    Cfree(obj->face);

  obj->line_count=0;

  // prepare points
  obj->point_max=1000;
  obj->point = Crealloc(NULL,obj->point_max*sizeof(struct SCAL_POINT));
  obj->point_count=0;

  // prepare face
  obj->face_max=1000;
  obj->face = Crealloc(NULL,obj->face_max*sizeof(struct SCAL_FACE));
  obj->face_count=0;

  // get limits
  ustart=obj->u_start;
  usize=obj->u_end-ustart+1;
  vstart=obj->v_start;
  vsize=obj->v_end-vstart+1;
  wstart=obj->w_start;
  wsize=obj->w_end-wstart+1;
  
  // allocate with +1, then boundary check below is not necessary
  layer = Cmalloc((usize+1)*(vsize+1)*sizeof(struct SCAL_ISO_CUBE));

  // reset layer once
  for(vc=0;vc<vsize+1;vc++) {
    for(uc=0;uc<usize+1;uc++) {
      for(i=0;i<28;i++) {
	layer[uc+vc*usize].coord_index[i]=-1;
      }
    }
  }
  
  // main loop through w-dimension
  for(wc=0;wc<wsize;wc++) {
    
    // make current uv-layer
    // copy & reset
    // +1 ok here
    for(vc=0;vc<vsize+1;vc++) {
      for(uc=0;uc<usize+1;uc++) {
	ccube = &layer[uc+vc*usize];
	// current index pointer
	cip = ccube->coord_index;
	// copy neighbour coord-indices from w to w+1
	cip[0]=cip[22]; cip[1]=cip[23]; cip[2]=cip[24]; 
	cip[7]=cip[25]; cip[8]=cip[26]; cip[13]=cip[27]; 
	// reset the rest
	cip[3]=-1; cip[4]=-1; cip[5]=-1; cip[6]=-1; 
	cip[9]=-1; cip[10]=-1; cip[11]=-1; cip[12]=-1; 
	cip[14]=-1; cip[15]=-1; cip[16]=-1; cip[17]=-1; 
	cip[18]=-1; cip[19]=-1; cip[20]=-1; cip[21]=-1; 
	cip[22]=-1; cip[23]=-1; cip[24]=-1; cip[25]=-1; 
	cip[26]=-1; cip[27]=-1;

	/*
	fprintf(stderr,"w %d\n",wc);
	for(i=0;i<28;i++) {
	  fprintf(stderr,"%2d ",cip[i]);
	}
	fprintf(stderr,"\n");
	*/

      }
    }

    // generate new coordinates
    for(vc=0;vc<vsize;vc++) {
      for(uc=0;uc<usize;uc++) {
	ccube = &layer[uc+vc*usize];

	// calculate pcode
	pcode=0;
	for(i=0;i<8;i++) {
	  // get values for corners
	  cval[i] = scalReadField(obj->field,
				  uc+ustart+cc_offsets[i][0]*step,
				  vc+vstart+cc_offsets[i][1]*step,
				  wc+wstart+cc_offsets[i][2]*step);
	  
	  // calculate the 8bit pattern code
	  if(cval[i]>level) pcode += (1<<i);
	}

	// get the amount of triangles as found in the lookup table
	tri_num = iso_edge_lookup_table[pcode].count;
	for(tri_count=0;tri_count<tri_num;tri_count++) {
	  /*
	    for each of the three corner pairs calculate 
	    the intersection with the level
	  */
	  
	  for(i=0;i<3;i++) {
	    cor1=iso_edge_lookup_table[pcode].tab[tri_count][i*2+0];
	    cor2=iso_edge_lookup_table[pcode].tab[tri_count][i*2+1];
	    // vertex id of corners
	    vert_id = corner2vid[cor1][cor2];
	    //fprintf(stderr,"%2d %2d -> %2d\n",cor1,cor2,vert_id);
	    // coordinate id
	    coord_id = ccube->coord_index[vert_id];
	    // if not yet generated, create new
	    if(coord_id<0) {
	      coord_id = gen_coord(obj,
				   cval[cor1],cval[cor2],
				   uc+ustart+cc_offsets[cor1][0]*step,
				   vc+vstart+cc_offsets[cor1][1]*step,
				   wc+wstart+cc_offsets[cor1][2]*step,
				   uc+ustart+cc_offsets[cor2][0]*step,
				   vc+vstart+cc_offsets[cor2][1]*step,
				   wc+wstart+cc_offsets[cor2][2]*step);
	      /*
	      fprintf(stderr,"%d %d %d - %d %d:  %d\n",
		      u,v,w, cor1,cor2, coord_id);
	      */
	      ccube->coord_index[vert_id] = coord_id;
	    }
	    tri_index[i] = coord_id;
	  }
	  add_face(obj,tri_index[0],tri_index[1],tri_index[2]);
	}

	// copy coordinate indexes to u and v neighbours
	cip = ccube->coord_index;

	/*
	  u+1:
	  1-2 = 0-3   7 =  2
	  1-5 = 0-4  10 =  3
	  1-6 = 0-7  11 =  6
	  2-5 = 3-4  15 = 18
	  2-6 = 3-7  16 = 21
	  5-6 = 4-7  25 = 24
	*/  

	// boundary check with +1 not necessary due to above +1 malloc
	nip = layer[(uc+1)+vc*usize].coord_index;
	nip[2] = cip[7];
	nip[3] = cip[10];
	nip[6] = cip[11];
	nip[18] = cip[15];
	nip[21] = cip[16];
	nip[24] = cip[25];

	/*
	  v+1:
	  2-3 = 1-0  13 =  0
	  2-6 = 1-5  16 = 10
	  2-7 = 1-4  17 =  9
	  3-6 = 0-5  20 =  4
	  3-7 = 0-4  21 =  3
	  6-7 = 5-4  27 = 22
	*/
	// boundary check with +1 not necessary due to above +1 malloc
	nip = layer[uc+(vc+1)*usize].coord_index;
	nip[0] = cip[13];
	nip[10] = cip[16];
	nip[9] = cip[17];
	nip[4] = cip[20];
	nip[3] = cip[21];
	nip[22] = cip[27];
	

      } // uc
    } // vc
  } // wc

  // make faces from indices
  gen_faces(obj);

  fprintf(stderr,"generated %d points and %d faces\n",
	  obj->point_count,obj->face_count);

  return 0;
}




static int gen_coord(scalObj *obj, float l1, float l2, int u1, int v1, int w1, int u2, int v2, int w2)
{
  float d1,d2,r;
  float uvw[3],xyz[3];

  d1 = (l1-l2);

  if(d1==0.0) {
    // this should actually never happen!
    return -1;
  }

  d2 = (obj->level-l2);
  r=d2/d1;

  uvw[0] = (float)u2+r*(float)(u1-u2);
  uvw[1] = (float)v2+r*(float)(v1-v2);
  uvw[2] = (float)w2+r*(float)(w1-w2);

  // temp: check limits
  if((uvw[0]<(float)u1 && uvw[0]<(float)u2) ||
     (uvw[0]>(float)u1 && uvw[0]>(float)u2) ||
     (uvw[1]<(float)v1 && uvw[1]<(float)v2) ||
     (uvw[1]>(float)v1 && uvw[1]>(float)v2) ||
     (uvw[2]<(float)w1 && uvw[2]<(float)w2) ||
     (uvw[2]>(float)w1 && uvw[2]>(float)w2)) {
    fprintf(stderr,"%d %d %d %.4g | %d %d %d %.4g | %.4g %.4g | %.4g %.4g %.4g\n",
	    u1,v1,w1,l1,
	    u2,v2,w2,l2,
	    obj->level,r,
	    uvw[0],uvw[1],uvw[2]);
  }

  scalUVWtoXYZf(obj->field, uvw, xyz);

  return add_point(obj,xyz);
}

static int add_point(scalObj *obj, float xyz[3])
{
  if(obj->point_count>=obj->point_max) {
    obj->point_max*=2;
    obj->point = Crealloc(obj->point,obj->point_max*sizeof(struct SCAL_POINT));
  }
  obj->point[obj->point_count].v[0]=xyz[0];
  obj->point[obj->point_count].v[1]=xyz[1];
  obj->point[obj->point_count].v[2]=xyz[2];

  //fprintf(stderr,"(%g %g %g)\n",xyz[0],xyz[1],xyz[2]);

  obj->point_count++;
  return (obj->point_count-1);
}

static void add_face(scalObj *obj, int i1, int i2, int i3)
{
  if(obj->face_count>=obj->face_max) {
    obj->face_max*=2;
    obj->face = Crealloc(obj->face,obj->face_max*sizeof(struct SCAL_FACE));
  }
  if(i1<0 || i2<0 || i3<0) {
    fprintf(stderr,"negative coord index!\n");
  } else {
    obj->face[obj->face_count].pi0 = i1;
    obj->face[obj->face_count].pi1 = i3;
    obj->face[obj->face_count].pi2 = i2;
    obj->face_count++;
  }
}

static void gen_faces(scalObj *obj)
{
  int i,k;
  struct SCAL_FACE *f;
  float v1[3],v2[3],n1[3];
  float area, inv_area;

  for(i=0;i<obj->face_count;i++) {
    f=&obj->face[i];
    
    // vertices
    for(k=0;k<3;k++) {
      f->v1[k] = obj->point[f->pi0].v[k];
      f->v2[k] = obj->point[f->pi1].v[k];
      f->v3[k] = obj->point[f->pi2].v[k];
    }
    // color

    f->c1[0]=obj->r;
    f->c1[1]=obj->g;
    f->c1[2]=obj->b;
    f->c1[3]=obj->render.transparency;
    f->c2[0]=obj->r;
    f->c2[1]=obj->g;
    f->c2[2]=obj->b;
    f->c2[3]=obj->render.transparency;
    f->c3[0]=obj->r;
    f->c3[1]=obj->g;
    f->c3[2]=obj->b;
    f->c3[3]=obj->render.transparency;
  }

  // reset point normals
  for(i=0;i<obj->point_count;i++) {
    obj->point[i].n[0]=0.0;
    obj->point[i].n[1]=0.0;
    obj->point[i].n[2]=0.0;
    obj->point[i].nc=0;
  }

  for(i=0;i<obj->face_count;i++) {
    f=&obj->face[i];

    // face normal
    v1[0]=f->v2[0] - f->v1[0];
    v1[1]=f->v2[1] - f->v1[1];
    v1[2]=f->v2[2] - f->v1[2];
    v2[0]=f->v3[0] - f->v1[0]; 
    v2[1]=f->v3[1] - f->v1[1]; 
    v2[2]=f->v3[2] - f->v1[2];
    matfCalcCross(v1,v2,n1);
    matfNormalize(n1,n1);

    // weigh by face area!
    area = matCalcTriArea(f->v1,f->v2,f->v3);
    if(area==0.0) {
      inv_area=1.0;
    } else {
      inv_area = 1000.0/area;
    }

    // not working ??
    //inv_area=1.0;
    
    // add to points
    for(k=0;k<3;k++) {
      obj->point[f->pi0].n[k]+=(n1[k]*area);
      obj->point[f->pi1].n[k]+=(n1[k]*area);
      obj->point[f->pi2].n[k]+=(n1[k]*area);
    }
  }

  // calculate point normals
  for(i=0;i<obj->point_count;i++) {
    matfNormalize(obj->point[i].n,obj->point[i].n);
  }

  // assign to face
  for(i=0;i<obj->face_count;i++) {
    f=&obj->face[i];
    for(k=0;k<3;k++) {
      f->n1[k]=obj->point[f->pi0].n[k];
      f->n2[k]=obj->point[f->pi1].n[k];
      f->n3[k]=obj->point[f->pi2].n[k];
    }

  }
}
