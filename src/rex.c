#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "rex.h"
#include "com.h"
#include "Cmalloc.h"
#include "mat.h"

int rex(const char *s1,const char *s2)
{
  int i,j;
  char cs1[256],cs2[256];
  char *p1,*p2;
  int t;

  if(!strcmp(s1,"*"))
    return 1;
  if(!strcmp(s2,"*"))
    return 1;

  i=0;
  strcpy(cs1,s1);
  p1=&cs1[0];
  /*
  num1=strlen(cs1);
  if(cs1[0]=='"' && cs1[num1-1]=='"') {
    p1=&cs1[1];
    cs1[num1-1]='\0';
  } 
  */
  strcpy(cs2,s2);
  p2=&cs2[0];
  while(strlen(p1)>0) {
    if(p1[0]=='*'){            
      if(strlen(p1)==1)        
	return 1;              
      
      t=0;
      for(j=0;j<strlen(p2);j++)
	if((t=rex(&p1[1],p2+j))!=0)
	  break;
      
      if(t==1)        
	return 1;
      else            
	return 0;
    } else if(p1[0]=='?') {  
      if(strlen(p2)==0)      
	return 0;
    } else if(strlen(p1)==0){     
      if(strlen(p2)==0)    
	return 1;      
      else             
	return 0;      
    } else if(strlen(p2)==0) {
      if(strlen(p1)>0) 
	return 0;      
      /*
    }else if(isalpha(p1[0]) && isalpha(p2[0])) {
      if(p1[0]!=p2[0])
	return 0;      
    }else if(isdigit(p1[0]) && isdigit(p2[0])) {
      if(p1[0]!=p2[0])
	return 0;
      */
    }else if(p1[0]!=p2[0]) {
      return 0;
    }
    p1++;
    p2++;
  }

  if(strlen(p2)>0)
    return 0;
  
  return 1;

}

lexStack lex_stack;

struct LEX_STACK * lexGenerate(char *raw_s)
{
  enum PREV_FLAGS {LEX_NONE,LEX_EXP,LEX_OP};

  char message[256];
  
  int i;
  lexStack *stack;

  char *raw_list[4096];
  int raw_count;

  char *word_list[4096];
  int bc,wc,ep,word_count;
  char *cpoe;

  char *stack_list[4096];
  int stack_count;
  int prev;

  char *raw_d,*lex_exp_buf;

  stack=&lex_stack;
  
  if(!strcmp(raw_s,"*") ||
     !strcmp(raw_s,"* ")) {
    stack->poe=Ccalloc(1,sizeof(lexPOE));
    stack->op=Ccalloc(1,sizeof(lexOP));
    strcpy(stack->poe[0].property,"*");
    strcpy(stack->op[0].op,"*");
    stack->count=1;
    return stack;
  }

  raw_d=Cmalloc(65536);
  lex_exp_buf=Cmalloc(65536);

  /* split raw string into words - memory area is raw_d */
  lexSplitString(raw_s,raw_d,&raw_count,raw_list);

  /* 
     now go through splitted list and assemble word_list 
     for GenStack - memory area that holds word_list is
     expr
  */
  
