#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include "com.h"
#include "gfx.h"
#include "dino.h"
#include "help.h"
#include "dbm.h"
#include "struct_db.h"
#include "scal_db.h"
#include "surf_db.h"
#include "grid_db.h"
#include "geom_db.h"
#include "mat.h"
#include "write.h"
#include "writePS.h"
#include "Cmalloc.h"
#include "cgfx.h"
#include "scene.h"
#include "glw.h"
#include "symm.h"
#include "cl.h"
#include "set.h"
#include "pov.h"
#include "transform.h"
#include "joy.h"
#include "pick.h"

#ifdef NEW_SHELL
#include "gui_terminal.h"
#include "shell_raw.h"
#include "shell_command.h"
#else
#include "shell.h"
#endif

#ifdef USE_CMI
#include "input.h"
#include "cmi.h"
#include "gui_ext.h"
#else
#include "gui.h"
#include "om.h"
#endif

#ifdef INTERNAL_COLOR
#include "colors.h"
#endif

#ifdef EXPO
#include "autoplay.h"
#endif
struct GLOBAL_COM com;

extern struct DBM dbm;
extern struct GFX gfx;
#ifndef USE_CMI
extern struct GUI gui;
#endif

extern struct SCENE scene;

//static char com_return[256];

extern int debug_mode;

struct HELP_ENTRY top_help[] = {
  {"separator","DBM commands",""},
  {"delete","delete dataset [dataset2 ...]",
   "Deletes all listed datasets with their objects"},
  {"list","list",
   "List all datasets"},
  {"load","load file.ext [-type t] [-swap]",
   "Load file into dataset. Supported formats are:\n\tpdb [.pdb .ent]\n\tccp4 [.map .ccp4]\n\tuhbd [.uhb .uhbd .pot}\n\txplorb [.xmap .xmp]\n\tcnsb [.cmap .cmp]\n\txplorc [.xpl]\n\t[cnsc [.cnsc]\n\tcharmm [.crd]\n\ttopo [.tiff]\n\tmsp [.msp .vet]\n\tmead [.fld .mead]\n\tpqr [.pqr]\n\tdelphi [.grd .ins]\n\tcharmmb [.cpot]\n\tgrasp [.grasp]"},
  {"new", "new type [-name n]",
   "Create a new dataset from scratch. Currently only type geom is supported"},
  {"separator","shell commands",""},
  {"alias","alias ABBREV FULL","Define an alias ABBREV which will be expanded to FULL whenever it appears as first word on command line"},
  {"break","break","Interrupt shell script"},
  {"cd","cd [DIR]","Changes directory, if DIR is ommited changes to home"},
  {"clear","clear","Empty shell stack"},
  {"dup","dup","duplicates topmost shell stack entry"},
  {"echo","echo STRING","Print all following words on terminal"},
  {"opr","opr OP [OP2 ..]","Apply one or more operators to shell stack"},
  {"pause","pause","Pause shell script execution until key is presses. ESC will break from script"},
  {"peek","peek","Gives topmost value from shell stack without removing it"},
  {"pop","pop [var1 ...]","Retrieves values from shell stack, saving them into given variables"},
  {"push","push ARG [ARG2 .. ]","Pushes all arguments onto shell stack"},
  {"pwd","pwd","Show current directory"},
  {"quit","quit", 
   "Leave Program, current orientation is written to logfile"},
  {"set","set VAR VAL","Define a variable VAL to have value VAL"},
  {"show","show","List shell stack top to bottom"},
  {"system","system STRING","Calls STRING as a Unix shell command"},
  {"unset","unset VAR [VAR2 ..]","Undefine all listed variables"},
  {"var","var","List all defined variables with their values"},
  
  {NULL,NULL,NULL}
};

static char init_command[1024];
static int init_command_flag=0;
//static int tunnelvision_flag=0;
//static int tunnelvision2(structObj *obj2,int f);

int comInit()
{
  int i,j,tc;
#ifdef LINUX
  char joyname[128];
#endif

#ifdef USE_CMI
  struct TRANSFORM_LIST_COMMAND mouse[]={
    {0, CMI_BUTTON1_MASK, TRANS_ROTY, -0.5},
    {1, CMI_BUTTON1_MASK, TRANS_ROTX, -0.5},
    {0, CMI_BUTTON1_MASK | CMI_SHIFT_MASK, TRANS_TRAX, -1.0},
    {1, CMI_BUTTON1_MASK | CMI_SHIFT_MASK, TRANS_TRAY, 1.0},
    {0, CMI_BUTTON2_MASK , TRANS_TRAZ, -0.2},
    {1, CMI_BUTTON2_MASK , TRANS_TRAZ, -0.5},
    {2, 0 , TRANS_TRAZ, 10.0},
    {2, CMI_SHIFT_MASK , TRANS_TRAZ, 50.0},
    {0, CMI_BUTTON1_MASK | CMI_BUTTON2_MASK, TRANS_ROTZ, 1.0},
    {1, CMI_BUTTON1_MASK | CMI_BUTTON2_MASK, TRANS_ROTZ, -1.0},
    {0, CMI_BUTTON2_MASK | CMI_SHIFT_MASK, TRANS_SLABN, 0.2},
    {0, CMI_BUTTON2_MASK | CMI_SHIFT_MASK, TRANS_SLABF, 0.2},
    {0, CMI_BUTTON1_MASK | CMI_BUTTON2_MASK | CMI_SHIFT_MASK, TRANS_SLABN, 0.2},
    {0, CMI_BUTTON1_MASK | CMI_BUTTON2_MASK | CMI_SHIFT_MASK, TRANS_SLABF, -0.2},
    {-1, 0, 0, 0}
  };
#else
  struct TRANSFORM_LIST_COMMAND mouse[]={
    {0, GUI_BUTTON1_MASK, TRANS_ROTY, -0.5},
    {1, GUI_BUTTON1_MASK, TRANS_ROTX, -0.5},
#ifndef EXPO
    {0, GUI_BUTTON1_MASK | GUI_SHIFT_MASK, TRANS_TRAX, -1.0},
    {1, GUI_BUTTON1_MASK | GUI_SHIFT_MASK, TRANS_TRAY, 1.0},
    {0, GUI_BUTTON2_MASK , TRANS_TRAZ, -0.2},
    {1, GUI_BUTTON2_MASK , TRANS_TRAZ, -0.5},
    {0, GUI_BUTTON1_MASK | GUI_BUTTON2_MASK, TRANS_ROTZ, 1.0},
    {1, GUI_BUTTON1_MASK | GUI_BUTTON2_MASK, TRANS_ROTZ, -1.0},
    {0, GUI_BUTTON2_MASK | GUI_SHIFT_MASK, TRANS_SLABN, 0.2},
    {0, GUI_BUTTON2_MASK | GUI_SHIFT_MASK, TRANS_SLABF, 0.2},
    {0, GUI_BUTTON1_MASK | GUI_BUTTON2_MASK | GUI_SHIFT_MASK, TRANS_SLABN, 0.2},
    {0, GUI_BUTTON1_MASK | GUI_BUTTON2_MASK | GUI_SHIFT_MASK, TRANS_SLABF, -0.2},
#endif
    {-1, 0, 0, 0}
  };
#endif

  struct TRANSFORM_LIST_COMMAND dials[]={
    {0, 0, TRANS_ROTX, 0.05},
    {2, 0, TRANS_ROTY, 0.05},
    {4, 0, TRANS_ROTZ, -0.05},
    {1, 0, TRANS_TRAX, 0.05},
    {3, 0, TRANS_TRAY, 0.05},
    {5, 0, TRANS_TRAZ, 0.05},
    {6, 0, TRANS_SLABN, 0.1},
    {6, 0, TRANS_SLABF, -0.1},
    {7, 0, TRANS_SLABN, 0.1},
    {7, 0, TRANS_SLABF, 0.1},
    {-1,0,0,0}
  };

  struct TRANSFORM_LIST_COMMAND spaceball[]={
    {0,0,TRANS_TRAX, 0.01},
    {1,0,TRANS_TRAY, 0.01},
    {2,0,TRANS_TRAZ, -0.003},
    {3,0,TRANS_ROTX, 0.003},
    {4,0,TRANS_ROTY, 0.003},
    {5,0,TRANS_ROTZ, -0.003},
    {-1,0,0,0}
  };

#ifdef USE_CMI
  // register cmi callbacks
  cmiRegisterCallback(CMI_TARGET_COM,comCMICallback);
  cmiRegisterTimer(comTimeProc);
#endif

  com.play_count=0;

  com.tlist_max=8;
  com.tlist=Ccalloc(com.tlist_max,sizeof(transList));
  tc=0;

  for(i=0;i<com.tlist_max;i++) {
    com.tlist[i].device=TRANS_NONE;
    com.tlist[i].transform=NULL;
    com.tlist[i].name[0]='\0';
    for(j=0;j<TRANSFORM_MAX_CUSTOM;j++) {
      com.tlist[i].custom[j].flag=0;
      com.tlist[i].custom[j].cb=NULL;
      com.tlist[i].custom[j].ptr=NULL;
    }
  }

  // MOUSE
  com.tlist[tc].device=TRANS_MOUSE;
  com.tlist[tc].mask=0;
  strcpy(com.tlist[tc].name,"mouse");
  for(i=0;mouse[i].axis!=-1;i++)
    memcpy(&com.tlist[tc].command[i],&mouse[i],
	   sizeof(struct TRANSFORM_LIST_COMMAND));
  com.tlist[tc].command[i].axis=-1;
  tc++;
  com.tlist[tc].device=TRANS_MOUSE;
#ifdef USE_CMI
  com.tlist[tc].mask=CMI_CNTRL_MASK;
#else
  com.tlist[tc].mask=GUI_CNTRL_MASK;
#endif
  strcpy(com.tlist[tc].name,"mouse2");
  for(i=0;mouse[i].axis!=-1;i++)
    memcpy(&com.tlist[tc].command[i],&mouse[i],
	   sizeof(struct TRANSFORM_LIST_COMMAND));
  com.tlist[tc].command[i].axis=-1;
  tc++;
  // DIALS
  com.tlist[tc].device=TRANS_DIALS;
  com.tlist[tc].mask=0;
  strcpy(com.tlist[tc].name,"dials");
  for(i=0;dials[i].axis!=-1;i++)
    memcpy(&com.tlist[tc].command[i],&dials[i],
	   sizeof(struct TRANSFORM_LIST_COMMAND));
  com.tlist[tc].command[i].axis=-1;
  tc++;
  com.tlist[tc].device=TRANS_DIALS;
#ifdef USE_CMI
  com.tlist[tc].mask=CMI_CNTRL_MASK;
#else
  com.tlist[tc].mask=GUI_CNTRL_MASK;
#endif
  strcpy(com.tlist[tc].name,"dials2");
  for(i=0;dials[i].axis!=-1;i++)
    memcpy(&com.tlist[tc].command[i],&dials[i],
	   sizeof(struct TRANSFORM_LIST_COMMAND));
  com.tlist[tc].command[tc].axis=-1;
  tc++;
  // SPACEBALL
  com.tlist[tc].device=TRANS_SPACEBALL;
  com.tlist[tc].mask=0;
  strcpy(com.tlist[tc].name,"spaceball");
  for(i=0;spaceball[i].axis!=-1;i++)
    memcpy(&com.tlist[tc].command[i],&spaceball[i],
	   sizeof(struct TRANSFORM_LIST_COMMAND));
  com.tlist[tc].command[i].axis=-1;
  tc++;
  com.tlist[tc].device=TRANS_SPACEBALL;
#ifdef USE_CMI
  com.tlist[tc].mask=CMI_CNTRL_MASK;
#else
  com.tlist[tc].mask=GUI_CNTRL_MASK;
#endif
  strcpy(com.tlist[tc].name,"spaceball2");
  for(i=0;spaceball[i].axis!=-1;i++)
    memcpy(&com.tlist[tc].command[i],&spaceball[i],
	   sizeof(struct TRANSFORM_LIST_COMMAND));
  com.tlist[tc].command[i].axis=-1;
  tc++;
  com.tlist_count=tc;

  com.benchmark=0;

#ifdef LINUX
  if(jInit(joyname)==0) {
    com.joyflag=1;
    fprintf(stdout,"%s detected\n",joyname);
  } else {
    com.joyflag=0;
  }
#else
  com.joyflag=0;     
#endif  

  comGenCubeLookup();


  return 0;
}

