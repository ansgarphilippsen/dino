#ifndef _OM_SDL
#define _OM_SDL

#include <SDL/SDL.h>

int omInit(void);

int omAddDB(const char *name);
int omDelDB(const char *name);
int omAddObj(const char *db, const char *name);
int omDelObj(const char *db, const char *name);
int omHideObj(const char *db, const char *name);
int omShowObj(const char *db, const char *name);


#endif
