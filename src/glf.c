/*****************************

glf library
after font.c from OpenGL book
(c) 1998 AP

*******************************/
#include <string.h>
#include <stdio.h>

#include "glf.h"
#include "font_24.h"
#include "font_16.h"

struct GLF glf;

int glfInit()
{
  glf.font_list_16=glGenLists(128);
  glf.font_list_24=glGenLists(128);
  return 0;
}


int glfGenFont()
{
  GLuint i;
  GLubyte c[72];
  int j;

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glPixelStorei(GL_UNPACK_LSB_FIRST,1);
  glScalef(1.0,-1.0,1.0);

  for(i=32;i<91;i++) {
    for(j=0;j<16;j++) {
      c[j*2+0]=font_16[i-32][(16-j)*2+0];
      c[j*2+1]=font_16[i-32][(16-j)*2+1];
      c[j*2+2]=font_16[i-32][(16-j)*2+2];
    }
    glNewList(i+glf.font_list_16,GL_COMPILE);
    glBitmap(16,16,1.0,0.0,14.0,0.0,c);
    glEndList();
  }

  for(i=32;i<91;i++) {
    for(j=0;j<24;j++) {
      c[j*3+0]=font_24[i-32][(23-j)*3+0];
      c[j*3+1]=font_24[i-32][(23-j)*3+1];
      c[j*3+2]=font_24[i-32][(23-j)*3+2];
    }
    glNewList(i+glf.font_list_24,GL_COMPILE);
    glBitmap(24,24,0.0,0.0,24.0,0.0,c);
    glEndList();
  }

  glf.font_list=glf.font_list_16;

  return 0;
}

int glfDrawString(char *s)
{
  int l,l2=strlen(s);
  char c,s2[256];

  for(l=0;l<l2;l++) {
    c=(char)toupper((int)s[l]);
    s2[l]=c;
  }  
  glPushAttrib(GL_LIST_BIT);
  glListBase(glf.font_list);
  glCallLists(l2,GL_UNSIGNED_BYTE,(GLubyte *)s2);
  glPopAttrib();
  return 0;
}

int glfSetSize(int size)
{
  switch(size) {
  case GLF_16:
    glf.font_list=glf.font_list_16;
    break;
  case GLF_24:
    glf.font_list=glf.font_list_24;
    break;
  default:
    return -1;
  }
  return 0;
}

