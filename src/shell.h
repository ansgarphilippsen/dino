#ifndef _SHELL_H
#define _SHELL_H

/*****************************************

The shell will be initialized with a
pointer to a function that is called after
all internal workings on the prompt have
been performed (e.g. variable substitution,
aliases, etc).

(int)(int word_count, char **word_list)

*******************************************/

typedef int (*shell_callback)(int, const char **);

int shellInit(shell_callback cb, const char *);
int shellSetInitCommand(char *c);
int shellOutit(void);

void shellOut(const char *);
void shellWriteLog(const char *s);

void shellPrompt(void);
void shellAddChar(char c);
int shellWork(void);
int shellWorkPrompt(const char *prompt, int pos, char **result);

int shellSetVar(int wc, const char **wl);



#endif




