#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "gui_terminal.h"
#include "shell_command.h"
#include "shell_raw.h"
#include "dino.h"
#include "com.h"
#include "Cmalloc.h"
#include "cl.h"

/*
  TODO
   var unset
   alias undef
   rpn stack non scalar
*/

#define RPN_TYPE_NULL   0
#define RPN_TYPE_SCALAR 1
#define RPN_TYPE_VECTOR 2
#define RPN_TYPE_MATRIX 3

static int work_script(void);
static int call_script(const char *f,const char **wl, int wc);
static void echo(int wc, const char **wl);
static void rpn_push(const char *v);
static const char *rpn_pop(void);
static void rpn_opr(const char *op);
static void rpn_clear(void);
static void rpn_dup(void);
static void rpn_swap(void);
static void rpn_show(void);
static const char *rpn_peek(void);
static int rpn_get_type(const char *v);
static void alias_list(const char *reg);
static const char * alias_get(const char *reg);
static void alias_def(const char *name, const char **wl, int wc);
static void alias_undef(const char *name);
static void init_vars(void);

#define MAXSCRIPTLEVEL 256

static struct SCRIPTBUF {
  char *buf;
  int count,max;
}scrbuf[MAXSCRIPTLEVEL];

static int scrlvl=-1;

#define STACK_BUFFER_INC 102400
#define STACK_ENTRY_INC  1024

static struct RPN_STACK {
  char *buf;
  int buf_size;
  char **entry;
  int count,max;
  char *next;
} rpn_stack;

#define ALIAS_MAX_NAME_SIZE 64
#define ALIAS_MAX_VALUE_SIZE 256
#define ALIAS_MAX_ENTRIES 512

static struct SHELL_ALIAS {
  struct SHELL_ALIAS_ENTRY {
    char name[ALIAS_MAX_NAME_SIZE];
    char value[ALIAS_MAX_VALUE_SIZE];
  }*entry;
  int count, max;
} shell_alias;

extern int shell_mode;

static char *shell_subexp;

int shellInit(void)
{
  shellInterpInit();

  rpn_stack.buf_size=STACK_BUFFER_INC;
  rpn_stack.buf=Crecalloc(NULL,sizeof(char),rpn_stack.buf_size);
  rpn_stack.count=0;
  rpn_stack.max=STACK_ENTRY_INC;
  rpn_stack.entry=Crecalloc(NULL,sizeof(char),rpn_stack.max);
  rpn_stack.next=rpn_stack.buf;

  shell_alias.count=0;
  shell_alias.max=ALIAS_MAX_ENTRIES;
  shell_alias.entry=Crecalloc(NULL,
			      shell_alias.max,
			      sizeof(struct SHELL_ALIAS_ENTRY));

  shell_subexp=Cmalloc(10240);

  init_vars();

  return 0;
}


int shellIsScript(void)
{
  if(scrlvl>=0)
    return 1;
  else
    return 0;
}

