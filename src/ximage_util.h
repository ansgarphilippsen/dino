#ifndef XIMAGE_UTIL_H
#define XIMAHE_UTIL_H

int xu_writeimage(XImage *ximage,char *name);
int xu_getpixel(XImage *image, int x, int y);

int xu_Reverse_Bytes (unsigned char *bpt, int nb);
int xu_normalizeimagebits (
    unsigned char *bpt, /* beginning pointer to image bits */
    int nb,             /* number of bytes to normalize */
    int byteorder,      /* swap bytes if byteorder == MSBFirst */
    int unitsize,       /* size of the bitmap_unit or Zpixel */
    int bitorder);       /* swap bits if bitorder == MSBFirst */
unsigned long xu_XGetPixel(XImage *ximage, int x, int y);
unsigned long xu_XGetPixel8(XImage *ximage, int x, int y);
unsigned long xu_XGetPixel1(XImage *ximage, int x, int y);
unsigned long xu_MyXGetPixel(XImage *ximage, int x, int y);

#endif