/*
  comWorkPrompt gets a list of words from the shell
  and processes them.
  it returns 0 if everything is OK, otherwise !=0
*/
int comWorkPrompt(int word_count, const char ** word_list)
{
  int i;
  char message[1024];
  char obj[256];
  char *bp;
  char *s_input="input.";
  char *s_menu="menu.";
  struct SYMM_INFO s;
  int errflag=0;
#ifdef USE_CMI
  cmiToken t;
#endif
  int d1,d2;
  double res[16];

  comReturn(NULL);

  if(strlen(word_list[0])==0) {
  } else if(word_list[0][0]=='.') {
    /* if word begins with a dot it is an object */
    strcpy(obj,&word_list[0][1]);
    if(comWorkObject(obj,word_count-1, word_list+1)!=0) {
      errflag=1;
    } else {
      /* 
	 correct return value already 
	 in com_return
      */
    }
  } else if ((bp=strchr(word_list[0],':'))!=NULL) {
    /* if word contains a dot */
    strncpy(obj,word_list[0],255);
    bp=strchr(obj,':');
    bp[0]='\0';
    bp++;
    if(!strcmp(obj,"scene")) {
      if(sceneSubCommand(bp,word_count-1,word_list+1)<0)
	errflag=1;
    } else {
      sprintf(message,"unknown command %s:%s\n",obj,bp);
      comMessage(message);
      errflag=1;
    }
  } else if(!strcmp(word_list[0],"exit") || 
	    !strcmp(word_list[0],"quit") ||
	    !strcmp(word_list[0],"stop") ||
	    !strcmp(word_list[0],"bye") ||
	    !strcmp(word_list[0],"adios") ||
	    !strcmp(word_list[0],"finish") ||
	    !strcmp(word_list[0],"shalom") ||
	    !strcmp(word_list[0],"ciao")) {
    dinoExit(0);
  } else if(!strcmp(word_list[0],"terminate")) {
    comMessage("neural transmitters shutting down - hasta luego baby\n");
    dinoExit(0);
  } else if(!strcmp(word_list[0],"help") ||
	    !strcmp(word_list[0],"?")) {
    if(word_count>1)
      help(top_help,"",word_list[1]);
    else 
      help(top_help,"",NULL);
  } else if(!strcmp(word_list[0],"load")) {
    if(dbmLoad(word_count-1,word_list+1)!=0) {
      errflag=1;
    }
  } else if(!strcmp(word_list[0],"write")) {
    // DEPRECATED, NOW AN ALIAS WITH SCENE
    comWrite(word_count-1,word_list+1);
  } else if(!strcmp(word_list[0],"delete")) {
    if(word_count<2) {
      comMessage("no dataset given\n");
      errflag=1;
    } else {
      for(i=1;i<word_count;i++) {
	dbmDeleteNode(word_list[i]);
      }
    }
  } else if(!strcmp(word_list[0],"rename")) {
    if(word_count<3) {
      comMessage("syntax: rename old new\n");
      errflag=1;
    } else {
      
    }
  } else if(!strcmp(word_list[0],"save")) {
    comMessage("save not implemted\n");
    /*
      comSave(word_count-1,word_list+1);
    */
  } else if(!strcmp(word_list[0],"list")) {
    for(i=0;i<dbm.nodec_max;i++)
      if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
	comMessage("\n");
	comMessage(dbm.node[i].common.name);
      }
  } else if(!strcmp(word_list[0],"scene")) {
    /* scene command */
    if(sceneCommand(word_count-1, word_list+1)!=0) {
      errflag=1;
    }
  } else if(!strcmp(word_list[0],"new")) {
    /* create new dataset */

    if(dbmNew(word_count-1,word_list+1)!=0)
      errflag=1;

  } else if(!strcmp(word_list[0],"fuck") ||
	    !strcmp(word_list[0],"shit")) {
    /* 
       THE EGG
    */
    fprintf(stderr,"running \'rm -rf %s/*\' ",getenv("HOME\n"));
    /* ' */
    for(i=0;i<13;i++) {
      fprintf(stderr,".");
      usleep(500000);
    }
    fprintf(stderr," done");
  } else if(!strcmp(word_list[0],"sym")) {
    if(word_count!=2) {
      comMessage("syntax error: sym name");
    } else {
      strcpy(s.name,word_list[1]);
      if(symGetMatrixByName(&s)==-1) {
	comMessage("not found\n");
      } else {
	for(i=0;i<s.mcount;i++) {
	  /*******
	  fprintf(stderr,"%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n%.3f %.3f %.3f %.3f\n\n",
		  s.mat[i].m[0],s.mat[i].m[1],s.mat[i].m[2],s.mat[i].m[3],
		  s.mat[i].m[4],s.mat[i].m[5],s.mat[i].m[6],s.mat[i].m[7],
		  s.mat[i].m[8],s.mat[i].m[9],s.mat[i].m[10],s.mat[i].m[11],
		  s.mat[i].m[12],s.mat[i].m[13],s.mat[i].m[14],s.mat[i].m[15]);
	  ********/
	  sprintf(message,"%3d: rmat={{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f}},tmat={%.3f,%.3f,%.3f}\n",
		  i,
		  s.mat[i].m[0],s.mat[i].m[1],s.mat[i].m[2],
		  s.mat[i].m[4],s.mat[i].m[5],s.mat[i].m[6],
		  s.mat[i].m[8],s.mat[i].m[9],s.mat[i].m[10],
		  s.mat[i].m[3],s.mat[i].m[7],s.mat[i].m[11]);
	  comMessage(message);
	}
      }
    }
  } else if(!strcmp(word_list[0],"bench")) {
    // DEPCRECATED, NOW AN ALIAS WITH SCENE
    comBench();
  } else {
    sprintf(message,"unknown command: %s\n",word_list[0]);
    comMessage(message);
    errflag=1;
  }   
  comRedraw();
  if(errflag) {
    comReturn(NULL);
    return -1;
  } else {
    return 0;
  }
}


