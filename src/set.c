#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "com.h"
#include "set.h"
#include "Cmalloc.h"
#include "cl.h"
#include "mat.h"

int selectNew(Select *s, int argc, char **oargv)
{
  int i,j,c,len;
  char *ob;
  int vcount,vmax;
  char defaults[]={"*\0\0\0\0\0\0\0\0"};
  char *defaultl[1];
  char **argv;

  if(argc==0 || oargv==NULL) {
    argc=1;
    defaultl[0]=defaults;
    argv=defaultl;
  } else {
    argv=oargv;
  }

  len=0;
  for(i=0;i<argc;i++) {
    len+=clStrlen(argv[i]);
  }
  len+=10;
  if(len<256)
    len=256;

  s->rbuf_max=len;
  s->rbuf=Ccalloc(len,sizeof(char));
  s->rbuf[0]='\0';

  c=0;
  for(i=0;i<argc;i++) {
    len=clStrlen(argv[i]);
    for(j=0;j<len;j++) {
      if(argv[i][j]=='(') {
	s->rbuf[c++]=' ';
	s->rbuf[c++]=argv[i][j];
	s->rbuf[c++]=' ';
      } else if(argv[i][j]==')') {
	s->rbuf[c++]=' ';
	s->rbuf[c++]=argv[i][j];
	s->rbuf[c++]=' ';
      } else if(argv[i][j]=='&') {
	s->rbuf[c++]=' ';
	s->rbuf[c++]=argv[i][j];
	s->rbuf[c++]=' ';
      } else if(argv[i][j]=='|') {
	s->rbuf[c++]=' ';
	s->rbuf[c++]=argv[i][j];
	s->rbuf[c++]=' ';
      } else if(argv[i][j]=='!') {
	if(j+1<len) {
	  if(argv[i][j+1]!='=') {
	    s->rbuf[c++]=' ';
	    s->rbuf[c++]=argv[i][j];
	    s->rbuf[c++]=' ';
	  } else {
	    s->rbuf[c++]=argv[i][j];
	  }
	} else {
	  s->rbuf[c++]=argv[i][j];
	}
      } else {
	s->rbuf[c++]=argv[i][j];
      }
      if(c+3>s->rbuf_max) {
	ob=s->rbuf;
	s->rbuf=Ccalloc(s->rbuf_max+256,sizeof(char));
	memcpy(s->rbuf,ob,sizeof(char)*c);
	Cfree(ob);
	s->rbuf_max+=1024;
      }
    }
    s->rbuf[c++]=' ';
    
  }
  s->rbuf[c]='\0';
  s->rbuf_len=c;

  if(selectSplit(s)<0) {
    Cfree(s->rwl);
    Cfree(s->rbuf);
    return -1;
  }

  if(s->rbuf_len<256)
    s->buf_max=256;
  else
    s->buf_max=s->rbuf_len;
  s->buf_len=0;
  s->buf=Ccalloc(s->buf_max,sizeof(char));
  s->buf[0]='\0';

  s->wl_max=s->rwl_max;
  s->wl=Ccalloc(s->wl_max,sizeof(char *));
  s->wc=1;

  if(s->wl_max<256)
    s->stack_max=256;

  s->stack_max=s->wl_max;
  s->stack=Ccalloc(s->stack_max,sizeof(int));
  s->stack_count=0;

  s->wl[0]=&s->buf[0];

  if(selectBuildStack(s,s->rwc,s->rwl)<0) {
    Cfree(s->wl);
    Cfree(s->buf);
    Cfree(s->rwl);
    Cfree(s->rbuf);
    return -1;
  }

  // now that the stack is build, the raw stuff is no longer used
  Cfree(s->rwl);
  Cfree(s->rbuf);

  // convert the strings in wl to POVs
  if(s->wl_max<64)
    s->pov_max=64;
  else
    s->pov_max=s->wl_max;

  vcount=0;
  vmax=64;
  s->vlist=Ccalloc(vmax,sizeof(struct POV_VALUE));

  s->pov=Ccalloc(s->pov_max,sizeof(POV));
  s->pov_count=s->wc;
  for(i=0;i<s->wc;i++) {
    if(povNew(&s->pov[i],s->wl[i],&vcount,&vmax,&s->vlist)<0) {
      Cfree(s->pov);
      Cfree(s->vlist);
      Cfree(s->wl);
      Cfree(s->buf);
      return -1;
    }
  }

  for(i=0;i<s->wc;i++)
    s->pov[i].val=s->vlist;

  // set up the eval and result stack
  s->eval=Ccalloc(s->stack_max, sizeof(int));
  s->eval_count=0;
  s->result=Ccalloc(s->stack_max, sizeof(int));
  s->result_count=0;

  s->compiled=0;
  
  return 0;
}

