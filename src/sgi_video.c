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

const int sgi_video_dir_count=6;

char sgi_video_static_buf[2048];
char *sgi_video_static_list[256];

#define SGI_NEW_DETECTION

int sgi_video_get_list(int * lc, char ***l)
{
  int i;
  DIR *d;
  struct direct *dp;
  char *lp;
  FILE *f;
  char buf[256];

  /* detect graphics board */
  
#ifdef SGI_NEW_DETECTION
  f=popen("find /usr/gfx/ucode -name '*x*_*s.*'","r");
  i=0;
  lp=sgi_video_static_buf;
  memset(lp,0,2048);
  sgi_video_static_list[0]=lp;
  while(!feof(f)) {
    memset(buf, 0, 256);
    fgets(buf, 256, f);
    strncpy(lp,strlen(buf),256);
    lp[strlen(buf)-1]='\0';
    sgi_video_static[++i]=lp+strlen(buf);
  }
  pclose(f);
#else
  d=NULL;
  for(i=0;i<sgi_video_dir_count;i++) {
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
#endif
  return 0;
}

