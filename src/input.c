#include <X11/X.h>

#include <stdio.h>
#include <string.h>

#include "dino.h"
#include "com.h"
#include "input.h"
#include "Cmalloc.h"

struct INPUT input;

static struct INPUT_ENTRY input_def_entry[]={
  /* device        num axis scale  */
  {INPUT_MOUSE,     0,  0,   1.0 },
  {INPUT_MOUSE,     0,  1,   1.0 },
  {INPUT_DIALBOX,   0,  0,   1.0 },
  {INPUT_DIALBOX,   0,  1,   1.0 },
  {INPUT_DIALBOX,   0,  2,   1.0 },
  {INPUT_DIALBOX,   0,  3,   1.0 },
  {INPUT_DIALBOX,   0,  4,   1.0 },
  {INPUT_DIALBOX,   0,  5,   1.0 },
  {INPUT_DIALBOX,   0,  6,   1.0 },
  {INPUT_DIALBOX,   0,  7,   1.0 },
  {INPUT_SPACEBALL, 0,  0,   1.0 },
  {INPUT_SPACEBALL, 0,  1,   1.0 },
  {INPUT_SPACEBALL, 0,  2,   1.0 },
  {INPUT_SPACEBALL, 0,  3,   1.0 },
  {INPUT_SPACEBALL, 0,  4,   1.0 },
  {INPUT_SPACEBALL, 0,  5,   1.0 },
  {INPUT_UNKNOWN,   0,  0,   0.0 }
};

int inputInit()
{
  int i;
  input.obj_max=32;
  input.obj=Ccalloc(input.obj_max,sizeof(struct INPUT_OBJECT));
  if(input.obj==NULL)
    return -1;
  input.obj_count=0;

  input.obj[0].type=INPUT_DEFAULT;
  strcpy(input.obj[0].name,"default");
  for(i=0;input_def_entry[i].device!=INPUT_UNKNOWN;i++) {
    memcpy(input.obj[0].entry+i,input_def_entry+i,sizeof(struct INPUT_ENTRY));
  }
  input.obj[0].entry_count=i;

  input.obj_count=1;
  return 0;
}

int inputCommand(int wc, char **wl, char *base)
{
  char message[256];

  if(wc<=0) {
    sprintf(message,"\ninput: missing command");
    comMessage(message);
  }
  if(base==NULL) {
    if(!strcmp(wl[0],"new")) {

    }
  }
  return 0;
}

/*
  return a UNIQUE register id!
  or -1 upon failure
*/

int inputRegister(struct INPUT_REGISTRY *reg)
{
  return 0;
}

/*
  use unique id to unregister
*/

int inputUnregister(int reg_id)
{
  return 0;
}
