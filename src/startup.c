/*************************************
  startup.c

  all global structs can be modified
  by a resource file upon startup
  
  > keywords:

  exec COMMAND : execute raw command
  exec {} : execute block of commands
  bind DEVICE AXIS STATE ACTION SCALE: event table settings 
  gfxwin GEOMETRY : geometry of _inner_ gfx window
  menu clear : clear user menu
  menu add LABEL COMMAND : add command (using some wildcards!)
  logfile NAME : name of logfile to use
  nologfile : don't write a logfile
  prompt: DINO prompt
  on KEY COMMAND: KEY is F1-F12 or C-* or M-*

  > default values for datasets and objects

  atomcolor ELE COLOR

  struct_ds
  struct_obj
  struct_obj_connect
  struct_obj_trace

  surf_ds
  surf_obj

  scal_ds
  scal_obj
  scal_obj_contour
  scal_obj_grid
  scal_obj_slab

  topo_ds
  topo_obj
  topo_obj_contour
  topo_obj_surface

  geom_ds
  geom_obj


*************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "startup.h"
#include "dino.h"
#include "com.h"
#include "gfx.h"
#include "scene.h"
#include "cl.h"

extern struct GFX gfx;
extern struct SCENE scene;

extern int debug_mode;

static int exec_flag;

static int eval_line(int argc, char **argv)
{
  int i;
  static char message[256];

  if(argv[0]==NULL) {
    // empty line, ignore
  } else if(argv[0][0]=='#') {
    // comment, ignore
  } else if(clStrcmp(argv[0],"exec")) {
    // execute either single command or block
    if(clStrcmp(argv[1],"{")) {
      // execute block
      exec_flag=1;
    } else {
      clStrcpy(message,"");
      for(i=1;i<argc;i++) {
	clStrcat(message,argv[i]);
	clStrcat(message," ");
      }
      comRawCommand(message);
    }
  } else if(clStrcmp(argv[0],"exec{")) {
    // execute block
    exec_flag=1;
  } else if(clStrcmp(argv[0],"}")) {
    if(exec_flag==1) {
      exec_flag=0;
    } else {
      sprintf(message,"startup: misplaced }\n");
      comMessage(message);
      return -1;
    }
  } else {
    /*
      stereo-mode 
      dials-device
      gfx-geometry
      menu-geometry
    */

    // unknown keyword
    /* ignore for now
      sprintf(message,"startup: unknown keyword %s\n",argv[0]);
      comMessage(message);
      return -1;
    */
  }
  
  return 0;
}

static int parse_line(char *s)
{
  char delim[]=" \t\n";
  int argc=0;
  char *argv[256],*p;

  // remove trailing whitespaces
  while(isspace(s[0]) && s[0]!='\0') s++;

  // string as whole token !!!

  if(exec_flag && s[0]!='}') {
    comRawCommand(s);
  } else {
    // split s into tokens
    argv[argc++]=strtok(s,delim);
    while((p=strtok(NULL,delim))!=NULL)
      argv[argc++]=p;
    
    return eval_line(argc,argv);
  }
  
  
  return 0;
}

/*
  the only externally visible function
  called from main()
*/

int load_startup()
{
  FILE *f;
  char path[256],*line;
  int ret=0;

  line=malloc(512);

  // check wether resource file exists
  // first in current directory
  debmsg("checking for startup file");
  clStrcpy(path,"dinorc");
  debmsg("  in current path");
  if((f=fopen(path,"r"))==NULL) {
    clStrcpy(path,".dinorc");
    if((f=fopen(path,"r"))==NULL) {
      // then in home directory
      sprintf(path,"%s/.dinorc",getenv("HOME"));
      debmsg("  in home dir");
      if((f=fopen(path,"r"))==NULL) {
	// none found
	debmsg("no startup file found");
	return -1;
      }
    }
  }

  // read in lines and parse them
  exec_flag=0;
  while (fgets(line,511,f)!=NULL) {
    if(parse_line(line)<0) {
      ret=-1;
      break;
    }
  }

  fclose(f);
  free(line);

  return ret;
}
