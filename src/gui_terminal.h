#ifndef _GUI_TERMINAL_H
#define _GUI_TERMINAL_H

#define KEY_UP    0x201
#define KEY_DOWN  0x202
#define KEY_LEFT  0x203
#define KEY_RIGHT 0x204

int guitInit(void);
void guiOutit(void);

void guitTimeProc(void);
void guitWrite(const char *s);
void guitAddChar(unsigned char c);
void guitSuspend(int f);

#endif
