/***********************************

 shell_tcl.c

 OLD DEPRECATED FILE
 REPLACED BY 
 shell_raw
 shell_command
 gui_terminal

************************************/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#ifdef SGI
#include <stropts.h>
#endif
#ifdef LINUX
#include <sys/ioctl.h>
#endif
#ifdef DEC
#include <sys/ioctl.h>
#endif

#include <tcl.h>

#include "dino.h"
#include "shell.h"
#include "mat.h"
#include "Cmalloc.h"
#include "com.h"
#include "cl.h"


#define SHELL_SAVEHISTLEN 100

#define shell_stack_expr_len 1024

enum             {SHELL_INTERACTIVE,
		  SHELL_SCRIPT,
		  SHELL_PAUSE,
		  SHELL_BREAK,
		  SHELL_INIT_COMMAND};

enum              {SHELL_OK, 
		   SHELL_ERROR,
		   SHELL_EOF,
		   SHELL_RET};

enum               {SHELL_NORMAL,
		    SHELL_WHILE,
		    SHELL_FOREACH,
		    SHELL_IF};

struct SHELL_ALIAS {
  char name[64];
  char value[2048];
};

struct SHELL_CHARBUF {
  char buf[1024];
  int count;
};

struct SHELL_VAR {
  char name[64];
  char *value;
};

struct SHELL_EXPR_STACK {
  char expr[shell_stack_expr_len];
};

struct SHELL_POS_STACK {
  int pos;
  int exec;
  char command[16];
  char param1[256];
  char param2[256];
  char param3[256];
};

struct SHELL_INSTANCE {
  int state,save_state;
  char *buf;
  int buf_size;
  int buf_pos;
  char *argv[1024];
  int argc;
  struct SHELL_POS_STACK *pstack;
  int pstackp;
  int pstack_max;
  struct SHELL_VAR *var;
  int var_max;
  struct SHELL_INSTANCE *parent;
  struct SHELL_INSTANCE *child;
};


struct SHELL {
  /* 
     terminal parameters 
  */
  sigset_t sig,savesig;
  struct termios modes,savemodes;
  shell_callback callback;

  struct winsize ws;

  /* 
     interactive mode
       - prompt editing
       - savelist
  */
  char *prompt;
  int max_prompt;
  int cpos,clen,cstart,maxlen;

  char *stdinbuf;
  int stdinbuflen;

  struct SAVE_LIST {
    char entry[256];
  }savehist[SHELL_SAVEHISTLEN];
  int savehistp,scrollhistp;
  int savebufpos,safebufmax;
  char *savebuf;

  FILE *logfile;

  struct SHELL_CHARBUF charbuf;

  int state;

  struct SHELL_INSTANCE top;
  struct SHELL_INSTANCE *current;
  int instance_count;

  struct SHELL_SCRIPT {
    char *buf;
    int buf_size;
    int buf_pos;
    int argc;
    char **argv;
  }script;

  struct SHELL_EXPR_STACK *estack;
  int estackp;
  int estack_max;

  struct SHELL_ALIAS *alias;
  int aliasc,aliasm;

  char logfilename[128];

  char *static_buf;
  int static_buf_max;

  char cwd[256];

  int virgin_flag;
  char init_cmd[1024];

  int greedy;
  int prompt_flag;

  Tcl_DString tcl_ds;
  Tcl_Interp *tcl_interp;
};



char shell_prompt[]={"dino> "};

extern int debug_mode;

struct SHELL shell;

/*
  static functions first
*/

static void shellWriteLine()
{
  int lpos,len_left,len_right,len_fill;
  char spc[256];
  char bckspc[256];

  memset(spc,32,256);
  memset(bckspc,8,256);

  lpos=shell.cpos-shell.cstart;

  len_right=(shell.clen-shell.cstart)-lpos;
  len_left=lpos;
  if(len_left+len_right>=shell.maxlen) {
    len_right=shell.maxlen-len_left;
    len_fill=0;
  } else {
    len_fill=shell.maxlen-len_right-len_left;
  }
  write(fileno(stdout),&shell.prompt[shell.cstart],len_left+len_right);
  write(fileno(stdout),spc,len_fill);
  write(fileno(stdout),bckspc,len_right+len_fill);
}

static void shellEraseLine()
{
  int lpos,len_left,len_right;
  char spc[256];
  char bckspc[256];

  memset(spc,32,256);
  memset(bckspc,8,256);

  lpos=shell.cpos-shell.cstart;

  len_right=shell.maxlen-lpos;
  len_left=lpos;

  write(fileno(stdout),bckspc,len_left);
  write(fileno(stdout),spc,len_left+len_right);
  write(fileno(stdout),bckspc,len_left+len_right);
}

static int shellAddPrompt(char **prompt,int *prompt_max, int *prompt_len,char c)
{
  char *os;
  (*prompt)[(*prompt_len)++]=c;
  if((*prompt_len)>=(*prompt_max)) {
    os=(*prompt);
    (*prompt)=Ccalloc((*prompt_max)+1024,sizeof(char));
    memcpy((*prompt),os,(*prompt_max)*sizeof(char));
    Cfree(os);
    (*prompt_max)+=1024;
  }
  return 0;
}

static const char *shellGetVar(const char *n)
{
  int i;
  static char *empty="\0";
  if(n==NULL)
    return NULL;

  for(i=0;i<shell.current->var_max;i++)
    if(strlen(shell.current->var[i].name)!=0)
      if(!strcmp(n,shell.current->var[i].name))
	return shell.current->var[i].value;

  return NULL;
}

static void shellSigWinch(int sig)
{
  ioctl(fileno(stdin),TIOCGWINSZ,&shell.ws);
  shell.maxlen=shell.ws.ws_col-strlen(shell_prompt)-1;
  shellEraseLine();
  shellWriteLine();
}


static int shellEcho(int wc, const char **wl)
{
  int i,nwc=wc;
  FILE *f=NULL;
  char message[256];

  for(i=0;i<wc;i++)
    if(!strcmp(wl[i],">") ||
       !strcmp(wl[i],">>")) {
      if(wc-i<2) {
	shellOut("Error: Missing file for redirection\n");
	return -1;
      } else if(wc-i>2) {
	shellOut("Error: Chars after filename\n");
	return -1;
      }
      if(!strcmp(wl[i],">")) {
	f=fopen(wl[i+1],"w");
      } else {
	f=fopen(wl[i+1],"a");
      }
      if(f==NULL) {
	sprintf(message,"Error: cannot open %s\n",wl[i+1]);
	shellOut(message);
	return -1;
      }
      nwc=i;
      break;
    }
  
  if(f==NULL) {
    shellOut("\n");
    for(i=0;i<nwc;i++) {
      shellOut(wl[i]);
      if(i+1<nwc)
	shellOut(" ");
    }
  } else {
    for(i=0;i<nwc;i++) {
      fprintf(f,wl[i]);
      if(i+1<nwc)
	fprintf(f," ");
    }
    fprintf(f,"\n");
    fclose(f);
  }
  return 0;
}

