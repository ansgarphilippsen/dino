/*************************************************
  main.c

  initialize the components of DINO

**************************************************/
#include <stdio.h>

#include "dino.h"
#include "gui_ext.h"
#include "gui_terminal.h"

#include "cmi.h"

int main(int argc, char **argv)
{
  cmiInit();

  if(dinoMain(argc, argv)<0) return -1;

  guitInit();

  guiMainLoop();

  // to make the compiler happy
  return 0;
}