  word_count=0;
  ep=0;
  bc=0;
  prev=LEX_NONE;

//  memset(lex_exp_buf,0,sizeof(lex_exp_buf));
  word_list[word_count]=&lex_exp_buf[ep];
  lex_exp_buf[ep]='\0';
  cpoe=NULL;
  for(wc=0;wc<raw_count;wc++) {
    if(!strcmp(raw_list[wc],"(")) {
      if(cpoe!=NULL) {
	sprintf(message,"syntax error: no logical operator before ()\n");
	comMessage(message);
	Cfree(raw_d);
	Cfree(lex_exp_buf);
	return NULL;
      }
      lex_exp_buf[ep++]='(';
      lex_exp_buf[ep++]='\0';
      lex_exp_buf[ep]='\0';
      word_list[++word_count]=&lex_exp_buf[ep];
      bc++;
      prev=LEX_OP;
    } else if(!strcmp(raw_list[wc],")")) {
      if(bc<=0) {
	sprintf(message,"syntax error: () unmatched\n");
	comMessage(message);
	Cfree(raw_d);
	Cfree(lex_exp_buf);
	return NULL;
      }
      if(cpoe!=NULL) {
	strcat(&lex_exp_buf[ep],cpoe);
	ep+=strlen(cpoe)+2;
	cpoe=NULL;
	lex_exp_buf[ep++]='\0';
	lex_exp_buf[ep]='\0';
	word_list[++word_count]=&lex_exp_buf[ep];
      } else {
	if(prev!=LEX_EXP) {
	  sprintf(message,"syntax error: expression missing before ')'\n");
	  comMessage(message);
	  Cfree(raw_d);
	  Cfree(lex_exp_buf);
	  return NULL;
	}
      }
      lex_exp_buf[ep++]=')';
      lex_exp_buf[ep++]='\0';
      lex_exp_buf[ep]='\0';
      word_list[++word_count]=&lex_exp_buf[ep];
      bc--;
      prev=LEX_EXP;
    } else if(!strcmp(raw_list[wc],"|") ||
	      !strcmp(raw_list[wc],"or")) {
      if(cpoe!=NULL) {
	strcat(&lex_exp_buf[ep],cpoe);
	ep+=strlen(cpoe)+2;
	cpoe=NULL;
	lex_exp_buf[ep++]='\0';
	lex_exp_buf[ep]='\0';
	word_list[++word_count]=&lex_exp_buf[ep];
      } else {
	if(prev!=LEX_EXP) {
	  sprintf(message,
		  "syntax error: expression missing before 'or'\n");
	  comMessage(message);
	Cfree(raw_d);
	Cfree(lex_exp_buf);
	  return NULL;
	}
      }
      lex_exp_buf[ep++]='|';
      lex_exp_buf[ep++]='\0';
      lex_exp_buf[ep]='\0';
      word_list[++word_count]=&lex_exp_buf[ep];
      
      prev=LEX_OP;
    } else if(!strcmp(raw_list[wc],"&") ||
	      !strcmp(raw_list[wc],"and")) {
      if(cpoe!=NULL) {
	strcat(&lex_exp_buf[ep],cpoe);
	ep+=strlen(cpoe)+2;
	cpoe=NULL;
	lex_exp_buf[ep++]='\0';
	lex_exp_buf[ep]='\0';
	word_list[++word_count]=&lex_exp_buf[ep];
      } else {
	if(prev!=LEX_EXP) {
	  sprintf(message,
		  "syntax error: expression missing before 'or'\n");
	  comMessage(message);
	Cfree(raw_d);
	Cfree(lex_exp_buf);
	  return NULL;
	}
      }
      lex_exp_buf[ep++]='&';
      lex_exp_buf[ep++]='\0';
      lex_exp_buf[ep]='\0';
      word_list[++word_count]=&lex_exp_buf[ep];
      
      prev=LEX_OP;
    } else if(!strcmp(raw_list[wc],"!") ||
	      !strcmp(raw_list[wc],"not")) {
      if(cpoe!=NULL) {
	sprintf(message,"syntax error: invalid 'not' placement\n");
	comMessage(message);
	Cfree(raw_d);
	Cfree(lex_exp_buf);
	return NULL;
      }
      lex_exp_buf[ep++]='!';
      lex_exp_buf[ep++]='\0';
      lex_exp_buf[ep]='\0';
      word_list[++word_count]=&lex_exp_buf[ep];
      prev=LEX_OP;
    } else {
      strcat(&lex_exp_buf[ep],cpoe);
      ep+=strlen(cpoe);
      cpoe=raw_list[wc];
      prev=LEX_EXP;
    }
    
  }
  if(bc>0) {
    sprintf(message,"syntax error: () unmatched\n");
    comMessage(message);
    Cfree(raw_d);
    Cfree(lex_exp_buf);
    return NULL;
  }

