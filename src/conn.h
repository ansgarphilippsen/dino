#ifndef CONN_H
#define CONN_H

typedef char conName[8];
typedef int conConn[2];

struct CONN_ENTRY {
  char name[256];
  char abbrev[16];
  conName *name_list;
  int name_count;
  conConn *conn_list;
  int conn_count;
  int *flag_list;
  int flag_count;
};

struct CONN_DEF_ENTRY {
  char name[256];
  char abbrev[16],single;
  char name_list[128][8];
  int name_count;
  int conn_list[128][2];
  int conn_count;
  int flag_list[128];
  int flag_count;
};

struct CONNECTIVITY {
  struct CONN_ENTRY *entry;
  int entry_count;
};

int conInit(void);
int conLoad(struct CONNECTIVITY *, char *filename);
int conAssembleEntry(struct CONN_ENTRY *entry, int *wc, char **wl, int context);
struct CONN_ENTRY * conGetEntry(char *abbrev);
int conGetAtomID(struct CONN_ENTRY *, char *aname);
int conIsConnected(struct CONN_ENTRY *,int , int);
int conGetNextID(struct CONN_ENTRY *);
int conGetPrevID(struct CONN_ENTRY *);
int conSplitLine(char *expr, char sep, int *wc, char ***wl);

#endif