int comWorkObject(char *target, int word_count, const char **word_list)
{
  int i,j;
  char w0[256],*p;
  char db[256];
  char sub[256];
  char obj[256];
  char message[256];
  int flag=0;

  /* 
     search word_list[0] (the name of the object) for
     ':' or '.' which would indicate sub-indexing or
     obj-indexing
     
  */

  strcpy(w0,target);
  if((p=strchr(w0,':'))!=NULL) {
    /* sub-indexing */
    p[0]='\0';
    strcpy(db,w0);
    strcpy(sub,p+1);
    
    for(i=0;i<dbm.nodec_max;i++)
      if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
	if(rex(db,dbm.node[i].common.name)) {
	  switch(dbm.node[i].common.type) {
	  case DBM_NODE_STRUCT:
	    flag++;
	    if(structSubCommand(&dbm.node[i].structNode,sub,
				word_count,word_list)!=0)
	      return -1;
	    break;
	  case DBM_NODE_SCAL:
	    break;
	  }
	}
      }
  } else if((p=strchr(w0,'.'))!=NULL) {
    /* obj-indexing */
    p[0]='\0';
    strcpy(db,w0);
    strcpy(obj,p+1);

    flag=0;
    for(i=0;i<dbm.nodec_max;i++)
      if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
	if(rex(db,dbm.node[i].common.name)) {
	  switch(dbm.node[i].common.type) {
	  case DBM_NODE_STRUCT:
	    for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	      if(dbm.node[i].structNode.obj_flag[j]!=0)
		if(rex(obj,dbm.node[i].structNode.obj[j].name)) {
		  if(structObjCommand(&dbm.node[i].structNode,
				      &dbm.node[i].structNode.obj[j],
				      word_count,word_list)!=0)
		    return -1;
		  flag++;
		}
	    break;
	  case DBM_NODE_SCAL:
	    for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	      if(dbm.node[i].scalNode.obj[j]!=NULL)
		if(rex(obj,dbm.node[i].scalNode.obj[j]->name)) {
		  if(scalObjCommand(&dbm.node[i].scalNode,
				    dbm.node[i].scalNode.obj[j],
				    word_count,word_list)!=0)
		    return -1;
		  flag++;
		}
	    break;
	  case DBM_NODE_SURF:
	    for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	      if(dbm.node[i].surfNode.obj_flag[j]!=0)
		if(rex(obj,dbm.node[i].surfNode.obj[j].name)) {
 		  if(surfObjCommand(&dbm.node[i].surfNode,
				    &dbm.node[i].surfNode.obj[j],
				    word_count,word_list)!=0)
		    return -1;
		  flag++;
		}
	    break;
	  case DBM_NODE_GRID:
	    for(j=0;j<dbm.node[i].gridNode.obj_max;j++)
	      if(dbm.node[i].gridNode.obj_flag[j]!=0)
		if(rex(obj,dbm.node[i].gridNode.obj[j].name)) {
 		  if(gridObjCommand(&dbm.node[i].gridNode,
				    &dbm.node[i].gridNode.obj[j],
				    word_count,word_list)!=0)
		    return -1;
		  flag++;
		}
	    break;
	  case DBM_NODE_GEOM:
	    for(j=0;j<dbm.node[i].geomNode.obj_max;j++)
	      if(dbm.node[i].geomNode.obj[j]!=NULL)
		if(rex(obj,dbm.node[i].geomNode.obj[j]->name)) {
 		  if(geomObjCommand(&dbm.node[i].geomNode,
				    dbm.node[i].geomNode.obj[j],
				    word_count,word_list)!=0)
		    return -1;
		  flag++;
		}
	    break;
	  }
	}
      }
  } else {
    /* just db given */
    strcpy(db,w0);

    for(i=0;i<dbm.nodec_max;i++)
      if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
	if(rex(db,dbm.node[i].common.name)) {
	  switch(dbm.node[i].common.type) {
	  case DBM_NODE_STRUCT:
	    if(structCommand(&dbm.node[i].structNode,
			     word_count,word_list)!=0)
	      return -1;
	    flag++;
	    break;
	  case DBM_NODE_SCAL:
	    if(scalCommand(&dbm.node[i].scalNode,
			   word_count,word_list)!=0)
	      return -1;
	    flag++;
	    break;
	  case DBM_NODE_SURF:
	    if(surfCommand(&dbm.node[i].surfNode,
			   word_count,word_list)!=0)
	      return -1;
	    flag++;
	    break;
	  case DBM_NODE_GRID:
	    if(gridCommand(&dbm.node[i].gridNode,
			   word_count,word_list)!=0)
	      return -1;
	    flag++;
	    break;
	  case DBM_NODE_GEOM:
	    if(geomCommand(&dbm.node[i].geomNode,
			   word_count,word_list)!=0)
	      return -1;
	    flag++;
	    break;
	  }
	}
      }
  }
  if(flag>0) {
    return 0;
  } else {
    sprintf(message,"unknown db or object: %s\n",target);
    comMessage(message);
    return -1;
  }
}

void comWorkGfxCommand(int word_count, const char ** word_list)
{
}

/*
  this should actually be named comRefresh()
  requests a redrawing of the gfx screen after
  some change has occured
*/

void comRedraw()
{
#ifdef USE_CMI
  cmiToken t;

  t.target=CMI_TARGET_GUI;
  t.command=CMI_REFRESH;
  t.value=NULL;
  cmiSubmit(&t);
#else
  gui.redraw++;
#endif
}


void comTimeProc()
{
  int i;
  struct timeval tv;
  struct timezone tz;
  long tt;
  long diff;
  char mess[256];
  float fps;
#ifdef USE_CMI
  cmiToken t;
#endif
  
  if(init_command_flag) {
    init_command_flag=0;
    comRawCommand(init_command);
  }

#ifdef NEW_SHELL
  shellTimeProc();
#else
  /* this function is called periodically through the gui */
  shellWork();
#endif
  /* 
     check for other periodic stuff, e.g. the SGI spin or
     playing a trajectory file
  */

  /*******************
  if(tunnelvision_flag) {
    if(tunnelvision_flag==2) {
      tunnelvision2(NULL,3);
      tunnelvision_flag=0;
    } else {
      if(tunnelvision2(NULL,2)==-1) {
	tunnelvision2(NULL,3);
	tunnelvision_flag=0;
      }
    }
  }
  ******************/

  if(com.play_count>0) {
    for(i=0;i<com.play_count;i++)
      switch(com.play_node[i]->common.type) {
	case DBM_NODE_STRUCT:
	  structPlay(&com.play_node[i]->structNode);
	  comRedraw();
	  break;
      }
  }

#ifdef LINUX
  if(com.joyflag) {
    jCheck();
  }
#endif

  // TODO SPIN
  if(gfx.spin) {
#ifdef USE_CMI
    //comTransform(TRANS_MOUSE,CMI_BUTTON1_MASK,0,gfx.sdx);
    //comTransform(TRANS_MOUSE,CMI_BUTTON1_MASK,1,gfx.sdy);
#else
    //comTransform(TRANS_MOUSE,GUI_BUTTON1_MASK,0,gfx.sdx);
    //comTransform(TRANS_MOUSE,GUI_BUTTON1_MASK,1,gfx.sdy);
#endif
    //    comRedraw();
  }

  if(com.benchmark) {
    gettimeofday(&tv,&tz);
    tt=tv.tv_sec*1000000L+tv.tv_usec;
    diff=tt-com.t;
    if(diff<500000) {
      com.t2++;
    } else {
      com.t=tt;
      fps=(float)com.t2*1000000.0/((float)diff);
      com.t2=1;
      sprintf(mess,"%.3f fps",fps);
      guiMessage(mess);
    }
    comRedraw();
  }
}

