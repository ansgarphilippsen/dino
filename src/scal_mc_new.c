#include <stdio.h>
#include <string.h>

#include "Cmalloc.h"
#include "scal_mc_new.h"
#include "mat.h"
#include "com.h"

#include "scal_mc_new_lookup.h"

struct SCAL_MC_NEW_ORG scalMCNOrg;

#define SCAL_MCN_DEBUG_X

#ifdef SCAL_MCN_DEBUG
int scal_mcn_flag;
#endif

int scalMCN(scalObj *obj, Select *sel)
{
  int umin,umax,uc,usize,u;
  int vmin,vmax,vc,vsize,v;
  int wmin,wmax,wc,wsize,w;

  int *save_w_u, *save_w_v;
  int *save2_w_u, *save2_w_v;
  int *save_v_u, *save_v_w;
  int *save2_v_u, *save2_v_w;
  int save_u_v, save_u_w;
  int *save_tmp;

  scalMCNOrg.obj=obj;
  scalMCNOrg.field=obj->field;
  scalMCNOrg.level=obj->level;
  scalMCNOrg.select=sel;

  if(obj->point_count>0)
    Cfree(obj->point);
  if(obj->line_count>0)
    Cfree(obj->line);
  if(obj->face_count>0)
    Cfree(obj->face);

  obj->point_count=0;
  obj->line_count=0;
  obj->face_count=0;

  umin=obj->u_start;
  umax=obj->u_end;
  usize=umax-umin+1;
  vmin=obj->v_start;
  vmax=obj->v_end;
  vsize=vmax-vmin+1;
  wmin=obj->w_start;
  wmax=obj->w_end;
  wsize=wmax-wmin+1;

  scalMCNOrg.vert_count=0;
  scalMCNOrg.vert_max=usize*vsize*wsize/2;
  scalMCNOrg.vert_add=scalMCNOrg.vert_max/2;
  scalMCNOrg.vert=Crecalloc(NULL,scalMCNOrg.vert_max, sizeof(scalMCNVert));

  scalMCNOrg.line_count=0;
  scalMCNOrg.line_max=usize*vsize*wsize*2;
  scalMCNOrg.line_add=scalMCNOrg.line_max/2;
  scalMCNOrg.line=Crecalloc(NULL,scalMCNOrg.line_max, sizeof(scalMCNLine));

  scalMCNOrg.face_count=0;
  scalMCNOrg.face_max=usize*vsize*wsize;
  scalMCNOrg.face_add=scalMCNOrg.face_max/2;
  scalMCNOrg.face=Crecalloc(NULL,scalMCNOrg.face_max, sizeof(scalMCNFace));


  /**
  fprintf(stderr,"\n%d %d %d   %d %d %d\n%f",umin,vmin,wmin,umax,vmax,wmax,obj->level);
  **/
  save_w_u=Ccalloc((usize+2)*(vsize+2),sizeof(int));
  save_w_v=Ccalloc((usize+2)*(vsize+2),sizeof(int));
  save2_w_u=Ccalloc((usize+2)*(vsize+2),sizeof(int));
  save2_w_v=Ccalloc((usize+2)*(vsize+2),sizeof(int));

  save_v_u=Ccalloc(usize+2,sizeof(int));
  save_v_w=Ccalloc(usize+2,sizeof(int));
  save2_v_u=Ccalloc(usize+2,sizeof(int));
  save2_v_w=Ccalloc(usize+2,sizeof(int));


  /* create first w array */
  for(vc=0,v=vmin;v<=vmax;v++,vc++) { /* NOTE the <= */
    for(uc=0,u=umin;u<=umax;u++,uc++) { /* NOTE the <= */
      save_w_u[vc*usize+uc]=scalMCNCalcVert(u,v,wmin,1);
      save_w_v[vc*usize+uc]=scalMCNCalcVert(u,v,wmin,0);
    }
  }

  /* now for the main loop */

  for(wc=0,w=wmin;w<wmax;wc++,w++) {

    /* create first v array */
    for(uc=0,u=umin;u<=umax;u++,uc++) {  /* NOTE the <= */
      save_v_u[uc]=scalMCNCalcVert(u,vmin,w,8);
      save_v_w[uc]=scalMCNCalcVert(u,vmin,w,4);
    }
    for(vc=0,v=vmin;v<vmax;vc++,v++) {
      save_u_v=scalMCNCalcVert(umin,v,w,9);
      save_u_w=scalMCNCalcVert(umin,v,w,5);

      for(uc=0,u=umin;u<umax;uc++,u++) {

	scalMCNOrg.sc.p[1]=save_w_u[vc*usize+uc];
	scalMCNOrg.sc.p[0]=save_w_v[vc*usize+uc];
	scalMCNOrg.sc.p[3]=save_w_u[vc*usize+uc+1];
	scalMCNOrg.sc.p[2]=save_w_v[(vc+1)*usize+uc];

	scalMCNOrg.sc.p[8]=save_v_u[uc];
	scalMCNOrg.sc.p[4]=save_v_w[uc];
	scalMCNOrg.sc.p[11]=save_v_u[uc+1];

	scalMCNOrg.sc.p[9]=save_u_v;
	scalMCNOrg.sc.p[5]=save_u_w;

	scalMCNOrg.sc.p[6]=scalMCNCalcVert(u,v,w,6);
	scalMCNOrg.sc.p[7]=scalMCNCalcVert(u,v,w,7);
	scalMCNOrg.sc.p[10]=scalMCNCalcVert(u,v,w,10);

	scalMCNCell();

	save2_w_u[vc*usize+uc]=scalMCNOrg.sc.p[5];
	save2_w_v[vc*usize+uc]=scalMCNOrg.sc.p[4];
	save2_v_u[uc]=scalMCNOrg.sc.p[9];
	save2_v_w[uc]=scalMCNOrg.sc.p[6];
	save_u_v=scalMCNOrg.sc.p[10];
	save_u_w=scalMCNOrg.sc.p[7];
      }
      /* uc==usize ! */
      save2_w_u[vc*usize+uc]=scalMCNOrg.sc.p[7];
      save2_v_u[uc]=scalMCNOrg.sc.p[10];

      save_tmp=save_v_u;
      save_v_u=save2_v_u;
      save2_v_u=save_tmp;
      save_tmp=save_v_w;
      save_v_w=save2_v_w;
      save2_v_w=save_tmp;
    }
    for(uc=0;uc<usize;uc++)
      save2_w_v[vc*usize+uc]=save_v_w[uc];
    save2_w_u[vc*usize+uc]=scalMCNOrg.sc.p[7];
//    save2_w_u[vc*usize+uc]=-1;

    save_tmp=save_w_u;
    save_w_u=save2_w_u;
    save2_w_u=save_tmp;
    save_tmp=save_w_v;
    save_w_v=save2_w_v;
    save2_w_v=save_tmp;
  }

  Cfree(save_w_u);
  Cfree(save_w_v);
  Cfree(save2_w_u);
  Cfree(save2_w_v);
  Cfree(save_v_u);
  Cfree(save_v_w);
  Cfree(save2_v_u);
  Cfree(save2_v_w);


  scalMCN2Obj();

  return 0;
}

