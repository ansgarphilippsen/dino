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
    
    guiInitGL();

    

    gui_reshape([dinoGL frame].size.width,[dinoGL frame].size.height);
    

    
    controlTimer=[[NSTimer timerWithTimeInterval: 0.01 target: self selector: @selector(timerControl) userInfo: nil repeats: YES ] retain];
 [[NSRunLoop currentRunLoop] addTimer: controlTimer forMode: NSDefaultRunLoopMode];
/*	
    [NSTimer scheduledTimerWithTimeInterval: 0.1 target: self selector: @selector(timerControl) userInfo: NULL repeats: NO];
*/
    
   [self updateStatusBox:@"Ready"];    

}

- (void)timerControl
{
    gui_timer(NULL);

 //   [[NSRunLoop currentRunLoop] addTimer: controlTimer forMode: NSDefaultRunLoopMode];

/*
    [NSTimer scheduledTimerWithTimeInterval: 0.1 target: self selector: @selector(timerControl) userInfo: NULL  repeats: NO];
*/
}


//------------------------------------------------------
// CLI interaction

- (void)command:(NSString *)theCommand from:(id)sender
{    
    if(shellWorkPrompt([theCommand cString],-1,NULL)==0){
	}
}

- (void) putText:(unsigned char *)tmp
{
    [dinoCLI putText:[NSString stringWithCString:tmp]];  
}

//------------------------------------------------------
// Mouse Events 




//------------------------------------------------------
// Update Display

- (void)updateDisplay
{
    [dinoGL setNeedsDisplay:YES];
}

- (void)swapBuffers
{
    [dinoGL swapBuffers];
}


- (void)updateStatusBox:(NSString *)text
{
    [statusBox setObjectValue:text];
}

@end
