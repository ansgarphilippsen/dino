#import "DinoGLView.h"

@implementation DinoGLView

- (id)initWithFrame:(NSRect) frame
{
    return self;
}

- (void)drawRect:(NSRect)rect
{
//    fprintf(stderr,"drawRect (%p) (%fx%f)\n",[NSOpenGLContext currentContext],rect.size.width,rect.size.height);
    cmiResize((int)rect.size.width,(int)rect.size.height);
    cmiRedraw();
}
 
//---------------------------------------------------------------------------------
// mouse control

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    NSRect frame = [self frame];
    float nx, ny, ctrx, ctry;
    nx = frame.size.width;
    ny = frame.size.height;
    
    // Figure the center of the view.
    ctrx = frame.origin.x + frame.size.width * 0.5;
    ctry = frame.origin.y + frame.size.height * 0.5;    
}

- (void)mouseUp:(NSEvent *)theEvent
{

}

- (void)mouseDragged:(NSEvent *)theEvent
{
  NSPoint pt = [theEvent locationInWindow];
  gui_mouse_drag(1,(int)pt.x,(int)pt.y);
}


@end