  if(cpoe!=NULL) {
    strcat(&lex_exp_buf[ep],cpoe);
    ep+=strlen(cpoe)+2;
    cpoe=NULL;
    lex_exp_buf[ep++]='\0';
    lex_exp_buf[ep]='\0';
    word_list[++word_count]=&lex_exp_buf[ep];
  } else {
    if(strcmp(word_list[word_count-1],")")!=0) {
      sprintf(message,"syntax error: must end with expression\n");
      comMessage(message);
	Cfree(raw_d);
	Cfree(lex_exp_buf);
      return NULL;
    }
  }

  
  for(i=0;i<word_count;i++) {
    stack_list[i]=Cmalloc(4096);
    strcpy(stack_list[i],"");
  }

  stack_count=0;

  lexGenStack(&stack_count,0,stack_list,word_count,word_list);

  stack->poe=Ccalloc(stack_count,sizeof(struct LEX_POE));
  stack->op=Ccalloc(stack_count,sizeof(struct LEX_OP));
  stack->eval=Ccalloc(stack_count,sizeof(int));
  stack->count=stack_count;

  for(i=0;i<stack_count;i++) {
    if(!strcmp(stack_list[i],"&") ||
       !strcmp(stack_list[i],"|") ||
       !strcmp(stack_list[i],"!")) {
      strcpy(stack->op[i].op,stack_list[i]);
    } else {
      strcpy(stack->op[i].op,"");
      lexExtractPOE(stack_list[i],&stack->poe[i]);
    }
  }

  Cfree(raw_d);
  Cfree(lex_exp_buf);

  for(i=0;i<word_count;i++)
    Cfree(stack_list[i]);
  
  return stack;
}

int lexGenStack(int *sc,int p,char **nl, int wc, char **wl)
{
  int c=0,i,bc,ps,pc,nc;
  char cop[128];

  strcpy(cop,"");
  while(c<wc){
    nc=0;
    /* get all the negations before an expression */
    while(!strcmp(wl[c],"!")) {
      c++;
      nc++;
    }
    /* does the new expression start with ( */
    if(!strcmp(wl[c],"(")) {
      bc=1;
      ps=c;
      c++;
      pc=0;
      /* scan to corresponding bracket */
      /* pc counts the words that are NOT brackets */
      while(bc>0) {
	if(!strcmp(wl[c],"(")) {
	  bc++;
	} else if(!strcmp(wl[c],")")) {
	  --bc;
	} else {
	  pc++;
	}
	c++;
      }
      /* insert subexpression recursively */
      lexGenStack(sc,p,nl,pc,&wl[ps+1]);
      p+=(pc);
    } else {
      /* move stack contents one up */
      for(i=(*sc);i>p;i--)
	strcpy(nl[i],nl[i-1]);
      (*sc)++;
      /* insert element */
      strcpy(nl[p++],wl[c++]);
    }
    
    /* if 'not' count was odd */
    if(nc & 1) {
      /* move stack contents one up */
      for(i=(*sc);i>p;i--)
	strcpy(nl[i],nl[i-1]);
      (*sc)++;
      /* insert element */
      strcpy(nl[p++],"!");
    }
    
    /* if there is a current operator for this expression */
    if(strlen(cop)>0) {
      /* move stack contents one up */
      for(i=(*sc);i>p;i--)
	strcpy(nl[i],nl[i-1]);
      (*sc)++;
      /* insert element*/
      strcpy(nl[p++],cop);
    }

    if(c>=wc)
      break;
    
    strcpy(cop,wl[c++]);
    
  }
  return 0;
}

int lexFree(struct LEX_STACK *stack)
{
  Cfree(stack->eval);
  Cfree(stack->op);
  Cfree(stack->poe);
  return 0;
}

