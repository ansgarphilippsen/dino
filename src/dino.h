#ifndef DINO_H
#define DINO_H

typedef char* string;

#define debmsg(m) if(debug_mode) fprintf(stderr,"\n[DEBUG] %s",m)

#define VERSION "0.8.5-2"

void dinoExit(int);

#endif