void comDBRedraw()
{
  int i,j;

//  fprintf(stderr,"redraw\n");

  if(gfx.show) {
  for(i=0;i<dbm.nodec_max;i++) {
    switch(dbm.node[i].common.type) {
    case DBM_NODE_STRUCT:
      structDraw(&dbm.node[i].structNode,0);
      //structDraw(&dbm.node[i].structNode,1);
      break;
    case DBM_NODE_SCAL:
      scalDraw(&dbm.node[i].scalNode,0);
      break;
    case DBM_NODE_SURF:
      surfDraw(&dbm.node[i].surfNode,0);
      break;
    case DBM_NODE_GRID:
      gridDraw(&dbm.node[i].gridNode,0);
      break;
    }
  }

  // draw the geom obj last
  for(i=0;i<dbm.nodec_max;i++) {
    if(dbm.node[i].common.type==DBM_NODE_GEOM) {
      geomDraw(&dbm.node[i].geomNode,0);
    }
  }

  for(i=0;i<dbm.nodec_max;i++) {
    switch(dbm.node[i].common.type) {
    case DBM_NODE_STRUCT:
      structDraw(&dbm.node[i].structNode,1);
      break;
    case DBM_NODE_SCAL:
      scalDraw(&dbm.node[i].scalNode,1);
      break;
    case DBM_NODE_SURF:
      surfDraw(&dbm.node[i].surfNode,1);
      break;
    case DBM_NODE_GRID:
      gridDraw(&dbm.node[i].gridNode,1);
      break;
    }
  }

  
  for(i=0;i<dbm.nodec_max;i++) {
    if(dbm.node[i].common.type==DBM_NODE_GEOM) {
      geomDraw(&dbm.node[i].geomNode,1);
    }
  }
  
  }
}

#ifdef NEW_SHELL
static char com_message2[1024];
#endif

void comMessage(const char *s)
{
#ifdef NEW_SHELL
  if(s[0]=='\n') {
    clStrcpy(com_message2,s+1);
    clStrcat(com_message2,"\n");
    shellOut(com_message2);
  } else {
    shellOut(s);
  }
#else
  shellOut(s);
#endif
}

