#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "geom_db.h"
#include "dino.h"
#include "com.h"
#include "Cmalloc.h"
#include "cl.h"

static char geom_return[256];

int geomCommand(struct DBM_GEOM_NODE *node,int wc, char **wl)
{
  int i;
  char message[256];

  strcpy(geom_return,"");

  if(wc<1) {
    comMessage("\nno command given");
    return -1;
  }

  if(!strcmp(wl[0],"help") ||
     !strcmp(wl[0],"?")) {
    comMessage("\npossible commands: new delete list get set help");
  } else if(!strcmp(wl[0],"new")) {
    geomNew(node,wc-1,wl+1);
  } else if(!strcmp(wl[0],"delete") ||
	    !strcmp(wl[0],"del")) {
    if(wc<2) {
      sprintf(message,"\n%s: missing parameter",node->name);
      comMessage(message);
      return -1;
    } else {
      for(i=1;i<wc;i++)
	geomDelObj(node,wl[i]);
    }
    comRedraw();
  } else if(!strcmp(wl[0],"list")) {
    for(i=0;i<node->obj_max;i++)
      if(node->obj[i]!=NULL) {
	comMessage("\n");
	comMessage(node->obj[i]->name);
      }
  } else if(!strcmp(wl[0],"get")) {
  } else if(!strcmp(wl[0],"set")) {
  } else {
    sprintf(message,"\nunknown command %s",wl[0]);
    comMessage(message);
    return -1;
  }
  wl[0]=geom_return;
  return 0;
}

int geomNew(struct DBM_GEOM_NODE *node, int wc, char **wl)
{
  int i,t;
  char message[256];
  char name[65],type[65];
  geomObj *obj;

  i=0;

  strcpy(name,node->name);

  strcpy(type,"prim");

  while(i<wc) {
    if(!strcmp(wl[i],"-name")) {
      i++;
      if(i>=wc) {
	comMessage("\nmissing name");
	return -1;
      }
      strncpy(name,wl[i],64);
    } else if(!strcmp(wl[i],"-type")) {
      i++;
      if(i>=wc) {
	comMessage("\nmissing type");
	return -1;
      }
      strncpy(type,wl[i],64);
    } else {
      sprintf(message,"\nunknown parameter %s",wl[i]);
      comMessage(message);
      return -1;
    }
    i++;
  }

  if(clStrcmp(type,"prim")) {
    t=GEOM_PRIM;
  } else {
    comMessage("\nerror: unknown geom object type: ");
    comMessage(type);
    return -1;
  }

  if((obj=geomNewObj(node,name,t))==NULL) {
    sprintf(message,"\n%s: memory allocation error in NewObj()",node->name);
    comMessage(message);
    return -1;
  }

  sprintf(geom_return,".%s.%s",node->name,obj->name);
    
  return 0;
}

/*************************************

   generate the name list from a string

   valid names begin with l,p,t or r
   and are followed by comma separated
   list of single numbers or ranges
      or
   just an asterix !

***************************************/
int geomGenNameList(geomNameList *list, char *expr)
{
  int j,c=0;
  char message[256];
  char num[256],n1[256],n2[256];
  char **nl,*cp;
  int nc;

  if(isalpha(expr[0])) {
    if(expr[0]=='p') {
      list[c].t=GEOM_TYPE_POINT;
      strcpy(num,expr+1);
    } else if(expr[0]=='l') {
      list[c].t=GEOM_TYPE_LINE;
      strcpy(num,expr+1);
    } else if(expr[0]=='t') {
      list[c].t=GEOM_TYPE_TRI;
      strcpy(num,expr+1);
    } else if(expr[0]=='r') {
      list[c].t=GEOM_TYPE_RECT;
      strcpy(num,expr+1);
    } else {
      sprintf(message,"\ninvalid element id: %c",expr[0]);
      comMessage(message);
      return -1;
    }
    /*
      now extract the num
      split it first in case it
      contains commas, then 
      go individually through
      the list and evaluate it
    */
    dbmSplit(num,',',&nc,&nl);
    
    for(j=0;j<nc;j++) {
      if(nl[j][0]=='<') {
	/* range */
	if(nl[j][strlen(nl[j])-1]!='>') {
	  sprintf(message,"\nerror in range expression: %s",nl[j]);
	  comMessage(message);
	  return -1;
	}
	cp=strchr(nl[0],',');
	nl[j][strlen(nl[j])-1]='\0';
	if(cp==NULL) {
	  strcpy(n1,nl[j]+1);
	  strcpy(n2,n1);
	} else {
	  cp[0]='\0';
	  strcpy(n1,nl[j]+1);
	  strcpy(n2,cp+1);
	}
	list[c].n1=atoi(n1);
	list[c].n2=atoi(n2);
      } else {
	/* just a number */
	if(isdigit(nl[j][0])) {
	  list[c].n1=atoi(nl[j]);
	  list[c].n2=atoi(nl[j]);
	} else if(!strcmp(num,"*") ||
		  !strcmp(num,"* ")){
	  /* all of 'em */
	  list[c].n1=-1;
	  list[c].n2=-1;
	} else {
	  sprintf(message,"\ninvalid number %s",num);
	  comMessage(message);
	  return -1;
	}
      }
      c++;
    }
  } else {
    if(!strcmp(expr,"*")) {
      list[c].t=GEOM_TYPE_ALL;
      list[c].n1=-1;
      list[c].n2=-1;
    } else {
      sprintf(message,"\nunknown name: %s",expr);
      comMessage(message);
      return -1;
    }
    c++;
  }

  return c;
}
