/*************************************************
  main.c

  initialize the components of DINO:

    - COM
    - GUI

**************************************************/
#include <stdio.h>

#include "dino.h"
#include "gui_ext.h"
#include "gui_terminal.h"

#ifndef USE_CMI
#include "com.h"
#endif

#ifdef USE_CMI
#include "cmi.h"
#endif

#ifdef WX_GUI
int main2(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
  int ret;


#ifdef USE_CMI
  cmiInit();

  dinoMain(argc, argv);
  if(guiInit(&argc, &argv)<0) {
    return -1;
  }
#else

#ifdef X11_GUI
  dinoMain(argc, argv);


  if(guiInit(comWorkGfxCommand,&argc,&argv)<0) {
    return -1;
  }
#else
  if(guiInit(comWorkGfxCommand,&argc,&argv)<0) {
    return -1;
  }
  dinoMain(argc, argv);
#endif

#endif

#ifdef NEW_SHELL
  guitInit();
#endif

  
  guiMainLoop();

  // to make the compiler happy
  return 0;
}