/***********************************************************

 shellOpr: apply mathematic operators to stack

                            | v1 |
 vectors are Nx1 matrices : | v2 |
                            | v3 |
 coded as : {v1,v2,v3}

                             | a11 a12 a13 |
 matrices are Row x Column : | a21 a22 a23 |
                             | a31 a32 a33 |
 
 coded as : {{a11,a12,a13},{a21,a22,a23},{a31,a32,a33}}

*************************************************************/

static char special_op[][8]={
  "dist","angle","torsion"
};

static char binary_op[][8]={
  "+","-","*","/","pow","x","rmat"
};

static char unary_op[][8]={
  "+-",  "inc","dec","abs",
  "log","ln","exp","sqrt","inv",
  "sin","cos","tan","asin","acos","atan",
  "int","float","det"
};

static int shellOpr(const char *op2)
{
  char *v1,*v2,*v3;  
  char * vc[4];
  double vec[4][16];
  double vec1[16],vec2[16],vec3[16],vec4[16];
  int indx[16];
  double det;
  int d1,d2,d3,d4;
  int i,j,k;
  char null[]="0";
  double result;
  char message[256];
  int bflag,sflag;
  char op[8];

  strncpy(op,op2,7);

  // first check for special ops
  if(clStrcmp("dist",op)) {
    // calculate distance based on two vectors
    for(i=0;i<2;i++) {
      if(shell.estackp<0)
	vc[i]=null;
      else
	vc[i]=shell.estack[shell.estackp--].expr;
    }

    shell.estackp++;

    for(i=0;i<2;i++) {
      if(vc[i][0]=='{') {
	if(matGetDim(vc[i],&d1,&d2)!=0) {
	  shellOut("matrix error in stack position 1\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	if(d1==3 && d2==1) {
	  matExtract1D(vc[i],d1,vec[i]);
	} else {
	  shellOut("expected vector length 3 in stack position 1\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
      } else {
	shellOut("expected vector length 3 in stack position 1\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    }
    // calculate distance between v1 and v2
    result=matCalcDistance(vec[0],vec[1]);
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
    return SHELL_OK;
  } else if(clStrcmp("angle",op)) {
    // calculate angle between v1v2 and v2v3 (three vectors)

    if(shell.estackp<2) {
      shellOut("error: dist requires two vectors on stack\n");
    }

    for(i=0;i<3;i++) {
      if(shell.estackp<0)
	vc[i]=null;
      else
	vc[i]=shell.estack[shell.estackp--].expr;
    }

    shell.estackp++;

    for(i=0;i<3;i++) {
      if(vc[i][0]=='{') {
	if(matGetDim(vc[i],&d1,&d2)!=0) {
	  shellOut("matrix error in stack position 1\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	if(d1==3 && d2==1) {
	  matExtract1D(vc[i],d1,vec[i]);
	} else {
	  shellOut("expected vector length 3 in stack position 1\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
      } else {
	shellOut("expected vector length 3 in stack position 1\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    }

    // calculate angle from v1 v2 v3
    result=matCalcAngle(vec[0],vec[1],vec[2],vec[1]);

    sprintf(shell.estack[shell.estackp].expr,"%g",result);
    return SHELL_OK;
  } else if(clStrcmp("torsion",op)) {
    /*
      calculate torsion angle between 
      4 given vectors, with first line
      defined by v1 and v2, the torsion
      line between v2 and v3 and the second
      line between v3 and v4
     */
    for(i=0;i<4;i++) {
      if(shell.estackp<0)
	vc[i]=null;
      else
	vc[i]=shell.estack[shell.estackp--].expr;
    }

    shell.estackp++;

    for(i=0;i<4;i++) {
      if(vc[i][0]=='{') {
	if(matGetDim(vc[i],&d1,&d2)!=0) {
	  shellOut("matrix error in stack position 1\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	if(d1==3 && d2==1) {
	  matExtract1D(vc[i],d1,vec[i]);
	} else {
	  shellOut("expected vector length 3 in stack position 1\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
      } else {
	shellOut("expected vector length 3 in stack position 1\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    }

    // calculate torsion from v1 v2 v3 v4

    result=matCalcTorsion(vec[0],vec[1],vec[2],vec[3]);
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
    return SHELL_OK;
  }


  bflag=1;
  for(i=0;i<sizeof(binary_op)/8;i++)
    if(!strcmp(binary_op[i],op))
      break;

  if(i==sizeof(binary_op)/8) {
    bflag=0;
    for(i=0;i<sizeof(unary_op)/8;i++)
      if(!strcmp(unary_op[i],op))
	break;
    if(i==sizeof(unary_op)/8) {
      shellOut("unknown operator \n");
      shellOut(op);
      return SHELL_ERROR;
    } else {
      if(shell.estackp<0)
	v1=null;
      else
	v1=shell.estack[shell.estackp--].expr;
    }  
  } else {
      if(shell.estackp<0)
	v1=null;
      else
	v1=shell.estack[shell.estackp--].expr;
      if(shell.estackp<0)
	v2=null;
      else
	v2=shell.estack[shell.estackp--].expr;
  }
  
  shell.estackp++;

  if(v1[0]=='{') {
    if(matGetDim(v1,&d1,&d2)!=0) {
      shellOut("matrix error in 1\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if((d2==1 && d1==3) || (d2==1 && d1==4) ||
       (d2==3 && d1==3) || (d2==4 && d1==4)) {
    if(d2>1)
      matExtract2D(v1,d2,d1,vec1);
    else
      matExtract1D(v1,d1,vec1);
    } else {
      sprintf(message,"invalid matrix dimension: %dx%d\n",d1,d2);
      shellOut(message);
      strcpy(op,"0");
      return SHELL_ERROR;
    }
  }

  if(bflag)
    if(v2[0]=='{') {
      if(matGetDim(v2,&d3,&d4)!=0) {
	shellOut("matrix error in 2\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
      if((d4==1 && d3==3) || (d4==1 && d3==4) ||
	 (d4==3 && d3==3) || (d4==4 && d3==4)) {
	if(d4>1)
	  matExtract2D(v2,d4,d3,vec2);
	else
	  matExtract1D(v2,d3,vec2);
      } else {
	sprintf(message,"invalid matrix dimension: %dx%d\n",d1,d2);
	shellOut(message);
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    }
  
  if(!strcmp(op,"+")) {
    /**************************
         Binary Operator +
    **************************/
    if(v1[0]=='{') {
      if(v2[0]=='{') {
	if(d1!=d3 || d2!=d4) {
	  shellOut("matrix dimensions not compatible\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	for(i=0;i<d1*d2;i++)
	  vec3[i]=vec1[i]+vec2[i];
	if(d2>1)
	  matAssemble2D(vec3,d2,d1,message);
	else
	  matAssemble1D(vec3,d1,message);
	
	strcpy(shell.estack[shell.estackp].expr,message);
      } else {
	shellOut("missmatch error\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    } else {
      result=atof(v2)+atof(v1);
      sprintf(shell.estack[shell.estackp].expr,"%g",result);
    }
  } else if(!strcmp(op,"-")) {
    /**************************
         Binary Operator -
     **************************/
    if(v1[0]=='{') {
      if(v2[0]=='{') {
	if(d1!=d3 || d2!=d4) {
	  shellOut("matrix dimensions not compatible\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	for(i=0;i<d1*d2;i++)
	  vec3[i]=vec2[i]-vec1[i];

	if(d2>1)
	  matAssemble2D(vec3,d2,d1,message);
	else
	  matAssemble1D(vec3,d1,message);

	strcpy(shell.estack[shell.estackp].expr,message);
	  
      } else {
	shellOut("missmatch error\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    } else {
      result=atof(v1)-atof(v2);
      sprintf(shell.estack[shell.estackp].expr,"%g",result);
    }
  } else if(!strcmp(op,"*")) {
    /**************************
         Binary Operator *
     **************************/
    if(v1[0]=='{') {
      if(v2[0]=='{') {
	if (d1==d2 && d2==d3 && d3==d4) {
	  /* square matrices */
	  for(i=0;i<d1;i++)
	    for(j=0;j<d1;j++) {
	      vec3[i*d1+j]=0.0;
	      for(k=0;k<d1;k++) {
		vec3[i*d1+j]+=vec1[i*d1+k]*vec2[k*d1+j];
	      }
	    }
	  matAssemble2D(vec3,d2,d1,message);
	  strcpy(shell.estack[shell.estackp].expr,message);
	} else if(d2==1 && d4==1 && d1==d3) {
	  /* identical vectors: dot product */
	  result=0.0;
	  for(i=0;i<d1;i++)
	    result+=vec1[i]*vec2[i];
	  sprintf(shell.estack[shell.estackp].expr,"%g",result);
	} else if(d1==d3 && d1==d4 && d2==1) {
	  /* NxN matrix * Nx1 vector */
	  for(i=0;i<d1;i++) {
	      vec3[i]=0.0;
	      for(k=0;k<d1;k++) {
		vec3[i]+=vec2[i*d1+k]*vec1[k];
	      }
	  }	  
	  matAssemble1D(vec3,d1,message);
	  strcpy(shell.estack[shell.estackp].expr,message);
	  
	} else {
	  shellOut("matrix dimensions not compatible\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
      } else {
	for(i=0;i<d1*d2;i++)
	  vec3[i]=vec1[i]*atof(v2);
	if(d2>1)
	  matAssemble2D(vec3,d2,d1,message);
	else
	  matAssemble1D(vec3,d1,message);
	
      strcpy(shell.estack[shell.estackp].expr,message);
      }
    } else if(v2[0]=='{') {
      for(i=0;i<d3*d4;i++)
	vec3[i]=vec2[i]*atof(v1);
      if(d4>1)
	matAssemble2D(vec3,d4,d3,message);
      else
	matAssemble1D(vec3,d3,message);

      strcpy(shell.estack[shell.estackp].expr,message);

    } else {
      result=atof(v1)*atof(v2);
      sprintf(shell.estack[shell.estackp].expr,"%g",result);
    }
  } else if(!strcmp(op,"/")) {
    /**************************
         Binary Operator /
     **************************/
    if(v1[0]=='{') {
      shellOut("divide by matrix error\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(atof(v1)==0.0) {
      shellOut("Divide by zero\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(v2[0]=='{') {
      for(i=0;i<d3*d4;i++)
	vec3[i]=vec2[i]/atof(v1);
      if(d4>1)
	matAssemble2D(vec3,d4,d3,message);
      else
	matAssemble1D(vec3,d3,message);

      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      result=atof(v2)/atof(v1);
      sprintf(shell.estack[shell.estackp].expr,"%g",result);
    }
  } else if(!strcmp(op,"pow")) {
    /**************************
	Binary Operator pow
    **************************/
    if(v1[0]=='{' || v2[0]=='{') {
      shellOut("invalid matrix operator pow\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    } else {
      result=pow(atof(v2),atof(v1));
      sprintf(shell.estack[shell.estackp].expr,"%g",result);
    }
  } else if(!strcmp(op,"x")) {
    /**************************
	  Binary Operator x
    **************************/
    if(v1[0]!='{' || v2[0]!='{') {
      shellOut("invalid scalar operator x\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(d2>1) {
      shellOut("invalid 2D matrix operator x\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(d1!=3 || d3!=3) {
      shellOut("vector dimension not 3\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    vec3[0]=vec1[1]*vec2[2]-vec1[2]*vec2[1];
    vec3[1]=vec1[0]*vec2[2]-vec1[2]*vec2[0];
    vec3[2]=vec1[0]*vec2[1]-vec1[2]*vec2[0];
    
    matAssemble1D(vec3,d1,message);
    
    strcpy(shell.estack[shell.estackp].expr,message);
    
  } else if(!strcmp(op,"rmat")) {
    /**************************
       Binary Operator rmat
    **************************/
    if(v1[0]=='{' && v2[0]!='{') {
      if(d1==4) {
	if(v1[3]==0.0) {
	  shellOut("w must be non-zero\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	v1[0]/=v1[3];
	v1[1]/=v1[3];
	v1[2]/=v1[3];
      }
      matMakeRotMat(atof(v2),v1[0],v1[1],v1[2],vec2);
      for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	  vec3[i*3+j]=vec2[i*4+j];
      matAssemble2D(vec3,3,3,message);
      strcpy(shell.estack[shell.estackp].expr,message);
    } else if(v1[0]!='{' && v2[0]=='{') {
      if(d3==4) {
	if(v2[3]==0.0) {
	  shellOut("w must be non-zero\n");
	  strcpy(op,"0");
	  return SHELL_ERROR;
	}
	v2[0]/=v2[3];
	v2[1]/=v2[3];
	v2[2]/=v2[3];
      }
      matMakeRotMat(atof(v1),v2[0],v2[1],v2[2],vec2);
      for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	  vec3[i*3+j]=vec2[i*4+j];
      matAssemble2D(vec3,3,3,message);
      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      shellOut("rmat requires one scalar and one vector\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }

  } else if(!strcmp(op,"+-")) {
    /**************************
         Unary Operator +-
     **************************/
    if(v1[0]=='{') {
      for(i=0;i<d1*d2;i++)
	vec3[i]=-1.0*vec1[i];
      if(d2>1)
	matAssemble2D(vec3,d2,d1,message);
      else
	matAssemble1D(vec3,d1,message);

      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      sprintf(shell.estack[shell.estackp].expr,"%g",-atof(v1));
    }
  } else if(!strcmp(op,"inc")) {
    /**************************
         Unary Operator inc
     **************************/
    if(v1[0]=='{') {
      for(i=0;i<d1*d2;i++)
	vec3[i]=vec1[i]+1.0;
      if(d2>1)
	matAssemble2D(vec3,d2,d1,message);
      else
	matAssemble1D(vec3,d1,message);

      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      sprintf(shell.estack[shell.estackp].expr,"%g",atoi(v1)+1.0);
    }
  } else if(!strcmp(op,"dec")) {
    /**************************
         Unary Operator dec
     **************************/
    if(v1[0]=='{') {
      for(i=0;i<d1*d2;i++)
	vec3[i]=vec1[i]-1.0;
      if(d2>1)
	matAssemble2D(vec3,d2,d1,message);
      else
	matAssemble1D(vec3,d1,message);

      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      sprintf(shell.estack[shell.estackp].expr,"%g",atoi(v1)-1.0);
    }
  } else if(!strcmp(op,"ln")) {
    /**************************
         Unary Operator ln
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator ln\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=log(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"log")) {
    /**************************
         Unary Operator log
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator log\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=log10(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"exp")) {
    /**************************
         Unary Operator exp
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator exp\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=exp(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"sin")) {
    /**************************
         Unary Operator sin
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator sin\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=sin(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"cos")) {
    /**************************
         Unary Operator cos
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator cos\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=cos(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"tan")) {
    /**************************
         Unary Operator tan
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator tan\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=tan(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"asin")) {
    /**************************
         Unary Operator asin
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator asin\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=asin(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"acos")) {
    /**************************
         Unary Operator acos
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator acos\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=acos(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"atan")) {
    /**************************
         Unary Operator atan
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator atan\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=atan(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"int")) {
    /**************************
         Unary Operator int
     **************************/
    if(v1[0]=='{') {
      for(i=0;i<d1*d2;i++)
	vec3[i]=(double)(int)vec1[i];
      if(d2>1)
	matAssemble2D(vec3,d2,d1,message);
      else
	matAssemble1D(vec3,d1,message);

      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      sprintf(shell.estack[shell.estackp].expr,"%d",atoi(v1));
    }
  } else if(!strcmp(op,"float")) {
    /**************************
         Unary Operator float
     **************************/
    if(v1[0]=='{') {
      for(i=0;i<d1*d2;i++)
	vec3[i]=1.0*vec1[i];
      if(d2>1)
	matAssemble2D(vec3,d2,d1,message);
      else
	matAssemble1D(vec3,d1,message);

      strcpy(shell.estack[shell.estackp].expr,message);
    } else {
      sprintf(shell.estack[shell.estackp].expr,"%g",atof(v1));
    }
  } else if(!strcmp(op,"abs")) {
    /**************************
         Unary Operator abs
     **************************/
    if(v1[0]=='{') {
      if(d2==1) {
	result=0;
	for(i=0;i<d1*d2;i++)
	  result+=(vec1[i]*vec1[i]);
	
	sprintf(shell.estack[shell.estackp].expr,"%g",sqrt(result));
      } else {
	shellOut("invalid 2D matrix operator abs\n");
	strcpy(op,"0");
	return SHELL_ERROR;
      }
    } else {
      sprintf(shell.estack[shell.estackp].expr,"%g",fabs(atof(v1)));
    }
  } else if(!strcmp(op,"sqrt")) {
    /**************************
         Unary Operator sqrt
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator sqrt\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=sqrt(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"inv")) {
    /**************************
         Unary Operator inv
     **************************/
    if(v1[0]=='{') {
      shellOut("invalid matrix operator inv\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(atof(v1)==0.0) {
      shellOut("Divide by zero\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    result=1.0/(atof(v1));
    sprintf(shell.estack[shell.estackp].expr,"%g",result);
  } else if(!strcmp(op,"det")) {
    /**************************
         Unary Operator det
     **************************/
    if(v1[0]!='{') {
      shellOut("Invalid scalar operator det\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(d1!=d2) {
      shellOut("non-square matrix\n");
      strcpy(op,"0");
      return SHELL_ERROR;
    }
    if(d1==3) {
      for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	  vec3[i*4+j]=vec1[i*3+j];
      vec3[3]=0.0;
      vec3[7]=0.0;
      vec3[11]=0.0;
      vec3[12]=0.0;vec3[13]=0.0;vec3[14]=0.0;vec3[15]=1.0;

    } else {
      for(i=0;i<4;i++)
	for(j=0;j<4;j++)
	  vec3[i*4+j]=vec1[i*4+j];
    }
    matLUdcmp(vec3,indx,&det,vec3);
    for(i=0;i<4;i++)
      det*=vec3[i*4+i];
    sprintf(shell.estack[shell.estackp].expr,"%g",det);
  } else {
    shellOut("Unknown operator\n");
    strcpy(op,"0");
    return SHELL_ERROR;
  }
  strcpy(op,shell.estack[shell.estackp].expr);
  return SHELL_OK;
}


/*************************************************

  shellParse
  ----------

  go through word list received from shellWork, 
  scan for internal shell commands, and if
  necessary pass to com module

**************************************************/

static int shellParse(int wc, const char **wl,int pos)
{
  int i,j;
  FILE *f;
  struct stat st;
  const char *scriptfile;
  char tcl_file[1024],*tcl_file_buf;
  char message[256];
  struct SHELL_INSTANCE *child;
  struct SHELL_EXPR_STACK *os;
  char subexp[1024];
  char es[shell_stack_expr_len];
  const char *varlist[2];
  int arglen;
  int testi;
  float *testf;

  if(wc<1)
    return SHELL_ERROR;

  if(shell.state==SHELL_SCRIPT) {
    // echo commands
    shellEcho(wc,wl);
  }

  if(wl[0][0]=='@') {

    if(shell.instance_count>1024) {
      shellOut("Maximum nesting level (1024) reached\n");
      return SHELL_ERROR;
    }
    scriptfile=&wl[0][1];
    if((f=fopen(scriptfile,"r"))==NULL) {
      sprintf(message,"error accessing file: %s\n",scriptfile);
      shellOut(message);
      return SHELL_ERROR;
    }
    fstat(fileno(f),&st);
    if(st.st_size==0) {
      sprintf(message,"zero length file: %s\n",scriptfile);
      shellOut(message);
      return SHELL_ERROR;
    }

    /* new code starts here*/
    tcl_file_buf=Cmalloc(st.st_size+1);
    fread(tcl_file_buf,st.st_size,1,f);
    tcl_file_buf[st.st_size]='\0';
    i=0;
    while(i<st.st_size-1) {
      if(tcl_file_buf[i]=='/' && tcl_file_buf[i+1]=='/') {
	j=i;
	while(j<st.st_size-1) {
	  if(tcl_file_buf[j]=='\n' ||
	     tcl_file_buf[j]=='\r')
	    break;
	  tcl_file_buf[j]=' ';
	  j++;
	}
      }
      i++;
    }
    fclose(f);
    shell.state=SHELL_SCRIPT;
    Tcl_EvalEx(shell.tcl_interp,tcl_file_buf,st.st_size,TCL_EVAL_GLOBAL);
    shell.state=SHELL_INTERACTIVE;
    Cfree(tcl_file_buf);

  } else if(wl[0][0]=='!') {
    // system subcommand
    shellOut("\n");
    system(wl[0]+1);
  } else if(!strcmp(wl[0],"echo")){
    shellEcho(wc-1,wl+1);
  } else if(!strcmp(wl[0],"push")) {
    for(i=1;i<wc;i++) {
      shell.estackp++;
      if(shell.estackp>shell.estack_max) {
	os=shell.estack;
	shell.estack=Ccalloc(shell.estack_max+1024,
			    sizeof(struct SHELL_EXPR_STACK));
	memcpy(shell.estack,os,
	       shell.estack_max*sizeof(struct SHELL_EXPR_STACK));
	Cfree(os);
	shell.estack_max+=1024;
      }
      strncpy(shell.estack[shell.estackp].expr,wl[i],shell_stack_expr_len);
      shell.estack[shell.estackp].expr[shell_stack_expr_len-1]='\0';
    }
    if(shell.estackp<0) {
      comReturn("");
    } else {
      comReturn(shell.estack[shell.estackp].expr);
    }
  } else if(!strcmp(wl[0],"pop")) {
    if(wc>1) {
      for(i=1;i<wc;i++) {
	if(shell.estackp>=0) {
	  shellSetVar(wl[i],shell.estack[shell.estackp].expr);
	  shell.estackp--;
	} else {
	  strcpy(message,"0");
	  shellSetVar(wl[i],"0");
	}
      }
    } else {
      if(shell.estackp>=0) {
	comReturn(shell.estack[shell.estackp].expr);
	shell.estackp--;
      } else {
	comReturn("");
      }
    }
  } else if(!strcmp(wl[0],"dup")) {
    shell.estackp++;
    if(shell.estackp>shell.estack_max) {
      os=shell.estack;
      shell.estack=Ccalloc(shell.estack_max+1024,
			  sizeof(struct SHELL_EXPR_STACK));
      memcpy(shell.estack,os,
	     shell.estack_max*sizeof(struct SHELL_EXPR_STACK));
      Cfree(os);
      shell.estack_max+=1024;
    }
    strcpy(shell.estack[shell.estackp].expr,
	   shell.estack[shell.estackp-1].expr);
  } else if(!strcmp(wl[0],"swap")) {
    if(shell.estackp>0) {
      strcpy(es,shell.estack[shell.estackp-1].expr);
      strcpy(shell.estack[shell.estackp-1].expr,
	     shell.estack[shell.estackp].expr);
      strcpy(shell.estack[shell.estackp].expr,es);
    }
  } else if(!strcmp(wl[0],"peek")) {
    if(shell.estackp<0)
      comReturn("0");
    else
      comReturn(shell.estack[shell.estackp].expr);
  } else if(!strcmp(wl[0],"show")) {
    if(shell.estackp<0)
      shellOut("stack is empty\n");
    
    for(i=shell.estackp;i>=0;i--) {
      shellOut("\n");
      shellOut(shell.estack[i].expr);
    }
  } else if(!strcmp(wl[0],"clear")) {
    shell.estackp=-1;
    comReturn("0");
  } else if(!strcmp(wl[0],"opr")) {
    if(wc<=1) {
      if(shell.estackp>=0) {
	comReturn(shell.estack[shell.estackp].expr);
      } else {
	comReturn("0");
      }
    } else {
      for(i=1;i<wc;i++) {
	if(shellOpr(wl[i])==SHELL_ERROR) {
	  return SHELL_ERROR;
	}
	comReturn(wl[i]);
      }
    }
  } else if(!strcmp(wl[0],"system")) {
    shellOut("\n");
    strcpy(subexp,"");
    for(i=1;i<wc;i++) {
      strcat(subexp,wl[i]);
      strcat(subexp," ");
    }
    system(subexp);
  } else if(!strcmp(wl[0],"cd")) {
    if(wc<2) {
      chdir(shell.cwd);
    } else {
      if(chdir(wl[1])<0) {
	shellOut("error during chdir\n");
      }
    }
  } else if(!strcmp(wl[0],"pwd")) {
    getcwd(message,256);
    shellOut("\n");
    shellOut(message);
  } else if(!strcmp(wl[0],"pause")) {
    shellOut("<Pause>\n");
    shell.current->save_state=shell.current->state;
    shell.current->state=SHELL_PAUSE;
  } else if(!strcmp(wl[0],"break")) {
    shell.current->save_state=shell.current->state;
    shell.current->state=SHELL_BREAK;
  } else if(!strcmp(wl[0],"alias")) {
    if(wc<3) { 
      if(wc==1) {
	for(i=0;i<shell.aliasc;i++) {
	  sprintf(message,"%s: %s\n",shell.alias[i].name,shell.alias[i].value);
	  shellOut(message);
	}
      } else {
	sprintf(message,"syntax error\n");
	return SHELL_ERROR;
      }
    } else {
      for(i=0;i<shell.aliasc;i++)
	if(!strcmp(shell.alias[i].name,wl[1]))
	  break;

      if(i==shell.aliasc)
	shell.aliasc++;
	   
      strcpy(shell.alias[i].name,wl[1]);
      strcpy(shell.alias[i].value,"");
      for(j=2;j<wc;j++) {
	strcat(shell.alias[i].value,wl[j]);
	if(j+1<wc)
	  strcat(shell.alias[i].value," ");
      }
    }
  } else if(!strcmp(wl[0],"unalias")) {
  } else {
      /* go through subfunction list, if no match then callback */
    
    if(shell.callback(wc,wl)!=0)
      return SHELL_ERROR;
//    wl[0]=comGetReturn();
  }
  return SHELL_OK;
}

/***********************************************************

  shellWorkInteractive
  --------------------							    

  Reads input from stdin and passes it along when a carriage
  return is encountered. Is called periodically through
  the timer routine (via shellWork).

  For the interactive shell, there are two memory areas:
  stdinbuf, to take the input from stdin,
  and prompt, which is the resulting string that is
  passed to the further processing routines. 
  Both are static in the sense that they are allocated once,
  so only a copy of them should be used further on.
*************************************************************/

static int shellWorkInteractive()
{
  char c;
  char *ob;
  int i,bc,j;

  /*
    read characters one by one from stdin and place them in buf
  */
  bc=0;
  while(read(fileno(stdin),&c,1)>0) {
    shell.stdinbuf[bc++]=c;
    if(bc>=shell.stdinbuflen) {
      ob=shell.stdinbuf;
      shell.stdinbuf=Ccalloc(shell.stdinbuflen+1024,sizeof(char));
      memcpy(shell.stdinbuf,ob,shell.stdinbuflen*sizeof(char));
      Cfree(ob);
      shell.stdinbuflen+=1024;
    }
  }
  /*
    read characters that were placed in charbuf (by gfx) 
  */

  while(shell.charbuf.count>0) {
    c=shell.charbuf.buf[0];
    for(i=0;i<shell.charbuf.count-1;i++)
      shell.charbuf.buf[i]=shell.charbuf.buf[i+1];
    shell.charbuf.count--;
    if(bc<shell.stdinbuflen)
      shell.stdinbuf[bc++]=c;
    /*
    shell.stdinbuf[bc++]=c;
    if(bc>=shell.stdinbuflen) {
      ob=shell.stdinbuf;
      shell.stdinbuf=Ccalloc(shell.stdinbuflen+1024,sizeof(char));
      memcpy(shell.stdinbuf,ob,shell.stdinbuflen*sizeof(char));
      Cfree(ob);
      shell.stdinbuflen+=1024;
    }
    */
  }

  shell.stdinbuf[bc]='\0';
  if(bc>0) {
    if(shell.stdinbuf[0]==27) {
      /* escape sequence */
      if(!strcmp(shell.stdinbuf+1,"[A") ||
	 !strcmp(shell.stdinbuf+1,"OA")) {
	/* up */
	if(shell.scrollhistp>0){
	  shellEraseLine();
	  strncpy(shell.prompt,shell.savehist[--shell.scrollhistp].entry,256);
	  shell.prompt[255]='\0';
	  shell.clen=strlen(shell.prompt);
	  shell.cpos=shell.clen;
	  shell.cstart=shell.clen-(shell.ws.ws_col-strlen(shell_prompt))+1;
	  if(shell.cstart<0)
	    shell.cstart=0;
	  shellWriteLine();
	}

      }else if(!strcmp(shell.stdinbuf+1,"[B") ||
	       !strcmp(shell.stdinbuf+1,"OB")){
	/* down */
	if(shell.scrollhistp<shell.savehistp){
	  if(shell.clen>0){
	    shellEraseLine();
	  }
	  strncpy(shell.prompt,shell.savehist[++shell.scrollhistp].entry,256);
	  shell.prompt[255]='\0';
	  shell.clen=strlen(shell.prompt);
	  shell.cpos=shell.clen;
	  shell.cstart=shell.clen-(shell.ws.ws_col-strlen(shell_prompt))+1;
	  if(shell.cstart<0)
	    shell.cstart=0;
	  shellWriteLine();
	}
	
      }else if(!strcmp(shell.stdinbuf+1,"[C") ||
	       !strcmp(shell.stdinbuf+1,"OC")){
	/* right */
	if(shell.cpos<shell.clen) {
	  shellEraseLine();
	  shell.cpos++;
	  if((shell.cpos-shell.cstart)>=shell.maxlen)
	    shell.cstart++;
	  shellWriteLine();
	}
      }else if(!strcmp(shell.stdinbuf+1,"[D") ||
	       !strcmp(shell.stdinbuf+1,"OD")){
	/* left */
	if(shell.cpos>0) {
	  shellEraseLine();
	  shell.cpos--;
	  if(shell.cpos<shell.cstart && shell.cstart>0)
	    shell.cstart--;
	 
	  shellWriteLine();
	}
	
      }else if(!strcmp(shell.stdinbuf+1,"[11~")){
	/* f1 */
      }else if(!strcmp(shell.stdinbuf+1,"[12~")){
	/* f2 */
      }
    } else {
      /* NOT an escape sequence */
      for(i=0;i<bc;i++) {
	if(isprint(shell.stdinbuf[i])) {
	  if(!isspace(shell.stdinbuf[i]) || 
	     (shell.cpos<1 && isspace(shell.stdinbuf[i])) ||
	     (isspace(shell.stdinbuf[i]) && !isspace(shell.prompt[shell.cpos-1]))) { 
	    for(j=shell.clen;j>shell.cpos;--j)
	      shell.prompt[j]=shell.prompt[j-1];
	    shellEraseLine();
	    shell.prompt[shell.cpos++]=shell.stdinbuf[i];
	    if((shell.cpos-shell.cstart)>=shell.maxlen)
	      shell.cstart++;
	    shell.clen++;
	    if(shell.clen>=shell.max_prompt) {
	      ob=shell.prompt;
	      shell.prompt=Ccalloc(shell.max_prompt+1024,sizeof(char));
	      memcpy(shell.prompt,ob,shell.max_prompt*sizeof(char));
	      Cfree(ob);
	      shell.max_prompt+=1024;
	    }
	    shell.prompt[shell.clen]='\0';
	    shellWriteLine();
	  }
	} else if(shell.stdinbuf[i]==10 || shell.stdinbuf[i]==13) {
	  /* CR or LF */
	  shell.prompt[shell.clen]='\0';
	  
	  if(strlen(shell.prompt)>0){
	    if(shell.savehistp>=SHELL_SAVEHISTLEN){
	      for(j=1;j<SHELL_SAVEHISTLEN;j++)
		strcpy(shell.savehist[j-1].entry,shell.savehist[j].entry);
	      shell.savehistp--;
	    }
	    strncpy(shell.savehist[shell.savehistp++].entry,shell.prompt,256);
	    shell.scrollhistp=shell.savehistp;
	  }
	  
	  /* write the raw prompt out into the logfile */
	  if(shellWorkPrompt(shell.prompt,-1,NULL)!=SHELL_ERROR){
	    shellWriteLog(shell.prompt);
	  }
	  shell.prompt_flag=1;
	  memset(shell.prompt,0,256);
	  shell.cpos=0;
	  shell.clen=0;
	  shell.cstart=0;
	  
	  return SHELL_RET;
	} else if(shell.stdinbuf[i]==8 || shell.stdinbuf[i]==127) {
	  if(shell.cpos>0) {
	    /*
	      write(fileno(stdout),&bkspc,1);
	    */
	    shellEraseLine();
	    for(j=shell.cpos;j<shell.clen;j++)
	      shell.prompt[j-1]=shell.prompt[j];
	    shell.cpos--;
	    if(shell.cstart>0)
	      shell.cstart--;
	    shell.clen--;
	    shell.prompt[shell.clen]='\0';
	    shellWriteLine();
	    
	  }
	}
      }
    }
  }  
  
  return SHELL_OK;
}

/********************************************
 
  WorkScript
  ----------

  read line by line from a scriptfile, ignoring
  a new line '\n' when preceded by \
**********************************************/

static int shellWorkScript()
{
  char *ob;
  int lc;
  int pos;
  char c,pc;
  int ret,j;
  char *res;

  if(shell.current->buf_pos>shell.current->buf_size)
    return SHELL_EOF;
  if(!strcmp(&shell.current->buf[shell.current->buf_pos],"EOF"))
    return SHELL_EOF;

  lc=0;
  c=shell.current->buf[shell.current->buf_pos++];
  pc=0;
  while(-1) {
    if(c=='\n' && pc!='\\')
      break;
    shell.static_buf[lc++]=c;
    if(lc>=shell.static_buf_max) {
      ob=shell.static_buf;
      shell.static_buf=Ccalloc(shell.static_buf_max+1024,sizeof(char));
      memcpy(shell.static_buf,ob,shell.static_buf_max*sizeof(char));
      Cfree(ob);
      shell.static_buf_max+=1024;
    }
    if(shell.current->buf_pos>shell.current->buf_size)
      break;
    pc=c;
    c=shell.current->buf[shell.current->buf_pos++];
  }
  shell.static_buf[lc]='\0';
  pos=shell.current->buf_pos;

  if(strlen(shell.static_buf)>0) {
    if(shellGetVar("silent")==NULL && shellGetVar("quiet")==NULL)
      fprintf(stderr,"%s\n",shell.static_buf);
    /*
      write the raw prompt out into the logfile
      this does not make much sense, since the
      @ command is already recorded !

      fprintf(shell.logfile,"%s\n",buf);
    */
    /* add the script command to the global history */

    /*
      removed!
    if(shell.savehistp>=SHELL_SAVEHISTLEN){
      for(j=1;j<SHELL_SAVEHISTLEN;j++)
	strcpy(shell.savehist[j-1].entry,shell.savehist[j].entry);
      shell.savehistp--;
    }
    strncpy(shell.savehist[shell.savehistp].entry,shell.static_buf,255);
    shell.savehist[shell.savehistp++].entry[255]='\0';
    shell.scrollhistp=shell.savehistp;
    */

    if(shellWorkPrompt(shell.static_buf,pos,&res)==SHELL_ERROR) {
      /*
      shellOut("Error during parsing\n");
      */
    }

  }
  if(!strcmp(comGetReturn(),"0"))
    ret=SHELL_ERROR;
  else
    ret=SHELL_OK;

  //  Cfree(res);

  return ret;
}



static int shell_unknown_cmd(ClientData clientData,
		       Tcl_Interp *interp,
		       int objc, Tcl_Obj *CONST objv[])
{
  int argc,i;
  const char *argv[1024];

  argc=objc-1;
  for(i=0;i<argc;i++)
    argv[i]=Tcl_GetString(objv[i+1]);

  shellParse(argc,argv,0);
  //  fprintf(stderr,"%s\n",comGetReturn());

  //  fprintf(stderr,"caught unknown tcl command\n");
  Tcl_SetResult(interp,comGetReturn(),TCL_STATIC);
  return TCL_OK;
}

static void shell_tcl_init()
{
  int v1,v2;

  // tcl stuff
  shell.tcl_interp=Tcl_CreateInterp();

  Tcl_CreateObjCommand(shell.tcl_interp, "unknown", shell_unknown_cmd,
		       (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

  // unfortunate name clashes are hacked away for now
  Tcl_DeleteCommand(shell.tcl_interp, "load");

  Tcl_DStringInit(&shell.tcl_ds);

  Tcl_GetVersion(&v1,&v2,NULL,NULL);

  fprintf(stdout,"Using TCL interpreter v%d.%d (http://www.scriptics.com)\n",v1,v2);
}

/*
static char *shell_preset_var[2][]={
  {"CS",""},
  {"CP","{0,0,0}"}
};
*/

/*
  externally visible functions follow
*/

int shellInit(shell_callback func, const char *logfile)
{
  int i;
  char m1[512],m2[256];

  /*
    terminal functionality 
  */
  debmsg("shellInit: setting terminal properties");
  sigemptyset(&shell.sig);
  sigaddset(&shell.sig,SIGQUIT);
  sigaddset(&shell.sig,SIGTSTP);
  sigprocmask(SIG_BLOCK,&shell.sig,&shell.savesig);

//  sigaction(SIGINT, 
  
  if(tcgetattr(0,&shell.modes)<0){
    perror("tcgetattr");
    dinoExit(1);
  }

  strcpy(shell.logfilename,logfile);
  
  shell.savemodes=shell.modes;
  shell.modes.c_cc[VMIN]=0;
  shell.modes.c_cc[VTIME]=0;
  shell.modes.c_lflag &= ~ICANON;
  shell.modes.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE);
  shell.modes.c_cflag &= ~PARENB |  CS8;
  shell.modes.c_iflag &= ~ICRNL | ~INPCK |~ISTRIP | ~IXON;

  if(tcsetattr(0,TCSAFLUSH,&shell.modes)<0){
    perror("tcsetattr");
    dinoExit(1);
  }

  
  debmsg("shellInit: initializing tcl");
  shell_tcl_init();

  debmsg("shellInit: filling default values");
  shell.callback=func;
  shell.max_prompt=65536;
  shell.prompt=Ccalloc(shell.max_prompt,sizeof(char));
  memset(shell.prompt,0,shell.max_prompt);
  shell.cstart=0;
  shell.cpos=0;
  shell.clen=0;

  shell.stdinbuflen=65536;
  shell.stdinbuf=Ccalloc(shell.stdinbuflen,sizeof(char));

  for(i=0;i<SHELL_SAVEHISTLEN;i++)
    strcpy(shell.savehist[i].entry,"");
  shell.savehistp=0;
  shell.scrollhistp=0;

  shell.state=SHELL_INTERACTIVE;

  shell.charbuf.count=0;

  shell.top.state=SHELL_INTERACTIVE;
  shell.top.buf=NULL;
  shell.top.buf_size=0;
  shell.top.buf_pos=0;
  /*
  shell.top.argv=NULL;
  */
  shell.top.argc=0;
  shell.top.var_max=100;
  shell.top.var=Ccalloc(shell.top.var_max,sizeof(struct SHELL_VAR));
  memset(shell.top.var,0,shell.top.var_max*sizeof(struct SHELL_VAR));

  shell.top.parent=NULL;
  shell.top.child=NULL;

  shell.current=&shell.top;

  shell.instance_count=0;

  shell.estack_max=1024;
  shell.estack=Ccalloc(1024,sizeof(struct SHELL_EXPR_STACK));
  shell.estackp=-1;

  shell.top.pstack_max=1024;
  shell.top.pstack=Ccalloc(shell.top.pstack_max,sizeof(struct SHELL_POS_STACK));
  shell.top.pstackp=0;
  strcpy(shell.top.pstack[0].command,"");
  shell.top.pstack[0].pos=-1;
  shell.top.pstack[0].exec=1;

  shell.virgin_flag=1;
  strcpy(shell.init_cmd,"");

  shell.greedy=0;
  shell.prompt_flag=1;

  debmsg("shellInit: defining preset variables");

  sprintf(m1,"%g",M_PI);
  strcpy(m2,"PI");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=ALA,CYS,ASP,GLU,PHE,GLY,HIS,ILE,LYS,LEU,MET,ASN,PRO,GLN,ARG,SER,THR,VAL,TRP,TYR)");
  strcpy(m2,"protein");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=A,C,G,T)");
  strcpy(m2,"dna");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=A,C,G,U)");
  strcpy(m2,"rna");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=ALA,GLY,ILE,LEU,MET,PRO,VAL)");
  strcpy(m2,"aliphatic");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=PHE,TYR,TRP)");
  strcpy(m2,"aromatic");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=ALA,VAL,PHE,PRO,MET,ILE,LEU,TRP)");
  strcpy(m2,"hydrophobic");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=ARG,LYS)");
  strcpy(m2,"basic");
  shellSetVar(m2,m1);

  strcpy(m1,"((rname=LYS and aname=NZ) or (rname=ARG and aname=NH1,NH2))");
  strcpy(m2,"basic2");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=ASP,GLU)");
  strcpy(m2,"acidic");
  shellSetVar(m2,m1);

  strcpy(m1,"((rname=GLU and aname=OE1,OE2) or (rname=ASP and aname=OD1,OD2))");
  strcpy(m2,"acidic2");
  shellSetVar(m2,m1);

  strcpy(m1,"(rname=SER,THR,TYR,HIS,CYS,ASN,GLN)");
  strcpy(m2,"polar");
  shellSetVar(m2,m1);

  strcpy(m1,"((rname=SER and aname=OG) or (rname=THR and aname=OG1) or (rname=TYR and aname=OH) or (rname=HIS and aname=ND1,NE2) or (rname=CYS and aname=SG) or (rname=ASN and aname=OD1,ND1) or (rname=GLN and aname=OE1,NE1) or (rname=TRP and aname=NE1))");
  strcpy(m2,"polar2");
  shellSetVar(m2,m1);

  strcpy(m2,"CS");
  strcpy(m1,"{0,0,0}");
  shellSetVar(m2,m1);

  strcpy(m2,"CP");
  strcpy(m1,"{0,0,0}");
  shellSetVar(m2,m1);

  debmsg("shellInit: trapping SIGWINCH");
#ifdef LINUX
  signal(SIGWINCH,shellSigWinch);
#endif
#ifdef SGI
  sigset(SIGWINCH,shellSigWinch);
#endif
#ifdef DEC
  sigset(SIGWINCH,shellSigWinch);
#endif
#ifdef SUN
  sigset(SIGWINCH,shellSigWinch);
#endif

  ioctl(fileno(stdin),TIOCGWINSZ,&shell.ws);
  shell.maxlen=shell.ws.ws_col-strlen(shell_prompt)-1;

  /* open shell logfile */
  debmsg("shellInit: opening logfile");
  if(strlen(shell.logfilename)==0) {
    shell.logfile=fopen("/dev/null","w");
  } else {
    if((shell.logfile=fopen(shell.logfilename,"w"))==NULL) {
      fprintf(stderr,"error opening logfile\n");
      shell.logfile=fopen("/dev/null","w");
    }
  }  
  shell.aliasc=0;
  shell.aliasm=100;

  shell.alias=Ccalloc(shell.aliasm,sizeof(struct SHELL_ALIAS));
  memset(shell.alias,0,shell.aliasm*sizeof(struct SHELL_ALIAS));

  strcpy(shell.alias[shell.aliasc].name,"stereo");
  strcpy(shell.alias[shell.aliasc].value,"scene stereo");
  shell.aliasc++;

  strcpy(shell.alias[shell.aliasc].name,"mono");
  strcpy(shell.alias[shell.aliasc].value,"scene mono");
  shell.aliasc++;

  strcpy(shell.alias[shell.aliasc].name,"write");
  strcpy(shell.alias[shell.aliasc].value,"scene write");
  shell.aliasc++;

  shell.static_buf_max=4096;
  shell.static_buf=Ccalloc(shell.static_buf_max,sizeof(char));

  getcwd(shell.cwd,256);


  return SHELL_OK;
}

void shellPrompt(void)
{
  if(shell.prompt_flag) {
    shellOut("\n");
    shellOut(shell_prompt);
    shell.prompt_flag=0;
  }
}

int shellOutit(void)
{
  shellOut("\n");
  // write scene rtc
  comWriteModelview(shell.logfile);
  if(tcsetattr(0,TCSAFLUSH,&shell.savemodes)<0){
    perror("tcsetattr");
  }

  sigprocmask(SIG_SETMASK,&shell.savesig,(sigset_t*)0);

  Cfree(shell.static_buf);

  return SHELL_OK;
}

void shellOut(const char *string)
{
  if(shellGetVar("silent")==NULL) {
    write(fileno(stdout),string,strlen(string));
    shell.prompt_flag=1;
  }
}


/*********************************************************

  shellWork()
  -----------

  Is periodically called from the timer routine and is
  responsible for providing the next command input, 
  either from the command line (interactive), or 
  from the next line in the script

***********************************************************/

int shellWork()
{
  int ret;
  struct SHELL_INSTANCE *si;
  char c;

  if(shell.virgin_flag) {
    if(strlen(shell.init_cmd)>0) {
      shellOut(shell.init_cmd);
      comRawCommand(shell.init_cmd);
    }
    shell.virgin_flag=0;
  }

  if(shell.current->state==SHELL_INTERACTIVE) {
    ret=shellWorkInteractive();
  }
  if(shell.current->state==SHELL_SCRIPT) {
#ifdef EXPO
    while(shell.greedy) {
#endif
    ret=shellWorkScript();
    if(ret==SHELL_ERROR || ret==SHELL_EOF || 
       shell.current->state==SHELL_BREAK) {
      shell.current->state=shell.current->save_state;
      ret=SHELL_RET;
      shell.instance_count--;
      shell.state=shell.current->parent->state;
      Cfree(shell.current->argv[0]);
      Cfree(shell.current->buf);
      si=shell.current->parent;
      Cfree(shell.current);
      shell.current=si;
#ifdef EXPO
      if(shell.current->state==SHELL_INTERACTIVE) 
	shell.greedy=0;
#endif
    }
#ifdef EXPO
    }
#endif
  }
  if(shell.current->state==SHELL_PAUSE) {
    if(read(fileno(stdin),&c,1)>0) {
      if(c==27) {
	// is like break
	shell.current->state=shell.current->save_state;
	shell.instance_count--;
	shell.state=shell.current->parent->state;
	Cfree(shell.current->argv[0]);
	Cfree(shell.current->buf);
	si=shell.current->parent;
	Cfree(shell.current);
	shell.current=si;
      } else {
	shell.current->state=shell.current->save_state;
      }
    }
    ret=SHELL_RET;

  }
  if(shell.current->state==SHELL_INTERACTIVE && ret==SHELL_RET) {
    shell.prompt_flag=1;
    //    shellPrompt();
  }

  if(shell.prompt_flag)
    shellPrompt();

  return 0;
}





/******************************************************

  shellWorkPrompt
  ---------------

  process raw prompts

*********************************************************/

int shellWorkPrompt(const char *raw_prompt2, int pos, char **result)
{
  const char *s;

  s=raw_prompt2;
  Tcl_DStringSetLength(&shell.tcl_ds,0);
  Tcl_EvalEx(shell.tcl_interp,
	     Tcl_ExternalToUtfDString(NULL,s,clStrlen(s),&shell.tcl_ds),
	     -1,TCL_EVAL_GLOBAL);

  return SHELL_OK;
}


static char shell_static_var_val[2048];

int shellSetVar(const char *cvar, const char *cval)
{
  static char var[256],*val;

  clStrncpy(var,cvar,255);

  if(clStrlen(cval)>2047) {
    val=Cmalloc(clStrlen(cval));
    clStrcpy(val,cval);
    Tcl_SetVar(shell.tcl_interp,var,val,TCL_GLOBAL_ONLY);
    Cfree(val);
  } else {
    val=shell_static_var_val;
    clStrcpy(val,cval);
    Tcl_SetVar(shell.tcl_interp,var,val,TCL_GLOBAL_ONLY);
  }

  return 0;
}


int shellSetInitCommand(char *c)
{
  strncpy(shell.init_cmd,c,1024);
  return 0;
}

      
void shellWriteLog(const char *s)
{
  /* write string into logfile */
  fprintf(shell.logfile,"%s\n",s);
  fflush(shell.logfile);
}

void shellAddChar(char c) 
{
  if(shell.charbuf.count<1024)
    shell.charbuf.buf[shell.charbuf.count++]=c;
}