int scalMCNCell()
{
  int i;

  int lookup;
  int index_count;
  int entry_count;
  int entry_offset;
  int *entry,*p;
  int p1,p2,p3,p4,p5,p6;
 
  lookup=0;
  if(scalMCNOrg.sc.p[0]>-1) lookup += (1<<0);
  if(scalMCNOrg.sc.p[1]>-1) lookup += (1<<1);
  if(scalMCNOrg.sc.p[2]>-1) lookup += (1<<2);
  if(scalMCNOrg.sc.p[3]>-1) lookup += (1<<3);
  if(scalMCNOrg.sc.p[4]>-1) lookup += (1<<4);
  if(scalMCNOrg.sc.p[5]>-1) lookup += (1<<5);
  if(scalMCNOrg.sc.p[6]>-1) lookup += (1<<6);
  if(scalMCNOrg.sc.p[7]>-1) lookup += (1<<7);
  if(scalMCNOrg.sc.p[8]>-1) lookup += (1<<8);
  if(scalMCNOrg.sc.p[9]>-1) lookup += (1<<9);
  if(scalMCNOrg.sc.p[10]>-1) lookup += (1<<10);
  if(scalMCNOrg.sc.p[11]>-1) lookup += (1<<11);

  p=scalMCNOrg.sc.p;

  index_count=mc_hash_index[lookup].c;
  for(i=0;i<index_count;i++) {
    entry_count=mc_hash_index[lookup].i[i][0];
    entry_offset=mc_hash_index[lookup].i[i][1];
    entry=&mc_hash_list[entry_offset];
#ifdef SCAL_MCN_DEBUG
    scal_mcn_flag=entry_count;
#endif
    switch(entry_count) {
    case 3:
      p1=entry[0]; p2=entry[1]; p3=entry[2];
      scalMCNAddLine(p[p1],p[p2]);
      scalMCNAddLine(p[p2],p[p3]);
      scalMCNAddLine(p[p3],p[p1]);
      scalMCNAddFace(p[p1],p[p2],p[p3]);
      break;
    case 4:
      p1=entry[0]; p2=entry[1]; p3=entry[2]; p4=entry[3];
      scalMCNAddLine(p[p1],p[p2]);
      scalMCNAddLine(p[p2],p[p3]);
      scalMCNAddLine(p[p3],p[p4]);
      scalMCNAddLine(p[p4],p[p1]);
      scalMCNAddFace(p[p1],p[p2],p[p3]);
      scalMCNAddFace(p[p3],p[p4],p[p1]);
      break;
    case 5:
      p1=entry[0]; p2=entry[1]; p3=entry[2]; p4=entry[3]; p5=entry[4];
      scalMCNAddLine(p[p1],p[p2]);
      scalMCNAddLine(p[p2],p[p3]);
      scalMCNAddLine(p[p3],p[p4]);
      scalMCNAddLine(p[p4],p[p5]);
      scalMCNAddLine(p[p5],p[p1]);
      scalMCNAddFace(p[p1],p[p2],p[p3]);
      scalMCNAddFace(p[p1],p[p3],p[p4]);
      scalMCNAddFace(p[p1],p[p4],p[p5]);
      break;
    case 6:
      p1=entry[0]; p2=entry[1]; p3=entry[2]; p4=entry[3]; p5=entry[4]; p6=entry[5];
      scalMCNAddLine(p[p1],p[p2]);
      scalMCNAddLine(p[p2],p[p3]);
      scalMCNAddLine(p[p3],p[p4]);
      scalMCNAddLine(p[p4],p[p5]);
      scalMCNAddLine(p[p5],p[p6]);
      scalMCNAddLine(p[p6],p[p1]);
      scalMCNAddFace(p[p1],p[p2],p[p3]);
      scalMCNAddFace(p[p3],p[p4],p[p5]);
      scalMCNAddFace(p[p5],p[p6],p[p1]);
      scalMCNAddFace(p[p1],p[p3],p[p5]);
      break;
    }

  }

  return 0;
}