int shellParseCommand(const char **wl, int wc)
{
  int i;
  char message[256],*subexp=shell_subexp;
  const char *al;

  if(wc<1)
    return SHELL_ERROR;
  

  if(wl[0][0]=='@') {
    // scriptfile
    call_script(wl[0]+1,wl+1,wc-1);

  } else if(wl[0][0]=='!') {
    // system subcommand (abbrev)
    clStrcpy(subexp,wl[0]+1);
    for(i=1;i<wc;i++) {
      clStrcat(subexp," ");
      clStrcat(subexp,wl[i]);
    }
    system(subexp);

  } else if(clStrcmp(wl[0],"system")) {
    // system subcommand
    clStrcpy(subexp,wl[1]);
    for(i=2;i<wc;i++) {
      clStrcat(subexp," ");
      clStrcat(subexp,wl[i]);
    }
    system(subexp);

  } else if(clStrcmp(wl[0],"set")) {    
    // set variable
    if(wc<3) {
      sprintf(message,"syntax error: set var value\n");
      shellOut(message);
      return SHELL_ERROR;
    }
    clStrcpy(subexp,"");
    for(i=2;i<wc;i++) {
      clStrcat(subexp,wl[i]);
      if(i+1<wc)
	clStrcat(subexp," ");
    }
    shellSetVar(wl[1],subexp);

  } else if(clStrcmp(wl[0],"unset")) {
    // remove variable
    for(i=1;i<wc;i++)
      shellUnsetVar(wl[i]);
    
  } else if(clStrcmp(wl[0],"var")) {
    shellListVars(NULL);

  } else if(clStrcmp(wl[0],"echo")){
    echo(wc-1,wl+1);

  } else if(clStrcmp(wl[0],"push")) {
    // push values onto rpn stack
    for(i=1;i<wc;i++)
      rpn_push(wl[i]);
    
  } else if(clStrcmp(wl[0],"pop")) {
    // pop values from the stack, possibly into supplied vars
    if(wc>1) {
      for(i=1;i<wc;i++)
	shellSetVar(wl[i],rpn_pop());
    } else {
      rpn_pop();
    }

  } else if(clStrcmp(wl[0],"dup")) {
    // duplicate topmost stack entry
    rpn_dup();

  } else if(clStrcmp(wl[0],"swap")) {
    // swap to top entries
    rpn_swap();

  } else if(clStrcmp(wl[0],"peek")) {
    // peek at top most entry
    rpn_peek();

  } else if(clStrcmp(wl[0],"show")) {
    // dump stack
    rpn_show();

  } else if(clStrcmp(wl[0],"clear")) {
    // clear stack
    rpn_clear();

  } else if(clStrcmp(wl[0],"opr")) {
    // apply opr to stack
    for(i=1;i<wc;i++)
      rpn_opr(wl[i]);

  } else if(clStrcmp(wl[0],"cd")) {
    // change directory
    if(wc<2) {
      // TODO chdir(shell.cwd);
    } else {
      if(chdir(wl[1])<0) {
	shellOut("error during chdir\n");
      }
    }

  } else if(clStrcmp(wl[0],"pwd")) {
    // print current working dir
    getcwd(message,256);
    shellOut(message);
    shellOut("\n");

  } else if(clStrcmp(wl[0],"pause")) {
    // TODO pause

  } else if(clStrcmp(wl[0],"break")) {
    if(scrlvl>=0) {
      Cfree(scrbuf[scrlvl].buf);
      scrlvl--;
    }

  } else if(clStrcmp(wl[0],"alias")) {
    // define or list aliases
    if(wc==1) {
      // list all aliases
      alias_list(".*");
    } else if(wc==2) {
      alias_list(wl[1]);
    } else {
      alias_def(wl[1],wl+2,wc-2);
    }

  } else if(clStrcmp(wl[0],"unalias")) {
    // unalias
    for(i=1;i<wc;i++)
      alias_undef(wl[i]);

  } else {
    
    comWorkPrompt(wc,wl);

  }
  return SHELL_OK;

}


/*
  this function is periodically called from the timer routine
*/

void shellTimeProc(void)
{
  // a script is active
  if(scrlvl>=0) {
    work_script();
  }
}

/*
  script stuff
*/

/*
  designate a scriptfile to be parsed

  TODO: command line args
*/

static int call_script(const char *filename, const char **wl, int wc)
{
  FILE *f;
  char *b;
  int s,t;
  struct stat st;

  if(scrlvl>=MAXSCRIPTLEVEL) {
    shellOut("error: maximum script nesting level reached\n");
    return -1;
  }

  if((f=fopen(filename,"r"))==NULL) {
    shellOut("error opening ");
    shellOut(filename);
    shellOut("\n");
    return -1;
  }
    
  guitSuspend(1);

  scrlvl++;

  fstat(fileno(f),&st);

  s=st.st_size;
  b=Cmalloc(s);
  fread(b,sizeof(char),s,f);

  scrbuf[scrlvl].buf=b;
  scrbuf[scrlvl].max=s;

  fclose(f);
  return 0;
}

/*
  read next line from current script
  and pass it to the raw processing routine
*/

static char script_line[2048];

static int work_script(void)
{
  int i,p;
  char c;

  i=0; p=0;
  do {
    c=scrbuf[scrlvl].buf[scrbuf[scrlvl].count++];
 
    if(p==1) {
      p=0;
      if(c=='\n') {
	c=0;
      } else {
	script_line[i++]=c;
      }
    } else {
      if(c=='\\') {
	p=1;
      } else {
	script_line[i++]=c;
      }
    }

  } while (c!='\n' && scrbuf[scrlvl].count<scrbuf[scrlvl].max && i<2046);

  if(c!='\n')
    script_line[i++]='\n';

  script_line[i]='\0';

  // echo script line
  if(shell_mode==1)
    shellOut("[INPUT] ");
  shellOut(script_line);

  shellParseRaw(script_line);

  if(scrbuf[scrlvl].count>=scrbuf[scrlvl].max) {
    Cfree(scrbuf[scrlvl].buf);
    scrlvl--;
  }

  if(scrlvl<0)
    guitSuspend(0);

  return 0;
}

