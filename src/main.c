/*************************************************
  main.c

  initialize the components of DINO:

    - COM
    - GUI

**************************************************/
#include <stdio.h>

#include "dino.h"
#include "gui_ext.h"

#ifndef USE_CMI
#include "com.h"
#endif

#ifdef USE_CMI
#include "cmi.h"
#endif

int main(int argc, char **argv)
{
  int ret;

#ifdef USE_CMI
  cmiInit();
#endif
  

#ifndef USE_CMI
  if(guiInit(comWorkGfxCommand,&argc,&argv)<0) {
    return -1;
  }
#endif

  dinoMain(argc, argv);
  
#ifdef USE_CMI
  if(guiInit(&argc, &argv)<0) {
    return -1;
  }
#endif
  
  guiMainLoop();

  // to make the compiler happy
  return 0;
}
