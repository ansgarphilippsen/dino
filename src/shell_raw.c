#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "shell_raw.h"
#include "shell_command.h"
#include "gui_terminal.h"
#include "Cmalloc.h"
#include "cl.h"
#include "com.h"

#ifdef USE_TCL
#include <tcl.h>
#endif

#ifndef USE_TCL
static int parse(const char *raw_prompt2);
static int add_prompt(char **prompt,int *prompt_max, int *prompt_len,char c);
#define is_valid_var_char(c) (isalnum(c) || c=='_' || c=='-')
#endif
static void add_history(const char *s);
#ifdef USE_TCL
static int shell_unknown_cmd(ClientData clientData,
			     Tcl_Interp *interp,
			     int objc, Tcl_Obj *CONST objv[]);

static void trace_proc(ClientData clientData,
		       Tcl_Interp *interp,
		       int level,
		       char *command,
		       Tcl_CmdProc *cmdProc,
		       ClientData cmdClientData,
		       int argc,
		       char *argv[]);
#endif

#define SHELL_HISTORY_MAX_ENTRIES 1024
#define SHELL_HISTORY_BUFFER_SIZE 102400

static struct SHELL_HISTORY {
  char buffer[SHELL_HISTORY_BUFFER_SIZE+1];
  char *entry[SHELL_HISTORY_MAX_ENTRIES];
  char *next;
  int entry_count, entry_max;

} shell_history;

static int current_history_entry=0;

static FILE *logfile;

#define VAR_MAX_NAME_SIZE 64
#define VAR_MAX_VALUE_SIZE 256
#define VAR_MAX_ENTRIES 512

static struct SHELL_VAR {
  struct SHELL_VAR_ENTRY {
    char name[VAR_MAX_NAME_SIZE];
    char value[VAR_MAX_VALUE_SIZE];
    int level;
  } *entry;
  int count, max;
} shell_var;

#ifdef USE_TCL
  Tcl_DString tcl_ds;
  Tcl_Interp *tcl_interp;
#endif

extern int interrupt_flag; // in shell_command.c

static int script_flag;

/*
  Initialization
*/


