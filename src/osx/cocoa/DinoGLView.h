/* DinoGLView */

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <OpenGL/CGLRenderers.h>

@interface DinoGLView : NSOpenGLView
{
    IBOutlet NSSlider *sliderX;
    IBOutlet NSSlider *sliderY;
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
- (void)keyDown:(NSEvent *)theEvent;

@end

