#include <stdio.h>
#include <string.h>

#include "com.h"
#include "scal_db.h"
#include "scal_write.h"

int scalObjWrite(scalObj *obj, int wc, char **wl)
{
#ifdef SCAL_OBJ_WRITE
  char n1[256],n2[256];
  FILE *f1,*f2;
  int i;
  

  if(obj->type!=SCAL_CONTOUR) {
    comMessage("\nonly contour type output supported");
    return -1;
  }

  strncpy(n1,wl[0],250);
  strncpy(n2,wl[0],250);

  strcat(n1,".vert");
  strcat(n2,".face");

  if((f1=fopen(n1,"w"))==NULL || (f2=fopen(n2,"w"))==NULL) {
    comMessage("\nerror opening output file");
    return -1;
  }

  fprintf(f1,"# DINO contour to surface output\n");
  fprintf(f1,"#vertex - res n.d.\n");
  fprintf(f1,"%7d %7d %2.2f %2.2f\n",obj->point_count,0,0.0,0.0);

  for(i=0;i<obj->point_count;i++) {
    fprintf(f1,"%9.3f %9.3f %9.3f %9.3f %9.3f %9.3f %7d %7d %2d\n",
	    obj->point[i].v[0],obj->point[i].v[1],obj->point[i].v[2],
	    obj->point[i].n[0],obj->point[i].n[1],obj->point[i].n[2],
	    0,0,0);
	    
  }

  fprintf(f2,"# DINO contour to surface output\n");
  fprintf(f2,"#faces - res n.d.\n");
  fprintf(f2,"%7d %7d %2.2f %2.2f\n",obj->face_count,0,0.0,0.0);

  for(i=0;i<obj->face_count;i++) {
    fprintf(f2,"%6d %6d %6d %2d %6d\n",
	    obj->face[i].pi0+1,obj->face[i].pi1+1,obj->face[i].pi2+1,
	    0,0);
  }

  fclose(f1);
  fclose(f2);
#endif
  return 0;
}
