#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "dino.h"
#include "scene.h"
#include "com.h"
#include "gfx.h"
#include "mat.h"
#include "Cmalloc.h"
#include "glw.h"
#include "cl.h"
#include "set.h"
#include "help.h"
#include "colors.h"

#include "gui_ext.h"

struct SCENE scene;
extern struct GFX gfx;

extern int debug_mode,gfx_mode;

struct HELP_ENTRY scene_help[]={
  {"separator","Scene commands",""},
  {"autoslab","autoslab","Automatically adjust slab to fit minimal and maximal z-extension of all visible objects"},
  {"center","center {x,y,z}","Sets current center of rotation"},
  {"clear","clear","Empty scene stack"},
  {"get","get PROPERTY",
   "Gets scene property:\n\tcenter: Center of rotation\n\tnear: Near clipping plane\n\tfar: Far clipping plane\n\tslabw: Width of clipping plane\n\tfogd: Fog distance multiplier from far clipping plane\n\teyedist: Eyedistance (stereo parameter)\n\tfov: Field of view in degree\n\tfixz: Flag that determines wether clipping planes move along with translation\n\tdither: Turn dithering on or off\n\tdepthc: Turn dpthcueing on or off\n\tbg: Background color\n\tfogc: Fog color\n\tview: can be center, left o right\n\ttrans: absolute translation vector as {x,y,z}\n\trot: absolute rotation matrix as {{a,b,c},{d,e,g},{g,h,i}}\n\tmmat: Modelview matrix\n\trtc: Compact matrix of rotation, translation and center"},
  {"grab","grab DEVICE","Grab an input device"},
  {"hide","hide","Turns off graphics display"},
  {"message","message STRING","Displays STRING in gui message banner"},
  {"mono","mono","Forces mono mode"},
  {"peek","peek","Retrieve topmost scene stack value without removing it"},
  {"pop","pop","Retrieve topmost scene stack value"},
  {"push","push ARG [ARG2 ..],","Push supplied arguments onto scene stack"},
  {"reset","reset","Resets viewing matrix to identity"},
  {"rotm","rotm {{a,b,c},{d,e,f},{g,h,i}}","Applies given rotation matrix"},
  {"rotx","rotx ANGLE","Rotates ANGLE degrees around x-axis"},
  {"roty","roty ANGLE","Rotates ANGLE degrees around y-axis"},
  {"rotz","rotz ANGLE","Rotates ANGLE degrees around z-axis"},
  {"set","set PROPERTY=VALUE",
   "Sets scene property:\n\tnear: Near clipping plane\n\tfar: Far clipping plane\n\tslabw: Width of clipping plane\n\tfogo: Fog distance multiplier from far clipping plane\n\teyedist: Eyedistance (stereo parameter)\n\tfov: Field of view in degree\n\tfixz: Flag that determines wether clipping planes move along with translation\n\tdither: Turn dithering on or off\n\tdepthc: Turn dpthcueing on or off\n\tbg: Background color\n\tfogc: Fog color\n\tview: can be center, left o right\n\ttrans: absolute translation vector as {x,y,z}\n\trot: absolute rotation matrix as {{a,b,c},{d,e,g},{g,h,i}}\n\tmmat: Modelview matrix\n\trtc: Compact matrix of rotation, translation and center"},
  {"show","show","Turns on graphics display"},
  {"spin","spin","Toggle spinning after mouse movement"},
  {"split","split","Toggles between split-screen stereo and normal stereo"},
  {"stereo","stereo [on | off]","Toggles between stereo and mono (if supported)"},
  {"transm","transm {a,b,d}","Applies given translation vector"},
  {"transx","transx VAL","Translates VAL Angstrom along x-axis"}, 
  {"transy","transy VAL","Translates VAL Angstrom along y-axis"}, 
  {"transz","transz VAL","Translates VAL Angstrom along z-axis"}, 
  {"write","write file.ext [-s n]",
   "Writes scene to file. Supported formats are png tiff pov r3d ps"},
  {NULL,NULL,NULL}
};

static int scene_virgin_flag;

int sceneInit(void)
{
  debmsg("sceneInit: filling default values");
  scene.stack_p=-1;
  scene.stack_m=32;
  scene.stack=Ccalloc(scene.stack_m,sizeof(struct SCENE_STACK));

  gfx.fixz=1;
  gfx.smooth=1;


  /* grab the dial box */
  //  comGrabInput(GUI_DIALS, (comInputFunc)gfxCommand, NULL);
  comGrab(&gfx.transform,0,0, "mouse");
  comGrab(&gfx.transform,0,0, "mouse2");
  comGrab(&gfx.transform,0,0, "dials");
  comGrab(&gfx.transform,0,0, "dials2");
  comGrab(&gfx.transform,0,0, "spaceball");
  comGrab(&gfx.transform,0,0, "spaceball2");

  gfx.axisflag=0;
  
  scene.cpflag=0;
  scene.rulerflag=0;

  scene_virgin_flag=1;
  
  return 0;

}

