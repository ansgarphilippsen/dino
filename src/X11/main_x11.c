/*************************************************
  main_x11.c

  provides main() for X11 GUI

**************************************************/
#include <stdio.h>

#include "dino.h"
#include "gui_ext.h"
#include "gui_terminal.h"
#include "cmi.h"

int main(int argc, char **argv)
{
  int ret;

  cmiInit();

  dinoMain(argc, argv);

  if(guiInit(&argc, &argv)<0) {
    return -1;
  }

  guitInit();

  guiMainLoop();

  return 0;
}