int shellInterpInit(FILE *lf)
{
#ifdef USE_TCL
  int v1,v2;
  char message[256];
#endif
  shell_history.entry_max=SHELL_HISTORY_MAX_ENTRIES;
  shell_history.entry_count=0;
  shell_history.next=shell_history.buffer+0;

#ifndef USE_TCL
  shell_var.count=0;
  shell_var.max=VAR_MAX_ENTRIES;
  shell_var.entry=Crecalloc(NULL,shell_var.max,sizeof(struct SHELL_VAR_ENTRY));
#endif

  logfile=lf;

#ifdef USE_TCL
  // new interpreter instance
  tcl_interp=Tcl_CreateInterp();

  // install trace
  Tcl_CreateTrace(tcl_interp,999,trace_proc,0);

  // catch unknown commands
  Tcl_CreateObjCommand(tcl_interp, "unknown", shell_unknown_cmd,
		       (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

  // unfortunate name clashes are hacked away for now
  Tcl_DeleteCommand(tcl_interp, "load");

  Tcl_DStringInit(&tcl_ds);

  Tcl_GetVersion(&v1,&v2,NULL,NULL);
  
  sprintf(message,"Using TCL interpreter v%d.%d (http://www.scriptics.com)\n",v1,v2);
  shellOut(message);
  script_flag=0;
#endif

  return 0;
}

#ifdef USE_TCL

static int shell_unknown_cmd(ClientData clientData,
			     Tcl_Interp *interp,
			     int objc, Tcl_Obj *CONST objv[])
{
  int argc,i;
  const char *argv[1024];
  const char *aliasvalue;

  // TODO aliases must be handled here!

  if(interrupt_flag) {
    interrupt_flag=0;
    Tcl_SetResult(interp,"",TCL_STATIC);
    shellOut("\nInterrupt\n");
    return TCL_ERROR;
  }

  argc=objc-1;
  for(i=0;i<argc;i++)
    argv[i]=Tcl_GetString(objv[i+1]);
  
  shellParseCommand(argv,argc);
  
  Tcl_SetResult(interp,comGetReturn(),TCL_STATIC);
  
  return TCL_OK;
}

static void trace_proc(ClientData clientData,
		      Tcl_Interp *interp,
		      int level,
		      char *command,
		      Tcl_CmdProc *cmdProc,
		      ClientData cmdClientData,
		      int argc,
		      char *argv[])
{
  if(clStrcmp(argv[0],"unknown")) {
    cmiCheckRedraw();
  }
}

int shellCallScript(const char *filename, int argc, const char **argv)
{
  struct stat st;
  char message[256];

  if(stat(filename,&st)<0) {
    sprintf(message,"error reading %s\n",filename);
    return -1;
  }

  // TODO command line args
  script_flag++;
  if(Tcl_EvalFile(tcl_interp,filename)!=TCL_OK) {
    shellOut(Tcl_GetStringResult(tcl_interp));
    shellOut("\n");
    comReturn("");
  } else {
    comReturn(Tcl_GetStringResult(tcl_interp));
  }
  script_flag--;
}


#endif

/*
  parse raw string, pass result 
  to shell_command
*/


int shellParseRaw(const char *s, int hf)
{
#ifdef USE_TCL
  static char *parse_buf=NULL;
  static int parse_buf_len=0;
  Tcl_Parse tparse;
  int i,ret;
#endif
  int scr_f;
  
  comReturn("");

#ifdef USE_TCL
  while(clStrlen(s)>parse_buf_len) {
    parse_buf=Crecalloc(parse_buf,parse_buf_len+1024,sizeof(char));
    parse_buf_len+=1024;
  };
  clStrcpy(parse_buf,s);
  i=0;
  // pre-scan buf to handle dino-tcl incompatibilities
  while(i<clStrlen(s)-1) {
    // morph comments
    if(parse_buf[i]=='/' && parse_buf[i+1]=='/') {
      parse_buf[i]='#';
      parse_buf[i+1]='#';
      i+=2;
    } else {
      i++;
    }
  }
  
  Tcl_DStringSetLength(&tcl_ds,0);
  // pass to tcl interpreter
  // any non-tcl stuff will end up in shell_unknown_command()

  if(Tcl_EvalEx(tcl_interp,
	     Tcl_ExternalToUtfDString(NULL,
				      parse_buf,
				      clStrlen(parse_buf),
				      &tcl_ds),
		-1,TCL_EVAL_GLOBAL)!=TCL_OK) {
    shellOut(Tcl_GetStringResult(tcl_interp));
    shellOut("\n");
    comReturn("");
  } else {
    comReturn(Tcl_GetStringResult(tcl_interp));
  }

#else
  scr_f=shellIsScript();
  if(parse(s)==SHELL_OK) {
    if(!scr_f)
      shellLog(s);
  }
#endif

  if(hf)
    add_history(s);
  return 0;
}

void shellLog(const char *s)
{
  if(logfile && clStrlen(s)>0) {
    fprintf(logfile,"%s",s);
    if(s[clStrlen(s)-1]!='\n')
      fprintf(logfile,"\n");
    fflush(logfile);
  }
}

/*
  TODO: use local scope for variables
*/
int shellSetVar(const char *name2, const char *value2, int level)
{
  int d1,d2;
  double res[16];
  int i;
  char name[VAR_MAX_NAME_SIZE];
  char value[VAR_MAX_VALUE_SIZE];
  
  // special variables
  if(clStrcmp(name2,"CP")) {
    if(matExtractMatrix(value2,&d1,&d2,res)==0) {
      if(d1==3 && d2==1) {
	comSetCP(res[0],res[1],res[2]);
      } else {
	comMessage("value for CP must be a vector\n");
	return -1;
      }
    } else {
      comMessage("value for CP must be a vector\n");
      return -1;
    }
  }

#ifdef USE_TCL
  Tcl_SetVar(tcl_interp,name2,value2,TCL_GLOBAL_ONLY);
#else
  
  clStrncpy(name,name2,VAR_MAX_NAME_SIZE-1);
  clStrncpy(value,value2,VAR_MAX_VALUE_SIZE-1);

  for(i=0;i<shell_var.count;i++) {
    if(clStrcmp(shell_var.entry[i].name,name))
      break;
  }

  if(i<shell_var.max) {
    clStrcpy(shell_var.entry[i].name,name);
    clStrcpy(shell_var.entry[i].value,value);

    if(i==shell_var.count) shell_var.count++;
  }


#endif

  return 0;
}

const char *shellGetVar(const char *name, int level)
{
#ifdef USE_TCL
  return Tcl_GetVar(tcl_interp,name,TCL_GLOBAL_ONLY);
#else
  int i;

  for(i=0;i<shell_var.count;i++) {
    if(clStrcmp(shell_var.entry[i].name,name)) {
      return shell_var.entry[i].value;
    }
  }
  return NULL;
#endif
}

int shellUnsetVar(const char *name, int level)
{
#ifdef USE_TCL
  Tcl_UnsetVar(tcl_interp,name,TCL_GLOBAL_ONLY);
#else
  int i,j;

  for(i=0;i<shell_var.count;i++) {
    if(clStrcmp(shell_var.entry[i].name,name)) {
      break;
    }
  }

  if(i<shell_var.count) {
    for(j=i+1;j<shell_var.count;j++) {
      clStrcpy(shell_var.entry[j-1].name,shell_var.entry[j].name);
      clStrcpy(shell_var.entry[j-1].value,shell_var.entry[j].value);
    }
    shell_var.count--;
  }

  return 0;
#endif
}

void shellListVars(const char *rg)
{
#ifndef USE_TCL
  int i;
  // TODO use regexp for var listing
  for(i=0;i<shell_var.count;i++) {
    shellOut(shell_var.entry[i].name);
    shellOut(" ");
    shellOut(shell_var.entry[i].value);
    shellOut("\n");
  }
#endif
}

/*
  calls guitWrite for now
*/

void shellOut(const char *s)
{
  guitWrite(s);
}


const char *shellGetHistory(int dir)
{
  char *empty="\0";

  int c=current_history_entry-dir;
  int e=shell_history.entry_count-c;

  if(c>0 && e>=0) {
    current_history_entry=c;
    return shell_history.entry[e];
  } else if(c==0) {
    return empty;
  } else {
    return NULL;
  }
}

static void add_history(const char *s)
{
  int i,l;
  int sl=clStrlen(s)+1;
  
  while(shell_history.entry_count==shell_history.entry_max ||
	shell_history.next+sl >= shell_history.buffer+SHELL_HISTORY_BUFFER_SIZE) {
    l=clStrlen(shell_history.entry[0])+1;
    memmove(shell_history.entry[0],
	    shell_history.entry[1], 
	    SHELL_HISTORY_BUFFER_SIZE-l);
    for(i=0;i<shell_history.entry_count-1;i++) {
      shell_history.entry[i]=shell_history.entry[i+1]-l;
    }
    shell_history.entry_count--;
    shell_history.next-=l;
  }
  shell_history.entry[shell_history.entry_count++]=shell_history.next;
  clStrcpy(shell_history.next,s);
  shell_history.next+=clStrlen(s)+1;
  current_history_entry=0;
}

/*
  major parsing routine, called recursively
  if a subprompt is encountered
  calls shellParseCommand in shell_command.c
  with a list of words

  this is one of the early dino functions
  and really a mess! the myriad of local 
  variables are really _not_ suited for
  a recursive function

  for tcl this is replaced with the tcl parser
*/
#ifndef USE_TCL

enum {SHELL_NONE=0,
      SHELL_PRIME,  /* '' */
      SHELL_WBRACE, /* {} */
      SHELL_SBRACE, /* [] */
      SHELL_DPRIME  /* "" */
};

static int parse(const char *raw_prompt2)
{
  int i,j,c;
  char *prompt=NULL;
  int prompt_max,prompt_len;
  char * sub_prompt=NULL;
  int sub_prompt_max,sub_prompt_len;
  char * raw_prompt;
  int raw_prompt_len,raw_prompt_max;
  const char *word[2048]; /* 2048 is the maximal number of words */
  int wordi[2048];
  int wc;
  char var[256];
  int var_c;

  char message[256];
  int varcheck=0;
  int error_code=SHELL_OK;
  static char alias[1024];
  static const char *aliasvalue;
  int ac1,ac2;
  int nest_history[1024];
  int nest_bc[1024];
  int nest_level;
  int add_char_flag;
  int sub_prompt_flag;
  int exit_flag;
  char add_char;
  const char *res;

  /* allocate space for copy of raw_prompt */
  raw_prompt_max=clStrlen(raw_prompt2)+1024;
  raw_prompt=Crecalloc(NULL,raw_prompt_max,sizeof(char));
  raw_prompt_len=0;

  clStrcpy(raw_prompt,raw_prompt2);

  /* allocate space for prompt */
  prompt_max=clStrlen(raw_prompt2)+1024;
  prompt=Crecalloc(NULL,prompt_max,sizeof(char));
  prompt_len=0;


  /* main loop over range of raw_prompt */

  c=0;
  prompt[prompt_len]='\0';
  wc=0;
  wordi[wc++]=0;

  /* check for aliases in first word */
  
  // eliminate initial whitespaces
  while(isspace(raw_prompt[c]) && c<clStrlen(raw_prompt)) c++;
  ac1=c;
  ac2=0;
  // get first word
  while(!isspace(raw_prompt[ac1]) && ac1<clStrlen(raw_prompt)){
    alias[ac2++]=raw_prompt[ac1++];
  }
  alias[ac2]='\0';
  if((aliasvalue=shellResolveAlias(alias))!=NULL) {
    // replace old raw_prompt
    clStrcpy(raw_prompt,aliasvalue);
    clStrcat(raw_prompt," ");
    // reset length
    raw_prompt_len=clStrlen(raw_prompt);
    // add rest of prompt
    for(j=ac2;j<clStrlen(raw_prompt2);j++)
      add_prompt(&raw_prompt,&raw_prompt_max,&raw_prompt_len,
		 raw_prompt2[j]);
    raw_prompt[raw_prompt_len]='\0';
  }

  // start main loop

  nest_history[nest_level=0]=SHELL_NONE;

  do {
    while(isspace(raw_prompt[c]) && c<clStrlen(raw_prompt))
      c++;
    sub_prompt_flag=0;
    exit_flag=0;
    while(c<clStrlen(raw_prompt)){
      add_char_flag=1;

      add_char=raw_prompt[c++];
      if(isspace(add_char)) {
	if(nest_history[nest_level]==SHELL_NONE) {
	  exit_flag=1;
	  add_char_flag=0;
	} else {
	  add_char_flag=1;
	}
      } else if(add_char=='"') {
	if(nest_history[nest_level]>SHELL_DPRIME) {
	  add_char_flag=1;
	} else {
	  if(nest_history[nest_level]==SHELL_DPRIME) {
	    nest_level--;
	  } else {
	    nest_history[++nest_level]=SHELL_DPRIME;
	  }
	  add_char_flag=0;
	}      
      } else if(add_char=='[') {
	if(nest_history[nest_level]>SHELL_SBRACE) {
	  add_char_flag=1;
	} else if(nest_history[nest_level]==SHELL_SBRACE) {
	  add_char_flag=1;
	  nest_bc[nest_level]++;
	} else {
	  
	  nest_history[++nest_level]=SHELL_SBRACE;

	  nest_bc[nest_level]=1;

	  /* prepare the subprompt */
	  sub_prompt_max=clStrlen(raw_prompt)+1024;
	  sub_prompt=Crecalloc(NULL,sub_prompt_max,sizeof(char));
	  sub_prompt_len=0;
	  sub_prompt_flag=1;

	  add_char_flag=0;
	}
      } else if(add_char==']') {
	if(nest_history[nest_level]>SHELL_SBRACE) {
	  add_char_flag=1;
	} else if(nest_history[nest_level]==SHELL_SBRACE) {
	  nest_bc[nest_level]--;
	  if(nest_bc[nest_level]>0) {
	    add_char_flag=1;
	  } else {
	    sub_prompt[sub_prompt_len]='\0';
	    /* send sub_prompt to evaluation */
	    if(clStrlen(sub_prompt)>0) {
	      if(parse(sub_prompt)==SHELL_ERROR) {
		error_code=SHELL_ERROR;
	      } else {
		res=comGetReturn();
		for(i=0;i<clStrlen(res);i++) {
		  add_prompt(&prompt,&prompt_max,&prompt_len,res[i]);
		}
	      }
	    }
	    nest_level--;
	    Cfree(sub_prompt);
	    sub_prompt_flag=0;
	    add_char_flag=0;
	  }
	} else {
	  shellOut("error during prompt parsing: superfluous ]\n");
	  error_code=SHELL_ERROR;
	}
      } else if(add_char=='{') {
	if(nest_history[nest_level]>SHELL_WBRACE) {
	  add_char_flag=1;
	} else if(nest_history[nest_level]==SHELL_WBRACE) {
	  add_char_flag=1;
	  nest_bc[nest_level]++;
	} else {
	  add_char_flag=1;
	  nest_history[++nest_level]=SHELL_WBRACE;
	  nest_bc[nest_level]=1;
	}
      } else if(add_char=='}') {
	if(nest_history[nest_level]>SHELL_WBRACE) {
	  add_char_flag=1;
	} else if(nest_history[nest_level]==SHELL_WBRACE) {
	  nest_bc[nest_level]--;
	  if(nest_bc[nest_level]>0) {
	    add_char_flag=1;
	  } else {
	    nest_level--;
	    add_char_flag=1;
	  }
	} else {
	  shellOut("error during prompt parsing: superfluous }\n");
	  error_code=SHELL_ERROR;
	}
      } else if(add_char=='$') {
	if(nest_history[nest_level]>=SHELL_SBRACE) {
	  add_char_flag=1;
	} else {
	  add_char_flag=0;
	  // var detected 
	  if((++varcheck)>1013) {
	    shellOut("Variable nesting limit reached\n");
	    error_code=SHELL_ERROR;
	  } else {
	    // get variable name
	    var_c=0;
	    while(is_valid_var_char(raw_prompt[c]) && c<clStrlen(raw_prompt)) {
	      var[var_c++]=raw_prompt[c++];
	    }
	    var[var_c]='\0';
	    // TODO: get var in current level
	    res=shellGetVar(var,0);
	    if(res!=NULL) {
	      // var found
	      for(j=0;j<clStrlen(res);j++) {
		add_prompt(&prompt,&prompt_max,&prompt_len,res[j]);
	      }
	    } else {
	      sprintf(message,"error: unkown variable %s\n",var);
	      shellOut(message);
	      error_code=SHELL_ERROR;
	    }
	  }
	}
      } else if(add_char=='\\') {
	/* some sort of protection */
	if(c>=clStrlen(raw_prompt)) {
	  add_char_flag=0;
	} else if(raw_prompt[c]=='\n') {
	  if(/* TODO current->state==SHELL_SCRIPT*/ 1 ) {
	    c++;
	    add_char_flag=0;
	  } else {
	    add_char_flag=1;
	  }
	} else if(raw_prompt[c]=='\\') {
	  add_char='\\';
	  c++;
	  add_char_flag=1;
	} else {
	  add_char_flag=0;
	}
      } else if(add_char==';') {
	if(nest_history[nest_level]>SHELL_NONE) {
	  add_char_flag=1;
	} else {
	  /* command line split */
	  if(prompt_len>wordi[wc-1]){
	    add_prompt(&prompt,&prompt_max,&prompt_len,'\0');
	    wordi[wc++]=prompt_len;
	  }
	  
	  for(i=0;i<wc;i++) {
	    word[i]=&prompt[wordi[i]];
	  }
	  wc--;
	  if(shellParseCommand(word,wc)==SHELL_ERROR)
	    error_code=SHELL_ERROR;
	  
	  prompt_len=0;
	  wc=0;
	  wordi[wc++]=prompt_len;
	  add_char_flag=0;
	}
      } else if(add_char=='/') {
	if(nest_history[nest_level]<SHELL_DPRIME) {
	  if(c<clStrlen(raw_prompt)) {
	    if(raw_prompt[c]=='/') {
	      /* comment */
	      exit_flag=1;
	      add_char_flag=0;
	      c=clStrlen(raw_prompt)+1;
	      break;
	    } else {
	      add_char_flag=1;
	    }
	  } else {
	    add_char_flag=1;
	  }
	} else {
	  add_char_flag=1;
	}
      } else {
	add_char_flag=1;
      }

      if(add_char_flag) {
	if(sub_prompt_flag) {
	  add_prompt(&sub_prompt,&sub_prompt_max,&sub_prompt_len,add_char);
	} else {
	  add_prompt(&prompt,&prompt_max,&prompt_len,add_char);
	}
      }
      if(exit_flag || error_code!=SHELL_OK)
	break;
    }
    if(error_code==SHELL_ERROR)
      break;
    
    /* end of word */
    if(prompt_len>wordi[wc-1]){
      add_prompt(&prompt,&prompt_max,&prompt_len,0);
      wordi[wc++]=prompt_len;
    }
  } while(c<clStrlen(raw_prompt));

  for(i=0;i<wc;i++) {
    word[i]=&prompt[wordi[i]];
  }
  
  wc--;
  
  if(error_code!=SHELL_ERROR)
    if(shellParseCommand(word,wc)==SHELL_ERROR)
      error_code=SHELL_ERROR;

  // return value is in comReturn
  
  Cfree(raw_prompt);
  Cfree(prompt);

  return error_code;
}

static int add_prompt(char **prompt,int *prompt_max, int *prompt_len,char c)
{
  (*prompt)[(*prompt_len)++]=c;
  if((*prompt_len)>=(*prompt_max)) {
    (*prompt)=Crecalloc((*prompt),(*prompt_max)+1024,sizeof(char));
    (*prompt_max)+=1024;
  }
  return 0;
}

#endif
