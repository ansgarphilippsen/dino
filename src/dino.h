#ifndef DINO_H
#define DINO_H

typedef char* string;

#define debmsg(m) if(debug_mode) fprintf(stderr,"[DEBUG] %s\n",m)

void dinoExit(int);

#endif