float scal_mc_new_cube_coord[][3]={
  {0,0,0},{1,0,0},{0,1,0},{1,1,0},
  {0,0,1},{1,0,1},{0,1,1},{1,1,1}
};

int scal_mc_new_cube_coordi[][3]={
  {0,0,0},{1,0,0},{0,1,0},{1,1,0},
  {0,0,1},{1,0,1},{0,1,1},{1,1,1}
};

int scal_mc_new_cube_edge[][2]={
  {0,1},{0,2},{2,3},{1,3},
  {4,5},{4,6},{6,7},{5,7},
  {0,4},{2,6},{3,7},{1,5}  
};

int scalMCNCalcVert(int u, int v, int w, int id)
{
  int *p1,*p2;
  float *c1,*c2;
  float s1,s2,d1,d2,r;
  float uvw[4],xyz[4];
  float u1,u2,v1,v2,w1,w2;

  p1=scal_mc_new_cube_coordi[scal_mc_new_cube_edge[id][0]];
  p2=scal_mc_new_cube_coordi[scal_mc_new_cube_edge[id][1]];

  c1=scal_mc_new_cube_coord[scal_mc_new_cube_edge[id][0]];
  c2=scal_mc_new_cube_coord[scal_mc_new_cube_edge[id][1]];

  // apply selection here
  if(scalIsSelected(scalMCNOrg.obj->node, 
		     p1[0]+u, p1[1]+v, p1[2]+w,
		     scalMCNOrg.select)!=1)
    return -1;

  if(scalIsSelected(scalMCNOrg.obj->node, 
		     p2[0]+u, p2[1]+v, p2[2]+w,
		     scalMCNOrg.select)!=1)
    return -1;

  s1=scalReadField(scalMCNOrg.field,
		   p1[0]+u,p1[1]+v,p1[2]+w);
  s2=scalReadField(scalMCNOrg.field,
		   p2[0]+u,p2[1]+v,p2[2]+w);

  d1=s2-s1;
  d2=scalMCNOrg.level-s1;

  if(d1==0.0) {
    return -1;
//    r=0.5; causes huge vert_count in weird datasets
  } else {

    r=d2/d1;
    
    if(r>1.0 || r<0.0)
      return -1;
  }

  u1=(float)u+c1[0];
  v1=(float)v+c1[1];
  w1=(float)w+c1[2];
  u2=(float)u+c2[0];
  v2=(float)v+c2[1];
  w2=(float)w+c2[2];
  uvw[0]=u1+r*(u2-u1);
  uvw[1]=v1+r*(v2-v1);
  uvw[2]=w1+r*(w2-w1);

  scalUVWtoXYZf(scalMCNOrg.field,uvw,xyz);

  r=scalMCNAddVert(xyz);

  return r;
}

