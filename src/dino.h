#ifndef DINO_H
#define DINO_H

typedef char* string;

#define debmsg(m) if(debug_mode) fprintf(stderr,"\n[DEBUG] %s",m)

#define VERSION "0.8.4-12"

void dinoExit(int);

#endif
