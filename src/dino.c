/*********************************************

     DINO
     ====

  Visualizing Structural Biology

  Version 0.8

  (c) 1998-2002 Ansgar Philippsen

************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gui_ext.h"
#include "dbm.h"
#include "com.h"
#include "gfx.h"
#include "dino.h"
#include "scene.h"
#include "startup.h"

#ifdef NEW_SHELL
#include "shell_command.h"
#include "gui_terminal.h"
#else
#include "shell.h"
#endif

#ifndef USE_CMI
//#include "gui_ext.h"
#include "gui.h"
extern struct GUI gui;
#endif

const char usage[]={"Usage: dino [-debug] [-nostereo] [-help] [-s script] [-log filename] [+log] [X toolkit params]\n"};

char welcome[]={"Welcome to dino v%s    (http://www.dino3d.org)\n\n"};

int debug_mode,gfx_mode,stereo_mode,video_mode, shell_mode;

/***************************

  main
  ----

  process command line
  params

  initialize
   -dbm
   -scene
   -shell

  jump to event processing

****************************/

int dinoMain(int argc,char **argv)
{
  int gf,i;
  char files[64][256];
  char script[256];
  char expr[256];
  char startup[256];
  int filec;
  int startup_flag=1;
  char logfile[128];

  gfx_mode=1;
  stereo_mode=1;
  debug_mode=0;
  video_mode=0;
  shell_mode=0;

  strcpy(logfile,"logfile.dino");
  fprintf(stderr,welcome,VERSION);

  /* go through options */
  debmsg("checking command line");
  i=1;
  filec=0;
  strcpy(script,"");
  while(i<argc) {
    if(argv[i][0]=='-' ||
       argv[i][0]=='+') {
      if(!strcmp(argv[i],"-h") ||
         !strcmp(argv[i],"-help")) {
        fprintf(stderr,usage);
        exit(0);
      } else if(!strcmp(argv[i],"-d") ||
                !strcmp(argv[i],"-debug")) {
        debug_mode=1;
      } else if(!strcmp(argv[i],"-t") ||
                !strcmp(argv[i],"-trace")) {
        shell_mode=1;
        fprintf(stderr,"debug mode ON\n");
      } else if(!strcmp(argv[i],"-nostereo")) {
        stereo_mode=0;
      } else if(!strcmp(argv[i],"-vidmode")) {
        if(i+1>=argc) {
          fprintf(stderr,"error: expected argument after -vidmode\n");
          exit(1);
        }
        i++;
        video_mode=atoi(argv[i]);
      } else if(!strcmp(argv[i],"-nostartup")) {
        startup_flag=0;
      } else if(!strcmp(argv[i],"-log")) {
        if(i+1>=argc) {
          fprintf(stderr,"%s missing argument\n",argv[i]);
          fprintf(stderr,usage);
          exit(-1);
        }
        strncpy(logfile,argv[++i],128);
      } else if(!strcmp(argv[i],"+log")) {
        strcpy(logfile,"");
      } else if(!strcmp(argv[i],"-s")) {
        if(i+1>=argc) {
          fprintf(stderr,"%s missing argument\n",argv[i]);
          fprintf(stderr,usage);
          exit(-1);
        } else {
          strcpy(script,argv[i+1]);
          i++;
        }
      } else {
        fprintf(stderr,"Unknown flag %s\n",argv[i]);
        fprintf(stderr,usage);
        exit(-1);
      }
    } else {
      strcpy(files[filec++],argv[i]);
    }
    i++;
  }
 
#ifdef USE_CMI
  // debmsg("calling guiInit");
  //if(guiInit(argc, argv)<0)  return -1;
#endif
  debmsg("calling comInit");
  if(comInit()<0) return -1;
  // in main.c
  //debmsg("calling gfxInit");
  //if(gfxInit()<0) return -1;
  debmsg("calling dbmInit");
  if(dbmInit()<0) return -1;
  debmsg("calling sceneInit");
  if(sceneInit()<0) return -1;

#ifdef NEW_SHELL
  if(shellInit()<0) return -1;
#else
  debmsg("calling shellInit");
  shellInit((shell_callback)comWorkPrompt,logfile);
#endif

  if(startup_flag) {
    load_startup();
  }

  for(i=0;i<filec;i++) {
    sprintf(expr,"load %s",files[i]);
    comRawCommand(expr);
  }

  if(strlen(script)>0) {
    sprintf(expr,"@%s",script);
#ifdef USE_CMI
    comRawCommand(expr);
#else
    comSetInitCommand(expr);
#endif
  }

#ifdef USE_CMI
  comMessage("\n");
#endif

#ifndef NEW_SHELL
  shellPrompt();
#endif

  return 0;
}

void dinoExit(int n)
{
#ifdef NEW_SHELL
  guitOutit();
#else
  shellOutit();
#endif

#ifdef SGI
  cmiStereo(0);
#endif
#ifdef LINUX
#ifndef USE_CMI
#ifndef INTERNAL_COLOR
  if(strlen(gui.gdbm_tmpfile)>0)
    remove(gui.gdbm_tmpfile);
#endif
#endif
#endif
  exit(n);
}
