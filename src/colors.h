/*
  color definition table and lookup routines
*/

#ifndef _COLORS_H
#define _COLORS_H

struct COLOR_ENTRY {
  char *name;
  unsigned char r,g,b;
};

const struct COLOR_ENTRY * colorGetTab(void);
int colorResolveF(const char *name, float *r, float *g, float *b); 
int colorResolveB(const char *name, unsigned char *r, unsigned char *g, unsigned char *b);

#endif
