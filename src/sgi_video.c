#include <stdio.h>

#include "sgi_video.h"

char *sgi_video_dir[]={
  "/usr/gfx/ucode/NG1/vof",         /* XL Graphics (Indy or Indigo2) */
  "/usr/gfx/ucode/CRM/vof",         /* CRM Graphics */
  "/usr/gfx/ucode/MGRAS/vof",       /* IMPACT Graphics */
  "/usr/gfx/ucode/MGRAS/vof/2RSS",  /* IMPACT Graphics II*/
  "/usr/gfx/ucode/RE/dg2/vof",      /* Reality Engine */
  "/usr/gfx/ucode/KONA/dg4/vof",    /*       "        */
  ""
};

char sgi_video_static_buf[2048];
char *sgi_video_static_list[256];

int sgi_video_get_list(int * lc, char ***l)
{
  int i;
  DIR *d;
  struct direct *dp;
  char *lp;

  /* detect graphics board */
  
  d=NULL;
  for(i=0;sgi_video_dir[i][0]!='\0';i++) {
    d=opendir(sgi_video_dir[i]);
    if(d!=NULL)
      break;
    
  }
  
  if(d==NULL)
    return -1;

  i=0;
  lp=sgi_video_static_buf;
  sgi_video_static_list[0]=lp;
  while((dp=readdir(d)) != NULL) {
    strncpy(lp,dp->d_name,63);
    lp+=dp->d_namlen+1;
    sgi_video_static_list[++i]=lp;
    if(i>256)
      break;
  }

  closedir(d);

  (*lc)=i;
  (*l)=sgi_video_static_list;

  return 0;
}
