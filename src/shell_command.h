#ifndef _SHELL_COMMAND_H
#define _SHELL_COMMAND_H

int shellInit(void);
int shellParseCommand(const char **wl, int wc);
int shellIsScript(void);
const char * shellResolveAlias(const char *reg);


#endif
