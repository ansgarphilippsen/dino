#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef USE_MESA
#include <MesaGL/gl.h>
#include <MesaGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "dino.h"
#include "scene.h"
#include "com.h"
#include "gui.h"
#include "gfx.h"
#include "mat.h"
#include "Cmalloc.h"
#include "GLwStereo.h"
#include "input.h"
#include "cl.h"
#include "set.h"
#include "help.h"

struct SCENE scene;
extern struct GUI gui;
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
   "Sets scene property:\n\tnear: Near clipping plane\n\tfar: Far clipping plane\n\tslabw: Width of clipping plane\n\tfogd: Fog distance multiplier from far clipping plane\n\teyedist: Eyedistance (stereo parameter)\n\tfov: Field of view in degree\n\tfixz: Flag that determines wether clipping planes move along with translation\n\tdither: Turn dithering on or off\n\tdepthc: Turn dpthcueing on or off\n\tbg: Background color\n\tfogc: Fog color\n\tview: can be center, left o right\n\ttrans: absolute translation vector as {x,y,z}\n\trot: absolute rotation matrix as {{a,b,c},{d,e,g},{g,h,i}}\n\tmmat: Modelview matrix\n\trtc: Compact matrix of rotation, translation and center"},
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

int sceneInit(void)
{
  debmsg("sceneInit: filling default values");
  scene.stack_p=-1;
  scene.stack_m=32;
  scene.stack=Ccalloc(scene.stack_m,sizeof(struct SCENE_STACK));

#ifdef EXPO
  scene.label_m=256;
  scene.label_c=0;
  scene.label=Ccalloc(scene.label_m,sizeof(struct SCENE_LABEL));
#endif
  
  gfx.fixz=1;
  gfx.smooth=1;

  /*
  for(i=0;scene_def_reg[i].oname!=NULL;i++) {

  }
  */

  /* grab the dial box */
  //  comGrabInput(GUI_DIALS, (comInputFunc)gfxCommand, NULL);
  comGrab(&gfx.transform, "mouse");
  comGrab(&gfx.transform, "mouse2");
  comGrab(&gfx.transform, "dials");
  comGrab(&gfx.transform, "dials2");
  comGrab(&gfx.transform, "spaceball");
  comGrab(&gfx.transform, "spaceball2");

  gfx.axisflag=0;
  
  scene.cpflag=0;
  
  return 0;

}

