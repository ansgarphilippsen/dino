#ifndef INPUT_H
#define INPUT_H

#include "dbm.h"

enum               {INPUT_UNKNOWN=0,
		    INPUT_MOUSE,
		    INPUT_DIALBOX,
		    INPUT_SPACEBALL,
		    INPUT_PAD};
enum                   {INPUT_DEFAULT,
			INPUT_CUSTOM};

#define INPUT_NONE     0
#define INPUT_BUTTON1  1<<0
#define INPUT_BUTTON2  1<<1
#define INPUT_BUTTON3  1<<2
#define INPUT_SHIFT    1<<3
#define INPUT_CNTRL    1<<4
#define INPUT_ALT      1<<5
#define INPUT_ALL      0xffff 


struct INPUT_LIST_ENTRY {
  int device;  /* device type */
  int num;     /* number of device, usually 0, except for pads */
  int axis;    /* axis */
  int target;  /* the target for the input */
  char ext[256]; /* extended info if necessary */
};

struct INPUT_MESSAGE {
  float abs_value,rel_value;
};

struct INPUT_ENTRY {
  int device;  /* hardware device */
  int num;     /* number, always 0 except for pads */
  int axis;    /* device axis of freedom */
  float scale; /* pre-scale for hardware calibration */
};


typedef void (*inputCallbackFunc)(struct INPUT_MESSAGE *m, void *p);

#define INPUT_OBJECT_MAX_ENTRY 32

struct INPUT_OBJECT {
  int type;
  char name[64];
  struct INPUT_ENTRY entry[INPUT_OBJECT_MAX_ENTRY];
  int entry_count;
};

struct INPUT_REGISTRY {
  char oname[64];
  int obj,entry;
  unsigned int state,mask;
  float scale;
  inputCallbackFunc f;
  void *ptr;
};

struct INPUT {
  struct INPUT_OBJECT *obj;
  int obj_count,obj_max;
  struct INPUT_REGISTRY *reg;
  int reg_count,reg_max;
};

int inputInit(void);
int inputCommand(int wc, char **wl, char *base);

/* rewrite! to include mask, state, scale etc */
int inputRegister(struct INPUT_REGISTRY *reg);
int inputUnregister(int reg_id);

#endif

