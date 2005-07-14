#import "DinoGLView.h"

#import  "Controller.h"
#include "gui_osx.h"
#include "cmi.h"

@implementation DinoGLView

- (id)initWithFrame:(NSRect)frameRect
{
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 1,
        NSOpenGLPFAStencilSize, 1,
		NSOpenGLPFAMaximumPolicy,
        nil };
    NSOpenGLPixelFormat *pixFmt;

    pixFmt = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
    self = [super initWithFrame:frameRect pixelFormat:pixFmt];

    [[self openGLContext] makeCurrentContext];
    
    return self;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

- (void)drawRect:(NSRect)rect
{
//    [sliderX setMaxValue:(double)rect.size.width];
//    [sliderY setMaxValue:(double)rect.size.height];
//    [[Controller dinoController] updateStatusBox:[NSString stringWithFormat:@"%f",[sliderX maxValue]]];  
    
	if([self inLiveResize])	cmiResize((int)rect.size.width,(int)rect.size.height);
    cmiRedraw();
}
 
//---------------------------------------------------------------------------------
// mouse control

- (void)mouseDown:(NSEvent *)theEvent
{
    int eventType, mask;
    float px, py;

    unsigned int flags = [theEvent modifierFlags];
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    eventType = CMI_BUTTON_PRESS;
    mask = CMI_BUTTON1_MASK;
    if(flags & NSCommandKeyMask) mask = CMI_BUTTON2_MASK;
    if(flags & NSAlternateKeyMask) mask += CMI_BUTTON2_MASK;
    if(flags & NSShiftKeyMask) mask += CMI_SHIFT_MASK;
    if(flags & NSAlphaShiftKeyMask) mask += CMI_CNTRL_MASK;
//    if(flags & NSCommandKeyMask) mask += CMI_CNTRL_MASK;

    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gfx_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
}

- (void)mouseUp:(NSEvent *)theEvent
{
    int eventType, mask;
    float px, py;

    unsigned int flags = [theEvent modifierFlags];
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    eventType = CMI_BUTTON_RELEASE;
    mask = CMI_BUTTON1_MASK;
    if(flags & NSCommandKeyMask) mask = CMI_BUTTON2_MASK;
    if(flags & NSAlternateKeyMask) mask += CMI_BUTTON2_MASK;
    if(flags & NSShiftKeyMask) mask += CMI_SHIFT_MASK;
    if(flags & NSAlphaShiftKeyMask) mask += CMI_CNTRL_MASK;
//    if(flags & NSCommandKeyMask) mask += CMI_CNTRL_MASK;
    
    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gfx_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    int eventType, mask;
    float px, py;

    unsigned int flags = [theEvent modifierFlags];
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    eventType = CMI_MOTION;
    mask = CMI_BUTTON1_MASK;
    if(flags & NSCommandKeyMask) mask = CMI_BUTTON2_MASK;
    if(flags & NSAlternateKeyMask) mask += CMI_BUTTON2_MASK;
    if(flags & NSShiftKeyMask) mask += CMI_SHIFT_MASK;
    if(flags & NSAlphaShiftKeyMask) mask += CMI_CNTRL_MASK;
//    if(flags & NSCommandKeyMask) mask += CMI_CNTRL_MASK;
    
    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gfx_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
    int eventType, mask;
    float px, py;

    unsigned int flags = [theEvent modifierFlags];
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    eventType = CMI_BUTTON_PRESS;
    mask = CMI_BUTTON2_MASK;
    if(flags & NSShiftKeyMask) mask += CMI_SHIFT_MASK;
    if(flags & NSAlphaShiftKeyMask) mask += CMI_CNTRL_MASK;
//    if(flags & NSCommandKeyMask) mask += CMI_CNTRL_MASK;
    
    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gfx_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
    int eventType, mask;
    float px, py;

    unsigned int flags = [theEvent modifierFlags];
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    eventType = CMI_BUTTON_RELEASE;
    mask = CMI_BUTTON2_MASK;
    if(flags & NSShiftKeyMask) mask += CMI_SHIFT_MASK;
    if(flags & NSAlphaShiftKeyMask) mask += CMI_CNTRL_MASK;
//    if(flags & NSCommandKeyMask) mask += CMI_CNTRL_MASK;

    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gfx_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
    int eventType, mask;
    float px, py;

    unsigned int flags = [theEvent modifierFlags];
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    eventType = CMI_MOTION;
    mask = CMI_BUTTON2_MASK;
    if(flags & NSShiftKeyMask) mask += CMI_SHIFT_MASK;
    if(flags & NSAlphaShiftKeyMask) mask += CMI_CNTRL_MASK;
//    if(flags & NSCommandKeyMask) mask += CMI_CNTRL_MASK;
    
    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gfx_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
}

@end

@implementation GFXWindow

- (void)keyDown:(NSEvent *)theEvent
{
    [[Controller dinoController] sendEventToCLI:theEvent];
}

@end


