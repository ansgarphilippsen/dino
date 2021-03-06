#include "cl.h"

#include "Cmalloc.h"

int clNew(clOrg *org, int argc, char **argv)
{
  int i,j,c;
  clParam *op;

  org->param_max=16;
  org->param_count=0;
  org->param=Ccalloc(org->param_max,sizeof(clParam));

  clStrcpy(org->message,"");

  if(argc<64)
    c=64;
  else
    c=argc;

  for(i=0;i<org->param_max;i++) {
    org->param[i].p=NULL;
    org->param[i].wl=Ccalloc(c,sizeof(char *));
    org->param[i].wc=0;
  }

  i=0;
  op=&org->param[org->param_count++];
  while(i<argc) {
    if(argv[i][0]=='-') {
      if(argv[i][1]=='\0') {
	clStrcpy(org->message,"syntax error: isolated -");
	return -1;
      }
      for(j=0;j<org->param_count;j++) {
	if(clStrcmp(org->param[j].p,argv[i]+1))
	  break;
      }
      if(j==org->param_count) {
	op=&org->param[org->param_count++];
	op->p=argv[i]+1;
      } else {
	op=&org->param[j];
      }
    } else {
      op->wl[op->wc++]=argv[i];
    }
    i++;
  }
  return 0;
}

int clDelete(clOrg *org)
{
  int i;
  for(i=0;i<org->param_max;i++)
    Cfree(org->param[i].wl);
  Cfree(org->param);
  return 0;
}

int clStrcmp(const char *s1, const char *s2) 
{
  int l1,l2,c;

  if(s1==NULL && s2==NULL)
    return 1;

  if(s1==NULL || s2==NULL)
    return 0;

  l1=clStrlen(s1);
  l2=clStrlen(s2);

  if(l1!=l2)
    return 0;

  for(c=0;c<l1;c++)
    if(s1[c]!=s2[c])
      return 0;

  return 1;
}

int clStrncmp(const char *s1, const char *s2, int m) 
{
  int l1,l2,c;

  if(s1==NULL && s2==NULL)
    return 1;

  if(s1==NULL || s2==NULL)
    return 0;

  l1=clStrlen(s1);
  l2=clStrlen(s2);

  if(l1>m)
    l1=m;
  if(l2>m)
    l2=m;

  if(l1!=l2)
    return 0;

  for(c=0;c<l1;c++)
    if(s1[c]!=s2[c])
      return 0;

  return 1;
}

int clStrcpy(char *d, const char *s)
{
  int c;

  if(d==NULL || s==NULL)
    return 0;
  
  for(c=0;s[c]!='\0';c++)
    d[c]=s[c];
  d[c]='\0';
  return c;
}

int clStrncpy(char *d, const char *s, int m)
{
  int c;

  if(d==NULL || s==NULL)
    return 0;
  
  for(c=0;s[c]!='\0' && c<m;c++) {
    d[c]=s[c];
  }
  d[c]='\0';
  return c;
}

int clStrlen(const char *s)
{
  int c=0;
  if(s==NULL)
    return 0;
  while(s[c]!='\0') c++;
  return c;
}

int clStrncat(char *d, const char *s, int m)
{
  int i,j,k;
  if(d==NULL || s==NULL || m==0)
    return 0;
  k=clStrlen(s);
  i=clStrlen(d);
  j=0;
  //  while(s[j]!='\0' && j<m && k+j<m) d[i++]=s[j++];
  while(s[j]!='\0' && j<m) d[i++]=s[j++];
  d[i]='\0';
  return j;
}

int clStrcat(char *d, const char *s)
{
  int i,j;
  if(d==NULL || s==NULL)
    return 0;
  i=clStrlen(d);
  j=0;
  while(s[j]!='\0') d[i++]=s[j++];
  d[i]='\0';
  return j;
}

const char *clStrchr(const char *s, int c)
{
  int i,l;

  if(s==NULL || c==0)
    return NULL;

  l=clStrlen(s);
  for(i=0;i<l;i++) {
    if(s[i]==c)
      return &s[i];
  }
  return NULL;
}

const char *clStrrchr(const char *s, int c)
{
  int i,l;

  if(s==NULL || c==0)
    return NULL;

  l=clStrlen(s);
  for(i=l-1;i>=0;i--) {
    if(s[i]==c)
      return &s[i];
  }
  return NULL;
}

static char cl_empty[]="\0";

char *clStrdup(const char *s)
{
  char *p=cl_empty;
  if(clStrlen(s)) {
    p=Cmalloc(clStrlen(s));
    clStrcpy(p,s);
  }
  return p;
}

/*
  return pointer to substr
*/

static char cl_substr_buffer[2048];

const char *clSubstr(const char *s, int indx1, int indx2)
{
  char *ret=cl_substr_buffer;
  int len,p,i,p1,p2;

  clStrcpy(ret,"");
  if(s==NULL)
    return ret;

  if((len=clStrlen(s))==0)
    return ret;

  if(indx1<0) {
    // substr from right offset, ignore indx2
    p2=len;
    p1=len+indx1;
    if(p1<0)
      p1=0;
  } else {
    // substr from indx1 to indx2
    if(indx2<indx1) {
      return ret;
    }
    p1=indx1;
    if(p1>len)
      p1=len;
    p2=indx2+1;
    if(p2>len)
      p2=len;
  }

  for(i=0,p=p1;p<p2;i++,p++)
    ret[i]=s[p];
  ret[i]='\0';

  return ret;
}
