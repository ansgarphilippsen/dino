#ifndef REX_H
#define REX_H

typedef struct LEX_POE {
  char property[64];
  char op[16];
  char value[2048];
  int oflag;
  float dist,dist2,v[3];
}lexPOE;

typedef struct LEX_OP {
  char op[16];
}lexOP;

typedef struct LEX_STACK {
  struct LEX_POE *poe;
  struct LEX_OP *op;
  int *eval;
  int count;
}lexStack;

int rex(const char *,const char *);

struct LEX_STACK * lexGenerate(char *s);
int lexGenStack(int *size, int p, char **nl, int wc, char **wl);
int lexFree(struct LEX_STACK *stack);
int lexExtractPOE(char *s,struct LEX_POE *poe); 
int lexSplitString(char *s,char *d, int *wc,char *wl[]);

#endif

