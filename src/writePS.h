#ifndef _WRITE_PS_H
#define _WRITE_PS_H

struct FEEDBACK_INDEX {
  GLfloat *entry;
  GLfloat depth;
};

struct FEEDBACK_3D_COLOR {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLfloat red;
  GLfloat green;
  GLfloat blue;
  GLfloat alpha;
};

int writeGenScene(GLfloat **buf);

int writePS(FILE *f);
int writePSHeader(FILE *f);
int writePSObj(FILE *f, GLfloat *buf, int n);
int writePSPrimitive(FILE *f, GLfloat *entry);

int writeCompare(const void *, const void *);

#endif