int selectDelete(Select *s)
{
  Cfree(s->result);
  Cfree(s->eval);
  Cfree(s->pov);
  Cfree(s->vlist);
  Cfree(s->wl);
  Cfree(s->buf);
  return 0;
}

const int SELECT_AND=-1,SELECT_OR=-2,SELECT_NOT=-3,SELECT_NONE=-9;


int selectBuildStack(Select *s, int argc,char **argv)
{
  int op=SELECT_NONE;
  int c=0;
  int notc;
  int pc;
  char **nargv;
  int nargc;
  int i;

  while(c<argc) {
    // ignore empty strings
    if(clStrlen(argv[c])==0) {
      c++;
      continue;
    }    

    // check negations
    notc=0;
    while(clStrcmp(argv[c],"!") ||
	  clStrcmp(argv[c],"not") ||
	  clStrcmp(argv[c],"NOT")) {
      notc++;
      c++;
      if(c>=argc) {
	comMessage("error: select: not must be followed by expression\n");
	return -1;
      }
    }

    // expression
    if(clStrcmp(argv[c],"(")) {
      // go recursively into BuildStack
      pc=1;
      c++;
      nargv=&argv[c];
      nargc=0;

      while(pc>0) {
	if(clStrcmp(argv[c],"("))
	  pc++;
	else if(clStrcmp(argv[c],")"))
	  pc--;
	nargc++;
	c++;
	if(c>=argc && pc>0) {
	  comMessage("error: select: missmatched ( )\n");
	  return -1;
	}
      }
      nargc--;
      if(selectBuildStack(s,nargc,nargv)<0)
	return -1;
    } else {
      while(c<argc) {
	// check for operators
  	if(clStrcmp(argv[c],"&") ||
	   clStrcmp(argv[c],"and") ||
	   clStrcmp(argv[c],"AND")) {
	  break;
	}
	if(clStrcmp(argv[c],"|") ||
	   clStrcmp(argv[c],"or") ||
	   clStrcmp(argv[c],"OR")) {
	  break;
	}
	for(i=0;i<clStrlen(argv[c]);i++)
	  if(!isspace(argv[c][i]))
	    s->buf[s->buf_len++]=argv[c][i];
	s->buf[s->buf_len]='\0';
	c++;
      }
      s->stack[s->stack_count++]=s->wc-1;
    }

    if(notc & 0x1) {
      s->stack[s->stack_count++]=SELECT_NOT;
    }
    
    if(op!=SELECT_NONE) {
      s->stack[s->stack_count++]=op;
    }

    while(clStrlen(argv[c])==0 && c<argc) c++;
    
    if(c>=argc)
      return 0;
    
    s->buf[s->buf_len++]='\0';
    s->wl[s->wc++]=&s->buf[s->buf_len];

    if(clStrcmp(argv[c],"&") ||
       clStrcmp(argv[c],"and") ||
       clStrcmp(argv[c],"AND")) {
      op=SELECT_AND;
    } else if(clStrcmp(argv[c],"|") ||
       clStrcmp(argv[c],"or") ||
       clStrcmp(argv[c],"OR")) {
      op=SELECT_OR;
    } else {
      comMessage("error: select: unexpected internal error\n");
      return -1;
    }
    
    c++;
  }
  comMessage("error: select: expected expression after logical operator\n");
  return -1;
}

int selectSplit(Select *s)
{
  int i;
  char **ol;
  
  s->rwl_max=64;
  s->rwl=Ccalloc(s->rwl_max,sizeof(char *));
  s->rwc=0;
  
  s->rwl[s->rwc++]=&s->rbuf[0];
  
  i=0;
  while(i<s->rbuf_len) {
    if(isspace(s->rbuf[i])) {
      s->rbuf[i++]='\0';
      if(clStrlen(s->rwl[s->rwc-1])>0)
	s->rwl[s->rwc++]=&s->rbuf[i];
      else
	s->rwl[s->rwc-1]=&s->rbuf[i];

    } else {
      i++;
    }
    if(s->rwc>=s->rwl_max) {
      ol=s->rwl;
      s->rwl=Ccalloc(s->rwl_max+32,sizeof(char *));
      memcpy(s->rwl,ol,sizeof(char *)*s->rwl_max);
      Cfree(ol);
      s->rwl_max+=32;
    }
  }
  return 0;
}