int scalMCNAddVert(float *p)
{
  scalMCNVert *ov;

  scalMCNOrg.vert[scalMCNOrg.vert_count].p[0]=p[0];
  scalMCNOrg.vert[scalMCNOrg.vert_count].p[1]=p[1];
  scalMCNOrg.vert[scalMCNOrg.vert_count].p[2]=p[2];
  scalMCNOrg.vert[scalMCNOrg.vert_count].cc=0;

  scalMCNOrg.vert_count++;

  if(scalMCNOrg.vert_count>=scalMCNOrg.vert_max) {
    /*
    ov=scalMCNOrg.vert;
    scalMCNOrg.vert=Ccalloc(scalMCNOrg.vert_max+scalMCNOrg.vert_add,sizeof(scalMCNVert));
    memcpy(scalMCNOrg.vert,ov,sizeof(scalMCNVert)*scalMCNOrg.vert_max);
    Cfree(ov);
    */
//    fprintf(stderr,"\nrecalloc vert");
    scalMCNOrg.vert=Crecalloc(scalMCNOrg.vert,
			      scalMCNOrg.vert_max+scalMCNOrg.vert_add,
			      sizeof(scalMCNVert));

    scalMCNOrg.vert_max+=scalMCNOrg.vert_add;
  }

  return scalMCNOrg.vert_count-1;
}

int scalMCNAddLine(int v1,int v2)
{
#ifdef SCAL_MCN_DEBUG
  int i;

  for(i=0;i<scalMCNOrg.line_count;i++) {
    if(scalMCNOrg.line[i].v1==v1) {
      if(scalMCNOrg.line[i].v2==v2) {
	return 1;
	fprintf(stderr,"\ndouble entry detected  %d",scal_mcn_flag);
      }
    }
  }
#endif

  scalMCNOrg.line[scalMCNOrg.line_count].v1=v1;
  scalMCNOrg.line[scalMCNOrg.line_count].v2=v2;

  scalMCNOrg.line_count++;

  if(scalMCNOrg.line_count>=scalMCNOrg.line_max) {
    scalMCNOrg.line=Crecalloc(scalMCNOrg.line,
			      scalMCNOrg.line_max+scalMCNOrg.line_add,
			      sizeof(scalMCNLine));

    scalMCNOrg.line_max+=scalMCNOrg.line_add;
  }

  return 0;
}