/*
  echo
*/

static void echo(int wc, const char **wl)
{
  int i,nwc=wc;
  FILE *f=NULL;
  char message[256];

  if(shell_mode==1)
    shellOut("[ECHO] ");

  for(i=0;i<wc;i++)
    if(clStrcmp(wl[i],">") ||
       clStrcmp(wl[i],">>")) {
      if(wc-i<2) {
	shellOut("error: Missing file for redirection\n");
	return;
      } else if(wc-i>2) {
	shellOut("error: Chars after filename\n");
	return;
      }
      if(clStrcmp(wl[i],">")) {
	f=fopen(wl[i+1],"w");
      } else {
	f=fopen(wl[i+1],"a");
      }
      if(f==NULL) {
	sprintf(message,"error: cannot open %s\n",wl[i+1]);
	shellOut(message);
	return;
      }
      nwc=i;
      break;
    }
  
  if(f==NULL) {
    for(i=0;i<nwc;i++) {
      shellOut(wl[i]);
      if(i+1<nwc)
	shellOut(" ");
    }
    shellOut("\n");
  } else {
    for(i=0;i<nwc;i++) {
      fprintf(f,wl[i]);
      if(i+1<nwc)
	fprintf(f," ");
    }
    fprintf(f,"\n");
    fclose(f);
  }
}


/*
  RPN routines
*/


static void rpn_push(const char *v)
{
  int sl=clStrlen(v)+1;
  if(rpn_stack.next+sl>=rpn_stack.buf+rpn_stack.buf_size) {
    // stack full
    return;
  } else if(rpn_stack.count>=rpn_stack.max) {
    // stack full
    return;
  } else {
    clStrcpy(rpn_stack.next,v);
    rpn_stack.entry[rpn_stack.count++]=rpn_stack.next;
    rpn_stack.next+=sl;
  }
}

static const char *rpn_pop(void)
{
  static char empty[]="";
  if(rpn_stack.count>0) {
    rpn_stack.next=rpn_stack.entry[--rpn_stack.count];
    return rpn_stack.next;
  } else {
    return empty;
  }
}

static const char *rpn_peek(void)
{
  static char empty[]="";
  if(rpn_stack.count>0) {
    return rpn_stack.entry[rpn_stack.count-1];
  } else {
    return empty;
  }
}


static void rpn_clear(void)
{
  rpn_stack.count=0;
  rpn_stack.next=rpn_stack.buf;
}

static void rpn_dup(void)
{
  rpn_push(rpn_peek());
}

static void rpn_swap(void)
{
  int l1,l2;
  if(rpn_stack.count>=2) {
    rpn_push(rpn_stack.entry[rpn_stack.count-2]);
    l1=clStrlen(rpn_stack.entry[rpn_stack.count-3])+1;
    l2=clStrlen(rpn_stack.entry[rpn_stack.count-2])+1;

    memmove(rpn_stack.entry[rpn_stack.count-3],
	    rpn_stack.entry[rpn_stack.count-2],
	    l1+l2);

    rpn_stack.entry[rpn_stack.count-3]=rpn_stack.entry[rpn_stack.count-2]-l1;
    rpn_stack.entry[rpn_stack.count-2]=rpn_stack.entry[rpn_stack.count-1]-l1;
    rpn_stack.count--;
    rpn_stack.next-=l1;
  }
}

static void rpn_show(void)
{
  int i;
  char mess[32];
  if(rpn_stack.count==0) {
    shellOut("stack is empty\n");
  } else {
    for(i=0;i<rpn_stack.count-1;i++) {
      sprintf(mess,"%3d: ",i);
      shellOut(mess);
      shellOut(rpn_stack.entry[i]);
      shellOut("\n");
    }
  }
}

static int rpn_get_type(const char *v)
{
  if(v==NULL) {
    return RPN_TYPE_NULL;
  } else {
    return RPN_TYPE_SCALAR;
  }
}

static char op_special[][8]={
  "dist","angle","torsion"
};

static char op_binary[][8]={
  "+","-","*","/","pow","x","rmat"
};

static char op_unary[][8]={
  "+-",  "inc","dec","abs",
  "log","ln","exp","sqrt","inv",
  "sin","cos","tan","asin","acos","atan",
  "int","float","det"
};

