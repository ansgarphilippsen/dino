#include "gui.h"
#include "jci.h"


int guiInit(void (*func)(int, char **), int *argc, char ***argv)
{
  debmsg("initializing JGUI");
  jciInit();

  return -1;
}