int scalMCNAddFace(int v1, int v2, int v3)
{
  scalMCNFace *of;

  scalMCNOrg.face[scalMCNOrg.face_count].v1=v1;
  scalMCNOrg.face[scalMCNOrg.face_count].v2=v2;
  scalMCNOrg.face[scalMCNOrg.face_count].v3=v3;

  if(scalMCNOrg.vert[v1].cc<7)
    scalMCNOrg.vert[v1].c[scalMCNOrg.vert[v1].cc++]=scalMCNOrg.face_count;
  if(scalMCNOrg.vert[v2].cc<7)
    scalMCNOrg.vert[v2].c[scalMCNOrg.vert[v2].cc++]=scalMCNOrg.face_count;
  if(scalMCNOrg.vert[v3].cc<7)
    scalMCNOrg.vert[v3].c[scalMCNOrg.vert[v3].cc++]=scalMCNOrg.face_count;

  scalMCNOrg.face_count++;

  if(scalMCNOrg.face_count>=scalMCNOrg.face_max) {
    /*
    of=scalMCNOrg.face;
    scalMCNOrg.face=Ccalloc(scalMCNOrg.face_max+scalMCNOrg.face_add,sizeof(scalMCNFace));
    memcpy(scalMCNOrg.face,of,sizeof(scalMCNFace)*scalMCNOrg.face_max);
    Cfree(of);
    */

//    fprintf(stderr,"\nrecalloc face");
    scalMCNOrg.face=Crecalloc(scalMCNOrg.face,
			      scalMCNOrg.face_max+scalMCNOrg.face_add,
			      sizeof(scalMCNFace));

    scalMCNOrg.face_max+=scalMCNOrg.face_add;
  }

  return 0;
}

