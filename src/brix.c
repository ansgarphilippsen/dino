#include "dino.h"
#include "com.h"
#include "Cmalloc.h"

#include "brix.h"

extern int debug_mode;

int brixRead(FILE *f, dbmScalNode *sn)
{
  char head[512];
  int origin[3],extend[3],grid[3];
  float cell[6], par[3];
  char mesg[512];
  int size;
  int inx,iny,inz;
  int xc,yc,zc;
  float val,bval;
  int bc;
  int u,v,w;
  float fx,fy,fz;
  int ret;
  float tv1,tv2;

  fread(head,sizeof(char),512,f);


  if(!(head[0]==':' && head[1]=='-' && head[2]==')')) {
    comMessage("file header magic number incorrect\n");
    return -1;
  }

  sscanf(head,
	 "%*10c%5d%5d%d%*7c%5d%5d%5d%*5c%5d%5d%5d%*6c%10f%10f%10f%10f%10f%10f%*5c%12f%*5c%8f%*7c%12f",
	 origin+0,origin+1,origin+2,
	 extend+0,extend+1,extend+2,
	 grid+0,grid+1,grid+2,
	 cell+0,cell+1,cell+2,cell+3,cell+4,cell+5,
	 par+0,par+1,par+2);

  sprintf(mesg,
	  "header:\n%d %d %d  %d %d %d  %d %d %d\n%f %f %f %f %f %f\n%f %f %f\n",
	  origin[0],origin[1],origin[2],
	  extend[0],extend[1],extend[2],
	  grid[0],grid[1],grid[2],
	  cell[0],cell[1],cell[2],cell[3],cell[4],cell[5],
	  par[0],par[1],par[2]);

  debmsg(mesg);
  

  size=extend[0]*extend[1]*extend[2];

  sn->field=Cmalloc(sizeof(struct SCAL_FIELD));
  sn->field->size=size;

  if(sn->field==NULL) {
    comMessage("brixRead: memory allocation error\n");
    return -1;
  }

  sn->field->data=Ccalloc(size,sizeof(float));

  memset(sn->field->data,0,size);

  sn->field->u_size=extend[0];
  sn->field->v_size=extend[1];
  sn->field->w_size=extend[2];
  sn->field->u1=origin[0];
  sn->field->v1=origin[1];
  sn->field->w1=origin[2];
  sn->field->u2=sn->field->u1+sn->field->u_size-1;
  sn->field->v2=sn->field->v1+sn->field->v_size-1;
  sn->field->w2=sn->field->w1+sn->field->w_size-1;
  sn->field->wrap=0;
  sn->field->offset_x=0.0;
  sn->field->offset_y=0.0;
  sn->field->offset_z=0.0;
  sn->field->sigma=par[2];
  sn->field->scale=1.0;

  // read blocks / brix of 8x8x8 grid points
  inx = extend[0]/8;
  iny = extend[1]/8;
  inz = extend[2]/8;
  
  if(extend[0]%8) inx++;
  if(extend[1]%8) iny++;
  if(extend[2]%8) inz++;

  tv1=0.0;
  for(zc=0;zc<inz;zc++) {
    for(yc=0;yc<iny;yc++) {
      for(xc=0;xc<inx;xc++) {
	//fprintf(stderr,"%d %d %d\n",xc,yc,zc);
	// each value is stored in a byte
	fread(head,sizeof(char),512,f);
	for(bc=0;bc<512;bc++) {
	  // conversion from byte-value to float
	  // THIS IS NOT WORKING !!!
	  bval=(float)(head[bc]-0x20);
	  val=(bval-par[1])/par[0];
	  tv1+=val;
	  u=xc*8+origin[0];
	  v=yc*8+origin[1];
	  w=zc*8+origin[2];
	  ret=scalWriteField(sn->field,u,v,w,val);
	  //fprintf(stderr,"%d %d %d: %f %f  %d\n",u,v,w,bval,val,ret);
	}
      }
    }
  }

  fprintf(stderr,"rmsd: %f\n",scalRMSD(sn->field));

  fx=1.0/(float)cell[0];
  fy=1.0/(float)cell[1];
  fz=1.0/(float)cell[2];

  scalCELLtoVECT(sn->field,
		 cell[0],cell[1],cell[2],cell[3],cell[4],cell[5],
		 fx,fy,fz);

  sn->xtal=NULL;
  sn->show_cell=0;

  return 0;
} 
