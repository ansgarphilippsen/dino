#ifndef FONT_16_H
#define FONT_16_H

/* 16x16 pixel font (c) 1998 AP */

#define EMPTY_FONT_16 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

/* start at first printable char 0x20, which is space */

unsigned char font_16[][32] = {
  /* 0x20 space */
  EMPTY_FONT_16,
  /* 0x21 ! */
  EMPTY_FONT_16,
  /* 0x22 " */
  EMPTY_FONT_16,
  /* 0x23 # */
  EMPTY_FONT_16,
  /* 0x24 $ */
  EMPTY_FONT_16,
  /* 0x25 % */
  EMPTY_FONT_16,
  /* 0x26 & */
  EMPTY_FONT_16,
  /* 0x27 '  */
  EMPTY_FONT_16,
  /* 0x28 ( */
  EMPTY_FONT_16,
  /* 0x29 ) */
  EMPTY_FONT_16,
  /* 0x2a * */
  EMPTY_FONT_16,
  /* 0x2b + */
  EMPTY_FONT_16,
  /* 0x2c , */
  EMPTY_FONT_16,
  /* 0x2d - */
  EMPTY_FONT_16,
  /* 0x2e . */
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01,
   0xc0, 0x03, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00},
  /* 0x2f / */
  EMPTY_FONT_16,
  /* 0x30 0 */
  {0x00, 0x00, 0xf0, 0x0f, 0xf8, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1c,
   0x18, 0x1e, 0x18, 0x1b, 0x98, 0x19, 0xd8, 0x18, 0x78, 0x18, 0x38, 0x18,
   0x18, 0x18, 0xf8, 0x1f, 0xf0, 0x0f, 0x00, 0x00},
  /* 0x31 1 */
  {0x00, 0x00, 0x80, 0x01, 0x80, 0x01, 0xc0, 0x01, 0xe0, 0x01, 0xf0, 0x01,
   0xb0, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x00, 0x00},
  /* 0x32 2 */
  {0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x18, 0x18,
   0x00, 0x1c, 0x00, 0x0e, 0x80, 0x07, 0xc0, 0x03, 0xe0, 0x00, 0x70, 0x00,
   0x38, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0x00, 0x00},
  /* 0x33 3 */
  {0x00, 0x00, 0xf0, 0x0f, 0xf8, 0x1f, 0x18, 0x18, 0x00, 0x18, 0x00, 0x18,
   0x00, 0x18, 0xc0, 0x0f, 0xc0, 0x0f, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18,
   0x18, 0x18, 0xf8, 0x1f, 0xf0, 0x0f, 0x00, 0x00},
  /* 0x34 4 */
  {0x00, 0x00, 0x30, 0x03, 0x30, 0x03, 0x30, 0x03, 0x30, 0x03, 0x38, 0x03,
   0x18, 0x03, 0x18, 0x03, 0xf8, 0x1f, 0xf8, 0x1f, 0x00, 0x03, 0x00, 0x03,
   0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00},
  /* 0x35 5 */
  {0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0x18, 0x00, 0x18, 0x00, 0xd8, 0x07,
   0xf8, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x38, 0x1c, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00},
  /* 0x36 6 */
  {0x00, 0x00, 0x80, 0x0f, 0xe0, 0x0f, 0xf0, 0x00, 0x30, 0x00, 0x38, 0x00,
   0xd8, 0x07, 0xf8, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x38, 0x1c, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00},
  /* 0x37 7 */
  {0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0x00, 0x0e, 0x00, 0x07, 0x80, 0x03,
   0x80, 0x01, 0xc0, 0x01, 0xc0, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0xc0, 0x00,
   0xc0, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x00, 0x00},
  /* 0x38 8 */
  {0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c,
   0xe0, 0x07, 0xf0, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x38, 0x1c, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00},
  /* 0x39 9 */
  {0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x18, 0x18,
   0x38, 0x1c, 0xf0, 0x1f, 0xe0, 0x1f, 0x00, 0x0c, 0x00, 0x0e, 0x00, 0x07,
   0xc0, 0x03, 0xf0, 0x01, 0x70, 0x00, 0x00, 0x00},
  /* 0x3a */
  EMPTY_FONT_16,
  /* 0x3b */
  EMPTY_FONT_16,
  /* 0x3c */
  EMPTY_FONT_16,
  /* 0x3d */
  EMPTY_FONT_16,
  /* 0x3e */
  EMPTY_FONT_16,
  /* 0x3f */
  EMPTY_FONT_16,
  /* 0x40 @ */
  EMPTY_FONT_16,
  /* 0x41 A */
  {0x00, 0x00, 0xc0, 0x03, 0xe0, 0x07, 0x60, 0x06, 0x60, 0x06, 0x70, 0x0e,
   0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0xf8, 0x1f, 0xf8, 0x1f, 0x18, 0x18,
   0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00},
  /* 0x42 B  */
  {0x00, 0x00, 0xf0, 0x03, 0xf8, 0x07, 0x18, 0x0e, 0x18, 0x0c, 0x18, 0x0c,
   0x18, 0x06, 0xf8, 0x07, 0xf8, 0x0f, 0x18, 0x1c, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x1c, 0xf8, 0x0f, 0xf0, 0x07, 0x00, 0x00},
  /* 0x43 C  */
  {0x00, 0x00, 0xe0, 0x0f, 0xf0, 0x1f, 0x38, 0x18, 0x18, 0x18, 0x18, 0x00,
   0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x18,
   0x38, 0x18, 0xf0, 0x1f, 0xe0, 0x0f, 0x00, 0x00},
  /* 0x44 D  */
  {0x00, 0x00, 0xf0, 0x03, 0xf8, 0x07, 0x18, 0x0e, 0x18, 0x0c, 0x18, 0x1c,
   0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1c, 0x18, 0x0c,
   0x18, 0x0e, 0xf8, 0x07, 0xf0, 0x03, 0x00, 0x00},
  /* 0x45 E  */
  {0x00, 0x00, 0xf0, 0x1f, 0xf8, 0x1f, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0xf8, 0x0f, 0xf8, 0x0f, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0xf8, 0x1f, 0xf0, 0x1f, 0x00, 0x00},
  /* 0x46 F  */
  {0x00, 0x00, 0xf0, 0x1f, 0xf8, 0x1f, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0xf8, 0x07, 0xf8, 0x07, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00},
  /* 0x47 G  */
  {0x00, 0x00, 0xf0, 0x0f, 0xf8, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
   0x18, 0x00, 0x18, 0x00, 0x98, 0x1f, 0x98, 0x1f, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0xf8, 0x1f, 0xf0, 0x0f, 0x00, 0x00},
  /* 0x48 H  */
  {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0xf8, 0x1f, 0xf8, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00},
  /* 0x49 I  */
  {0x00, 0x00, 0xf0, 0x0f, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0xf0, 0x0f, 0x00, 0x00},
  /* 0x4a J  */
  {0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18,
   0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x1c,
   0x38, 0x0c, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00},
  /* 0x4b K  */
  {0x00, 0x00, 0x18, 0x18, 0x18, 0x1c, 0x18, 0x0e, 0x18, 0x07, 0x98, 0x03,
   0xd8, 0x01, 0xf8, 0x00, 0xf8, 0x00, 0xd8, 0x01, 0x98, 0x03, 0x18, 0x07,
   0x18, 0x0e, 0x18, 0x1c, 0x18, 0x18, 0x00, 0x00},
  /* 0x4c L  */
  {0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0xf8, 0x1f, 0xf0, 0x1f, 0x00, 0x00},
  /* 0x4d M  */
  {0x00, 0x00, 0x0c, 0x30, 0x1c, 0x38, 0x1c, 0x38, 0x3c, 0x3c, 0x3c, 0x3c,
   0x6c, 0x36, 0x6c, 0x36, 0xcc, 0x33, 0xcc, 0x33, 0x8c, 0x31, 0x8c, 0x31,
   0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x00, 0x00},
  /* 0x4e N  */
  {0x00, 0x00, 0x38, 0x18, 0x38, 0x18, 0x78, 0x18, 0x78, 0x18, 0xd8, 0x18,
   0xd8, 0x18, 0x98, 0x19, 0x98, 0x19, 0x18, 0x1b, 0x18, 0x1b, 0x18, 0x1e,
   0x18, 0x1e, 0x18, 0x1c, 0x18, 0x1c, 0x00, 0x00},
  /* 0x4f O  */
  {0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x38, 0x1c, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00},
  /* 0x50 P  */
  {0x00, 0x00, 0xf0, 0x0f, 0xf8, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0xf8, 0x1f, 0xf8, 0x0f, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00,
   0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00},
  /* 0x51 Q  */
  {0x00, 0x00, 0xe0, 0x07, 0xf0, 0x0f, 0x38, 0x1c, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x98, 0x19, 0x98, 0x1b, 0x18, 0x1f,
   0x38, 0x0e, 0xf0, 0x1f, 0xe0, 0x1b, 0x00, 0x00},
  /* 0x52 R  */
  {0x00, 0x00, 0xf0, 0x0f, 0xf8, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0xf8, 0x1f, 0xf8, 0x0f, 0x18, 0x06, 0x18, 0x0e, 0x18, 0x0c,
   0x18, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00},
  /* 0x53 S  */
  {0x00, 0x00, 0xe0, 0x0f, 0xf0, 0x1f, 0x38, 0x18, 0x18, 0x18, 0x18, 0x00,
   0x98, 0x07, 0xf8, 0x0f, 0xf0, 0x1c, 0x00, 0x18, 0x00, 0x18, 0x18, 0x18,
   0x18, 0x1c, 0xf8, 0x0f, 0xf0, 0x07, 0x00, 0x00},
  /* 0x54 T  */
  {0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x00, 0x00},
  /* 0x55 U  */
  {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
   0x38, 0x1c, 0xf0, 0x0f, 0xe0, 0x07, 0x00, 0x00},
  /* 0x56 V  */
  {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x38, 0x1c, 0x30, 0x0c,
   0x30, 0x0c, 0x70, 0x0e, 0x60, 0x06, 0x60, 0x06, 0xe0, 0x07, 0xc0, 0x03,
   0xc0, 0x03, 0x80, 0x01, 0x80, 0x01, 0x00, 0x00},
  /* 0x57 W  */
  {0x00, 0x00, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x0c, 0x30, 0x8c, 0x31,
   0x9c, 0x39, 0x98, 0x19, 0xd8, 0x1b, 0xd8, 0x1b, 0xf8, 0x1f, 0x70, 0x0e,
   0x70, 0x0e, 0x30, 0x0c, 0x30, 0x0c, 0x00, 0x00},
  /* 0x58 X  */
  {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x30, 0x0c, 0x60, 0x06, 0x60, 0x06,
   0xc0, 0x03, 0x80, 0x01, 0x80, 0x01, 0xc0, 0x03, 0x60, 0x06, 0x60, 0x06,
   0x30, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00},
  /* 0x59 Y  */
  {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x30, 0x0c, 0x30, 0x0c, 0x60, 0x06,
   0x60, 0x06, 0xc0, 0x03, 0xc0, 0x03, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01,
   0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x00, 0x00},
  /* 0x5a Z  */
  {0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0x00, 0x18, 0x00, 0x1c, 0x00, 0x0e,
   0x00, 0x07, 0x80, 0x03, 0xc0, 0x01, 0xe0, 0x00, 0x70, 0x00, 0x38, 0x00,
   0x18, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0x00, 0x00},
  /* 0x5b [ */
  EMPTY_FONT_16,
  /* 0x5c \ */
  EMPTY_FONT_16,
  /* 0x5d ] */
  EMPTY_FONT_16,
  /* 0x5e ^ */
  EMPTY_FONT_16,
  /* 0x5f _ */
  EMPTY_FONT_16,
  /* 0x60 ` */
  EMPTY_FONT_16,
  /* 0x61 a */
} ;
#endif