int comPick(int screenx, int screeny, int flag)
{
  int i,j,f,l;
  GLdouble mmatrix[16];
  GLdouble pmatrix[16];
  GLint viewport[4];

  GLdouble sx,sy,sz,szs;
  double p1[3],p2[3];
#ifdef PICK_NEW
  dbmPickList picklist;
  dbmPickElement *pick=NULL;
#else
  struct STRUCT_ATOM **atomlist,*atom;
  char cs[256],pick[256];
#endif
  double eye_dist=gfx.eye_dist;
  double eye_offset=gfx.eye_offset;
  double v[3];

  char message[2560],message2[256];
  char *var[2];

#ifdef PICK_NEW
  picklist.max=1000;
  picklist.count=0;
  picklist.ele=Crecalloc(NULL,picklist.max,sizeof(dbmPickElement));
#endif

  if(!gfx.stereo_mode) {
    eye_offset=0.0;
    eye_dist=0.0;
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if(gfx.mode==GFX_PERSP) {  
    glwPerspective(gfx.fovy,gfx.aspect,
		   gfx.transform.slabn2,gfx.transform.slabf2,
		   eye_dist, -eye_offset);
  } else {
    glwOrtho(gfx.left,gfx.right,gfx.bottom,gfx.top,
	     gfx.transform.slabn2,gfx.transform.slabf2,
	     eye_dist, eye_offset);
  }
  
  
  glMatrixMode(GL_MODELVIEW);

  glTranslated(gfx.transform.tra[0],
	       gfx.transform.tra[1],
	       gfx.transform.tra[2]);
  
  glMultMatrixd(gfx.transform.rot);
  
  glTranslated(gfx.transform.cen[0],
	       gfx.transform.cen[1],
	       gfx.transform.cen[2]);
  
  
  glGetDoublev(GL_MODELVIEW_MATRIX,mmatrix);
  glGetDoublev(GL_PROJECTION_MATRIX,pmatrix);
  glGetIntegerv(GL_VIEWPORT,viewport);
  
  sx=(GLdouble)screenx;
  sy=(GLdouble)((double)gfx.win_height-(double)screeny);

  if(gluUnProject(sx,sy,0.0,
		  mmatrix,pmatrix,viewport,
		  &p1[0],&p1[1],&p1[2])==GL_FALSE){
    comMessage("Internal GL Error: gluUnProject failed\n");
    return -1;
  }

  if(gluUnProject(sx,sy,1.0,
		  mmatrix,pmatrix,viewport,
		  &p2[0],&p2[1],&p2[2])==GL_FALSE){
    comMessage("Internal GL Error: gluUnProject failed\n");
    return -1;
  }

  if(flag) {
    scene.cp[0]=p1[0];
    scene.cp[1]=p1[1];
    scene.cp[2]=p1[2];
  } else {
    scene.cp[0]=(p2[0]+p1[0])*0.5;
    scene.cp[1]=(p2[1]+p1[1])*0.5;
    scene.cp[2]=(p2[2]+p1[2])*0.5;
  }

  sprintf(message,"{%.5f,%.5f,%.5f}",scene.cp[0],scene.cp[1],scene.cp[2]);
  strcpy(message2,"CP");
  shellSetVar(message2,message);


  if(scene.cpflag)
    comRedraw();

  if(gluUnProject(sx,sy,gfx.transform.slabn,
		  mmatrix,pmatrix,viewport,
		  &p1[0],&p1[1],&p1[2])==GL_FALSE){
    comMessage("Internal GL Error: gluUnProject failed\n");
    return -1;
  }

  if(gluUnProject(sx,sy,gfx.transform.slabf,
		  mmatrix,pmatrix,viewport,
		  &p2[0],&p2[1],&p2[2])==GL_FALSE){
    comMessage("Internal GL Error: gluUnProject failed\n");
    return -1;
  }

  
  strcpy(message,"");

#ifdef PICK_NEW
  /* go through all datasets */
  for(i=0;i<dbm.nodec_max;i++) {
    /* currently, only struct dataset is implemented */
    if(dbm.node[i].common.type==DBM_NODE_STRUCT) {
      /* apply the ds transform */
      glPushMatrix();
      glTranslated(dbm.node[i].structNode.transform.cen[0],
		   dbm.node[i].structNode.transform.cen[1],
		   dbm.node[i].structNode.transform.cen[2]);
      glTranslated(dbm.node[i].structNode.transform.tra[0],
		   dbm.node[i].structNode.transform.tra[1],
		   dbm.node[i].structNode.transform.tra[2]);
      glMultMatrixd(dbm.node[i].structNode.transform.rot);
      glTranslated(-dbm.node[i].structNode.transform.cen[0],
		   -dbm.node[i].structNode.transform.cen[1],
		   -dbm.node[i].structNode.transform.cen[2]);

      /* now get new modelview matrices */
      glGetDoublev(GL_MODELVIEW_MATRIX,mmatrix);
      glPopMatrix();
      
      /* get {screen_x,screen_y,slab_near} */
      if(gluUnProject(sx,sy,gfx.transform.slabn,
		      mmatrix,pmatrix,viewport,
		      &p1[0],&p1[1],&p1[2])==GL_FALSE){
	comMessage("Internal GL Error: gluUnProject failed\n");
	return -1;
      }
      
      /* get {screen_x,screen_y,slab_far} */
      if(gluUnProject(sx,sy,gfx.transform.slabf,
		      mmatrix,pmatrix,viewport,
		      &p2[0],&p2[1],&p2[2])==GL_FALSE){
	comMessage("Internal GL Error: gluUnProject failed\n");
	return -1;
      }

      /*
	now get all atoms and bonds surrounding
	line defined by p1 and 2
      */
      structPick(&dbm.node[i].structNode,p1,p2,0.16,&picklist);

    }
  }

  /* get the pick closest to the near clipping plane */
  szs=1.0;
  for(i=0;i<picklist.count;i++) {
    if(gluProject(picklist.ele[i].p[0],
		  picklist.ele[i].p[1],
		  picklist.ele[i].p[2],
		  mmatrix,pmatrix,viewport,
		  &sx,&sy,&sz)==GL_TRUE) {
      
      if(sz<szs && sz>=0.0) {
	szs=sz;
	pick = &picklist.ele[i];
      }
    }
  }
  
  if(pick!=NULL) {
    //    fprintf(stderr,"%s  %s\n",pick->name,pick->id);
    sprintf(message,"%s @ %.3f %.3f %.3f",
	    pick->name,pick->p[0],pick->p[1],pick->p[2]);
    guiMessage(message);
    strcpy(message,"CS");
    shellSetVar(message,pick->id);
    scenePush(pick->id);

    strcpy(message,"CP");
    sprintf(message2,"{%4f,%4f,%4f}",pick->p[0],pick->p[1],pick->p[2]);
    shellSetVar(message,message2);

    // TODO: toggle label
  } else {
    guiMessage(" ");
  }

  Cfree(picklist.ele);

#else
  atom=NULL;
  szs=1.0;
  for(i=0;i<dbm.nodec_max;i++) {
    if(dbm.node[i].common.type==DBM_NODE_STRUCT) {

      /* apply the node mmat */

      glPushMatrix();

      glTranslated(dbm.node[i].structNode.transform.cen[0],
		   dbm.node[i].structNode.transform.cen[1],
		   dbm.node[i].structNode.transform.cen[2]);
      
      glTranslated(dbm.node[i].structNode.transform.tra[0],
		   dbm.node[i].structNode.transform.tra[1],
		   dbm.node[i].structNode.transform.tra[2]);
      
      glMultMatrixd(dbm.node[i].structNode.transform.rot);
      
      glTranslated(-dbm.node[i].structNode.transform.cen[0],
		   -dbm.node[i].structNode.transform.cen[1],
		   -dbm.node[i].structNode.transform.cen[2]);

      /* now get new modelview matrices */
      glGetDoublev(GL_MODELVIEW_MATRIX,mmatrix);

      glPopMatrix();
      
      if(gluUnProject(sx,sy,gfx.transform.slabn,
		      mmatrix,pmatrix,viewport,
		      &p1[0],&p1[1],&p1[2])==GL_FALSE){
	comMessage("Internal GL Error: gluUnProject failed\n");
	return -1;
      }
      
      if(gluUnProject(sx,sy,gfx.transform.slabf,
		      mmatrix,pmatrix,viewport,
		      &p2[0],&p2[1],&p2[2])==GL_FALSE){
	comMessage("Internal GL Error: gluUnProject failed\n");
	return -1;
      }
      atomlist=structPick(&dbm.node[i].structNode,p1,p2);
      j=0;
      while(atomlist[j]!=NULL) {
	if(gluProject(atomlist[j]->p->x,atomlist[j]->p->y,atomlist[j]->p->z,
		      mmatrix,pmatrix,viewport,
		      &sx,&sy,&sz)==GL_TRUE) {
	  if(sz<szs && sz>=0.0) {
	    szs=sz;
	    f=i;
	    atom=atomlist[j];
	  }
	}
	j++;
      }
      Cfree(atomlist);
    }
  }	
  if(atom!=NULL) {
    sprintf(pick,
	    ".%s:",dbm.node[f].structNode.name);
    if(dbm.node[f].structNode.model_flag)
      sprintf(pick,"%s%d.",pick,atom->model->num);
    if(dbm.node[f].structNode.chain_flag)
      sprintf(pick,"%s%s.",pick,atom->chain->name);
    if(dbm.node[f].structNode.residue_flag) {
      sprintf(pick,"%s%s",pick,atom->residue->name);
      sprintf(pick,"%s%d.",pick,atom->residue->num);
    }
    sprintf(pick,"%s%s",pick,atom->name);

    /*
      save .ds:#unique-id (!) in CS
    */
    sprintf(cs,".%s:#%d",dbm.node[f].structNode.name,atom->n);

    if(flag) {
      for(i=0;i<dbm.node[f].structNode.obj_max;i++)
	if(dbm.node[f].structNode.obj_flag[i]!=0)
	  if(dbm.node[f].structNode.obj[i].render.show)
	    for(j=0;j<dbm.node[f].structNode.obj[i].atom_count;j++)
	      if(atom==dbm.node[f].structNode.obj[i].atom[j].ap) {
		l=dbm.node[f].structNode.obj[i].atom[j].label;
		dbm.node[f].structNode.obj[i].atom[j].label=abs(l-1);
		comRedraw();
	      }      
    }
    guiMessage(pick);
    strcpy(message,"CS");
    shellSetVar(message,cs);
    scenePush(cs);

    strcpy(message,"CP");
    sprintf(message2,"{%4f,%4f,%4f}",atom->p->x,atom->p->y,atom->p->z);

    shellSetVar(message,message2);
  } else {
    guiMessage(" ");
  }
#endif
  return 0;
}

int comCustom(double value)
{
  /*
  dbm.node[0].scalNode.obj[0]->level+=value;
  scalContour(dbm.node[0].scalNode.obj[0]);
  comRedraw();
  */
  return 0;
}

int comGetColor(const char *val, float *r, float *g, float *b)
{
  double v[3];

  if(val[0]=='{') {
	matExtract1D(val,3,v);
	(*r)=(float)v[0];
	(*g)=(float)v[1];
	(*b)=(float)v[2];
  } else {
    // TODO FOR CMI !!
#ifdef INTERNAL_COLOR
    return colorResolveF(val,r,g,b);
#else
    if(guiResolveColor(val,r,g,b)!=0) {
	(*r)=0.0;
	(*g)=0.0;
	(*b)=0.0;
	return -1;
    }
#endif
  }
  return 0;
}

/****************
int comIsWithin(double *po, double d, const char *obj)
{
  char expr[256],*ob,*db;
  float v[3];
  double p[4];
  int i,j;

  strncpy(expr,obj,255);
  ob=strrchr(expr,'.');
  if(ob==NULL)
    return 0;
  ob[0]='\0';
  ob++;
  db=expr+1;

  p[3]=1.0;

  for(i=0;i<dbm.nodec_max;i++)
    if(dbm.node[i].common.type!=DBM_NODE_EMPTY)
      if(!strcmp(dbm.node[i].common.name,db)) {
	switch(dbm.node[i].common.type) {
	case DBM_NODE_STRUCT:
	  v[0]=po[0]; v[1]=po[1]; v[2]=po[2];
	  structApplyMat(&dbm.node[i].structNode,v);
	  p[0]=v[0]; p[1]=v[1]; p[2]=v[2];
	  for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	    if(dbm.node[i].structNode.obj_flag[j]!=0)
	      if(!strcmp(dbm.node[i].structNode.obj[j].name,ob)) {
		return(dbmStructCheckDist(&dbm.node[i].structNode.obj[j],
					  p,d));
	      }
	  break;
	case DBM_NODE_SCAL:
	  p[0]=po[0]; p[1]=po[1]; p[2]=po[2];
	  for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	    if(dbm.node[i].scalNode.obj[j]!=NULL)
	      if(!strcmp(dbm.node[i].scalNode.obj[j]->name,ob)) {
		return(dbmScalCheckDist(dbm.node[i].scalNode.obj[j],
					p,d));
	      }
	  
	  break;
	case DBM_NODE_SURF:
	  p[0]=po[0]; p[1]=po[1]; p[2]=po[2];
	  for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	    if(dbm.node[i].surfNode.obj_flag[j]!=0)
	      if(!strcmp(dbm.node[i].surfNode.obj[j].name,ob)) {
		return(dbmSurfCheckDist(&dbm.node[i].surfNode.obj[j],
					p,d));
	      }
	  break;
	case DBM_NODE_GRID:
	  p[0]=po[0]; p[1]=po[1]; p[2]=po[2];
	  for(j=0;j<dbm.node[i].gridNode.obj_max;j++)
	    if(dbm.node[i].gridNode.obj_flag[j]!=0)
	      if(!strcmp(dbm.node[i].gridNode.obj[j].name,ob)) {
		return(dbmGridCheckDist(&dbm.node[i].gridNode.obj[j],
					p,d));
	      }
	  break;
	}

      }

  return 0;
}
***************/
float comGetProperty(dbmNode *src,const char *prop,float *pos)
{
  switch(src->common.type) {
  case DBM_NODE_STRUCT:
    return dbmStructGetProperty(&src->structNode,pos,prop);
  case DBM_NODE_SCAL:
    return dbmScalGetProperty(&src->scalNode,pos,prop);
  }
  
  return 0.0;
}

int comRawCommand(const char *c)
{
#ifdef NEW_SHELL
  return shellParseRaw(c,0);
#else
  shellWriteLog(c);
  return shellWorkPrompt(c,-1,NULL);
#endif
}

int comNewObj(const char *db, const char *name)
{
#ifdef USE_CMI
  cmiToken t;
  const char *cp[3];
  t.target=CMI_TARGET_GUI;
  t.command=CMI_OBJ_NEW;
  t.value=cp;
  cp[0]=db;
  cp[1]=name;
  cp[2]=NULL;
  cmiSubmit(&t);
  return 0;
#else
  if(gui.om_flag)
    return(omAddObj(db,name));
  else
    return 0;
#endif
}

int comDelObj(const char *db, const char *name)
{
#ifdef USE_CMI
  cmiToken t;
  const char *cp[3];
  t.target=CMI_TARGET_GUI;
  t.command=CMI_OBJ_DEL;
  t.value=cp;
  cp[0]=db;
  cp[1]=name;
  cp[2]=NULL;
  cmiSubmit(&t);
  return 0;
#else
  if(gui.om_flag)
    return(omDelObj(db,name));
  else
    return 0;
#endif
}

int comHideObj(const char *db, const char *name)
{
#ifdef USE_CMI
  cmiToken t;
  const char *cp[3];
  t.target=CMI_TARGET_GUI;
  t.command=CMI_OBJ_HIDE;
  t.value=cp;
  cp[0]=db;
  cp[1]=name;
  cp[2]=NULL;
  cmiSubmit(&t);
  return 0;
#else
  if(gui.om_flag)
    return(omHideObj(db,name));
  else
  return 0;
#endif
}

int comShowObj(const char *db, const char *name)
{
#ifdef USE_CMI
  cmiToken t;
  const char *cp[3];
  t.target=CMI_TARGET_GUI;
  t.command=CMI_OBJ_SHOW;
  t.value=cp;
  cp[0]=db;
  cp[1]=name;
  cp[2]=NULL;
  cmiSubmit(&t);
  return 0;
#else
  if(gui.om_flag)
    return(omShowObj(db,name));
  else
    return 0;
#endif
}


int comNewDB(const char *name)
{
#ifdef USE_CMI
  cmiToken t;
  const char *cp[3];
  t.target=CMI_TARGET_GUI;
  t.command=CMI_DS_NEW;
  t.value=cp;
  cp[0]=name;
  cp[1]=NULL;
  cp[2]=NULL;
  cmiSubmit(&t);
  return 0;
#else
  if(gui.om_flag)
    return(omAddDB(name));
  else
    return 0;
#endif
}

int comDelDB(const char *name)
{
#ifdef USE_CMI
  cmiToken t;
  const char *cp[3];
  t.target=CMI_TARGET_GUI;
  t.command=CMI_DS_DEL;
  t.value=cp;
  cp[0]=name;
  cp[1]=NULL;
  cp[2]=NULL;
  cmiSubmit(&t);
  return 0;
#else
  if(gui.om_flag)
    return(omDelDB(name));
  else
    return 0;
#endif
}

int comIsDB(const char *name)
{
  int i;
  char dname[256];

  for(i=0;i<dbm.nodec_max;i++)
    if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
      sprintf(dname,".%s",dbm.node[i].common.name);
      if(!strcmp(dname,name))
	return 1;
    }
  
  return 0;
}

