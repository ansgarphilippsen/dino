#import "Controller.h"

static id dinoController;

@implementation Controller

//------------------------------------------------------
// Initialization

+ (id)dinoController
{
    if (!dinoController)
        dinoController=[[self alloc] init];
    return dinoController;
}

- (void)awakeFromNib
{
    dinoController=self;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
    [dinoCLI setCommandHandler:dinoController];

    guiInit(0,0);
    cmiInitGL();
    cmiResize([dinoGL frame].size.width,[dinoGL frame].size.height);
    
    controlTimer=[[NSTimer timerWithTimeInterval: 0.01 target: self selector: @selector(timerControl) userInfo: nil repeats: YES ] retain];

    [[NSRunLoop currentRunLoop] addTimer: controlTimer forMode: NSDefaultRunLoopMode];

    [self updateStatusBox:@"Ready"];    

}

- (void)timerControl
{
    guiTimeProc(NULL);
}


//------------------------------------------------------
// CLI interaction

- (void)command:(NSString *)theCommand from:(id)sender
{
    shellParseRaw([theCommand cString],0);
}

- (void)commandResult:(const char *)tmp
{
    [dinoCLI putText:[NSString stringWithCString:tmp]];  
}

- (void)notifyUser:(const char *)message
{
    [dinoCLI notifyUser:[NSString stringWithCString:message]]; 
}

//------------------------------------------------------
// Update Display

- (void)swapBuffers
{
    [[dinoGL openGLContext] flushBuffer];
}

- (void)updateStatusBox:(NSString *)text
{
    [statusBox setObjectValue:text];
}

- (void)resetCurrentContext
{
    [[dinoGL openGLContext] makeCurrentContext];
}

//------------------------------------------------------
// OffScreen OpenGL Context

- (int)offScreenContextWidth:(int)width Height:(int)height Accum:(int)af
{
    int n, rowBytes, pixDepth=32;
    unsigned memSize;
    NSOpenGLPixelFormat *pixFmt;
    NSOpenGLPixelFormatAttribute attrs[] = {
	NSOpenGLPFAOffScreen,
//	NSOpenGLPFAStencilSize, 8,
//	NSOpenGLPFAAccumSize, pixDepth,
	nil };

    pixFmt = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
    offScreenContext = [[[NSOpenGLContext alloc] initWithFormat:pixFmt shareContext:nil] autorelease];
    if(offScreenContext != nil){
	rowBytes = width * pixDepth/8;
	memSize = height * rowBytes;
	memPointer=malloc(memSize);
	[offScreenContext setOffScreen:memPointer width:(long)width height:(long)height rowbytes:(long)rowBytes];
	[offScreenContext makeCurrentContext];
	n=1;
    }
    else{
	fprintf(stderr,"Initialization of the offscreen context failed.\n");
	n =0;
    }
	    
    return n;
}

- (void)releaseOffScreenContext
{
    free((void *)memPointer);
    [[dinoGL openGLContext] makeCurrentContext];
}

@end
