#include "help.h"
#include "cl.h"
#include "com.h"

int help(struct HELP_ENTRY *entry, const char *pre, const char *keyw)
{
  int i,f;
  char message[256];

  if(keyw==0)
    f=1;
  else
    f=0;

  for(i=0;entry[i].keyw!=NULL;i++) {
    if(keyw==NULL) {
      if(clStrcmp(entry[i].keyw,"separator")) {
	sprintf(message,"%s:\n\n",entry[i].brief);
	comMessage(message);
      } else {
	//	sprintf(message," %s:   \t%s %s\n",entry[i].keyw,pre,entry[i].brief);
	sprintf(message,"%s ",entry[i].keyw);
	comMessage(message);
      }
    } else {
      if(clStrcmp(entry[i].keyw,keyw)) {
	sprintf(message,"Syntax: %s %s\n\n",pre, entry[i].brief);
	comMessage(message);
	comMessage(entry[i].detail);
	f=1;
      }
    }
  }

  if(f==0) {
    sprintf(message,"no help for %s\n",keyw);
    comMessage(message);
  }
  
  return 0;
}
