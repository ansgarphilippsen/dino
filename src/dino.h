#ifndef _DINO_H
#define _DINO_H

typedef char* string;

#ifdef NEW_SHELL
#define debmsg(m) if(debug_mode) fprintf(stderr,"[DEBUG] %s\n",m)
#else
#define debmsg(m) if(debug_mode) fprintf(stderr,"\n[DEBUG] %s",m)
#endif

#define VERSION "0.8.5-4"

#ifdef __cplusplus
extern "C" {
#endif

int dinoMain(int argc, char **argv);
void dinoExit(int);

#ifdef __cplusplus
}
#endif

#endif
