#ifndef _DINO_H
#define _DINO_H

typedef char* string;

#define debmsg(m) if(debug_mode) fprintf(stderr,"[DEBUG] %s\n",m)

#define VERSION "0.8.6"

#define DINO_FLAG_NOGFX     1<<0
#define DINO_FLAG_NOSTENCIL 1<<1
#define DINO_FLAG_STEREO    1<<2
#define DINO_FLAG_NOSTEREO  1<<3
#define DINO_FLAG_NOOBJMENU 1<<4

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