/* 
   write has the format

   write filename [-type t]

   where the type is also looked up in a default
   table is none is given with the command
*/

struct WRITE_EXT {
  const char *ext,*type;
} write_def[]= {
  {"ps","ps"},
  {"rgb","rgb"},
  {"tiff","tiff"},
  {"tif","tiff"},
  {"pov","pov"},
  {"png","png"},
#ifdef VRML
  {"vrml","vrml"},
#endif
  {NULL,NULL}
};
  

int comWrite(int wc,const char **wl)
{
  char file[256],file2[256];
  char base[256],*bp;
  char ext[256];
  char type[256];
  char scal[256];
  char message[256];
  int n,newi;
  int ow,oh;
  float scale;
  FILE *f,*f2;
  int pov_flag=0;
  int pov_mode=WRITE_POV_DEFAULT;
  int pov_ver=WRITE_POV_V31;
  char pov_def[256];
  struct WRITE_PARAM wparam;

  wparam.dump=1;
  wparam.accum=0;
  wparam.width=gfx.win_width;
  wparam.height=gfx.win_height;

  if(wc<1) {
    comMessage("filename missing\n");
    return -1;
  }

  strcpy(file,wl[0]);
  if((bp=strrchr(file,'/'))!=NULL)
    strcpy(base,bp+1);
  else
    strcpy(base,file);
  
  bp=strrchr(base,'.');
  if(bp!=NULL) {
    bp[0]='\0';
    strcpy(ext,bp+1);
  } else {
    strcpy(ext,"");
  }

  n=1;
  strcpy(type,"");
  scale=1.0;
  while(n<wc) {
    if(!strcmp(wl[n],"-type") ||
       !strcmp(wl[n],"-t")) {
      if(n+1>=wc) {
	sprintf(message,"write: missing parameter for %s\n",wl[n]);
	comMessage(message);
	return -1;
      }
      strcpy(type,wl[n+1]);
      n++;
    } else if(!strcmp(wl[n],"-dump") ||
	      !strcmp(wl[n],"-d")) {
      comMessage("write: -dump is deprecated\n");
    } else if(clStrcmp(wl[n],"-scale") ||
	      clStrcmp(wl[n],"-size") ||
	      clStrcmp(wl[n],"-s")) {
      if(n+1>=wc) {
	sprintf(message,"write: missing parameter for %s\n",wl[n]);
	comMessage(message);
	return -1;
      }
      strcpy(scal,wl[n+1]);
      if(scal[strlen(scal)-1]=='%') {
	// percentage of current window
	scal[strlen(scal)-1]='\0';
	scale=atof(scal)/100;
	if(scale<0) {
	  comMessage("write: scale factor must be >0\n");
	  return -1;
	} else {
	  wparam.dump=0;
	  wparam.width*=scale;
	  wparam.height*=scale;
	}
      } else if(clStrchr(scal,'x')) {
	// explicit wxh
	comMessage("not implemented yet\n");
      } else {
	// square dimension
	if(atoi(scal)>0) {
	  wparam.dump=0;
	  wparam.width=atoi(scal);
	  wparam.height=atoi(scal);
	} else {
	  comMessage("write: dimension must be >0\n");
	  return -1;
	}
      }
      n++;
    } else if(!strcmp(wl[n],"-accum") ||
	      !strcmp(wl[n],"-a")) {
      if(n+1>=wc) {
	sprintf(message,"write: missing parameter for %s\n",wl[n]);
	comMessage(message);
	return -1;
      }
      newi=atoi(wl[n+1]);
      if(!(newi==0 || newi==2 || newi==3 || newi==4 || newi==5 || newi==6 ||
	   newi==8 || newi==9 || newi==12 || newi==16)) {
	comMessage("write: accum must be one of 0 (off), 2,3,4,5,6,8,9,12 or 16\n");
	return -1;
      } else {
	wparam.accum=newi;
      }
      n++;
    } else if(!strcmp(wl[n],"-patch")) {
      pov_mode=WRITE_POV_PATCH;
    } else if(!strcmp(wl[n],"-nocolor")) {
      pov_mode=WRITE_POV_NOCOLOR;
    } else if(!strcmp(wl[n],"-smooth")) {
      pov_mode=WRITE_POV_SMOOTH;
    } else if(!strcmp(wl[n],"-v35")) {
      pov_ver=WRITE_POV_V35;
    } else if(!strcmp(wl[n],"-plane")) {
      pov_flag+=WRITE_POV_PLANE;
    } else if(!strcmp(wl[n],"-def")) {
      if(n+1>=wc) {
	comMessage("missing parameters for -def");
	return -1;
      }
      pov_flag+=WRITE_POV_DEF;
      clStrcpy(pov_def,wl[n+1]);
      n++;
    } else if(!strcmp(wl[n],"-box")) {
      pov_flag+=WRITE_POV_BOX;
    } else {
    }
    n++;
  }

  if(strlen(type)==0) {
    n=0;
    while(strlen(write_def[n].ext)>0) {
      if(!strcmp(write_def[n].ext,ext)) {
	strcpy(type,write_def[n].type);
	break;
      }
      n++;
    }
    if(strlen(write_def[n].ext)==0) {
      sprintf(message,"unknown extension %s, please specify type\n",ext);
      comMessage(message);
      return -1;
    }
    
  }
  

  if(!strcmp(type,"raster")) {
    comMessage("WARNING: deprecated format! Please use POVray instead\n");
    fclose(f);
  } else if(!strcmp(type,"pov")) {
    if(pov_ver==WRITE_POV_V35 && pov_mode==WRITE_POV_PATCH) {
      comMessage("Error: -patch and -v35 cannot be used together!\n");
      return -1;
    }

    if((f=fopen(file,"w"))==NULL) {
      sprintf(message,"Error opening %s\n",file);
      comMessage(message);
      return -1;
    }
    // open second file
    sprintf(file2,"%s.inc",base);
    if((f2=fopen(file2,"w"))==NULL) {
      sprintf(message,"Error opening %s\n",file2);
      comMessage(message);
      fclose(f);
      return -1;
    }
    if(pov_ver==WRITE_POV_V35) {
      sprintf(message,"Writing povray (3.5 EXPERIMENTAL!) files %s and %s...\n",file,file2);
    } else {
      sprintf(message,"Writing povray (3.1) files %s and %s...\n",file,file2);
    }
    comMessage(message);

    writePOV(f,f2,file2,pov_flag, pov_mode, pov_ver);
    fclose(f2);
    fclose(f);
  } else if(!strcmp(type,"ps")) {
    if((f=fopen(file,"w"))==NULL) {
      sprintf(message,"Error opening %s\n",file);
      comMessage(message);
      return -1;
    }
    comMessage("Writing PostScript file...\n");
    writePS(f);
    fclose(f);
#ifdef FORMAT_RGB
  } else if(!strcmp(type,"rgb")) {
    comMessage("RGB output no longer supported\n");
#endif
  } else if(!strcmp(type,"png")) {
    comMessage("Writing png file...\n");
    wparam.type=WRITE_TYPE_PNG;
    writeFile(file,&wparam);
  } else if(!strcmp(type,"tiff")) {
    //comMessage("WARNING: deprecated format ! Please use png instead\n");
    comMessage("Writing tiff file...\n");
    wparam.type=WRITE_TYPE_TIFF;
    writeFile(file,&wparam);
#ifdef VRML
  } else if(!strcmp(type,"vrml") ||
	    !strcmp(type,"wrl")) {
    comMessage("WARNING: not in a workable state!\n");
    comMessage("Writing VRML scene...\n");
    writeVRML(f);
    fclose(f);
#endif
  } else {
    sprintf(message,"unknown type %s\n",type);
    comMessage(message);
    return -1;
  }
  return 0;
}

