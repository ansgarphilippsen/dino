#ifndef _DINO_H
#define _DINO_H

typedef char* string;

#ifdef NEW_SHELL
#define debmsg(m) if(debug_mode) fprintf(stderr,"[DEBUG] %s\n",m)
#else
#define debmsg(m) if(debug_mode) fprintf(stderr,"\n[DEBUG] %s",m)
#endif

#define VERSION "0.8.5-snapshot"

#define DINO_FLAG_NOGFX     1<<0
#define DINO_FLAG_NOSTENCIL 1<<1
#define DINO_FLAG_NOSTEREO  1<<2

#ifdef __cplusplus
extern "C" {
#endif

int dinoParseArgs(int argc, char **argv);
int dinoMain(int argc, char **argv);
void dinoExit(int);

#ifdef __cplusplus
}
#endif

#endif
