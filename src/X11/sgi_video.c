#include <stdio.h>

#include "sgi_video.h"

char sgi_video_static_buf[2048];
char *sgi_video_static_list[256];

int sgi_video_get_list(int * lc, char ***l)
{
  int i,j;
  DIR *d;
  struct direct *dp;
  char *lp,*sp;
  FILE *f;
  char buf[256];

  /* detect graphics board */
  
  f=popen("find /usr/gfx/ucode -name '*x*_*s.*'","r");
  if(f==NULL)
    return -1;

  i=0;
  lp=sgi_video_static_buf;
  memset(lp,0,2048);
  sgi_video_static_list[0]=lp;
  while(!feof(f)) {
    memset(buf, 0, 256);
    fgets(buf, 256, f);

    if((sp=strrchr(buf,'.'))!=NULL) 
      sp[0]='\0';
    
    sp=strrchr(buf,'/');
    if(sp==NULL) {
      sp=buf;
    } else {
      sp++;
    }
    if(strlen(sp)>0) {
      strncpy(lp,sp,256);
      lp+=strlen(sp)+1;
      sgi_video_static_list[++i]=lp;
    }
  }
  pclose(f);

  if(i==0)
    return -1;

  (*lc)=i;
  (*l)=sgi_video_static_list;

  return 0;
}