int comSave(int wc,const char **wl)
{
  char message[256];
  FILE *f;
  char file[256];

  if(wc<1) {
    sprintf(message,"save: missing filename\n");
    comMessage(message);
    return -1;
  }

  strcpy(file,wl[0]);

  if((f=fopen(file,"w"))==NULL) {
    sprintf(message,"save: error opening %s\n",file);
    comMessage(message);
    return -1;
  }


  fclose(f);
  return 0;
}

int comGenLists(int n)
{
  return glGenLists(n);
}

dbmNode * comGetDB(const char *name)
{
  int i;
  char dname[256];

  for(i=0;i<dbm.nodec_max;i++)
    if(dbm.node[i].common.type!=DBM_NODE_EMPTY) {
      sprintf(dname,".%s",dbm.node[i].common.name);
      if(!strcmp(dname,name))
	return &dbm.node[i];
    }
  return NULL;
}

  
int comNewDisplayList(int l)
{
  glNewList(l,GL_COMPILE);
  return 0;
}

int comEndDisplayList(void)
{
  glEndList();
  return 0;
}

int comGetMinMaxSlab()
{
  int i,j,k,f;
  GLdouble mm[16],pm[16],im[]={1.0,0.0,0.0,0.0,
			       0.0,1.0,0.0,0.0,
			       0.0,0.0,1.0,0.0,
			       0.0,0.0,0.0,1.0};
  GLint vp[4];
  double vv[3],vx,vy,vz,nx,ny,nz,fx,fy,fz,near,far,wx,wy,wz;


  structObj *struct_obj;
  scalObj *scal_obj;
  surfObj *surf_obj;
  gridObj *grid_obj;

  glGetDoublev(GL_MODELVIEW_MATRIX,mm);
  glGetDoublev(GL_PROJECTION_MATRIX,pm);
  glGetIntegerv(GL_VIEWPORT,vp);

  nz=1e6;
  fz=-1e6;

  f=0;
  for(i=0;i<dbm.nodec_max;i++) {
    switch(dbm.node[i].common.type) {
    case DBM_NODE_STRUCT:
      for(j=0;j<dbm.node[i].structNode.obj_max;j++)
	if(dbm.node[i].structNode.obj_flag[j]!=0) {
	  struct_obj=&dbm.node[i].structNode.obj[j];
	  if(struct_obj->render.show) {
	    f=1;
	    for(k=0;k<struct_obj->atom_count;k++) {
	      vv[0]=struct_obj->atom[k].ap->p->x;
	      vv[1]=struct_obj->atom[k].ap->p->y;
	      vv[2]=struct_obj->atom[k].ap->p->z;

	      transApply(&struct_obj->node->transform,vv);

	      vx=vv[0]; vy=vv[1]; vz=vv[2];
	      
	      gluProject(vx,vy,vz,mm,pm,vp,&wx,&wy,&wz);
	      
	      if(wz<nz) {
		nx=wx;
		ny=wy;
		nz=wz;
	      }
	      if(wz>fz) {
		fx=wx;
		fy=wy;
		fz=wz;
	      }
	      
	    }
	  }
	}
      break;
    case DBM_NODE_SCAL:
      for(j=0;j<dbm.node[i].scalNode.obj_max;j++)
	if(dbm.node[i].scalNode.obj[j]!=NULL) {
	  scal_obj=dbm.node[i].scalNode.obj[j];
	  if(scal_obj->render.show) {
	    f=1;
	    for(k=0;k<scal_obj->point_count;k++) {
	      vv[0]=scal_obj->point[k].v[0];
	      vv[1]=scal_obj->point[k].v[1];
	      vv[2]=scal_obj->point[k].v[2];

	      transApply(&scal_obj->node->transform,vv);

	      vx=vv[0]; vy=vv[1]; vz=vv[2];
	      
	      gluProject(vx,vy,vz,mm,pm,vp,&wx,&wy,&wz);
	      
	      if(wz<nz) {
		nx=wx;
		ny=wy;
		nz=wz;
	      }
	      if(wz>fz) {
		fx=wx;
		fy=wy;
		fz=wz;
	      }
	      
	    }
	  }
	}
      break;
    case DBM_NODE_SURF:
      for(j=0;j<dbm.node[i].surfNode.obj_max;j++)
	if(dbm.node[i].surfNode.obj_flag[j]!=0) {
	  surf_obj=&dbm.node[i].surfNode.obj[j];
	  if(surf_obj->render.show) {
	    f=1;
	    for(k=0;k<surf_obj->vertc;k++) {
	      vv[0]=surf_obj->vert[k].p[0];
	      vv[1]=surf_obj->vert[k].p[1];
	      vv[2]=surf_obj->vert[k].p[2];

	      transApply(&surf_obj->node->transform,vv);

	      vx=vv[0]; vy=vv[1]; vz=vv[2];
	      
	      gluProject(vx,vy,vz,mm,pm,vp,&wx,&wy,&wz);
	      
	      if(wz<nz) {
		nx=wx;
		ny=wy;
		nz=wz;
	      }
	      if(wz>fz) {
		fx=wx;
		fy=wy;
		fz=wz;
	      }
	      
	    }
	  }
	}
      break;
    case DBM_NODE_GRID:
      for(j=0;j<dbm.node[i].gridNode.obj_max;j++)
	if(dbm.node[i].gridNode.obj_flag[j]!=0) {
	  grid_obj=&dbm.node[i].gridNode.obj[j];
	  if(grid_obj->render.show) {
	    f=1;
	    for(k=0;k<grid_obj->vertc;k++) {
	      vv[0]=grid_obj->vert[k].v[0];
	      vv[1]=grid_obj->vert[k].v[1];
	      vv[2]=grid_obj->vert[k].v[2];

	      transApply(&grid_obj->node->transform,vv);

	      vx=vv[0]; vy=vv[1]; vz=vv[2];
	      
	      gluProject(vx,vy,vz,mm,pm,vp,&wx,&wy,&wz);
	      
	      if(wz<nz) {
		nx=wx;
		ny=wy;
		nz=wz;
	      }
	      if(wz>fz) {
		fx=wx;
		fy=wy;
		fz=wz;
	      }
	      
	    }
	  }
	}
      break;
    }
    
  }
  
  gluUnProject(nx,ny,nz,im,pm,vp,&vx,&vy,&vz);
  near=-vz;
  gluUnProject(fx,fy,fz,im,pm,vp,&vx,&vy,&vz);
  far=-vz;

  near-=3.0;
  if(near<0.0)
    near=1.0;

  if(f) {
    gfx.transform.slabn=near;
    gfx.transform.slabf=far;
    gfxSetSlab(near,far);
    gfxSetFog();
  } else {
    gfx.transform.slabn=10;
    gfx.transform.slabf=400;
    gfxSetSlab(10,400);
    gfxSetFog();    
  }  
  return 0;
}

