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

#ifdef USE_CMI
#include "cmi.h"
#else
#include "com.h"
#endif


int main(int argc, char **argv)
{


#ifdef USE_CMI
  cmiInit();

  if(dinoMain(argc, argv)<0) return -1;

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