int selectPush(Select *s, int v)
{
  s->result[s->result_count++]=v;
  return 0;
}

int selectPop(Select *s)
{
  return s->result[--s->result_count];
}

int selectPeek(Select *s)
{
  return s->result[s->result_count-1];
}

int selectGetPOVCount(Select *s)
{
  return s->pov_count;
}

POV *selectGetPOV(Select *s, int n)
{
  return &s->pov[n];
}

int selectSetResult(Select *s, int n, int v)
{
  s->eval[n]=v;
  return 0;
}

int selectResult(Select *s)
{
  int i;
  int a,b;
  s->result_count=0;
  for(i=0;i<s->stack_count;i++) {
    if(s->stack[i]==SELECT_NOT) {
      a=selectPop(s);
      if(a==0)
	selectPush(s,1);
      else
	selectPush(s,0);
    } else if(s->stack[i]==SELECT_OR) {
      a=selectPop(s);
      b=selectPop(s);
      if(a | b)
	selectPush(s,1);
      else
	selectPush(s,0);
    } else if(s->stack[i]==SELECT_AND) {
      a=selectPop(s);
      b=selectPop(s);
      if(a & b)
	selectPush(s,1);
      else
	selectPush(s,0);
    } else {
      selectPush(s,s->eval[s->stack[i]]);
    }
  }
  return selectPop(s);
}


int setNew(Set *s, int argc, char **argv)
{
  int i,j,c,len,ret;
  int vcount,vmax;
  clOrg co;

  s->pov_count=0;
  s->range_flag=0;
  s->select_flag=0;

  if(argc==0) {
    s->valid=0;
    return 0;
  } else {
    s->valid=1;
  }

  ret=0;
  s->blend=0;
  clNew(&co,argc,argv);
  for(i=0;i<co.param_count;i++) {

    if(co.param[i].p==NULL) {

    } else if(clStrcmp(co.param[i].p,"selection") ||
	      clStrcmp(co.param[i].p,"sel")) {
      if(selectNew(&s->select,co.param[i].wc,co.param[i].wl)<0) {
	ret=-1;
	break;
      }
      s->select_flag=1;
    } else if(clStrcmp(co.param[i].p,"range") ||
	      clStrcmp(co.param[i].p,"r")) {
      if(rangeNew(&s->range,co.param[i].wc,co.param[i].wl)<0) {
	ret=-1;
	break;
      }
      s->range_flag=1;
    } else if(clStrcmp(co.param[i].p,"blend")) {
      s->blend=1;
    } else {
      comMessage("error: set: unknown flag: -\n");
      comMessage(co.param[i].p);
      ret=-1;
      break;
    }
  }
  if(ret<0) {
    clDelete(&co);
    if(s->select_flag)
      selectDelete(&s->select);
    if(s->range_flag)
      rangeDelete(&s->range);
    return -1;
  }

  len=0;
  for(i=0;i<co.param[0].wc;i++) {
    len+=clStrlen(co.param[0].wl[i]);
  }
  len+=10;
  if(len<256)
    len=256;

  s->buf_max=len;
  s->buf=Ccalloc(len,sizeof(char));

  c=0;
  for(i=0;i<co.param[0].wc;i++) {
    len=clStrlen(co.param[0].wl[i]);
    for(j=0;j<len;j++)
      if(!isspace(co.param[0].wl[i][j]))
	s->buf[c++]=co.param[0].wl[i][j];
    
  }
  s->buf[c]='\0';
  s->buf_len=c;

  if(setSplit(s)<0) {
    Cfree(s->wl);
    Cfree(s->buf);
    return -1;
  }

  if(s->wc<64)
    s->pov_max=64;
  else
    s->pov_max=s->wc;

  s->pov_count=0;
  s->pov=Ccalloc(s->pov_max,sizeof(POV));

  vcount=0;
  vmax=64;
  s->vlist=Ccalloc(vmax,sizeof(struct POV_VALUE));

  for(i=0;i<s->wc;i++)
    if(povNew(&s->pov[i],s->wl[i],&vcount,&vmax,&s->vlist)<0) {
      Cfree(s->pov);
      Cfree(s->wl);
      Cfree(s->buf);
      return -1;
    }

  s->pov_count=i;

  for(i=0;i<s->pov_count;i++)
    s->pov[i].val=s->vlist;

  return 0;
}