int scalMCN2Obj()
{
  int i,j,k;
  float nt[3];
  float *np;
  int pi[3];
  char message[256];

  scalObj *obj=scalMCNOrg.obj;


  obj->face_count=scalMCNOrg.face_count;
  obj->face=Ccalloc(obj->face_count,sizeof(struct SCAL_FACE));


  for(i=0;i<obj->face_count;i++) {
    obj->face[i].pi0=scalMCNOrg.face[i].v1;
    obj->face[i].pi1=scalMCNOrg.face[i].v2;
    obj->face[i].pi2=scalMCNOrg.face[i].v3;
    obj->face[i].v1[0]=scalMCNOrg.vert[obj->face[i].pi0].p[0];
    obj->face[i].v1[1]=scalMCNOrg.vert[obj->face[i].pi0].p[1];
    obj->face[i].v1[2]=scalMCNOrg.vert[obj->face[i].pi0].p[2];
    obj->face[i].v2[0]=scalMCNOrg.vert[obj->face[i].pi1].p[0];
    obj->face[i].v2[1]=scalMCNOrg.vert[obj->face[i].pi1].p[1];
    obj->face[i].v2[2]=scalMCNOrg.vert[obj->face[i].pi1].p[2];
    obj->face[i].v3[0]=scalMCNOrg.vert[obj->face[i].pi2].p[0];
    obj->face[i].v3[1]=scalMCNOrg.vert[obj->face[i].pi2].p[1];
    obj->face[i].v3[2]=scalMCNOrg.vert[obj->face[i].pi2].p[2];

    scalMCNFaceNormal(obj->face[i].v1,obj->face[i].v2,obj->face[i].v3,
		      scalMCNOrg.face[i].n);

  }

  for(i=0;i<obj->face_count;i++) {
    pi[0]=scalMCNOrg.face[i].v1;
    pi[1]=scalMCNOrg.face[i].v2;
    pi[2]=scalMCNOrg.face[i].v3;
    for(j=0;j<3;j++) {
      nt[0]=scalMCNOrg.face[i].n[0];
      nt[1]=scalMCNOrg.face[i].n[1];
      nt[2]=scalMCNOrg.face[i].n[2];
      for(k=0;k<scalMCNOrg.vert[pi[j]].cc;k++) {
	np=scalMCNOrg.face[scalMCNOrg.vert[pi[j]].c[k]].n;
	if(matfCalcDot(nt,np)>=0.0) {
	  nt[0]+=np[0];
	  nt[1]+=np[1];
	  nt[2]+=np[2];
	} else {
	  nt[0]-=np[0];
	  nt[1]-=np[1];
	  nt[2]-=np[2];
	}
      }
      nt[0]-=scalMCNOrg.face[i].n[0];
      nt[1]-=scalMCNOrg.face[i].n[1];
      nt[2]-=scalMCNOrg.face[i].n[2];

      nt[0]/=(float)k;
      nt[1]/=(float)k;
      nt[2]/=(float)k;

      if(j==0) 
	matfNormalize(nt,obj->face[i].n1);
      else if(j==1)
	matfNormalize(nt,obj->face[i].n2);
      else
	matfNormalize(nt,obj->face[i].n3);
    }
  }


  Cfree(scalMCNOrg.face);

  obj->point_count=scalMCNOrg.vert_count;
  obj->point=Ccalloc(obj->point_count,sizeof(struct SCAL_POINT));
  for(i=0;i<obj->point_count;i++) {
    obj->point[i].v[0]=scalMCNOrg.vert[i].p[0];
    obj->point[i].v[1]=scalMCNOrg.vert[i].p[1];
    obj->point[i].v[2]=scalMCNOrg.vert[i].p[2];
    obj->point[i].nc=0;
  }

  Cfree(scalMCNOrg.vert);

  for(i=0;i<obj->face_count;i++) {
    if(matfCalcNDot(obj->point[obj->face[i].pi0].n,obj->face[i].n1)<0) {
      obj->point[obj->face[i].pi0].n[0]+=-obj->face[i].n1[0];
      obj->point[obj->face[i].pi0].n[1]+=-obj->face[i].n1[1];
      obj->point[obj->face[i].pi0].n[2]+=-obj->face[i].n1[2];
    } else {
      obj->point[obj->face[i].pi0].n[0]+=obj->face[i].n1[0];
      obj->point[obj->face[i].pi0].n[1]+=obj->face[i].n1[1];
      obj->point[obj->face[i].pi0].n[2]+=obj->face[i].n1[2];
    }
    obj->point[obj->face[i].pi0].nc++;

    if(matfCalcNDot(obj->point[obj->face[i].pi1].n,obj->face[i].n2)<0) {
      obj->point[obj->face[i].pi1].n[0]+=-obj->face[i].n2[0];
      obj->point[obj->face[i].pi1].n[1]+=-obj->face[i].n2[1];
      obj->point[obj->face[i].pi1].n[2]+=-obj->face[i].n2[2];
    } else {
      obj->point[obj->face[i].pi1].n[0]+=obj->face[i].n2[0];
      obj->point[obj->face[i].pi1].n[1]+=obj->face[i].n2[1];
      obj->point[obj->face[i].pi1].n[2]+=obj->face[i].n2[2];
    }
    obj->point[obj->face[i].pi1].nc++;

    if(matfCalcNDot(obj->point[obj->face[i].pi2].n,obj->face[i].n3)<0) {
      obj->point[obj->face[i].pi2].n[0]+=-obj->face[i].n3[0];
      obj->point[obj->face[i].pi2].n[1]+=-obj->face[i].n3[1];
      obj->point[obj->face[i].pi2].n[2]+=-obj->face[i].n3[2];
    } else {
      obj->point[obj->face[i].pi2].n[0]+=obj->face[i].n3[0];
      obj->point[obj->face[i].pi2].n[1]+=obj->face[i].n3[1];
      obj->point[obj->face[i].pi2].n[2]+=obj->face[i].n3[2];
    }
    obj->point[obj->face[i].pi2].nc++;


  }

  for(i=0;i<obj->point_count;i++) {
    if(obj->point[i].nc>0) {
      obj->point[i].n[0]/=(float)obj->point[i].nc;
      obj->point[i].n[1]/=(float)obj->point[i].nc;
      obj->point[i].n[2]/=(float)obj->point[i].nc;
      matfNormalize(obj->point[i].n,obj->point[i].n);
    }
  }


  obj->line_count=scalMCNOrg.line_count;
  obj->line=Ccalloc(obj->line_count,sizeof(struct SCAL_LINE));

  for(i=0;i<obj->line_count;i++) {
    obj->line[i].pi0=scalMCNOrg.line[i].v1;
    obj->line[i].pi1=scalMCNOrg.line[i].v2;
  }

  Cfree(scalMCNOrg.line);

  sprintf(message," %d points  %d lines  %d faces",
	  obj->point_count,obj->line_count, obj->face_count);
  comMessage(message);

  return 0;
}

void scalMCNFaceNormal(float *a, float *b, float *c, float *n)
{
  float d1[3],d2[3];

  d1[0]=b[0]-a[0];
  d1[1]=b[1]-a[1];
  d1[2]=b[2]-a[2];
  d2[0]=c[0]-a[0];
  d2[1]=c[1]-a[1];
  d2[2]=c[2]-a[2];

  matfCalcCross(d1,d2,n);
//matfNormalize(d3,n);

}