int sceneCommand(int wc, const char **wl)
{
  char message[256];
  char set[1024];
  double v1[16],v2[16];
  float c[3];
  int i,swc;
  char **swl,prop[256],op[256],val[256];
  double newd,oldd,d2;
  GLdouble im[16]={1.0,0.0,0.0,0.0,
		   0.0,1.0,0.0,0.0,
		   0.0,0.0,1.0,0.0,
		   0.0,0.0,0.0,1.0};
  char *ret;

#ifdef EXPO
  float cr,cg,cb;
#endif
#ifdef INTERNAL_COLOR
  struct COLOR_ENTRY *coltab;
#endif

  if(wc<=0) {
    sprintf(message,"scene: error: no command given\n");
    comMessage(message);
    return -1;
  }

  if(clStrcmp(wl[0],"help") ||
     clStrcmp(wl[0],"?")) {
    if(wc<2)
      help(scene_help,"scene",NULL);
    else
      help(scene_help,"scene",wl[1]);
  } else if(clStrcmp(wl[0],"write")) {
    // write
    comWrite(wc-1,wl+1);
  } else if(clStrcmp(wl[0],"show")) {
    // show
    gfx.show=1;
    comRedraw();
  } else if(clStrcmp(wl[0],"hide")) {
    // hide
    gfx.show=0;
    comRedraw();
  } else if(clStrcmp(wl[0],"reset")) {
    if(wc>1) {
      for(i=1;i<wc;i++) {
	if(clStrcmp(wl[i],"center")) {
	  transResetCen(&gfx.transform);
	} else if(clStrcmp(wl[i],"rot")) {
	  transResetRot(&gfx.transform);
	} else if(clStrcmp(wl[i],"trans")) {
	  transResetTra(&gfx.transform);
	  gfx.transform.tra[2]=-100.0;
	} else if(clStrcmp(wl[i],"clip")) {
	  transResetSlab(&gfx.transform);
	  gfx.transform.slabn=1.0;
	  gfx.transform.slabf=1000.0;
	} else {
	  // ignore invalid expression
	}
      }
    } else {
      // reset all
      transReset(&gfx.transform);
      gfx.transform.tra[2]=-100.0;
      gfx.transform.slabn=1.0;
      gfx.transform.slabf=1000.0;
    }
    gfxSetViewport();
    gfxSetProjection(gfx.current_view);
    gfxSetFog();
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"center")) {
    /**********************
            center
     **********************/
    if(wc<2) {
      sprintf(message,"scene: missing value for center\n");
      comMessage(message);
      return -1;
    }
    if(matExtract1D(wl[1],3,v1)!=0) {
      sprintf(message,"scene: error in vector\n");
      comMessage(message);
      return -1;
    }
    oldd=gfx.transform.cen[2];
    gfx.transform.cen[0]=-v1[0];
    gfx.transform.cen[1]=-v1[1];
    gfx.transform.cen[2]=-v1[2];

    gfx.transform.tra[0]=0.0;
    gfx.transform.tra[1]=0.0;

    /*
      set slab properly
      get current center on screen, adjust with
      slab width
    */

    newd=(gfx.transform.slabf-gfx.transform.slabn)*0.5;

    gfx.transform.slabn=(-gfx.transform.tra[2])-newd;
    gfx.transform.slabf=(-gfx.transform.tra[2])+newd;
    gfxSetSlab(gfx.transform.slabn, gfx.transform.slabf);
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"rotx")) {
    /**********************
             rotx
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_ROTX,-1,atof(wl[1]));
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"roty")) {
    /**********************
            roty
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_ROTY,-1,atof(wl[1]));
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"rotz")) {
    /**********************
             rotz
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_ROTZ,-1,atof(wl[1]));
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"rotm")) { 
    /**********************
             rotm
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    if(matExtract2D(wl[1],3,3,v1)!=0) {
      sprintf(message,"scene: error in 3x3 matrix %s\n",wl[1]);
      comMessage(message);
      return -1;
    }
    /* create 4x4 matrix */
    /*
    matMultMM(v1,gfx.transform.rot,v2);
    */    
    transMultM(&gfx.transform,v1);
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"transx")) {
    /**********************
	     transx
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAX,-1,atof(wl[1]));
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"transy")) {
    /**********************
             transy
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAY,-1,atof(wl[1]));
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"transz")) {
    /**********************
             transz
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAZ,-1,atof(wl[1]));
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"transm")) {
    /**********************
             transm
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    if(matExtract1D(wl[1],3,v1)!=0) {
      sprintf(message,"scene: expected vector: %s\n",wl[1]);
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAX,-1, v1[0]);
    transCommand(&gfx.transform,TRANS_TRAY,-1, v1[1]);
    transCommand(&gfx.transform,TRANS_TRAZ,-1, v1[2]);
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"set")) {
    /**********************
             set
     **********************/
    if(wc<2) {
      sprintf(message,"missing arguments for set\n");
      comMessage(message);
      return -1;
    }
    strcpy(set,"");
    for(i=1;i<wc;i++)
      strcat(set,wl[i]);

    dbmSplit(set,',',&swc,&swl);
    for(i=0;i<swc;i++) {
      dbmSplitPOV(swl[i],prop,op,val);

      if(clStrcmp(prop,"near")) {
	/**********************
              set near
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.transform.slabn;
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.transform.slabn=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.transform.slabn+=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.transform.slabn-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"far")) {
	/**********************
	       set far
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.transform.slabf;
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.transform.slabf=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.transform.slabf+=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.transform.slabf-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"slabw")) {
	/**********************
	       slabwidth
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	newd=atof(val);
	if(newd<=0.0) {
	  comMessage("error: slab width must be larger than 0.0\n");
	  return -1;
	}
	oldd=gfx.transform.slabf-gfx.transform.slabn;
	d2=(gfx.transform.slabf+gfx.transform.slabn)*0.5;
	if(clStrcmp(op,"=")) {
	  oldd=newd;
	} else if(clStrcmp(op,"+=")) {
	  oldd+=newd;
	} else if(clStrcmp(op,"-=")) {
	  oldd-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfx.transform.slabn=d2-oldd*0.5;
	gfx.transform.slabf=d2+oldd*0.5;
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"fogo")) {
	/**********************
	       set fog offset
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.fog_far_offset=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.fog_far_offset+=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.fog_far_offset-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetFog();
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"fognearoffset") ||
		clStrcmp(prop,"fogno")) {
	/**********************
	  set fog near offset
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.fog_near_offset=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.fog_near_offset+=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.fog_near_offset-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetFog();
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"fogfaroffset") ||
		clStrcmp(prop,"fogfo")) {
	/**********************
	  set fog far offset
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.fog_far_offset=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.fog_far_offset+=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.fog_far_offset-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetFog();
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"persp")) {
	if(gfx.mode==GFX_ORTHO) {
	  sceneOrtho2Persp();
	  gfx.mode=GFX_PERSP;
	  glDisable(GL_NORMALIZE);
	}
	gfxSetProjection(gfx.current_view);
	gfxSetFog();
	comRedraw();
      } else if(clStrcmp(prop,"ortho")) {
	comMessage("warning: orthographic projection deprecated!\n");
	if(gfx.mode==GFX_PERSP) {
	  scenePersp2Ortho();
	  gfx.mode=GFX_ORTHO;
	  glEnable(GL_NORMALIZE);
	}
	gfxSetProjection(gfx.current_view);
	gfxSetFog();
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"eyedist")) {
	/**********************
	       set eyedist
	**********************/
      if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}

	oldd=gfx.eye_dist;
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.eye_dist=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.eye_dist+=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.eye_dist-=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	
	if(gfx.eye_dist<0.0)
	  gfx.eye_dist=0.0;
	
	gfxSetProjection(gfx.current_view);
	comRedraw();
      } else if(clStrcmp(prop,"splitmode")) {
	/******************
            splitmode
	******************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(atoi(val)==0) {
	  gfx.split_mode=0;
	} else {
	  gfx.split_mode=1;
	}
      } else if(clStrcmp(prop,"fov")) {
	/**********************
	       set fovy
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.fovy;
	newd=atof(val);
	if(clStrcmp(op,"=")) {
	  gfx.fovy=newd;
	} else if(clStrcmp(op,"+=")) {
	  gfx.fovy=newd;
	} else if(clStrcmp(op,"-=")) {
	  gfx.fovy=newd;
	} else {
	  sprintf(message,"scene: unknown operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(gfx.fovy<5.0) {
	  gfx.fovy=5.0;
	  comMessage("fov too small, reset to 5.0\n");
	} else if(gfx.fovy>85.0) {
	  gfx.fovy=85.0;
	  comMessage("fov too large, reset to 85.0\n");
	}
	gfxSetProjection(gfx.current_view);
	comRedraw();
      } else if(clStrcmp(prop,"fixz")) {
	/**********************
	       set fixz
	**********************/
	if(clStrcmp(op,"!")) {
	  gfx.fixz=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  gfx.fixz=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"scene: unknown operator: %s\n",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(clStrcmp(val,"0") || 
	       clStrcmp(val,"false") ||
	       clStrcmp(val,"no")) {
	      gfx.fixz=0;
	    } else {
	      gfx.fixz=1;
	    }
	  }
	}
	comRedraw();
      } else if(clStrcmp(prop,"dither")) {
	/**********************
	       set dither
	**********************/
	if(clStrcmp(op,"!")) {
	  gfx.dither=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  gfx.dither=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"scene: unknown operator: %s\n",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(clStrcmp(val,"0") || 
	       clStrcmp(val,"false") ||
	       clStrcmp(val,"no")) {
	      gfx.dither=0;
	    } else {
	      gfx.dither=1;
	    }
	  }
	}
	if(gfx.dither)
	  glEnable(GL_DITHER);
	else
	  glDisable(GL_DITHER);
	comRedraw();
      } else if(clStrcmp(prop,"axis")) {
	/**********************
	       set axis
	**********************/
	if(clStrcmp(op,"!")) {
	  gfx.axisflag=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  gfx.axisflag=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"scene: unknown operator: %s\n",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(clStrcmp(val,"0") || 
	       clStrcmp(val,"false") ||
	       clStrcmp(val,"no")) {
	      gfx.axisflag=0;
	    } else {
	      gfx.axisflag=1;
	    }
	  }
	}
	comRedraw();
      } else if(clStrcmp(prop,"ruler")) {
	/**********************
	       set ruler
	**********************/
	if(clStrcmp(op,"!")) {
	  scene.rulerflag=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  scene.rulerflag=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"scene: unknown operator: %s\n",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(clStrcmp(val,"0") || 
	       clStrcmp(val,"false") ||
	       clStrcmp(val,"no")) {
	      scene.rulerflag=0;
	    } else {
	      scene.rulerflag=1;
	    }
	  }
	}
	comRedraw();
