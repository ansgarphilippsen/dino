/* DinoGLView */

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLRenderers.h>

#include "cmi.h"
#include "gui_osx.h"

@interface DinoGLView : NSOpenGLView
{

}

// Overrides
- (id)initWithFrame:(NSRect)frameRect;
- (void)drawRect:(NSRect)rect;

// These are standard methods in NSView.
- (void)mouseDown:(NSEvent *)theEvent;
- (void)mouseUp:(NSEvent *)theEvent;
- (void)mouseDragged:(NSEvent *)theEvent;
- (void)otherMouseDown:(NSEvent *)theEvent;
- (void)otherMouseUp:(NSEvent *)theEvent;
- (void)otherMouseDragged:(NSEvent *)theEvent;
- (void)rightMouseDown:(NSEvent *)theEvent;
- (void)rightMouseUp:(NSEvent *)theEvent;
- (void)rightMouseDragged:(NSEvent *)theEvent;

@end