int setDelete(Set *s)
{
  if(s->valid) {
    Cfree(s->vlist);
    Cfree(s->pov);
    Cfree(s->wl);
    Cfree(s->buf);
    if(s->select_flag)
      selectDelete(&s->select);
    if(s->range_flag)
      rangeDelete(&s->range);
      
  }
  return 0;
}

int setSplit(Set *s)
{
  int i,pc;
  char **ol;
  
  s->wl_max=32;
  s->wl=Ccalloc(s->wl_max,sizeof(char *));
  s->wc=0;

  s->wl[s->wc++]=&s->buf[0];

  i=0;
  while(i<s->buf_len) {
    if(s->buf[i]==',') {
      s->buf[i]='\0';
      i++;
      s->wl[s->wc++]=&s->buf[i];

      if(s->wc>=s->wl_max) {
	ol=s->wl;
	s->wl=Ccalloc(s->wl_max+32,sizeof(char *));
	memcpy(s->wl,ol,sizeof(char *)*s->wl_max);
	Cfree(ol);
	s->wl_max+=32;
      }
    } else if(s->buf[i]=='<') {
      pc=1;
      i++;
      while(pc>0) {
	if(i>=s->buf_len) {
	  comMessage("error: set: unmatched <\n");
	  return -1;
	}
	if(s->buf[i]=='<')
	  pc++;
	else if(s->buf[i]=='>')
	  pc--;
	i++;
      }
    } else if(s->buf[i]=='{') {
      pc=1;
      i++;
      while(pc>0) {
	if(i>=s->buf_len) {
	  comMessage("error: set: unmatched {\n");
	  return -1;
	}
	if(s->buf[i]=='{')
	  pc++;
	else if(s->buf[i]=='}')
	  pc--;
	i++;
      }
    } else {
      i++;
    }
  }
  return 0;
}

int povNew(POV *pov, char *s, int *vc, int *vm, struct POV_VALUE **vl)
{
  int i,j,l,pc;
  char *p;
  char *val;

  if(s==NULL) {
    comMessage("internal error in povNew\n");
    return -1;
  }
  pov->prop=s;
  pov->op=POV_OP_NN;
  pov->val=NULL;
  pov->val_offset=(*vc);
  pov->val_count=0;
  pov->id=-1;

  l=clStrlen(s);
  val=NULL;
  if(s[0]=='!') {
    pov->op=POV_OP_NOT;
    pov->prop++;
  } else {
  for(i=0;i<l;i++) {
    if(clStrncmp(s+i,"!=",2)) {
      pov->op=POV_OP_NE;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"<>",2)) {
      pov->op=POV_OP_WI;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"+=",2)) {
      pov->op=POV_OP_PE;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"-=",2)) {
      pov->op=POV_OP_ME;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"*=",2)) {
      pov->op=POV_OP_SE;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"/=",2)) {
      pov->op=POV_OP_DE;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"<=",2)) {
      pov->op=POV_OP_LE;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,"<",1)) {
      pov->op=POV_OP_LT;
      s[i]='\0';
      val=&s[i+1];
      break;
    } else if(clStrncmp(s+i,">=",2)) {
      pov->op=POV_OP_GE;
      s[i]='\0';
      s[i+1]='\0';
      val=&s[i+2];
      break;
    } else if(clStrncmp(s+i,">",1)) {
      pov->op=POV_OP_GT;
      s[i]='\0';
      val=&s[i+1];
      break;
    } else if(clStrncmp(s+i,"=",1)) {
      pov->op=POV_OP_EQ;
      s[i]='\0';
      val=&s[i+1];
      break;
    }
  }

  if(s[0]=='.' && pov->op==POV_OP_NN) {
    val=s;
    pov->op=POV_OP_OBJ;
  }
  }
  if(val==NULL) {
    pov->val_count=0;
    return 0;
  }


  l=clStrlen(val);
  j=0;

  for(i=0;i<l;i++) {
    if(val[i]=='<') {
      j++;
      if(j>1) {
	comMessage("error: pov: unexpected nesting of <>\n");
	return -1;
      }
      val[i]=' ';
    } else if(val[i]=='>') {
      j--;
      if(j<0) {
	comMessage("error: pov: unbalanced <>\n");
	return -1;
      }
      val[i]=' ';
    } else if(val[i]==',' && j!=0) {
      val[i]=':';
    }
  }

  p=&val[0];
  pc=0;
  for(i=0;i<l;i++) {
    if(val[i]=='{')
      pc++;
    if(val[i]=='}')
      pc--;
    if(val[i]==',' && pc==0) {
      val[i]='\0';
      if(povSplitVal(pov,p,vc,vm,vl)<0)
	return -1;
      pov->val_count++;
      p=&val[i+1];
    }
  }
  povSplitVal(pov,p,vc,vm,vl);
  pov->val_count++;

  if(pc!=0) {
    comMessage("error: unmatched { }\n");
    return -1;
  }
  
  return 0;
}

