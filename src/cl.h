#ifndef _CL_H
#define _CL_H

typedef struct CL_PARAM {
  char *p;
  char **wl;
  int wc;
}clParam;


typedef struct CL_ORG {
  clParam *param;
  int param_count,param_max;
  char message[256];
}clOrg;

int clNew(clOrg *org, int argc, char **argv);

int clDelete(clOrg *org);
int clStrcmp(const char *s1, const char *s2);
int clStrncmp(const char *s1, const char *s2, int m);
int clStrcpy(char *d, const char *s);
int clStrncpy(char *d, const char *s, int m);
int clStrlen(const char *s);
int clStrncat(char *d, const char *s, int m);
const char *clStrchr(const char *s, int c);
const char *clStrrchr(const char *s, int c);

#endif
