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

@end