int povSplitVal(POV *pov, char *val, int *vc, int *vm, struct POV_VALUE **vl)
{
  struct POV_VALUE *v,*ovl;
  char *p,*d,*o;

  v=(*vl);

  if((p=clStrchr(val,':'))!=NULL) {
    p[0]='\0';
    v[(*vc)].range_flag=1;
    v[(*vc)].val1=val;
    if(isspace(v[(*vc)].val1[0])) v[(*vc)].val1++;
    v[(*vc)].val2=p+1;
    if(isspace(v[(*vc)].val2[clStrlen(v[(*vc)].val2)-1]))
      v[(*vc)].val2[clStrlen(v[(*vc)].val2)-1]='\0';
  } else {
    v[(*vc)].range_flag=0;
    v[(*vc)].val1=val;
    v[(*vc)].val2=val;
  }

  if(pov->op==POV_OP_WI) {
    if(v[(*vc)].range_flag) {
      comMessage("error: range not supported for <>\n");
      return -1;
    }

    // actually, a range would be tolerable for {}
    // if val starts with a dot, its an object
    if(val[0]=='.') {
      v[(*vc)].wi_flag=1;
      d=val+1;
      p=clStrchr(d,'.');
      if(p==NULL) {
	// local object
	v[(*vc)].val1=NULL;
	v[(*vc)].val2=d;
      } else {
	p[0]='\0';
	o=p+1;
	v[(*vc)].val1=d;
	v[(*vc)].val2=o;
      }

    } else {
      // see if val is a coordinate
      v[(*vc)].wi_flag=0;
      if(matExtract1Df(val,3,v[(*vc)].vect)!=0) {
	comMessage("error: in expected vector: \n");
	comMessage(val);
	return -1;
      }
    }
    if(val[0]=='{') {
    } else {
      if(val[0]!='.') {
	comMessage("error: expected a value beginning with . for <>\n");
	return -1;
      }
    }

  }

  (*vc)++;

  if((*vc)>=(*vm)) {
    ovl=v;
    v=Ccalloc((*vm)+64,sizeof(struct POV_VALUE));
    memcpy(v,ovl,sizeof(struct POV_VALUE)*(*vm));
    (*vm)+=64;
    Cfree(ovl);
    (*vl)=v;
  }
  return 0;
}

struct POV_VALUE *povGetVal(POV *pov, int n)
{
  return &pov->val[pov->val_offset+n];
}