#ifdef USE_DLIST
      } else if(clStrcmp(prop,"usedlist")) {
	/**********************
	       set usedlist
	**********************/
	if(clStrcmp(op,"!")) {
	  gfx.use_dlist_flag=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  gfx.use_dlist_flag=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"scene: unknown operator: %s\n",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(clStrcmp(val,"0") || 
	       clStrcmp(val,"false") ||
	       clStrcmp(val,"no")) {
	      gfx.use_dlist_flag=0;
	    } else {
	      gfx.use_dlist_flag=1;
	    }
	  }
	}
	comRedraw();
#endif
      } else if(clStrcmp(prop,"depthc")) {
	/**********************
	       set depthc
	**********************/
	if(clStrcmp(op,"!")) {
	  glDisable(GL_FOG);
	  gfx.fog=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  glEnable(GL_FOG);
	  gfx.fog=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"scene: unknown operator: %s\n",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(clStrcmp(val,"0") || 
	       clStrcmp(val,"false") ||
	       clStrcmp(val,"no")) {
	      glDisable(GL_FOG);
	      gfx.fog=0;
	    } else {
	      glEnable(GL_FOG);
	      gfx.fog=1;
	    }
	  }
	}
	comRedraw();
      } else if(clStrcmp(prop,"bg")) {
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(comGetColor(val, &c[0], &c[1], &c[2])!=0) {
	  sprintf(message,"scene: unknown color: %s\n", val);
	  comMessage(message);
	  return -1;
	}
	gfx.r=c[0];
	gfx.g=c[1];
	gfx.b=c[2];
	gfx.fog_color[0]=c[0];
	gfx.fog_color[1]=c[1];
	gfx.fog_color[2]=c[2];
	glClearColor(gfx.r,gfx.g,gfx.b,0.0);
	glFogfv(GL_FOG_COLOR,gfx.fog_color);
	comRedraw();
      } else if(clStrcmp(prop,"fogc")) {
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(comGetColor(val, &c[0], &c[1], &c[2])!=0) {
	  sprintf(message,"scene: unknown color: %s\n", val);
	  comMessage(message);
	  return -1;
	}
	gfx.fog_color[0]=c[0];
	gfx.fog_color[1]=c[1];
	gfx.fog_color[2]=c[2];
	glFogfv(GL_FOG_COLOR,gfx.fog_color);
	comRedraw();
      } else if(clStrcmp(prop,"fogm")) {
	if(clStrlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(clStrlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(!clStrcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(clStrcmp(val,"linear")) {
	  gfx.fog_mode=GL_LINEAR;
	} else if(clStrcmp(val,"exp")) {
	  gfx.fog_mode=GL_EXP;
	} else if(clStrcmp(val,"exp2")) {
	  gfx.fog_mode=GL_EXP2;
	} else {
	  comMessage("scene: expected linear, exp or exp2 as fog mode\n");
	}
	glFogi(GL_FOG_MODE, gfx.fog_mode);
      } else if(clStrcmp(prop,"fogd")) {
	if(clStrlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(clStrlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(!clStrcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	gfx.fog_density=atof(val);
	glFogf(GL_FOG_DENSITY,gfx.fog_density);
	
      } else if(clStrcmp(prop,"view")) {
	/**********************
	       set view
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(clStrcmp(val,"center"))
	  gfx.current_view=GFX_CENTER;
	else if(clStrcmp(val,"left"))
	  gfx.current_view=GFX_LEFT;
	else if(clStrcmp(val,"right"))
	  gfx.current_view=GFX_RIGHT;
	else {
	  sprintf(message,"invalid value for view\n");
	  comMessage(message);
	  return -1;
	}
	gfxSetProjection(gfx.current_view);
	comRedraw();
      } else if(clStrcmp(prop,"transmat") ||
		clStrcmp(prop,"trans")) {
	/**********************
	     set transmat
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(matExtract1D(val,3,v1)!=0) {
	  sprintf(message,"scene: error in matrix\n");
	  comMessage(message);
	  return -1;
	}
	gfx.transform.tra[0]=v1[0];
	gfx.transform.tra[1]=v1[1];
	gfx.transform.tra[2]=v1[2];
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"rotmat") ||
		clStrcmp(prop,"rot")) {
	/**********************
	     set rotmat
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(matExtract2D(val,3,3,v1)!=0) {
	  sprintf(message,"scene: error in matrix\n");
	  comMessage(message);
	  return -1;
	}
	v2[0]=v1[0]; v2[1]=v1[1]; v2[2]=v1[2]; v2[3]=0.0;
	v2[4]=v1[3]; v2[5]=v1[4]; v2[6]=v1[5]; v2[7]=0.0;
	v2[8]=v1[6]; v2[9]=v1[7]; v2[10]=v1[8]; v2[11]=0.0;
	v2[12]=0.0; v2[13]=0.0; v2[14]=0.0; v2[15]=1.0;
	for(i=0;i<16;i++)
	  gfx.transform.rot[i]=v2[i];
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"mmat")) {
	/**********************
	     set modelmat
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	if(matExtract2D(val,4,4,v1)!=0) {
	  sprintf(message,"scene: error in 4x4 matrix\n");
	  comMessage(message);
	  return -1;
	}
	v2[0]=v1[0]; v2[1]=v1[1]; v2[2]=v1[2]; v2[3]=0.0;
	v2[4]=v1[4]; v2[5]=v1[5]; v2[6]=v1[6]; v2[7]=0.0;
	v2[8]=v1[8]; v2[9]=v1[9]; v2[10]=v1[10]; v2[11]=0.0;
	v2[12]=0.0; v2[13]=0.0; v2[14]=0.0; v2[15]=1.0;
	for(i=0;i<16;i++)
	  gfx.transform.rot[i]=v2[i];
	gfx.transform.tra[0]=v1[12];
	gfx.transform.tra[1]=v1[13];
	gfx.transform.tra[2]=v1[14];
	comRedraw();
	scene_virgin_flag=0;
      } else if(clStrcmp(prop,"rtc")) {
	/**********************
	     set rot trans cen
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"scene: missing operator\n");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"scene: missing value\n");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"scene: invalid operator: %s\n",op);
	  comMessage(message);
	  return -1;
	}
	transSetAll(&gfx.transform, val);
	comRedraw();
	scene_virgin_flag=0;
	//} else if(clStrcmp(prop,"center")) {
	//} else if(clStrcmp(prop,"cp")) {
      } else {
	sprintf(message,"unknown expression %s%s%s\n",prop,op,val);
	comMessage(message);
	return -1;
      }
    }
  } else if(clStrcmp(wl[0],"get")) {
    /**********************
             get
     **********************/
    if(wc!=2) {
      sprintf(message,"wrong number of arguments\n");
      comMessage(message);
      return -1;
    }
    if(clStrcmp(wl[1],"center")) {
      /*
      v1[0]=-gfx.transform.cen[0];
      v1[1]=-gfx.transform.cen[1];
      v1[2]=-gfx.transform.cen[2];
      matAssemble1D(v1,3,message);
      */
      clStrcpy(message,transGetCen2(&gfx.transform));
    } else if(clStrcmp(wl[1],"near")){
      sprintf(message,"%g",gfx.transform.slabn);
    } else if(clStrcmp(wl[1],"far")){
      sprintf(message,"%g",gfx.transform.slabf);
    } else if(clStrcmp(wl[1],"slabw")){
      sprintf(message,"%g",gfx.transform.slabf-gfx.transform.slabn);
    } else if(clStrcmp(wl[1],"transmat") ||
	      clStrcmp(wl[1],"trans")){
      /*
      v1[0]=gfx.transform.tra[0];
      v1[1]=gfx.transform.tra[1];
      v1[2]=gfx.transform.tra[2];
      matAssemble1D(v1,3,message);
      */
      clStrcpy(message,transGetTra(&gfx.transform));
    } else if(clStrcmp(wl[1],"rotmat") ||
	      clStrcmp(wl[1],"rot")) {
      /*
      for(i=0;i<16;i++)
	v1[i]=gfx.transform.rot[i];
      sprintf(message,"{{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f}}",
	      v1[0],v1[1],v1[2],
	      v1[4],v1[5],v1[6],
	      v1[8],v1[9],v1[10]);
      */
      clStrcpy(message,transGetRot(&gfx.transform));
    } else if(clStrcmp(wl[1],"mmat")){
      /*
      v2[0]=gfx.transform.tra[0];
      v2[1]=gfx.transform.tra[1];
      v2[2]=gfx.transform.tra[2];
      for(i=0;i<16;i++)
	v1[i]=gfx.transform.rot[i];

      sprintf(message,
	      "{{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f}}",
	      v1[0],v1[1],v1[2],0.0,
	      v1[4],v1[5],v1[6],0.0,
	      v1[8],v1[9],v1[10],0.0,
	      v2[0],v2[1],v2[2],1.0);
      */
      clStrcpy(message,transGetMM(&gfx.transform));
    } else if(clStrcmp(wl[1],"rtc")) {
      sprintf(message,transGetAll(&gfx.transform));
    } else if(clStrcmp(wl[1],"eyedist")){
      sprintf(message,"%g",gfx.eye_dist);
    } else if(clStrcmp(wl[1],"fov")){
      sprintf(message,"%g",gfx.fovy);
    } else if(clStrcmp(wl[1],"fogo")){
      sprintf(message,"%g",gfx.fog_far_offset);
    } else if(clStrcmp(wl[1],"fogneafoffset") ||
	      clStrcmp(wl[1],"fogno")){
      sprintf(message,"%g",gfx.fog_near_offset);
    } else if(clStrcmp(wl[1],"fogfaroffset") ||
	      clStrcmp(wl[1],"fogfo")) {
      sprintf(message,"%g",gfx.fog_far_offset);
    } else if(clStrcmp(wl[1],"fixz")){
      if(gfx.fixz)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(clStrcmp(wl[1],"dither")){
      if(gfx.dither)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(clStrcmp(wl[1],"persp")){
      if(gfx.mode==GFX_PERSP)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(clStrcmp(wl[1],"ortho")){
      if(gfx.mode==GFX_ORTHO)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(clStrcmp(wl[1],"depthc")){
      if(gfx.fog)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(clStrcmp(wl[1],"bg")) {
      sprintf(message,"{%.3f,%.3f,%.3f}",gfx.r,gfx.g,gfx.b);
    } else if(clStrcmp(wl[1],"view")) {
      if(gfx.current_view==GFX_RIGHT) {
	sprintf(message,"right");
      } else if(gfx.current_view==GFX_LEFT) {
	sprintf(message,"left");
      } else {
	sprintf(message,"center");
      }
    } else if(clStrcmp(wl[1],"cp")) {
      sprintf(message,"{%g,%g,%g}",scene.cp[0],scene.cp[1],scene.cp[2]);
    } else {    
      sprintf(message,"unkown property: %s\n",wl[1]);
      comMessage(message);
      return -1;
    }
    comReturn(message);
  } else if(clStrcmp(wl[0],"push")) {
    for(i=1;i<wc;i++)
      scenePush(wl[i]);
  } else if(clStrcmp(wl[0],"pop")) {
    ret=scenePop();
    if(ret==NULL) {
      comMessage("scene stack is empty\n");
      comReturn("");
      return -1;
    } else {
      sprintf(message,"%s",ret);
      comReturn(message);
    }
  } else if(clStrcmp(wl[0],"peek")) {
    ret=scenePeek();
    if(ret==NULL) {
      comMessage("scene stack is empty\n");
      comReturn("");
      return -1;
    } else {
      sprintf(message,"%s",ret);
      comReturn(message);
    }
    sprintf(message,"%s",ret);
    comReturn(message);
  } else if(clStrcmp(wl[0],"clear")) {
    sceneClear();
  } else if(clStrcmp(wl[0],"message")) {
    strcpy(set,"");
    for(i=1;i<wc;i++) {
      strcat(set,wl[i]);
      strcat(set," ");
    }
    guiMessage(set);
  } else if(clStrcmp(wl[0],"autoslab")) {
    comAutoFit(0,1);
    gfxSetFog();
    gfxSetProjection(gfx.current_view);
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"autofit")) {
    comAutoFit(1,1);
    gfxSetFog();
    gfxSetProjection(gfx.current_view);
    comRedraw();
    scene_virgin_flag=0;
  } else if(clStrcmp(wl[0],"split")) {
    if(gfx.stereo_mode==GFX_STEREO_OFF) {
      gfx.stereo_mode=GFX_STEREO_SPLIT;
      cmiRefresh();
#ifdef SGI_STEREO
    } else if(gfx.stereo_mode==GFX_STEREO_HW) {
      cmiStereo(0);
      gfx.stereo_mode=GFX_STEREO_SPLIT;
      comRedraw();
#endif
    } else {
      gfx.stereo_mode=GFX_STEREO_OFF;
      comRedraw();
    }
  } else if(clStrcmp(wl[0],"stereo")) {
    if(guiQueryStereo()) {
      if(wc==1) {
        // toggle mode only;
        if(gfx.stereo_active) {
          gfx.stereo_active=guiSetStereo(0);
        } else {
          gfx.stereo_active=guiSetStereo(1);
        }
        if(gfx.stereo_active)
          gfx.stereo_mode=GFX_STEREO_HW;
        else
          gfx.stereo_mode=GFX_STEREO_OFF;
      }
      if(gfx.stereo_active) {
        comMessage("stereo mode is ON\n");
      } else {
        comMessage("stereo mode is OFF\n");
      }
    } else {
      comMessage("hardware stereo not available\n");
    }
  } else if(clStrcmp(wl[0],"stereoi")) {
    if(gfx.stereo_active) {
      gfx.stereo_mode=GFX_STEREO_OFF;
      gfx.stereo_active=0;
      comMessage("interlaced stereo off\n");
    } else {
      gfx.stereo_mode=GFX_STEREO_INTERLACED;
      gfx.stereo_active=1;
      gfx.stencil_dirty=1;
      comMessage("interlaced stereo on\n");
    }
    comRedraw();
  } else if(clStrcmp(wl[0],"grab")) {
    if(wc!=2) {
      sprintf(message,"Syntax: grab devicename\n");
      comMessage(message);
    } else {
      if(comGrab(&gfx.transform,0,0,wl[1])<0)
        return -1;
    }
  } else if(clStrcmp(wl[0],"spin")) {
    if(gfx.anim==1)
      gfx.anim=0;
    else
      gfx.anim=1;
  } else if(clStrcmp(wl[0],"rock")) {
    if(gfx.anim==2)
      gfx.anim=0;
    else
      gfx.anim=2;
  } else if(clStrcmp(wl[0],"depthc")) {
    if(gfx.fog) {
      glDisable(GL_FOG);
      gfx.fog=0;
    } else {
      glEnable(GL_FOG);
      gfx.fog=1;
    }
  } else if (clStrcmp(wl[0],"showcp")) {
    scene.cpflag=1;
    comRedraw();
  } else if (clStrcmp(wl[0],"hidecp")) {
    scene.cpflag=0;
    comRedraw();
  } else if (clStrcmp(wl[0],"flat")) {
    gfx.smooth=0;
    glShadeModel(GL_FLAT);
  } else if (clStrcmp(wl[0],"smooth")) {
    gfx.smooth=1;
    glShadeModel(GL_SMOOTH);
  } else if(clStrcmp(wl[0],"nolight")) {
    glDisable(GL_LIGHTING);
  } else if(clStrcmp(wl[0],"light")) {
    glEnable(GL_LIGHTING);
  } else if(clStrcmp(wl[0],"bench")) {
    comBench();
#ifdef INTERNAL_COLOR
  } else if(clStrcmp(wl[0],"showrgb")) {
    if(wc>2) {
      comMessage("error: at most one argument after showrgb\n");
    } else {
      i=0;
      coltab=colorGetTab();
      if(wc==2) {
	sprintf(set,"*%s*",wl[1]);
      } else {
	clStrcpy(set,"*");
      }

      while(clStrlen(coltab[i].name)>0) {
	if(rex(set,coltab[i].name)) {
	  sprintf(message,"%s:\t%3d %3d %3d\n",
		  coltab[i].name,coltab[i].r,coltab[i].g,coltab[i].b); 
	  comMessage(message);
	}
	i++;
      }
    }
#endif
#ifndef OSX
  } else if(clStrcmp(wl[0],"excl")) {
    if(clStrcmp(wl[1],"1") ||
       clStrcmp(wl[1],"on")) {
       guiGrab(1);   
    } else {
       guiGrab(0);   
    }
#endif
  } else {
    sprintf(message,"scene: unknown command %s\n",wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}

void scenePush(char *s)
{
  int i;
  if(scene.stack_p+1<scene.stack_m) {
    scene.stack_p++;
    strcpy(scene.stack[scene.stack_p].expr,s);
  } else {
    /* stack is full, drop last item */
    for(i=scene.stack_m-1;i>1;i--)
      strcpy(scene.stack[i-1].expr,scene.stack[i].expr);
    strcpy(scene.stack[scene.stack_p].expr,s);
  }
}

char *scenePop(void)
{
  if(scene.stack_p>=0)
    return scene.stack[scene.stack_p--].expr;
  else
    return NULL;
}

char *scenePeek(void)
{
  if(scene.stack_p>=0)
    return scene.stack[scene.stack_p].expr;
  else
    return NULL;
}

void sceneClear(void)
{
  scene.stack_p=-1;
  strcpy(scene.stack[0].expr,"");
}

int scenePersp2Ortho(void)
{
  int i;
  double idm[]={1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0};
  GLdouble mm[16],pm[16],cm[16];
  GLint vp[4];
  GLdouble wx1,wy1,wz1,wx2,wy2,wz2;
  double w,sinw,q,fact,qn,qf;
  double nx,ny,fx,fy;

  glGetDoublev(GL_MODELVIEW_MATRIX,mm);
  glGetDoublev(GL_PROJECTION_MATRIX,pm);
  glGetIntegerv(GL_VIEWPORT,vp);

  for(i=0;i<16;i++)
    cm[i]=idm[i];
  cm[14]=gfx.transform.tra[2];

  vp[0]=0;
  vp[1]=0;
  vp[2]=1;
  vp[3]=1;

  gluProject(0,0,0,cm,pm,vp,&wx1,&wy1,&wz1);
  gluProject(1,1,0,cm,pm,vp,&wx2,&wy2,&wz2);

  q=fabs(wx1-wx2);
  if(q>0)
    fact=(float)(1.0/q);
  else
    fact=100.0;

  w=M_PI*gfx.fovy/180.0;
  sinw=sin(w);

  ny=sinw*gfx.transform.slabn;
  nx=gfx.aspect*ny;
  fy=sinw*gfx.transform.slabf;
  fx=gfx.aspect*fy;

  /*
  fprintf(stderr,"%g  %g %g  %g %g  %g\n",fact,nx,ny,fx,fy,sinw*gfx.transform.tra[2]);
  */
  gfx.left=-1.0;
  gfx.right=1.0;
  gfx.bottom=-1.0;
  gfx.top=1.0;

  cm[14]=(gfx.transform.slabn+gfx.transform.tra[2]);
  gluProject(0,0,0,cm,pm,vp,&wx1,&wy1,&wz1);
  gluProject(1,1,0,cm,pm,vp,&wx2,&wy2,&wz2);
  qn=fabs(wx1-wx2);
  qn=(gfx.transform.slabn+gfx.transform.tra[2]);

  cm[14]=(gfx.transform.slabf+gfx.transform.tra[2]);
  gluProject(0,0,0,cm,pm,vp,&wx1,&wy1,&wz1);
  gluProject(1,1,0,cm,pm,vp,&wx2,&wy2,&wz2);
  qf=fabs(wx1-wx2);
  qf=(gfx.transform.slabf+gfx.transform.tra[2]);

  /*
  fprintf(stderr,"%g %g %g\n",q,qn*q,qf*q);
  */

  gfx.transform.slabn=0.0;
  gfx.transform.slabf=qf*q*2.0-qn*q*2.0;
  gfx.transform.tra[2]=qn;

  gfx.scale=q*2.0;
  return 0;
}

int sceneOrtho2Persp(void)
{
  gfx.transform.slabn=10.0;
  gfx.transform.slabf=500.0;
  gfx.scale=1.0;
  gfx.transform.tra[2]=-50;
  return 0;
}

int sceneSubCommand(char *sub, int wc, const char **wl)
{
  char message[256];
  int ret;

  ret=0;

  if(!strncmp(sub,"light",5)) {
    if(rex(sub,"light0"))
      ret=sceneSubLightCom(0,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light1"))
      ret=sceneSubLightCom(1,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light2"))
      ret=sceneSubLightCom(2,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light3"))
      ret=sceneSubLightCom(3,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light4"))
      ret=sceneSubLightCom(4,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light5"))
      ret=sceneSubLightCom(5,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light6"))
      ret=sceneSubLightCom(6,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"light7"))
      ret=sceneSubLightCom(7,wc,wl);
    if(ret<0)
      return -1;

  } else if(!strncmp(sub,"clip",4)) {
    if(rex(sub,"clip0"))
      ret=sceneSubClipCom(0,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"clip1"))
      ret=sceneSubClipCom(1,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"clip2"))
      ret=sceneSubClipCom(2,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"clip3"))
      ret=sceneSubClipCom(3,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"clip4"))
      ret=sceneSubClipCom(4,wc,wl);
    if(ret<0)
      return -1;
    if(rex(sub,"clip5"))
      ret=sceneSubClipCom(5,wc,wl);
    if(ret<0)
      return -1;

  } else {
    sprintf(message,"error: scene: unknown sub expression %s\n",sub);
    comMessage(message);
    return -1;
  }
  return 0;
}

int sceneSubLightCom(int l, int wc, const char **wl)
{
  char message[256];
  if(wc<=0) {
    comMessage("error: scene: missing command\n");
    return -1;
  }
  if(clStrcmp(wl[0],"set")) {
    return sceneSubLightSet(l,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"get")) {
    return sceneSubLightGet(l,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"on")) {
    gfx.light[l].on=1;
    glEnable(GL_LIGHT0+l);
  } else if(clStrcmp(wl[0],"off")) {
    gfx.light[l].on=0;
    glDisable(GL_LIGHT0+l);
  } else if(clStrcmp(wl[0],"local")) {
    gfx.light[l].local=1;
  } else if(clStrcmp(wl[0],"global")) {
    gfx.light[l].local=0;
  } else if(clStrcmp(wl[0],"show")) {
    if(gfx.light[l].on)
      sprintf(message,"light%d is on\n",l);
    else
      sprintf(message,"light%d is off\n",l);
    comMessage(message);
    sprintf(message,"position {%.3f,%.3f,%.3f,%.3f}\n",
	    gfx.light[l].pos[0],gfx.light[l].pos[1],
	    gfx.light[l].pos[2],gfx.light[l].pos[3]);
    comMessage(message);
    sprintf(message,"ambient  {%.3f,%.3f,%.3f,%.3f}\n",
	    gfx.light[l].amb[0],gfx.light[l].amb[1],
	    gfx.light[l].amb[2],gfx.light[l].amb[3]);
    comMessage(message);
    sprintf(message,"diffuse  {%.3f,%.3f,%.3f,%.3f}\n",
	    gfx.light[l].diff[0],gfx.light[l].diff[1],
	    gfx.light[l].diff[2],gfx.light[l].diff[3]);
    comMessage(message);
    sprintf(message,"specular {%.3f,%.3f,%.3f,%.3f}\n",
	    gfx.light[l].spec[0],gfx.light[l].spec[1],
	    gfx.light[l].spec[2],gfx.light[l].spec[3]);
    comMessage(message);
    sprintf(message,"attenuation c %.3f  l %.3f   q %.3f",
	    gfx.light[l].kc,gfx.light[l].kl,gfx.light[l].kq);
  } else {
    comMessage("error: scene: unknown command \n");
    comMessage(wl[0]);
    return -1;
  }
  return 0;
}

int sceneSubLightSet(int l, int wc, const char **wl)
{
  Set set;
  int pc,vc;
  struct POV_VALUE *val;
  float v[4];
  char message[256];

  if(wc<=0) {
    comMessage("error: missing parameters for set\n");
    return -1;
  }

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  if(set.range_flag) {
    comMessage("error: unexpected range\n");
    return -1;
  }

  for(pc=0;pc<set.pov_count;pc++) {
    if(set.pov[pc].val_count>1) {
      comMessage("error: expected at most one value\n");
      return -1;
    }
    val=povGetVal(set.pov+pc,0);
    if(val->range_flag) {
      comMessage("error: unexpected range\n");
      return -1;
    }
    if(clStrcmp(set.pov[pc].prop,"on")) {
      gfx.light[l].on=1;
      glEnable(GL_LIGHT0+l);
    } else if(clStrcmp(set.pov[pc].prop,"off")) {
      gfx.light[l].on=0;
      glDisable(GL_LIGHT0+l);
    } else if(clStrcmp(set.pov[pc].prop,"local")) {
      gfx.light[l].local=1;
    } else if(clStrcmp(set.pov[pc].prop,"global")) {
      gfx.light[l].local=0;
    } else if(clStrcmp(set.pov[pc].prop,"pos")) {
      if(val->val1[0]!='{') {
	comMessage("error: expected {x,y,z,w} for pos\n");
	return -1;
      }
      if(matExtract1Df(val->val1,4,v)<0) {
	comMessage("error: in vector \n");
	comMessage(val->val1);
	return -1;
      }
      gfx.light[l].pos[0]=v[0];
      gfx.light[l].pos[1]=v[1];
      gfx.light[l].pos[2]=v[2];
      gfx.light[l].pos[3]=v[3];
      if(gfx.light[l].pos[3]<0.01)
	gfx.light[l].pos[3]=0.0;
      glLightfv(GL_LIGHT0+l,GL_POSITION, v);
    } else if(clStrcmp(set.pov[pc].prop,"amb")) {
      if(val->val1[0]!='{') {
	gfx.light[l].amb[0]=atof(val->val1);
	gfx.light[l].amb[1]=atof(val->val1);
	gfx.light[l].amb[2]=atof(val->val1);
      } else {
	if(matExtract1Df(val->val1,4,v)<0) {
	  comMessage("error: in vector \n");
	  comMessage(val->val1);
	  return -1;
	}
	gfx.light[l].amb[0]=v[0];
	gfx.light[l].amb[1]=v[1];
	gfx.light[l].amb[2]=v[2];
	gfx.light[l].amb[3]=v[3];
      }
      glLightfv(GL_LIGHT0+l,GL_AMBIENT, gfx.light[l].amb);
    } else if(clStrcmp(set.pov[pc].prop,"diff")) {
      if(val->val1[0]!='{') {
	gfx.light[l].diff[0]=atof(val->val1);
	gfx.light[l].diff[1]=atof(val->val1);
	gfx.light[l].diff[2]=atof(val->val1);
      } else {
	if(matExtract1Df(val->val1,4,v)<0) {
	  comMessage("error: in vector \n");
	  comMessage(val->val1);
	  return -1;
	}
	gfx.light[l].diff[0]=v[0];
	gfx.light[l].diff[1]=v[1];
	gfx.light[l].diff[2]=v[2];
	gfx.light[l].diff[3]=v[3];
      }
      glLightfv(GL_LIGHT0+l,GL_DIFFUSE, gfx.light[l].diff);
    } else if(clStrcmp(set.pov[pc].prop,"spec")) {
      if(val->val1[0]!='{') {
	gfx.light[l].spec[0]=atof(val->val1);
	gfx.light[l].spec[1]=atof(val->val1);
	gfx.light[l].spec[2]=atof(val->val1);
      } else {
	if(matExtract1Df(val->val1,4,v)<0) {
	  comMessage("error: in vector \n");
	  comMessage(val->val1);
	  return -1;
	}
	gfx.light[l].spec[0]=v[0];
	gfx.light[l].spec[1]=v[1];
	gfx.light[l].spec[2]=v[2];
	gfx.light[l].spec[3]=v[3];
      }
      glLightfv(GL_LIGHT0+l,GL_SPECULAR, gfx.light[l].spec);
    } else if(clStrcmp(set.pov[pc].prop,"kc")) {
      gfx.light[l].kc=atof(val->val1);
      glLightf(GL_LIGHT0+l,GL_CONSTANT_ATTENUATION, gfx.light[l].kc);
    } else if(clStrcmp(set.pov[pc].prop,"kl")) {
      gfx.light[l].kl=atof(val->val1);
      glLightf(GL_LIGHT0+l,GL_LINEAR_ATTENUATION, gfx.light[l].kl);
    } else if(clStrcmp(set.pov[pc].prop,"kq")) {
      gfx.light[l].kq=atof(val->val1);
      glLightf(GL_LIGHT0+l,GL_QUADRATIC_ATTENUATION, gfx.light[l].kq);
    } else if(clStrcmp(set.pov[pc].prop,"spotc")) {
      gfx.light[l].spotc=atof(val->val1);
      glLightf(GL_LIGHT0+l, GL_SPOT_CUTOFF, gfx.light[l].spotc);
    } else if(clStrcmp(set.pov[pc].prop,"spote")) {
      gfx.light[l].spote=atof(val->val1);
      glLightf(GL_LIGHT0+l, GL_SPOT_EXPONENT, gfx.light[l].spote);
    } else if(clStrcmp(set.pov[pc].prop,"spotd")) {
      if(val->val1[0]!='{') {
	comMessage("error: expected {x,y,z} for spotd\n");
	return -1;
      } else {
	if(matExtract1Df(val->val1,3,v)<0) {
	  comMessage("error: in vector \n");
	  comMessage(val->val1);
	  return -1;
	}
	gfx.light[l].spotd[0]=v[0];
	gfx.light[l].spotd[1]=v[1];
	gfx.light[l].spotd[2]=v[2];
	glLightfv(GL_LIGHT0+l, GL_SPOT_DIRECTION, gfx.light[l].spotd);
      }
    } else {
      comMessage("error: scene: unknown light property \n");
      comMessage(set.pov[pc].prop);
    }
  }

  setDelete(&set);
  return 0;
}

int sceneSubLightGet(int l, int wc, const char **wl)
{
  char message[256];
  if(wc!=1) {
    comMessage("error: expected one property for get\n");
    return -1;
  }
  if(clStrcmp(wl[0],"on")) {
    sprintf(message,"%d",gfx.light[l].on);
    comReturn(message);
  } else if(clStrcmp(wl[0],"pos")) {
    sprintf(message,"{%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].pos[0],gfx.light[l].pos[1],
	    gfx.light[l].pos[2],gfx.light[l].pos[3]);
    comReturn(message);
  } else if(clStrcmp(wl[0],"amb")) {
    sprintf(message,"{%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].amb[0],gfx.light[l].amb[1],
	    gfx.light[l].amb[2],gfx.light[l].amb[3]);
    comReturn(message);
  } else if(clStrcmp(wl[0],"diff")) {
    sprintf(message,"{%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].diff[0],gfx.light[l].diff[1],
	    gfx.light[l].diff[2],gfx.light[l].diff[3]);
    comReturn(message);
  } else if(clStrcmp(wl[0],"spec")) {
    sprintf(message,"{%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].spec[0],gfx.light[l].spec[1],
	    gfx.light[l].spec[2],gfx.light[l].spec[3]);
    comReturn(message);
  } else {
    comMessage("error: unknown light property \n");
    comMessage(wl[0]);
    return -1;
  }
  return 0;
}

int sceneSubClipCom(int c, int wc, const char **wl)
{
  char message[256];
  if(wc<=0) {
    comMessage("error: scene: missing command\n");
    return -1;
  }
  if(clStrcmp(wl[0],"set")) {
    return sceneSubClipSet(c,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"get")) {
    return sceneSubClipGet(c,wc-1,wl+1);
  } else if(clStrcmp(wl[0],"on")) {
    gfx.clip[c].on=1;
  } else if(clStrcmp(wl[0],"off")) {
    gfx.clip[c].on=0;
  } else if(clStrcmp(wl[0],"grab")) {
    if(wc!=2) {
      sprintf(message,"Syntax: grab devicename\n");
      comMessage(message);
    } else {
      if(comGrab(&gfx.clip[c].transform,0,0,wl[1])<0)
	return -1;
    }
  } else {
    sprintf(message,"error: unknown scene:clip command: %s",wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}

int sceneSubClipSet(int c, int wc, const char **wl)
{
  Set set;
  int pc,vc;
  struct POV_VALUE *val;
  float v[4];
  char message[256];

  if(wc<=0) {
    comMessage("error: missing parameters for set\n");
    return -1;
  }

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  if(set.range_flag) {
    comMessage("error: unexpected range\n");
    return -1;
  }

  for(pc=0;pc<set.pov_count;pc++) {
    if(set.pov[pc].val_count>1) {
      comMessage("error: expected at most one value\n");
      return -1;
    }
    val=povGetVal(set.pov+pc,0);
    if(val->range_flag) {
      comMessage("error: unexpected range\n");
      return -1;
    }
    if(clStrcmp(set.pov[pc].prop,"on")) {
      gfx.clip[c].on=1;
    } else if(clStrcmp(set.pov[pc].prop,"off")) {
      gfx.clip[c].on=0;
    } else if(clStrcmp(set.pov[pc].prop,"pos")) {
      if(val->val1[0]!='{') {
	comMessage("error: expected {x,y,z} for pos\n");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v)<0) {
	comMessage("error: in vector \n");
	comMessage(val->val1);
	return -1;
      }
      gfx.clip[c].pos[0]=v[0];
      gfx.clip[c].pos[1]=v[1];
      gfx.clip[c].pos[2]=v[2];
    } else if(clStrcmp(set.pov[pc].prop,"dir")) {
      if(val->val1[0]!='{') {
	comMessage("error: expected {x,y,z} for dir\n");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v)<0) {
	comMessage("error: in vector \n");
	comMessage(val->val1);
	return -1;
      }
      gfx.clip[c].dir[0]=v[0];
      gfx.clip[c].dir[1]=v[1];
      gfx.clip[c].dir[2]=v[2];
    }
    
  }

  return 0;
}

int sceneSubClipGet(int c, int wc, const char **wl)
{
  return 0;
}

void sceneSetCenter(double c[3])
{
  gfx.transform.cen[0]=-c[0];
  gfx.transform.cen[1]=-c[1];
  gfx.transform.cen[2]=-c[2];
}


int sceneIsVirgin(void)
{
  return scene_virgin_flag;
}