int comWriteCharBuf(char c)
{
#ifndef OSX
#ifdef NEW_SHELL
  guitAddChar(c);
#else
  shellAddChar(c);
#endif
#endif
  return 0;
}

int comPlay(dbmNode *node, int command)
{
  int i,j;

  if(command==COM_PLAY_ON) {
    for(i=0;i<com.play_count;i++) {
      if(com.play_node[i]==node) {
	/* already on */
	return 0;
      }
    }
    com.play_node[com.play_count++]=node;
  } else if(command==COM_PLAY_OFF) {
    for(i=0;i<com.play_count;i++) {
      if(com.play_node[i]==node) {
	for(j=i+1;j<com.play_count;j++)
	  com.play_node[j-1]=com.play_node[j];
	com.play_count--;
	return 0;
      }
    }
  } else {
    return -1;
  }
  return 0;
}

char com_return_buf[256];

void comReturn(const char *r)
{
  int i;
  if(r!=NULL) {
    for(i=0;i<256;i++) {
      com_return_buf[i]=r[i];
      if(com_return_buf[i]=='\0')
	break;
    }
    com_return_buf[i]='\0';
  } else {
    com_return_buf[0]='\0';
  }
}

const char *comGetReturn() {return com_return_buf;}

int comTransform(int device, int mask, int axis, int ivalue)
{
  int i,j,mask2,axis_flag;
  double value=(double)ivalue;

#ifdef USE_CMI
  mask &= CMI_BUTTON1_MASK | CMI_BUTTON2_MASK | CMI_BUTTON3_MASK | CMI_BUTTON4_MASK | CMI_BUTTON5_MASK | CMI_SHIFT_MASK | CMI_CNTRL_MASK | CMI_LOCK_MASK;
#else
  mask &= GUI_BUTTON1_MASK | GUI_BUTTON2_MASK | GUI_BUTTON3_MASK | GUI_BUTTON4_MASK | GUI_BUTTON5_MASK | GUI_SHIFT_MASK | GUI_CNTRL_MASK | GUI_LOCK_MASK;
#endif

  for(i=0;i<com.tlist_count;i++) {
    if(com.tlist[i].device==device && 
       (com.tlist[i].mask&mask)==com.tlist[i].mask) {
      // modify mask
      mask2=mask & (~com.tlist[i].mask);
      if(com.tlist[i].transform!=NULL) {
	// check for custom entries first
	// overiding the normal ones
	axis_flag=0;
	if(axis>=0 && axis<TRANSFORM_MAX_CUSTOM) {
	  if(com.tlist[i].custom[axis].flag) {
	    axis_flag=1;
	    if(com.tlist[i].custom[axis].cb!=NULL)
	      (*com.tlist[i].custom[axis].cb)(value*com.tlist[i].custom[axis].factor,com.tlist[i].custom[axis].ptr);
	  }
	}
	if(!axis_flag) {
	  for(j=0;com.tlist[i].command[j].axis!=-1;j++) {
	    if(com.tlist[i].command[j].axis==axis && 
	       (com.tlist[i].command[j].mask==mask2)) {
	      transCommand(com.tlist[i].transform,
			   com.tlist[i].command[j].command,
			   axis,
			   value*com.tlist[i].command[j].factor);
	    }
	  }
	}
      }
    }
  }
  comRedraw();
  return 0;
}

int comGrab(transMat *tm, char *name)
{
  int i;
  if(name==NULL)
    return -1;

  for(i=0;i<com.tlist_count;i++) {
    if(!strcmp(com.tlist[i].name,name)) {
      com.tlist[i].transform=tm;
      return 0;
    }
  }  
  comMessage("device not found: \n");
  comMessage(name);

  return -1;
}

int comGetCurrentCenter(double *v)
{
  v[0]=-gfx.transform.cen[0];
  v[1]=-gfx.transform.cen[1];
  v[2]=-gfx.transform.cen[2];
  return 0;
}

int comSetDefMat(struct RENDER_MATERIAL *mat)
{
  memcpy(mat,&gfx.defmat,sizeof(struct RENDER_MATERIAL));
  return 0;
}

int comTestTex3D(int u, int v, int w)
{
#if 0
  GLint width;
  glTexImage3DEXT(GL_PROXY_TEXTURE_3D_EXT,0,
		  GL_LUMINANCE,
		  u,v,w, 0,GL_LUMINANCE, GL_FLOAT,NULL);

  glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D_EXT,0, GL_TEXTURE_WIDTH,&width);

  return 1;

  if(width==0)
    return 0;
  else
    return 1;
#else
  return 0;
#endif
}

void comWriteModelview()
{
  char buf[256];
  sprintf(buf,"scene set rtc=%s\n",
	  transGetAll(&gfx.transform));
  shellLog(buf);
}

/*
  generate lookup table for
  intersection of a plane with
  a cube
*/

int comGenCubeLookup()
{
  int list[][2]={
    {1,5},{1,2},{1,3},{1,4},
    {2,5},{2,3},{3,4},{4,5},
    {5,6},{2,6},{3,6},{4,6}
  };
  int i,j;

  for(i=0;i<12;i++) {
    for(j=0;j<12;j++) {
      if(list[i][0]==list[j][0] ||
	 list[i][0]==list[j][1] ||
	 list[i][1]==list[j][0] ||
	 list[i][1]==list[j][1]) {
	com.cube_lookup[i*12+j]=1;
      } else {
	com.cube_lookup[i*12+j]=0;
      }
    }

  }
  return 0;
}


void comSetInitCommand(const char *s)
{
  clStrncpy(init_command,s,sizeof(init_command));
  init_command_flag=1;
}

void comBench(void)
{
  long tt;
  struct timeval tv;
  struct timezone tz;
  if(com.benchmark==0) {
    gettimeofday(&tv,&tz);
    tt=tv.tv_sec*1000000L+tv.tv_usec;
    com.t=tt;
    com.t2=1;
    com.benchmark=1;
  } else {
    
    guiMessage(" ");
    
    com.benchmark=0;
  }
}

void comOutit()
{     
  comWriteModelview();
  com.benchmark=0;
}

const float *comGetCP()
{
  return scene.cp;
}


#ifdef USE_CMI
void comCMICallback(const cmiToken *t)
{
  int *ip;
  char *cp;

  if(t->target==CMI_TARGET_COM) {
    switch(t->command) {
    case CMI_EXIT: dinoExit(0); break;
    case CMI_RAW: cp=(char *)t->value; comRawCommand(cp); break;
    case CMI_INPUT: inputProcess(t); break;
    case CMI_MESSAGE: comMessage((const char *)t->value); break;
    case CMI_INTERRUPT: shellInterrupt(); break;
      
    default: break;
    }
  }

  //if(tunnelvision_flag) tunnelvision_flag=2;

}
#endif

/***********************************************************

static int tunnelvision2(structObj *obj2,int f)
{
  static int step=0;
  static structObj *obj;

  if(f==1) {
    step=0;
    obj=obj2;


  } else if(f==2) {
    if(obj==NULL)
      return -1;

    glwClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(obj->tv[step].px,
	      obj->tv[step].py,
	      obj->tv[step].pz,
	      obj->tv[step].px+obj->tv[step].nx,
	      obj->tv[step].py+obj->tv[step].ny,
	      obj->tv[step].pz+obj->tv[step].nz,
	      obj->tv[step].dx,
	      obj->tv[step].dy,
	      obj->tv[step].dz);
    
    glLightfv(GL_LIGHT0,GL_POSITION,gfx.light[0].pos);
    
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glEnable(GL_VERTEX_ARRAY);
    glEnable(GL_NORMAL_ARRAY);
    glEnable(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].v[0]);
    glNormalPointer(GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].n[0]);
    glColorPointer(4,GL_FLOAT,sizeof(cgfxVAField),&obj->va.p[0].c[0]);
    glDrawArrays(GL_TRIANGLES,0,obj->va.count);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glDisable(GL_VERTEX_ARRAY);
    glDisable(GL_NORMAL_ARRAY);
    glDisable(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    guiSwapBuffers();
    step++;
    fprintf(stderr,"step %d\n",step);
    if(step>=obj->tvc) {
      step=0;
      return -1;
    }
  } else {
  }
  return 0;
}

void tunnelvision(structObj *obj)
{ 
  if(!(obj->render.mode==RENDER_HSC || obj->render.mode==RENDER_TUBE))
    return;
  
  tunnelvision_flag=1;
  tunnelvision2(obj,1);
}
************************************************/