static char op_result[1024];

static void rpn_opr(const char *op)
{
  int i,op_type;
  char message[256];
  const char *val1, *val2;
  int type1, type2,tresult;
  double fval1, fval2, fresult;
  
  if(clStrlen(op)==0) {
    return;
  }

  // determine type of operator
  op_type=0;
  for(i=0;i<sizeof(op_unary);i++) {
    if(clStrcmp(op_unary[i],op)) {
      op_type=1;
      break;
    }
  }
  if(!op_type) {
    for(i=0;i<sizeof(op_binary);i++) {
      if(clStrcmp(op_binary[i],op)) {
	op_type=2;
	break;
      }
    }
  }
  if(!op_type) {
    for(i=0;i<sizeof(op_special);i++) {
      if(clStrcmp(op_special[i],op)) {
	op_type=3;
	break;
      }
    }
  }

  clStrcpy(op_result,"");

  if(!op_type) {
    sprintf(message,"error: unkown stack op %s\n",op);
    shellOut(message);
  } else if(op_type==1) {
    // unary op
    val1=rpn_pop();
    type1=rpn_get_type(val1);
    if(type1==RPN_TYPE_SCALAR) fval1=atof(val1);

    if(clStrcmp(op,"+-")) {
      if(type1==RPN_TYPE_SCALAR) fresult=-fval1;
    } else if(clStrcmp(op,"inc")) {
      if(type1==RPN_TYPE_SCALAR) fresult=fval1+1.0;
    } else if(clStrcmp(op,"dec")) {
      if(type1==RPN_TYPE_SCALAR) fresult=fval1-1.0;
    } else if(clStrcmp(op,"abs")) {
      if(type1==RPN_TYPE_SCALAR) fresult=fabs(fval1);
    } else if(clStrcmp(op,"log")) {
      if(type1==RPN_TYPE_SCALAR) fresult=log10(fval1);
    } else if(clStrcmp(op,"ln")) {
      if(type1==RPN_TYPE_SCALAR) fresult=log(fval1);
    } else if(clStrcmp(op,"exp")) {
      if(type1==RPN_TYPE_SCALAR) fresult=exp(fval1);
    } else if(clStrcmp(op,"sqrt")) {
      if(type1==RPN_TYPE_SCALAR) fresult=sqrt(fval1);
    } else if(clStrcmp(op,"inv")) {
      if(type1==RPN_TYPE_SCALAR) fresult=1.0/fval1;
    } else if(clStrcmp(op,"sin")) {
      if(type1==RPN_TYPE_SCALAR) fresult=sin(fval1);
    } else if(clStrcmp(op,"cos")) {
      if(type1==RPN_TYPE_SCALAR) fresult=cos(fval1);
    } else if(clStrcmp(op,"tan")) {
      if(type1==RPN_TYPE_SCALAR) fresult=tan(fval1);
    } else if(clStrcmp(op,"asin")) {
      if(type1==RPN_TYPE_SCALAR) fresult=asin(fval1);
    } else if(clStrcmp(op,"acos")) {
      if(type1==RPN_TYPE_SCALAR) fresult=acos(fval1);
    } else if(clStrcmp(op,"atan")) {
      if(type1==RPN_TYPE_SCALAR) fresult=atan(fval1);
    } else if(clStrcmp(op,"int")) {
      if(type1==RPN_TYPE_SCALAR) fresult=floor(fval1);
    } else if(clStrcmp(op,"float")) {
      if(type1==RPN_TYPE_SCALAR) fresult=fval1;
    } else if(clStrcmp(op,"det")) {
      if(type1==RPN_TYPE_SCALAR) fresult=fval1;
    }
    if(type1==RPN_TYPE_SCALAR) {
      sprintf(op_result,"%f",fresult);
    }
  } else if(op_type==2) {
    // binary op
    val2=rpn_pop();
    val1=rpn_pop();
    type1=rpn_get_type(val1);
    type2=rpn_get_type(val2);
    if(type1==RPN_TYPE_SCALAR) fval1=atof(val1);
    if(type2==RPN_TYPE_SCALAR) fval2=atof(val1);

    if(clStrcmp(op,"+")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=fval1+fval2;
	tresult=RPN_TYPE_SCALAR;
      }
    } else if(clStrcmp(op,"-")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=fval1-fval2;
	tresult=RPN_TYPE_SCALAR;
      }
    } else if(clStrcmp(op,"*")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=fval1*fval2;
	tresult=RPN_TYPE_SCALAR;
      }
    } else if(clStrcmp(op,"/")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=fval1/fval2;
	tresult=RPN_TYPE_SCALAR;
      }
    } else if(clStrcmp(op,"pow")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=pow(fval1,fval2);
	tresult=RPN_TYPE_SCALAR;
      }
    } else if(clStrcmp(op,"x")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=0.0;
	tresult=RPN_TYPE_SCALAR;
      }
    } else if(clStrcmp(op,"rmat")) {
      if(type1==RPN_TYPE_SCALAR &&
	 type2==RPN_TYPE_SCALAR) {
	fresult=0.0;
	tresult=RPN_TYPE_SCALAR;
      }
    }
    if(tresult==RPN_TYPE_SCALAR) {
      sprintf(op_result,"%f",fresult);
    }
  } else {
    // special op
    if(clStrcmp(op,"dist")) {
    } else if(clStrcmp(op,"angle")) {
    } else if(clStrcmp(op,"torsion")) {
    }
  }

  
  
  // place result back on stack
  rpn_push(op_result);
}


