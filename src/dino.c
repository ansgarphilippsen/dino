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
#include "cl.h"
#include "shell_command.h"
#include "gui_terminal.h"

const char usage[]={"Usage: dino [-debug] [-stereo] [-nostereo] [-help] [-s script] [-f script] [-log filename] [+log] [X toolkit params]\n"};

char welcome[]={"Welcome to dino v%s    (http://www.dino3d.org)\n\n"};

int debug_mode,gfx_mode,stereo_mode,video_mode, shell_mode,gfx_flags,demo_flag;

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

static int startup_flag=1;
static char logfile[128];
static char startup[256];
static char script[256];
static int script_fast;

int dinoParseArgs(int argc, char **argv)
{
  int i;
  char expr[256];

  gfx_mode=1;
#ifdef SGI
  stereo_mode=1;
#else
  stereo_mode=0;
#endif
  debug_mode=0;
  video_mode=0;
  shell_mode=0;
  gfx_flags=0;
  demo_flag=0;

#ifndef OSX
  fprintf(stderr,welcome,VERSION);
#endif
  
  strcpy(script,"");
  strcpy(logfile,"logfile.dino");

  /* go through options */
  debmsg("checking command line");
  i=1;
  //  filec=0;
  while(i<argc) {
    if(argv[i][0]=='-' ||
       argv[i][0]=='+') {
      if(clStrcmp(argv[i],"-h") ||
         clStrcmp(argv[i],"-help")) {
        fprintf(stderr,usage);
        exit(0);
      } else if(clStrcmp(argv[i],"-d") ||
                clStrcmp(argv[i],"-debug")) {
        debug_mode=1;
      } else if(clStrcmp(argv[i],"-t") ||
                clStrcmp(argv[i],"-trace")) {
        shell_mode=1;
        fprintf(stderr,"debug mode ON\n");
      } else if(clStrcmp(argv[i],"-nostereo")) {
        stereo_mode=0;
	gfx_flags+=DINO_FLAG_NOSTEREO;
      } else if(clStrcmp(argv[i],"-stereo")) {
        stereo_mode=1;
	gfx_flags+=DINO_FLAG_STEREO;
      } else if(clStrcmp(argv[i],"-nostencil")) {
	gfx_flags+=DINO_FLAG_NOSTENCIL;
      } else if(clStrcmp(argv[i],"-nogfx")) {
	gfx_flags+=DINO_FLAG_NOGFX;
      } else if(clStrcmp(argv[i],"-noom")) {
	gfx_flags+=DINO_FLAG_NOOBJMENU;
      } else if(clStrcmp(argv[i],"-cons")) {
	gfx_flags+=DINO_FLAG_CONS;
      } else if(clStrcmp(argv[i],"-nodblbuff")) {
	gfx_flags+=DINO_FLAG_NODBLBUFF;
      } else if(clStrcmp(argv[i],"-vidmode")) {
        if(i+1>=argc) {
          fprintf(stderr,"error: expected argument after -vidmode\n");
          exit(1);
        }
        i++;
        video_mode=atoi(argv[i]);
      } else if(clStrcmp(argv[i],"-nostartup")) {
        startup_flag=0;
      } else if(clStrcmp(argv[i],"-log")) {
        if(i+1>=argc) {
          fprintf(stderr,"%s missing argument\n",argv[i]);
          fprintf(stderr,usage);
          exit(-1);
        }
        strncpy(logfile,argv[++i],128);
      } else if(clStrcmp(argv[i],"+log") ||
		clStrcmp(argv[i],"-nolog")) {
        strcpy(logfile,"");
      } else if(clStrcmp(argv[i],"-s")) {
        if(i+1>=argc) {
          fprintf(stderr,"%s missing argument\n",argv[i]);
          fprintf(stderr,usage);
          exit(-1);
        } else {
          strcpy(script,argv[i+1]);
	  script_fast=0;
          i++;
        }
      } else if(clStrcmp(argv[i],"-f")) {
        if(i+1>=argc) {
          fprintf(stderr,"%s missing argument\n",argv[i]);
          fprintf(stderr,usage);
          exit(-1);
        } else {
          strcpy(script,argv[i+1]);
	  script_fast=1;
          i++;
        }
      } else if(clStrcmp(argv[i],"-demo")) {
	demo_flag=1;
      } else {
	/*
	  TODO
	  X hasn't removed its params at this point!
        fprintf(stderr,"Unknown flag %s\n",argv[i]);
        fprintf(stderr,usage);
        exit(-1);
	*/
      }
    } else {
      //fprintf(stderr,"ignored superfluous word %s\n",argv[i]);
    }
    i++;
  }
  return 0;
}

static void reset_com_params(struct COM_PARAMS *p)
{
#ifdef SGI
  p->stereo_flag=1;
#else
  p->stereo_flag=0;
#endif
  p->mouse_rot_scale=1.0;
  p->mouse_tra_scale=1.0;
  p->sb_rot_scale=1.0;
  p->sb_tra_scale=1.0;
  p->dials_rot_scale=1.0;
  p->dials_tra_scale=1.0;
  p->trans_limit_flag=0;
  p->demo_flag=0;
}
 
int dinoMain(int argc,char **argv)
{
  int i;
  char expr[256];
  struct COM_PARAMS com_params;

  reset_com_params(&com_params);

  if(startup_flag) {
    load_startup(&com_params);
  }

  if(demo_flag) {
    com_params.demo_flag=1;
  }

  debmsg("calling comInit");
  if(comInit(&com_params)<0) return -1;
  //debmsg("calling gfxInit");
  //if(gfxInit()<0) return -1;
  debmsg("calling dbmInit");
  if(dbmInit()<0) return -1;
  debmsg("calling sceneInit");
  if(sceneInit()<0) return -1;

  if(shellInit(logfile)<0) return -1;


  /*
  for(i=0;i<filec;i++) {
    sprintf(expr,"load %s",files[i]);
    comRawCommand(expr);
  }
  */

  if(strlen(script)>0) {
    if(script_fast) {
      sprintf(expr,"@%s -f",script);
    } else {
      sprintf(expr,"@%s",script);
    }
    comRawCommand(expr);
  }

  comMessage("\n");

  return 0;
}

void dinoExit(int n)
{
  // must come first
  debmsg("comOutit\n");
  comOutit();

  debmsg("ensuring stereo off\n");
  cmiStereo(0);
  
#ifndef OSX
  debmsg("restoring terminal\n");
  guitOutit();
  
  debmsg("closing down gui\n");
  guiExit();
#endif
  
  debmsg("exit");
  exit(n);
}
