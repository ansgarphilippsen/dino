#ifndef _DINO_H
#define _DINO_H

typedef char* string;

#define debmsg(m) if(debug_mode) fprintf(stderr,"\n[DEBUG] %s",m)

#define VERSION "0.8.5-4"

int dinoMain(int argc, char **argv);
void dinoExit(int);

#endif
