#include <stdio.h>
#include <termios.h>
#include <signal.h>
#ifdef SGI
#endif
#ifdef LINUX
#include <sys/ioctl.h>
#endif
#ifdef DEC
#include <sys/ioctl.h>
#endif


#ifndef SHELL_H
#define SHELL_H

/*****************************************

The shell will be initialized with a
pointer to a function that is called after
all internal workings on the prompt have
been performed (e.g. variable substitution,
aliases, etc).

(int)(int word_count, char **word_list)

*******************************************/


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
  int (*callback)(int, char **);

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
};

int shellInit(int (*)(int,char **), const char *);
int shellOutit(void);
void shellOut(const char *);
void shellPrompt(void);
int shellWork(void);
int shellWorkInteractive(void);
void shellEraseLine(void);
void shellRedrawLine(void);
void shellWriteLine(void);
int shellWorkScript(void);
int shellWorkPrompt(const char *prompt, int pos, char **result);
int shellAddPrompt(char **prompt,int *prompt_max, int *prompt_len,char c);
int shellParse(int wc, char **wl,int pos);
int shellSetVar(int wc, char **wl);
const char *shellGetVar(const char *n);
int shellOpr(char *op);
void shellSigWinch(int sig);
int shellEcho(int wc, char **wl);

int shellSetInitCommand(char *c);

#endif




