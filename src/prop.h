#ifndef PROP_H
#define PROP_H

#define PROP_MAX_VALUES 32

typedef struct PROP_TABLE {
  char s[PROP_MAX_VALUES][64];
  int n[PROP_MAX_VALUES];
}PropxTable;

typedef struct PROP {
  float v[PROP_MAX_VALUES];
}Propx;

int propReset(PropxTable *);

int propAddName(PropxTable *, char *);

int propGetIndex(PropxTable *, char *);
float propGetValue(Propx *, int n);

#endif
