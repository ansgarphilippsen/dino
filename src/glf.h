#include <GL/gl.h>

#ifndef GLF_H
#define GLF_H

struct GLF {
  GLuint font_list;
  GLuint font_list_16;
  GLuint font_list_24;
};

enum          {GLF_16,
	       GLF_24};

int glfInit(void);
int glfGenFont(void);
int glfSetSize(int size);
int glfDrawString(char *string);
int glfDrawStringP(int x,int y, char *string);

#endif
