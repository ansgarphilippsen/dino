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

    [[NSFileManager defaultManager] changeCurrentDirectoryPath: NSHomeDirectory()];
    
    [self updateStatusBox:@"Ready"];    

}

- (void)timerControl
{
    guiTimeProc(NULL);
}


//------------------------------------------------------
// Menu Item

- (IBAction)setWorkingDirectory:(id)sender
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    [oPanel setAllowsMultipleSelection:NO];
    [oPanel setCanChooseFiles:NO];
    [oPanel setCanChooseDirectories:YES];
    [oPanel runModalForDirectory:nil file:nil types:nil];

    NSString *directory = [[oPanel filenames] objectAtIndex:0];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath: directory];
    
}

- (IBAction)runScript:(id)sender
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    [oPanel setAllowsMultipleSelection:NO];
    [oPanel setCanChooseFiles:YES];
    [oPanel setCanChooseDirectories:NO];
    [oPanel runModalForDirectory:nil file:nil types:nil];

    NSString *fullPath = [[oPanel filenames] objectAtIndex:0];
    NSString *directory = [fullPath stringByDeletingLastPathComponent];
    NSString *file = [fullPath lastPathComponent];

    [[NSFileManager defaultManager] changeCurrentDirectoryPath:directory];
    NSString *tag = @"@";
    NSString *theScript = [tag stringByAppendingString:file];
    [self command:theScript from:(id)sender];

}

//------------------------------------------------------
// CLI Interaction

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