int rangeNew(Range *r, int argc, char **argv)
{
  int i,l=0,d;
  char *p;
  POV *pov;
  int pov_count;
  struct POV_VALUE *vlist,*pv;
  int vcount,vmax;
  char message[256];

  if(argc<=0) {
    comMessage("error: range: missing parameters\n");
    return -1;
  }

  r->src=NULL;
  r->prop=NULL;
  r->val1=NULL;
  r->val2=NULL;
  r->clamp=0;

  if(argc<=0)
    return 0;

  for(i=0;i<argc;i++)
    l+=clStrlen(argv[i]);

  if(l<256)
    r->buf_max=256;
  else
    r->buf_max=l;

  r->buf_count=0;
  r->buf=Ccalloc(r->buf_max,sizeof(char));
  
  r->buf[0]='\0';
  for(i=0;i<argc;i++)
    clStrncat(r->buf,argv[i],l);

  r->buf_count=clStrlen(r->buf);

  // handle deprecated range
  l=0;
  d=0;
  for(i=0;i<r->buf_count;i++) {
    if(r->buf[i]=='<') {
      r->buf[i]=' ';
      l++;
    } else if(r->buf[i]=='>') {
      r->buf[i]=' ';
      l--;
    } else if(r->buf[i]==',') {
      if(l>0) {
	r->buf[i]=':';
	d++;
      }
    }
  }
  if(d) {
    comMessage("warning: range syntax <x,y> deprecated, use x:y instead\n");
  }
  if(l!=0) {
    comMessage("error: range: unbalanced < >\n");
    return -1;
  }

  pov_count=1;
  for(i=0;i<r->buf_count;i++)
    if(r->buf[i]==',')
      pov_count++;

  if(pov_count<64)
    pov=Ccalloc(64,sizeof(POV));
  else
    pov=Ccalloc(pov_count,sizeof(POV));

  vcount=0;
  vmax=64;
  vlist=Ccalloc(64,sizeof(struct POV_VALUE));
  pov_count=0;

  p=r->buf;


  for(i=0;i<r->buf_count;i++)
    if(r->buf[i]==',') {
      r->buf[i]='\0';
      if(povNew(&pov[pov_count],p,&vcount,&vmax,&vlist)<0) {
	Cfree(pov);
	Cfree(vlist);
	return -1;
      }
      pov_count++;
      p=&r->buf[i+1];
    }
  if(povNew(&pov[pov_count],p,&vcount,&vmax,&vlist)<0) {
    Cfree(pov);
    Cfree(vlist);
    return -1;
  }
  pov_count++;

  for(i=0;i<pov_count;i++)
    pov[i].val=vlist;

  for(i=0;i<pov_count;i++) {
    if(clStrcmp(pov[i].prop,"src")) {
      if(pov[i].op!=POV_OP_EQ) {
	comMessage("error: range: expected operator = for src\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      }
      if(pov[i].val_count==0) {
	comMessage("error: range: missing value for src\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      } else if(pov[i].val_count>1) {
	comMessage("error: range: expected only one value for src\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      } else {
	pv=povGetVal(&pov[i],0);
	if(pv->range_flag) {
	  comMessage("error: range: src cannot have range as value\n");
	  Cfree(pov);
	  Cfree(vlist);
	  return -1;
	}
	if(pv->val1[0]!='.') {
	  comMessage("error: range: expected src value to start with .\n");
	  return -1;
	}

	if(clStrcmp(pv->val1,".")) {
	  r->src=NULL;
	} else {
	  if((r->src=comGetDB(pv->val1))==NULL) {
	    comMessage("error: range: unknown dataset: \n");
	    comMessage(pv->val1);
	    Cfree(pov);
	    Cfree(vlist);
	    return -1;
	  }
	}
      }
    } else if(clStrcmp(pov[i].prop,"prop")) {
      if(pov[i].op!=POV_OP_EQ) {
	comMessage("error: range: expected operator = for prop\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      }
      if(pov[i].val_count==0) {
	comMessage("error: range: missing value for prop\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      } else if(pov[i].val_count>1) {
	comMessage("error: range: expected only one value for prop\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      } else {
	pv=povGetVal(&pov[i],0);
	if(pv->range_flag) {
	  comMessage("error: range: prop cannot have range as value\n");
	  Cfree(pov);
	  Cfree(vlist);
	  return -1;
	}
	r->prop=pv->val1;
      }
    } else if(clStrcmp(pov[i].prop,"val")) {
      if(pov[i].op!=POV_OP_EQ) {
	comMessage("error: range: expected operator = for val\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      }
      if(pov[i].val_count==0) {
	comMessage("error: range: missing value for val\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      } else if(pov[i].val_count>1) {
	comMessage("error: range: expected only one value for val\n");
	Cfree(pov);
	Cfree(vlist);
	return -1;
      } else {
	pv=povGetVal(&pov[i],0);
	if(pv->range_flag) {
	  r->val1=pv->val1;
	  r->val2=pv->val2;
	} else {
	  r->val1=pv->val1;
	  r->val2=pv->val1;
	}
      }
    } else if(clStrcmp(pov[i].prop,"clamp")) {
      r->clamp=1;
    } else {
      sprintf(message,"error: range: unknown property: %s (%d)\n",
	      pov[i].prop,i);
      comMessage(message);
      Cfree(pov);
      Cfree(vlist);
      return -1;
    }
  }
  
  Cfree(pov);
  Cfree(vlist);

  return 0;
}

int rangeDelete(Range *r)
{
  Cfree(r->buf);
  return 0;
}


int selectDump(Select *s)
{
  fprintf(stderr,"\npov: %p %d %d  vl: %p\n",
	  s->pov,s->pov_count,s->pov_max,s->vlist);

  fprintf(stderr,"buf: %p %d %d  wl: %p %d %d\n",
	  s->buf, s->buf_len, s->buf_max,
	  s->wl, s->wc, s->wl_max);

  fprintf(stderr,"stack: %p %d %d  %p  %p\n",
	  s->stack, s->stack_count, s->stack_max,
	  s->eval, s->result);

  return 0;
}
