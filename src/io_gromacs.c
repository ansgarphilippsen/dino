#include "io_gromacs.h"

#include "com.h"
#include "Cmalloc.h"
#include "struct_db.h"

static void copy_frame(rvec *x, struct STRUCT_TRJ_POSITION *pos, int count, double prec, dbmStructNode *node) 
{
  int i;
  for(i=0;i<count;i++) {
    //pos[i].x = node->atom[i].xx+x[i][0];
    //pos[i].y = node->atom[i].yy+x[i][1];
    //pos[i].z = node->atom[i].zz+x[i][2];
    pos[i].x = x[i][0]*10.0;
    pos[i].y = x[i][1]*10.0;
    pos[i].z = x[i][2]*10.0;
  }
}

int xtcTrjRead(char *filename, dbmStructNode *node, int swap_flag)
{
  int fd,i;
  int natoms, step;
  real time;
  matrix box;
  rvec *x;
  real prec;
  bool bok;
  char message[1024];
  int frame=0;
  int frame_max = 10;
  int frame_size;

  fd = open_xtc(filename,"r");

  if(read_first_xtc(fd,&natoms,&step,&time,box,&x,&prec,&bok)!=1) {
    comMessage("read_first_xtc failed\n");
    close_xtc(fd);
    return -1;
  }

  if(natoms != node->atom_count) {
    sprintf(message,"error: atom missmatch between dataset (%d) and trajectory (%d)\n",
	    node->atom_count,natoms);
    comMessage(message);
    close_xtc(fd);
    return -1;
  }

  if(node->trj_flag) {
    /* there is already a trajectory associated */
    Cfree(node->trj.pos);
  }

  node->trj_flag=1;
  node->trj.type=STRUCT_TRJ_XTC;
  node->trj.pos=Crecalloc(0,
			  frame_max*natoms,
			  sizeof(struct STRUCT_TRJ_POSITION));

  frame_size = natoms*sizeof(struct STRUCT_TRJ_POSITION);

  // copy first block of atoms into position
  copy_frame(x,&node->trj.pos[frame*natoms],natoms,prec,node);

  /*
  for(i=0;i<natoms;i++) {
    fprintf(stderr,"%d  %g %g %g\n",i,x[i][0],x[i][1],x[i][2]);
  }
  */

  while(read_next_xtc(fd,natoms,&step,&time,box,x,&prec,&bok)==1) {
    frame++;
    if(frame>=frame_max) {
      frame_max*=2;
      node->trj.pos=Crecalloc(node->trj.pos,
			      frame_max*natoms,
			      sizeof(struct STRUCT_TRJ_POSITION));
    }
    // copy next atoms
    copy_frame(x,&node->trj.pos[frame*natoms],natoms,prec,node);
  }

  close_xtc(fd);

  // now remaining trj attributes can be set
  node->trj.size = frame*frame_size;
  node->trj.frame_count = frame;
  node->trj.atom_count = natoms;

  // resize to free unused memory
  node->trj.pos = Crecalloc(node->trj.pos,
			    frame*natoms,
			    sizeof(struct STRUCT_TRJ_POSITION));
  

  return 0;
}
