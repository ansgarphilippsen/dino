#ifndef PROP_H
#define PROP_H

#define PROP_MAX_VALUES 32

typedef struct PROP_TABLE {
  char s[PROP_MAX_VALUES][64];
  int n[PROP_MAX_VALUES];
}PropTable;

typedef struct PROP {
  float v[PROP_MAX_VALUES];
}Prop;

int propReset(PropTable *);

int propAddName(PropTable *, char *);

int propGetIndex(PropTable *, char *);
float propGetValue(Prop *, int n);

#endif
