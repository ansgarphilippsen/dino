#include "prop.h"
#include "cl.h"

int propReset(PropTable *t)
{
  int i;
  for(i=0;i<PROP_MAX_VALUES;i++) {
    clStrcpy(t->s[i],"");
    t->n[i]=-1;
  }
  return 0;
}

int propAddName(PropTable *t, char *s)
{
  int i;
  for(i=0;i<PROP_MAX_VALUES;i++) {
    if(clStrcmp(s,t->s[i]))
      return t->n[i];
  }
  for(i=0;i<PROP_MAX_VALUES;i++)
    if(clStrlen(t->s[i])==0)
      break;

  clStrcpy(t->s[i],s);
  t->n[i]=i;
  return i;
}


int propGetIndex(PropTable *t, char *s)
{
  int i;
  for(i=0;i<PROP_MAX_VALUES;i++) {
    if(clStrcmp(s,t->s[i]))
      return t->n[i];
  }
  return -1;
}

float propGetValue(Prop *p, int n)
{
  if(n<0 && n>PROP_MAX_VALUES)
    return 0.0;

  return p->v[n];
}
