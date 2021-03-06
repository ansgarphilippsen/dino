#include <stdio.h>
#include <string.h>

#include "Cmalloc.h"

int cmalloc_free=1;

char *cmalloc_entry[65536];

void Cmalloc_init()
{
  memset(cmalloc_entry,0,sizeof(char *));
}

void *Cmalloc(size_t size)
{
#ifdef CMALLOC_DEBUG 
  void *p;
  int i;
  p=malloc(size);
  for(i=0;i<65536;i++)
    if(cmalloc_entry[i]==0) {
      cmalloc_entry[i]=p;
      break;
    }
  return p;
#else
#ifdef CMALLOC_VERBOSE
  fprintf(stderr,"MALLOC %.2f kb\n",size/1024.0);
#endif
  return malloc(size);
#endif
}

void *Ccalloc (size_t nelem, size_t elsize)
{
#ifdef CMALLOC_DEBUG 
  void *p;
  int i;
  p=calloc(nelem,elsize);
  for(i=0;i<65536;i++)
    if(cmalloc_entry[i]==0) {
      cmalloc_entry[i]=p;
      break;
    }
  return p;
#else
#ifdef CMALLOC_VERBOSE
  fprintf(stderr,"CALLOC %.2f kb\n",nelem*elsize/1024.0);
#endif
  return calloc(nelem, elsize);
#endif
}

void Cfree(void *p)
{
#ifdef CMALLOC_DEBUG 
  int i;
  for(i=0;i<65536;i++)
    if(cmalloc_entry[i]==p) {
      cmalloc_entry[i]=0;
      break;
    }

  if(i==65536)
    fprintf(stderr,"Cfree() error!\n");
#endif

  if(p!=NULL) {
    /*    if(cmalloc_free) */
    free(p);
  } else {
    /*
    fprintf(stderr,"warning: Cfree(NULL) called\n");
    */
  }
}

void *Crealloc(void *p, size_t size)
{
#ifdef CMALLOC_VERBOSE
  fprintf(stderr,"RECALLOC (%p) %.2f kb\n",p,size/1024.0);
#endif

  return realloc(p,size);
}

void *Crecalloc(void *p, size_t nelem, size_t elsize)
{
  return Crealloc(p,nelem*elsize);
}
