#ifndef SCENE_H
#define SCENE_H

struct SCENE_STACK {
  char expr[256];
};

#ifdef EXPO
struct SCENE_LABEL {
  char s[128];
  float p[3],c[3];
};
#endif

struct SCENE {
  struct SCENE_STACK *stack;
  int stack_p,stack_m;
  float cp[3];
  int cpflag;
#ifdef EXPO
  struct SCENE_LABEL *label;
  int label_c,label_m;
#endif
};

#endif

int sceneInit(void);
int sceneCommand(int wc, const char **wl);
void scenePush(char *s);
char *scenePop(void);
char *scenePeek(void);
void sceneClear(void);
int scenePersp2Ortho(void);
int sceneOrtho2Persp(void);

int sceneSubCommand(char *sub, int wc, const char **wl);
int sceneSubLightCom(int l, int wc, const char **wl);
int sceneSubLightSet(int l, int wc, const char **wl);
int sceneSubLightGet(int l, int wc, const char **wl);

int sceneSubClipCom(int c, int wc, const char **wl);
int sceneSubClipSet(int c, int wc, const char **wl);
int sceneSubClipGet(int c, int wc, const char **wl);

int sceneMorph(int wc, const char **wl);
