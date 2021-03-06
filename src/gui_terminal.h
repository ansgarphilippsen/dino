#ifndef _GUI_TERMINAL_H
#define _GUI_TERMINAL_H

#define KEY_UP    0x201
#define KEY_DOWN  0x202
#define KEY_LEFT  0x203
#define KEY_RIGHT 0x204

#ifdef __cplusplus
extern "C" {
#endif

int guitInit(void);
void guitOutit(void);

void guitTimeProc(void);
void guitWrite(const char *s);
void guitAddChar(unsigned char c);
void guitSuspend(int f);

#ifdef __cplusplus
}
#endif


#endif