int sceneCommand(int wc, char **wl)
{
  char message[256];
  char set[1024];
  double v1[16],v2[16];
  float c[3];
  int i,swc,newi;
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

  if(wc<=0) {
    sprintf(message,"\nscene: error: no command given");
    comMessage(message);
    return -1;
  }

  if(!strcmp(wl[0],"help") ||
     !strcmp(wl[0],"?")) {
    if(wc<2)
      help(scene_help,"scene",NULL);
    else
      help(scene_help,"scene",wl[1]);
  } else if(!strcmp(wl[0],"write")) {
    comWrite(wc-1,wl+1);
  } else if(!strcmp(wl[0],"show")) {
    gfx.show=1;
    comRedraw();
  } else if(!strcmp(wl[0],"hide")) {
    gfx.show=0;
    comRedraw();
  } else if(!strcmp(wl[0],"reset")) {
    transReset(&gfx.transform);
    gfx.transform.tra[2]=-100.0;
    gfx.transform.slabn=1.0;
    gfx.transform.slabf=1000.0;
    gfxSetViewport();
    gfxSetProjection(gfx.stereo_display);
    gfxSetFog();
    comRedraw();
  } else if(!strcmp(wl[0],"center")) {
    /**********************
            center
     **********************/
    if(wc<2) {
      sprintf(message,"\nscene: missing value for center");
      comMessage(message);
      return -1;
    }
    if(matExtract1D(wl[1],3,v1)!=0) {
      sprintf(message,"\nscene: error in vector");
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
  } else if(!strcmp(wl[0],"rotx")) {
    /**********************
             rotx
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_ROTX, atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"roty")) {
    /**********************
            roty
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_ROTY, atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"rotz")) {
    /**********************
             rotz
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_ROTZ, atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"rotm")) { 
    /**********************
             rotm
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    if(matExtract2D(wl[1],3,3,v1)!=0) {
      sprintf(message,"\nscene: error in 3x3 matrix %s",wl[1]);
      comMessage(message);
      return -1;
    }
    /* create 4x4 matrix */
    /*
    matMultMM(v1,gfx.transform.rot,v2);
    */    
    transMultM(&gfx.transform,v1);
    comRedraw();
  } else if(!strcmp(wl[0],"transx")) {
    /**********************
	     transx
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAX, atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transy")) {
    /**********************
             transy
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAY, atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transz")) {
    /**********************
             transz
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAZ, atof(wl[1]));
    comRedraw();
  } else if(!strcmp(wl[0],"transm")) {
    /**********************
             transm
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    if(matExtract1D(wl[1],3,v1)!=0) {
      sprintf(message,"\nscene: expected vector: %s",wl[1]);
      comMessage(message);
      return -1;
    }
    transCommand(&gfx.transform,TRANS_TRAX, v1[0]);
    transCommand(&gfx.transform,TRANS_TRAY, v1[1]);
    transCommand(&gfx.transform,TRANS_TRAZ, v1[2]);
    comRedraw();
  } else if(!strcmp(wl[0],"set")) {
    /**********************
             set
     **********************/
    if(wc<2) {
      sprintf(message,"\nmissing arguments for set");
      comMessage(message);
      return -1;
    }
    strcpy(set,"");
    for(i=1;i<wc;i++)
      strcat(set,wl[i]);

    dbmSplit(set,',',&swc,&swl);
    for(i=0;i<swc;i++) {
      dbmSplitPOV(swl[i],prop,op,val);

      if(!strcmp(prop,"near")) {
	/**********************
              set near
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.transform.slabn;
	newd=atof(val);
	if(!strcmp(op,"=")) {
	  gfx.transform.slabn=newd;
	} else if(!strcmp(op,"+=")) {
	  gfx.transform.slabn+=newd;
	} else if(!strcmp(op,"-=")) {
	  gfx.transform.slabn-=newd;
	} else {
	  sprintf(message,"\nscene: unknown operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
	comRedraw();
      } else if(!strcmp(prop,"far")) {
	/**********************
	       set far
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.transform.slabf;
	newd=atof(val);
	if(!strcmp(op,"=")) {
	  gfx.transform.slabf=newd;
	} else if(!strcmp(op,"+=")) {
	  gfx.transform.slabf+=newd;
	} else if(!strcmp(op,"-=")) {
	  gfx.transform.slabf-=newd;
	} else {
	  sprintf(message,"\nscene: unknown operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
	comRedraw();
      } else if(!strcmp(prop,"slabw")) {
	/**********************
	       slabwidth
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	newd=atof(val);
	if(newd<=0.0) {
	  comMessage("\nerror: slab width must be larger than 0.0");
	  return -1;
	}
	oldd=gfx.transform.slabf-gfx.transform.slabn;
	d2=(gfx.transform.slabf+gfx.transform.slabn)*0.5;
	if(!strcmp(op,"=")) {
	  oldd=newd;
	} else if(!strcmp(op,"+=")) {
	  oldd+=newd;
	} else if(!strcmp(op,"-=")) {
	  oldd-=newd;
	} else {
	  sprintf(message,"\nscene: unknown operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	gfx.transform.slabn=d2-oldd*0.5;
	gfx.transform.slabf=d2+oldd*0.5;
	gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
	comRedraw();
      } else if(!strcmp(prop,"fogd")) {
	/**********************
	       set fogd
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.fog_dist;
	newd=atof(val);
	if(!strcmp(op,"=")) {
	  gfx.fog_dist=newd;
	} else if(!strcmp(op,"+=")) {
	  gfx.fog_dist+=newd;
	} else if(!strcmp(op,"-=")) {
	  gfx.fog_dist-=newd;
	} else {
	  sprintf(message,"\nscene: unknown operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(gfx.fog_dist<-1.0) {
	  comMessage("\nfogd clamped to -1.0");
	  gfx.fog_dist=-1.0;
	}
	if(gfx.fog_dist>1.0) {
	  comMessage("\nfogd clamped to 1.0");
	  gfx.fog_dist=1.0;
	}
	gfxSetFog();
	comRedraw();
      } else if(!strcmp(prop,"persp")) {
	if(gfx.mode==GFX_ORTHO) {
	  sceneOrtho2Persp();
	  gfx.mode=GFX_PERSP;
	}
	gfxSetProjection(gfx.stereo_display);
	gfxSetFog();
	comRedraw();
      } else if(!strcmp(prop,"ortho")) {
	if(gfx.mode==GFX_PERSP) {
	  scenePersp2Ortho();
	  gfx.mode=GFX_ORTHO;
	}
	gfxSetProjection(gfx.stereo_display);
	gfxSetFog();
	comRedraw();
      } else if(!strcmp(prop,"eyedist")) {
	/**********************
	       set eyedist
	**********************/
      if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	oldd=gui.eye_dist;
	newd=atof(val);
	if(!strcmp(op,"=")) {
	  gui.eye_dist=newd;
	} else if(!strcmp(op,"+=")) {
	  gui.eye_dist+=newd;
	} else if(!strcmp(op,"-=")) {
	  gui.eye_dist-=newd;
	} else {
	  sprintf(message,"\nscene: unknown operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(gui.eye_dist<0.0)
	  gui.eye_dist=0.0;
	gfxSetProjection(gfx.stereo_display);
	comRedraw();
      } else if(!strcmp(prop,"fov")) {
	/**********************
	       set fovy
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	oldd=gfx.fovy;
	newd=atof(val);
	if(!strcmp(op,"=")) {
	  gfx.fovy=newd;
	} else if(!strcmp(op,"+=")) {
	  gfx.fovy=newd;
	} else if(!strcmp(op,"-=")) {
	  gfx.fovy=newd;
	} else {
	  sprintf(message,"\nscene: unknown operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(gfx.fovy<5.0)
	  gfx.fovy=5.0;
	if(gfx.fovy>85.0)
	  gfx.fovy=85.0;
	gfxSetProjection(gfx.stereo_display);
	comRedraw();
      } else if(!strcmp(prop,"fixz")) {
	/**********************
	       set fixz
	**********************/
	if(!strcmp(op,"!")) {
	  gfx.fixz=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  gfx.fixz=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"\nscene: unknown operator: %s",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(!strcmp(val,"0") || 
	       !strcmp(val,"false") ||
	       !strcmp(val,"no")) {
	      gfx.fixz=0;
	    } else {
	      gfx.fixz=1;
	    }
	  }
	}
	comRedraw();
      } else if(!strcmp(prop,"dither")) {
	/**********************
	       set dither
	**********************/
	if(!strcmp(op,"!")) {
	  gfx.dither=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  gfx.dither=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"\nscene: unknown operator: %s",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(!strcmp(val,"0") || 
	       !strcmp(val,"false") ||
	       !strcmp(val,"no")) {
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
      } else if(!strcmp(prop,"depthc")) {
	/**********************
	       set depthc
	**********************/
	if(!strcmp(op,"!")) {
	  glDisable(GL_FOG);
	  gfx.fog=0;
	} else if(strlen(val)==0 && strlen(op)==0) {
	  glEnable(GL_FOG);
	  gfx.fog=1;
	} else if(strlen(val)==0 && strlen(op)>0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	} else {
	  if(strcmp(op,"=")) {
	    sprintf(message,"\nscene: unknown operator: %s",op);
	    comMessage(message);
	    return -1;
	  } else {
	    if(!strcmp(val,"0") || 
	       !strcmp(val,"false") ||
	       !strcmp(val,"no")) {
	      glDisable(GL_FOG);
	      gfx.fog=0;
	    } else {
	      glEnable(GL_FOG);
	      gfx.fog=1;
	    }
	  }
	}
	comRedraw();
      } else if(!strcmp(prop,"bg")) {
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(comGetColor(val, &c[0], &c[1], &c[2])!=0) {
	  sprintf(message,"\nscene: unknown color: %s", val);
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
      } else if(!strcmp(prop,"fogc")) {
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(comGetColor(val, &c[0], &c[1], &c[2])!=0) {
	  sprintf(message,"\nscene: unknown color: %s", val);
	  comMessage(message);
	  return -1;
	}
	gfx.fog_color[0]=c[0];
	gfx.fog_color[1]=c[1];
	gfx.fog_color[2]=c[2];
	glFogfv(GL_FOG_COLOR,gfx.fog_color);
	comRedraw();
      } else if(!strcmp(prop,"view")) {
	/**********************
	       set view
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(!strcmp(val,"center"))
	  gfx.stereo_display=GFX_CENTER;
	else if(!strcmp(val,"left"))
	  gfx.stereo_display=GFX_LEFT;
	else if(!strcmp(val,"right"))
	  gfx.stereo_display=GFX_RIGHT;
	else {
	  sprintf(message,"\ninvalid value for view");
	  comMessage(message);
	  return -1;
	}
	gfxSetProjection(gfx.stereo_display);
	comRedraw();
      } else if(!strcmp(prop,"transmat") ||
		!strcmp(prop,"trans")) {
	/**********************
	     set transmat
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(matExtract1D(val,3,v1)!=0) {
	  sprintf(message,"\nscene: error in matrix");
	  comMessage(message);
	  return -1;
	}
	gfx.transform.tra[0]=v1[0];
	gfx.transform.tra[1]=v1[1];
	gfx.transform.tra[2]=v1[2];
	comRedraw();
      } else if(!strcmp(prop,"rotmat") ||
		!strcmp(prop,"rot")) {
	/**********************
	     set rotmat
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(matExtract2D(val,3,3,v1)!=0) {
	  sprintf(message,"\nscene: error in matrix");
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
      } else if(!strcmp(prop,"mmat")) {
	/**********************
	     set modelmat
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	if(matExtract2D(val,4,4,v1)!=0) {
	  sprintf(message,"\nscene: error in 4x4 matrix");
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
      } else if(!strcmp(prop,"rtc")) {
	/**********************
	     set rot trans cen
	**********************/
	if(strlen(op)==0) {
	  sprintf(message,"\nscene: missing operator");
	  comMessage(message);
	  return -1;
	}
	if(strlen(val)==0) {
	  sprintf(message,"\nscene: missing value");
	  comMessage(message);
	  return -1;
	}
	if(strcmp(op,"=")) {
	  sprintf(message,"\nscene: invalid operator: %s",op);
	  comMessage(message);
	  return -1;
	}
	transSetAll(&gfx.transform, val);
	comRedraw();
      } else {
	sprintf(message,"\nunknown expression %s%s%s",prop,op,val);
	comMessage(message);
	return -1;
      }
    }
  } else if(!strcmp(wl[0],"get")) {
    /**********************
             get
     **********************/
    if(wc!=2) {
      sprintf(message,"\nwrong number of arguments");
      comMessage(message);
      return -1;
    }
    if(!strcmp(wl[1],"center")) {
      v1[0]=-gfx.transform.cen[0];
      v1[1]=-gfx.transform.cen[1];
      v1[2]=-gfx.transform.cen[2];
      matAssemble1D(v1,3,message);
    } else if(!strcmp(wl[1],"near")){
      sprintf(message,"%g",gfx.transform.slabn);
    } else if(!strcmp(wl[1],"far")){
      sprintf(message,"%g",gfx.transform.slabf);
    } else if(!strcmp(wl[1],"slabw")){
      sprintf(message,"%g",gfx.transform.slabf-gfx.transform.slabn);
    } else if(!strcmp(wl[1],"transmat") ||
	      !strcmp(wl[1],"trans")){
      v1[0]=gfx.transform.tra[0];
      v1[1]=gfx.transform.tra[1];
      v1[2]=gfx.transform.tra[2];
      matAssemble1D(v1,3,message);
    } else if(!strcmp(wl[1],"rotmat") ||
	      !strcmp(wl[1],"rot")) {
      for(i=0;i<16;i++)
	v1[i]=gfx.transform.rot[i];
      sprintf(message,"{{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f}}",
	      v1[0],v1[1],v1[2],
	      v1[4],v1[5],v1[6],
	      v1[8],v1[9],v1[10]);
    } else if(!strcmp(wl[1],"mmat")){
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

    } else if(!strcmp(wl[1],"rtc")) {
      sprintf(message,transGetAll(&gfx.transform));
    } else if(!strcmp(wl[1],"eyedist")){
      sprintf(message,"%g",gui.eye_dist);
    } else if(!strcmp(wl[1],"fov")){
      sprintf(message,"%g",gfx.fovy);
    } else if(!strcmp(wl[1],"fogd")){
      sprintf(message,"%g",gfx.fog_dist);
    } else if(!strcmp(wl[1],"fixz")){
      if(gfx.fixz)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(!strcmp(wl[1],"dither")){
      if(gfx.dither)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(!strcmp(wl[1],"persp")){
      if(gfx.mode==GFX_PERSP)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(!strcmp(wl[1],"ortho")){
      if(gfx.mode==GFX_ORTHO)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(!strcmp(wl[1],"depthc")){
      if(gfx.fog)
	sprintf(message,"true");
      else
	sprintf(message,"false");
    } else if(!strcmp(wl[1],"bg")) {
      sprintf(message,"{%.3f,%.3f,%.3f}",gfx.r,gfx.g,gfx.b);
    } else if(!strcmp(wl[1],"view")) {
      if(gfx.stereo_display==GFX_RIGHT)
	sprintf(message,"right");
      else if(gfx.stereo_display==GFX_LEFT)
	sprintf(message,"left");
      else
	sprintf(message,"center");
    } else {    
      sprintf(message,"\nunkown property: %s",wl[1]);
      comMessage(message);
      return -1;
    }
    comReturn(message);
  } else if(!strcmp(wl[0],"push")) {
    for(i=1;i<wc;i++)
      scenePush(wl[i]);
  } else if(!strcmp(wl[0],"pop")) {
    ret=scenePop();
    if(ret==NULL) {
      comMessage("\nscene stack is empty");
      comReturn("");
      return -1;
    } else {
      sprintf(message,"%s",ret);
      comReturn(message);
    }
  } else if(!strcmp(wl[0],"peek")) {
    ret=scenePeek();
    if(ret==NULL) {
      comMessage("\nscene stack is empty");
      comReturn("");
      return -1;
    } else {
      sprintf(message,"%s",ret);
      comReturn(message);
    }
    sprintf(message,"%s",ret);
    comReturn(message);
  } else if(!strcmp(wl[0],"clear")) {
    sceneClear();
  } else if(!strcmp(wl[0],"message")) {
    strcpy(set,"");
    for(i=1;i<wc;i++) {
      strcat(set,wl[i]);
      strcat(set," ");
    }
    guiMessage(set);
  } else if(!strcmp(wl[0],"autoslab")) {
    comGetMinMaxSlab();
    gfxSetFog();
    gfxSetProjection(gfx.stereo_display);
    comRedraw();
  } else if(!strcmp(wl[0],"split")) {
    if(gui.stereo_mode==GUI_STEREO_NORMAL) {
      guiStereo(GUI_STEREO_OFF);
      gui.stereo_mode=GUI_STEREO_SPLIT;
    } else if(gui.stereo_mode==GUI_STEREO_OFF) {
      gfx.aspect=0.5*(double)gui.win_width/(double)gui.win_height;
      gui.stereo_mode=GUI_STEREO_SPLIT;
      comRedraw();
    } else if(gui.stereo_mode==GUI_STEREO_SPLIT) {
      gfx.aspect=1.0*(double)gui.win_width/(double)gui.win_height;
      gui.stereo_mode=GUI_STEREO_OFF;
      glViewport(0,0,gui.win_width, gui.win_height);
      comRedraw();
    }
  } else if(!strcmp(wl[0],"stereo")) {
    if(gui.stereo_available!=GLW_STEREO_NONE) {
      if(wc==1) {
	if(gui.stereo_mode) {
	  guiStereo(0);
	} else {
	  guiStereo(1);
	}
      } else {
	if(!strcmp(wl[1],"off")) {
	  guiStereo(0);
	} else if(!strcmp(wl[1],"on")) {
	  guiStereo(1);
	} else {
	  sprintf(message,"\nunknown command '%s'",wl[1]);
	  comMessage(message);
	  return -1;
	}
      }
    } else if(!strcmp(wl[0],"mono")) {
      if(gui.stereo_mode) {
	guiStereo(0);
      }
    } else {
      sprintf(message,"\nStereo mode not available");
      comMessage(message);
      return -1;
    }
  } else if(!strcmp(wl[0],"grab")) {
    if(wc!=2) {
      sprintf(message,"\nSyntax: grab devicename");
      comMessage(message);
    } else {
      if(comGrab(&gfx.transform,wl[1])<0)
	return -1;
    }
  } else if(!strcmp(wl[0],"spin")) {
    if(gfx.spin)
      gfx.spin=0;
    else
      gfx.spin=1;
  } else if (!strcmp(wl[0],"showcp")) {
    scene.cpflag=1;
    comRedraw();
  } else if (!strcmp(wl[0],"hidecp")) {
    scene.cpflag=0;
    comRedraw();
  } else if (!strcmp(wl[0],"flat")) {
    gfx.smooth=0;
    glShadeModel(GL_FLAT);
  } else if (!strcmp(wl[0],"smooth")) {
    gfx.smooth=1;
    glShadeModel(GL_SMOOTH);
  } else if(!strcmp(wl[0],"nolight")) {
    glDisable(GL_LIGHTING);
  } else if(!strcmp(wl[0],"light")) {
    glEnable(GL_LIGHTING);
#ifdef EXPO
  } else if(!strcmp(wl[0],"limit")) {
    if(wc!=5) {
      sprintf(message,"\nscene: wrong #args for limit");
      comMessage(message);
      return -1;
    }
    gfx.limx1=atof(wl[1]);
    gfx.limy1=atof(wl[2]);
    gfx.limz1=atof(wl[3]);
    gfx.limz2=atof(wl[4]);
    gfx.limx2=-gfx.limx1;
    gfx.limy2=-gfx.limy1;
  } else if(!strcmp(wl[0],"morph")) {
    if(wc!=3) {
      sprintf(message,"\nscene: wrong #args for morph");
      comMessage(message);
      return -1;
    }
    sceneMorph(wc-1,wl+1);
  } else if(!strcmp(wl[0],"clear")) {
    scene.label_c=0;
    comRedraw();
  } else if(!strcmp(wl[0],"label")) {
    if(wc!=4) {
      comMessage("\nscene: wrong #args for label");
      return -1;
    }
    matExtract1Df(wl[1],3,scene.label[scene.label_c].p);
    if(comGetColor(wl[2],&cr,&cg,&cb)!=0) {
      comMessage("\nscene: label: unknown color");
      cr=1.0;
      cg=1.0;
      cb=1.0;
    }
    scene.label[scene.label_c].c[0]=cr;
    scene.label[scene.label_c].c[1]=cg;
    scene.label[scene.label_c].c[2]=cb;

    strcpy(scene.label[scene.label_c].s,wl[3]);
    if(scene.label_c<scene.label_m);
    scene.label_c++;
    comRedraw();
#endif
  } else {
    sprintf(message,"\nscene: unknown command %s",wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}


void sceneInputCallback(struct INPUT_MESSAGE *m, void *p)
{

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
  fprintf(stderr,"\n%g  %g %g  %g %g  %g",fact,nx,ny,fx,fy,sinw*gfx.transform.tra[2]);
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
  fprintf(stderr,"\n%g %g %g",q,qn*q,qf*q);
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

int sceneSubCommand(char *sub, int wc, char **wl)
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
  } else {
    sprintf(message,"\nerror: scene: unknown sub expression %s",sub);
    comMessage(message);
    return -1;
  }
  return 0;
}

int sceneSubLightCom(int l, int wc, char **wl)
{
  char message[256];
  if(wc<=0) {
    comMessage("\nerror: scene: missing command");
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
      sprintf(message,"\nlight%d is on",l);
    else
      sprintf(message,"\nlight%d is off",l);
    comMessage(message);
    sprintf(message,"\nposition {%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].pos[0],gfx.light[l].pos[1],
	    gfx.light[l].pos[2],gfx.light[l].pos[3]);
    comMessage(message);
    sprintf(message,"\nambient  {%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].amb[0],gfx.light[l].amb[1],
	    gfx.light[l].amb[2],gfx.light[l].amb[3]);
    comMessage(message);
    sprintf(message,"\ndiffuse  {%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].diff[0],gfx.light[l].diff[1],
	    gfx.light[l].diff[2],gfx.light[l].diff[3]);
    comMessage(message);
    sprintf(message,"\nspecular {%.3f,%.3f,%.3f,%.3f}",
	    gfx.light[l].spec[0],gfx.light[l].spec[1],
	    gfx.light[l].spec[2],gfx.light[l].spec[3]);
    comMessage(message);
    sprintf(message,"attenuation c %.3f  l %.3f   q %.3f",
	    gfx.light[l].kc,gfx.light[l].kl,gfx.light[l].kq);
  } else {
    comMessage("\nerror: scene: unknown command ");
    comMessage(wl[0]);
    return -1;
  }
  return 0;
}

int sceneSubLightSet(int l, int wc, char **wl)
{
  Set set;
  int pc,vc;
  struct POV_VALUE *val;
  float v[4];
  char message[256];

  if(wc<=0) {
    comMessage("\nerror: missing parameters for set");
    return -1;
  }

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  if(set.range_flag) {
    comMessage("\nerror: unexpected range");
    return -1;
  }

  for(pc=0;pc<set.pov_count;pc++) {
    if(set.pov[pc].val_count>1) {
      comMessage("\nerror: expected at most one value");
      return -1;
    }
    val=povGetVal(set.pov+pc,0);
    if(val->range_flag) {
      comMessage("\nerror: unexpected range");
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
	comMessage("\nerror: expected {x,y,z,w} for pos");
	return -1;
      }
      if(matExtract1Df(val->val1,4,v)<0) {
	comMessage("\nerror: in vector ");
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
	  comMessage("\nerror: in vector ");
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
	  comMessage("\nerror: in vector ");
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
	  comMessage("\nerror: in vector ");
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
	comMessage("\nerror: expected {x,y,z} for spotd");
	return -1;
      } else {
	if(matExtract1Df(val->val1,3,v)<0) {
	  comMessage("\nerror: in vector ");
	  comMessage(val->val1);
	  return -1;
	}
	gfx.light[l].spotd[0]=v[0];
	gfx.light[l].spotd[1]=v[1];
	gfx.light[l].spotd[2]=v[2];
	glLightfv(GL_LIGHT0+l, GL_SPOT_DIRECTION, gfx.light[l].spotd);
      }
    } else {
      comMessage("\nerror: scene: unknown light property ");
      comMessage(set.pov[pc].prop);
    }
  }

  setDelete(&set);
  return 0;
}

int sceneSubLightGet(int l, int wc, char **wl)
{
  char message[256];
  if(wc!=1) {
    comMessage("\nerror: expected one property for get");
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
    comMessage("\nerror: unknown light property ");
    comMessage(wl[0]);
    return -1;
  }
  return 0;
}

int sceneSubClipCom(int c, int wc, char **wl)
{
  char message[256];
  if(wc<=0) {
    comMessage("\nerror: scene: missing command");
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
      sprintf(message,"\nSyntax: grab devicename");
      comMessage(message);
    } else {
      if(comGrab(&gfx.clip[c].transform,wl[1])<0)
	return -1;
    }
  } else {
    sprintf(message,"error: unknown scene:clip command: %s",wl[0]);
    comMessage(message);
    return -1;
  }
  return 0;
}

int sceneSubClipSet(int c, int wc, char **wl)
{
  Set set;
  int pc,vc;
  struct POV_VALUE *val;
  float v[4];
  char message[256];

  if(wc<=0) {
    comMessage("\nerror: missing parameters for set");
    return -1;
  }

  if(setNew(&set,wc,wl)<0)
    return -1;
  
  if(set.range_flag) {
    comMessage("\nerror: unexpected range");
    return -1;
  }

  for(pc=0;pc<set.pov_count;pc++) {
    if(set.pov[pc].val_count>1) {
      comMessage("\nerror: expected at most one value");
      return -1;
    }
    val=povGetVal(set.pov+pc,0);
    if(val->range_flag) {
      comMessage("\nerror: unexpected range");
      return -1;
    }
    if(clStrcmp(set.pov[pc].prop,"on")) {
      gfx.clip[c].on=1;
    } else if(clStrcmp(set.pov[pc].prop,"off")) {
      gfx.clip[c].on=0;
    } else if(clStrcmp(set.pov[pc].prop,"pos")) {
      if(val->val1[0]!='{') {
	comMessage("\nerror: expected {x,y,z} for pos");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v)<0) {
	comMessage("\nerror: in vector ");
	comMessage(val->val1);
	return -1;
      }
      gfx.clip[c].pos[0]=v[0];
      gfx.clip[c].pos[1]=v[1];
      gfx.clip[c].pos[2]=v[2];
    } else if(clStrcmp(set.pov[pc].prop,"dir")) {
      if(val->val1[0]!='{') {
	comMessage("\nerror: expected {x,y,z} for pos");
	return -1;
      }
      if(matExtract1Df(val->val1,3,v)<0) {
	comMessage("\nerror: in vector ");
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

int sceneSubClipGet(int c, int wc, char **wl)
{
  return 0;
}

#ifdef EXPO
int sceneMorph(int wc, char **wl)
{
  double m[16],r1[16],r2[16],t1[4],t2[4],rd[16],td[4];
  int i,j,step;

  step=atoi(wl[0]);
  matExtract2D(wl[1],4,4,m);

  for(i=0;i<16;i++)
    r2[i]=m[i];
  r2[3]=0.0; r2[7]=0.0;  r2[11]=0.0;  
  r2[12]=0.0;  r2[13]=0.0;  r2[14]=0.0;  r2[15]=1.0;

  t2[0]=m[12]; t2[1]=m[13]; t2[2]=m[14]; t2[3]=1.0;

  for(i=0;i<16;i++) {
    r1[i]=gfx.transform.rot[i];
    rd[i]=r2[i]-r1[i];
    rd[i]/=(double)step;
  }
  for(i=0;i<3;i++) {
    t1[i]=gfx.transform.tra[i];
    td[i]=t2[i]-t1[i];
    td[i]/=(double)step;
  }  

  fprintf(stderr,"\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
	  rd[0],rd[1],rd[2],rd[3],
	  rd[4],rd[5],rd[6],rd[7],
	  rd[8],rd[9],rd[10],rd[11],
	  rd[12],rd[13],rd[14],rd[15]);
  fprintf(stderr,"\n%f %f %f ",t1[0],t1[1],t1[2]);
  fprintf(stderr,"\n%f %f %f ",t2[0],t2[1],t2[2]);
  fprintf(stderr,"\n%f %f %f ",td[0],td[1],td[2]);


  for(j=0;j<=step;j++) {
    for(i=0;i<16;i++)
      gfx.transform.rot[i]=r1[i]+rd[i]*(double)j;
    for(i=0;i<3;i++) {
      gfx.transform.tra[i]=t1[i]+td[i]*(double)j;
      gfx.transform.slabn-=td[i];
      gfx.transform.slabf-=td[i];
      gfxSetSlab(gfx.transform.slabn,gfx.transform.slabf);
    }
    gfxRedraw();
  }

  return 0;
}

#endif