int lexExtractPOE(char *s, lexPOE *exp)
{
  const char ops[][8]={"=<","!=","<>","<=","<",">=",">","="};
  const int ops_len=8;

  int wc,cc,oc;
  char prop[64],op[16],value[4096];
  char s1[2048],s2[2048];
  char *p,*sn;
  
  cc=0;
  wc=0;

  strcpy(prop,"");
  strcpy(op,"");
  strcpy(value,"");

  strcpy(s1,s);
  sn=&s1[0];
  while(-1) {
    for(oc=0;oc<ops_len;oc++) {
      if((p=strstr(sn,ops[oc]))!=NULL)
	break;
    }
    if(oc>=ops_len || p==NULL) {
      strncpy(prop,s1,sizeof(prop)-1);
      break;
    }
    if(!strcmp(ops[oc],"=<"))
      oc=ops_len-1;
    strcpy(s2,p);
    p[0]='\0';
    strncpy(prop,s1,63);
    strncpy(op,s2,strlen(ops[oc]));
    op[strlen(ops[oc])]='\0';
    p=&s2[strlen(ops[oc])];
    strcpy(value,p);
  }

  if(strlen(prop)==0)
    return -1;
  /*
  if(strlen(op)==0)
    return -1;
  if(strlen(value)==0)
    return -1;
  */
  strcpy(exp->property,prop);
  strcpy(exp->op,op);
  strcpy(exp->value,value);

  /* pre-evaluate within expressions */
  if(!strcmp(op,"<>")) {
    exp->dist=atof(prop);
    exp->dist2=exp->dist*exp->dist;
    if(value[0]=='{') {
      if(matExtract1Df(value,3,exp->v)!=0) {
	exp->oflag=-1;
      } else {
	exp->oflag=0;
      }
    } else {
    }
  }
  
  return 0;
}

int lexSplitString(char *raw_s,char *raw_d, int *word_count,char *word_list[])
{
  char message[256];
  int sp,dp,wc;

  sp=0;
  dp=0;
  wc=0;

  word_list[wc]=&raw_d[dp];
  raw_d[dp]='\0';

  while(sp<strlen(raw_s)) {
    if(isspace(raw_s[sp])) {
      if(strlen(word_list[wc])>0) {
	raw_d[dp++]='\0';
	word_list[++wc]=&raw_d[dp];
	raw_d[dp]='\0';
      }
      sp++;
    } else if(raw_s[sp]=='|' || /* or */
	      raw_s[sp]=='&' || /* and */
	      raw_s[sp]=='(' || /* open */
	      raw_s[sp]==')' || /* close */
	      raw_s[sp]=='!') { /* not */

      if(strlen(word_list[wc])>0) {
	raw_d[dp++]='\0';
	word_list[++wc]=&raw_d[dp];
	raw_d[dp]='\0';
      }

      raw_d[dp++]=raw_s[sp++];
      raw_d[dp++]='\0';
      word_list[++wc]=&raw_d[dp];
      raw_d[dp]='\0';
      
    } else if(raw_s[sp]=='"') {
      if(strlen(word_list[wc])>0) {
	raw_d[dp++]='\0';
	word_list[++wc]=&raw_d[dp];
	raw_d[dp]='\0';
      }
      raw_d[dp++]=raw_s[sp++];

      while(raw_s[sp]!='"' && sp<strlen(raw_s))
	raw_d[dp++]=raw_s[sp++];

      if(sp>=strlen(raw_s)) {
	sprintf(message,"parse error: missing \"");
	comMessage(message);
	return -1;
      }
      raw_d[dp++]=raw_s[sp++];
      raw_d[dp++]='\0';
      word_list[++wc]=&raw_d[dp];
      raw_d[dp]='\0';
      
    } else {
      raw_d[dp++]=raw_s[sp++];
    }
  }
  if(strlen(word_list[wc])>0) {
    raw_d[dp++]='\0';
    word_list[++wc]=&raw_d[dp];
    raw_d[dp]='\0';
  }
  
  if(strlen(word_list[wc-1])==0)
    --wc;
  
  (*word_count)=wc;
  return 0;
}

