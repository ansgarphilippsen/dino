#ifndef HELP_H
#define HELP_H

struct HELP_ENTRY {
  char *keyw;
  char *brief;
  char *detail;
};

int help(struct HELP_ENTRY *, char *pre, char *keyw);

#endif

