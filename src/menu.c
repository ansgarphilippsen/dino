#include <stdio.h>

#include "dino.h"
#include "com.h"
#include "gui.h"
#include "om.h"
#include "menu.h"

extern struct GUI gui;
extern struct OBJECT_MENU om;


int menuCommand(int wc, char **wl)
{
  char message[256];

  if(wc<=0)
    return -1;

  if(!strcmp(wl[0],"add")) {

  } else {
    sprintf(message,"\nmenu: unknown command %s",wl[0]);
    comMessage(message);
    return -1;
  }

  return 0;
}
