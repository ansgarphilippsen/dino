#ifndef _SHELL_RAW_H
#define _SHELL_RAW_H

#define SHELL_ERROR -1
#define SHELL_OK 0

int shellInterpInit(void);
int shellParseRaw(const char *s, int hf);

#ifdef USE_TCL
int shellCallScript(const char *f, int argc, const char **argv);
#endif

int shellSetVar(const char *name, const char *value);
const char *shellGetVar(const char *name);
int shellUnsetVar(const char *name);
void shellListVars(const char *rg);

void shellTimeProc(void);

void shellOut(const char *s);

const char *shellGetHistory(int dir);

#endif
