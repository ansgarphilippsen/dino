/******************************

  GUI based on SDL library

*******************************/


#include "gui.h"
#include "dino.h"
#include "gfx.h"
#include "com.h"

struct GUI gui;

/*
  static functions only used locally
*/

// timer callback

static SDL_NewTimerCallback timer_callback(Uint32 interval, void *param)
{
  SDL_Event event;
  
  event.type = SDL_USEREVENT;
  event.user.code = 99;
  event.user.data1 = NULL;
  event.user.data2 = NULL;
  SDL_PushEvent(&event);

  SDL_AddTimer(10,(SDL_NewTimerCallback)timer_callback,NULL);
  
  return NULL;
}

// event handling

static void timer_event() 
{
  if(gui.redraw) {
    gui.redraw=0;
    gfxRedraw();
  }

  comTimeProc();
}


static void key_event(SDL_keysym* keysym)
{
}

static void mouse_motion_event(SDL_MouseMotionEvent *motion)
{
  int mask=0;
  SDLMod km;

  km=SDL_GetModState();

  if(motion->state & SDL_BUTTON(1))
    mask += GUI_BUTTON1_MASK;

  if(motion->state & SDL_BUTTON(2))
    mask += GUI_BUTTON2_MASK;

  if(motion->state & SDL_BUTTON(3))
    mask += GUI_BUTTON3_MASK;

  if(km & KMOD_SHIFT)
    mask += GUI_SHIFT_MASK;

  comTransform(TRANS_MOUSE, mask, 0, -motion->xrel);
  comTransform(TRANS_MOUSE, mask, 1, -motion->yrel);
}

static void mouse_button_event(SDL_MouseButtonEvent *button)
{
}

static void resize_event(SDL_ResizeEvent *res)
{
  gui.win_width=res->w;
  gui.win_height=res->h;
  SDL_SetVideoMode(gui.win_width,gui.win_height,gui.bpp,gui.sdl_flags);
  gfxSetViewport();
  comRedraw();
}

/*
  externally visible functions
*/

int guiInit(void (*func)(int, char **), int *argc, char ***argv)
{
  const SDL_VideoInfo* info = NULL;

  fprintf(stderr,"GUI based on SDL (www.libsdl.org)\n");

  if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 ) {
    fprintf(stderr,"SDL: Video initialization failed: %s\n", SDL_GetError());
    return -1;
  }

  info=SDL_GetVideoInfo();
  if(!info) {
    fprintf(stderr,"SDL: Video query failed: %s\n",SDL_GetError());
    return -1;
  }

  SDL_WM_SetCaption("DINO gfx","dino");

  gui.win_width=500;
  gui.win_height=500;
  gui.bpp=info->vfmt->BitsPerPixel;

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

  gui.sdl_flags = SDL_OPENGL | SDL_RESIZABLE;

  if(SDL_SetVideoMode(gui.win_width,gui.win_height,gui.bpp,gui.sdl_flags)==0) {
    fprintf( stderr, "SDL: Video setmode failed: %s\n",SDL_GetError());
    return -1;
  }

  // initialize OpenGL
  gfxGLInit();

  return 0;
}

int guiMainLoop()
{
  SDL_Event event;
  int ret;

  // register timer
  SDL_AddTimer(10,(SDL_NewTimerCallback)timer_callback,NULL);

  while(1) {
    ret=SDL_WaitEvent(&event);
    switch(event.type) {
    case SDL_USEREVENT:
      timer_event();
      break;
    case SDL_MOUSEMOTION:
      mouse_motion_event(&event.motion);
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      mouse_button_event(&event.button);
      break;
    case SDL_KEYDOWN:
      key_event(&event.key.keysym);
      break;
    case SDL_VIDEORESIZE:
      resize_event(&event.resize);
      break;
    case SDL_QUIT:
      dinoExit(0);
      break;
    }

  }
  return 0;
}

int guiResolveColor(const char *name, float *r, float *g, float *b)
{
  return 0;
}

int guiMessage(char *m)
{
  return 0;
}

void guiSwapBuffers()
{
  SDL_GL_SwapBuffers();
}
