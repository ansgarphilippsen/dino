/*************************************
  startup.c

  all global structs can be modified
  by a resource file upon startup

  TODO!!!
  
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

static int eval_line(int argc, char **argv, struct COM_PARAMS *params)
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
    if(clStrcmp(argv[0],"stereo")) {
      if(clStrcmp(argv[1],"1") ||
	 clStrcmp(argv[1],"on")) {
	params->stereo_flag=1;
      } else {
	params->stereo_flag=0;
      }
    } else if(clStrcmp(argv[0],"mouse_rot_scale")) {
      params->mouse_rot_scale = atof(argv[1]);
    } else if(clStrcmp(argv[0],"mouse_tra_scale")) {
      params->mouse_tra_scale = atof(argv[1]);
    } else if(clStrcmp(argv[0],"sb_rot_scale")) {
      params->sb_rot_scale = atof(argv[1]);
    } else if(clStrcmp(argv[0],"sb_tra_scale")) {
      params->sb_tra_scale = atof(argv[1]);
    } else if(clStrcmp(argv[0],"sb_thres")) {
      params->sb_thres = atof(argv[1]);
    } else if(clStrcmp(argv[0],"dials_rot_scale")) {
      params->dials_rot_scale = atof(argv[1]);
    } else if(clStrcmp(argv[0],"dials_tra_scale")) {
      params->dials_tra_scale = atof(argv[1]);
    } else if(clStrcmp(argv[0],"trans_limit")) {
      if(argc==7) {
	params->trans_limit_flag=1;
	for(i=0;i<6;i++) { 
	  params->trans_limit[i]=atof(argv[i+1]);
	}
      } else {
	fprintf(stderr,"startup parameter 'trans_limit' requires 6 parameters\n");
      }
    } else {
      // unknown keyword
      //fprintf(stderr,"startup: unknown keyword ignored: %s\n",argv[0]);
    }
  }
  
  return 0;
}

static int parse_line(char *s, struct COM_PARAMS *params)
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
    if(clStrlen(argv[0])>0) {
      return eval_line(argc,argv, params);
    }
  }
  
  
  return 0;
}

/*
  the only externally visible function
  called from main()
*/

int load_startup(struct COM_PARAMS *params)
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
    if(parse_line(line, params)<0) {
      ret=-1;
      break;
    }
  }

  fclose(f);
  free(line);

  return ret;
}
