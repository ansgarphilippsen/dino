/* DinoGLView */

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLRenderers.h>

@interface DinoGLView : NSOpenGLView
{
    NSOpenGLPixelFormatAttribute	pfAttribs[32];
    NSOpenGLPixelFormat			*nsglFormat;
    NSOpenGLContext			*glContext;

/*
    // Pixel format & context stuff
    long				renderID;
    unsigned long			bitsPerPixel, depthSize, stencilSize, numAuxBuffs;
    BOOL 				enableDepth, packedPixels, pause;
    BOOL				useDblBuffer, useAccel, useNoRecover, useAux, useStencil, useAllRender;
    
    // Timer & framerate info
    NSTimer				*renderTimer, *frameTimer, *frameRateTimer;
    NSTimeInterval			frameTimeInterval, timeInterval, frameRateTimeInterval;
    double 				fps, bufferSwaps, frMin, frMax, frAvg, elapseTime, delta, time;
    unsigned long 			start, totalFrames;
*/
}

// Overrides
//+ (id)dinoGL;
//- (void)awakeFromNib;
- (id)initWithFrame:(NSRect) frameRect;
- (id)swapBuffers;
- (void)drawRect:(NSRect)rect;

// These are standard methods in NSView.
- (void)mouseDown:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;

@end