/*
  alias handling
*/

const char * shellResolveAlias(const char *reg)
{
  return alias_get(reg);
}

static void alias_list(const char *reg)
{
  int i;
  // TODO use regexp
  for(i=0;i<shell_alias.count;i++) {
    shellOut(shell_alias.entry[i].name);
    shellOut(" ");
    shellOut(shell_alias.entry[i].value);
    shellOut("\n");
  }
}

static void alias_def(const char *name2, const char **wl, int wc)
{
  int i;
  char name[ALIAS_MAX_NAME_SIZE];
  char value[ALIAS_MAX_VALUE_SIZE];
  
  clStrncpy(name,name2,ALIAS_MAX_NAME_SIZE-1);
  clStrcpy(value,"");
  for(i=0;i<wc;i++) {
    clStrcat(value,wl[i]);
    if(i+1<wc)
      clStrcat(value," ");
  }

  for(i=0;i<shell_alias.count;i++) {
    if(clStrcmp(shell_alias.entry[i].name,name))
      break;
  }
  

  if(i<shell_alias.max) {
    clStrcpy(shell_alias.entry[i].name,name);
    clStrcpy(shell_alias.entry[i].value,value);
  }

  if(i==shell_alias.count) shell_alias.count++;

}

static void alias_undef(const char *name)
{
  // TODO
}

static const char *alias_get(const char *name)
{
  int i;
  for(i=0;i<shell_alias.count;i++) {
    if(clStrcmp(shell_alias.entry[i].name,name)) {
      return shell_alias.entry[i].value;
    }
  }
  return NULL;
}

// variable initialization
static char *varlist[]={
  "protein", "(rname=ALA,CYS,ASP,GLU,PHE,GLY,HIS,ILE,LYS,LEU,MET,ASN,PRO,GLN,ARG,SER,THR,VAL,TRP,TYR)",
  "dna","(rname=A,C,G,T,ADE,CYT,GUA,THY)",
  "dna","(rname=A,C,G,T,ADE,CYT,GUA,THY)",
  "rna","(rname=A,C,G,U,ADE,CYT,GUA,URA)",
  "aliphatic","(rname=ALA,GLY,ILE,LEU,MET,PRO,VAL)",
  "aromatic","(rname=PHE,TYR,TRP)",
  "hydrophobic","(rname=ALA,VAL,PHE,PRO,MET,ILE,LEU,TRP)",
  "basic","(rname=LYS,ARG)",
  "basic2","((rname=LYS and aname=NZ) or (rname=ARG and aname=NH1,NH2))",
  "acidic","(rname=ASP,GLU)",
  "acidic2","((rname=GLU and aname=OE1,OE2) or (rname=ASP and aname=OD1,OD2))",
  "polar","(rname=SER,THR,TYR,HIS,CYS,ASN,GLN)",
  "polar2","((rname=SER and aname=OG) or (rname=THR and aname=OG1) or (rname=TYR and aname=OH) or (rname=HIS and aname=ND1,NE2) or (rname=CYS and aname=SG) or (rname=ASN and aname=OD1,ND1) or (rname=GLN and aname=OE1,NE1) or (rname=TRP and aname=NE1))",
  "CP","{0,0,0}",
  NULL,NULL

};

static void init_vars(void)
{
  int i;
  for(i=0;varlist[i]!=NULL;i+=2)
    shellSetVar(varlist[i],varlist[i+1]);

}
