/*********************************************

                     DINO 
                     ====

         Visualizing Structural Biology

                  Version 0.8

          (c) 1998-2001 Ansgar Philippsen

************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbm.h"
#include "com.h"
#include "shell.h"
#include "gui.h"
#include "gfx.h"
#include "dino.h"
#include "scene.h"

#ifdef EXPO
#include <sys/types.h>
#include <sys/stat.h>
#include "autoplay.h"
#endif


extern struct GUI gui;

const char usage[]={"Usage: dino [-debug] [-nostereo] [-help] [-s script] [-log filename] [+log] [X toolkit params]\n"};

char welcome[]={"\nWelcome to dino v%s    (http://www.dino3d.org)\n\n"};

int debug_mode,gfx_mode,stereo_mode,video_mode;

#ifdef EXPO
extern struct AUTOPLAY ap;
#endif

#ifdef CORE_DEBUG
FILE *core_debug;
#endif

/***************************

  main
  ----

  process command line
  params

  initialize
      -gui
      -dbm
      -scene
      -shell

  jump to event processing

****************************/

int main(int argc,char **argv)
{
  int gf,i;
  char files[64][256];
  char script[256];
  char expr[256];
  char startup[256];
  int filec;
  int startup_flag=1;
  char logfile[128];

#ifdef CMALLOC_DEBUG
  Cmalloc_init();
#endif  
  gfx_mode=1;
  stereo_mode=1;
  debug_mode=0;
  video_mode=0;


  strcpy(startup,"dinorc");
  strcpy(logfile,"logfile.dino");
  fprintf(stderr,welcome,VERSION);
#ifdef EXPO
  fprintf(stderr,"EXPO 2000 SPECIAL VERSION\n\n");
#endif

#ifdef CORE_DEBUG
  core_debug=fopen("debug.dino","w");
  if(core_debug==NULL)
    core_debug=fopen("/dev/null","w");
#endif
  
  /*
    command line parsing needs to be split
    because some options need to be evaluated
    before anything else is done, while some
    need evaluation after X has removed the
    toolkit stuff
  */
  i=1;
  while(i<argc) {
    if(argv[i][0]=='-') {
      if(!strcmp(argv[i],"-h") ||
	 !strcmp(argv[i],"-help")) {
	fprintf(stderr,usage);
	exit(0);

      } else if(!strcmp(argv[i],"-d") ||
		!strcmp(argv[i],"-debug")) {
	debug_mode=1;
	fprintf(stderr,"debug mode ON\n");
      } else if(!strcmp(argv[i],"-nogfx")) {
	gfx_mode=0;
      } else if(!strcmp(argv[i],"-nostereo")) {
	stereo_mode=0;
      } else if(!strcmp(argv[i],"-vidmode")) {
	if(i+1>=argc) {
	  fprintf(stderr,"error: expected argument after -vidmode\n");
	  exit(1);
	}
	i++;
	video_mode=atoi(argv[i]);
      }
    }
    i++;
  }


  debmsg("calling comInit");
  comInit();

  debmsg("calling gfxInit");
  gfxInit();

#ifdef EXPO
  /* check for the autoplay file */
  if((ap.f=fopen("autoplay","r"))==NULL) {
    fprintf(stderr,"\nautoplay not found!\n");
    return -1;
  } else {
    ap.flag=1;
    apInit();
  }
#endif

  if(gfx_mode) {
    debmsg("calling guiInit");
    gf=guiInit(comWorkGfxCommand,&argc,&argv);
  } else {
    return 0;
    debmsg("calling guiMInit");
    gf=guiMInit(comWorkGfxCommand, &argc, &argv);
  }

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
	/* already had this */
      } else if(!strcmp(argv[i],"-d") ||
		!strcmp(argv[i],"-debug")) {
	/* already had this */
      } else if(!strcmp(argv[i],"-nogfx")) {
	/* already had this */
      } else if(!strcmp(argv[i],"-nostereo")) {
	/* already had this */
      } else if(!strcmp(argv[i],"-vidmode")) {
	/* already had this */
	i++;
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


  if(gf!=0) {
    fprintf(stderr,"graphics mode is off\n");
  }


  debmsg("calling dbmInit");
  dbmInit();
  debmsg("calling sceneInit");
  sceneInit();
  debmsg("calling shellInit");
  shellInit(comWorkPrompt,logfile);


  if(startup_flag) {
    /* TODO
      check if a .dinorc exists
       in $HOME
    */
  }

#ifdef EXPO
  if(ap.flag) {
    if(apPrep()!=0) {
      return -1;
    }
  }
#endif

#ifndef EXPO
  for(i=0;i<filec;i++) {
    sprintf(expr,"load %s",files[i]);
    comRawCommand(expr);
  }

  if(strlen(script)>0) {
    sprintf(expr,"@%s",script);
//    comRawCommand(expr);
    shellSetInitCommand(expr);
  }
#endif

  if(gf==0) {
#ifndef GLUT_GUI
    debmsg("setting Xtimer");
    XtAppAddTimeOut(gui.app,100,(XtTimerCallbackProc)guiTimeProc,NULL);
#endif
    debmsg("calling guiMainLoop");
    shellPrompt();
    guiMainLoop();
  } else {
    debmsg("running pseudo loop");
    shellPrompt();
    while(1) {
      usleep(5);
      comTimeProc();
    }
  }  
  
  /* to make the compiler happy */
  return 0;
}

void dinoExit(int n)
{
  shellOutit();
#ifdef SGI
  if(gui.stereo_mode==GUI_STEREO_NORMAL)
    system("/usr/gfx/setmon -n 72HZ");
#endif
#ifdef LINUX
  if(strlen(gui.gdbm_tmpfile)>0)
    remove(gui.gdbm_tmpfile);
#endif
  exit(n);
}
