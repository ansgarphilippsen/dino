#import "DinoGLView.h"

@implementation DinoGLView

- (id)initWithFrame:(NSRect)frameRect
{
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFADoubleBuffer,
	NSOpenGLPFAAccelerated,
	NSOpenGLPFAColorSize, 16,
	NSOpenGLPFADepthSize, 1,
        nil };
    NSOpenGLPixelFormat *pixFmt;

    pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    self = [super initWithFrame:frameRect pixelFormat:pixFmt];

    [[self openGLContext] makeCurrentContext];
    
    return self;
}

- (void)drawRect:(NSRect)rect
{
    cmiResize((int)rect.size.width,(int)rect.size.height);
    cmiRedraw();
}
 
//---------------------------------------------------------------------------------
// mouse control

- (void)mouseDown:(NSEvent *)theEvent
{ 
    float px, py;
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];
    
    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gui_mouse_down(1,(int)px,(int)py);  
}

- (void)mouseUp:(NSEvent *)theEvent
{
    float px, py;
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gui_mouse_up(1,(int)px,(int)py);  
 
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    float px, py;
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];

    pt = [self convertPoint:pt fromView:nil];
    px = pt.x;
    py = frame.size.height - pt.y;

    gui_mouse_drag(1,(int)px,(int)py);
}


@end



