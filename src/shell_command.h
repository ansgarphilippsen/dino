#ifndef _SHELL_COMMAND_H
#define _SHELL_COMMAND_H

int shellInit(const char *logfile);
int shellParseCommand(const char **wl, int wc);
int shellIsScript(void);
const char * shellResolveAlias(const char *reg);
void shellInterrupt(void);
int shellSystem(const char *c);


#endif
