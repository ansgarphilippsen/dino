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

    [dinoCLI setCommandHandler:dinoController];
    NSDrawer *CLIDrawer = [[[dinoGL window] drawers] objectAtIndex:0];
    [CLIDrawer setContentSize:NSMakeSize(0,160)];	
    [CLIDrawer openOnEdge:NSMinYEdge];

}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
    [[dinoGL window] makeKeyWindow];
    
    [self notifyUser:[NSString stringWithFormat:@"Welcome to dino v%s (http://www.dino3d.org)",VERSION] returnPrompt:NO];
    [self updateVersionBox:[NSString stringWithFormat:@"dino v%s",VERSION]];
	
    guiInit(0,0);
    cmiInitGL();
    cmiResize([dinoGL frame].size.width,[dinoGL frame].size.height);
     
    controlTimer=[[NSTimer timerWithTimeInterval: 0.01 target: self selector: @selector(timerControl) userInfo: nil repeats: YES ] retain];
    [[NSRunLoop currentRunLoop] addTimer: controlTimer forMode: NSDefaultRunLoopMode];

    [self notifyUser:[@"Current directory: " stringByAppendingString:[[NSFileManager defaultManager] currentDirectoryPath]] returnPrompt:YES];
    [self updateStatusBox:@"Ready"];

}

- (void)timerControl
{
    guiTimeProc(NULL);
}

- (void)dealloc{
    [super dealloc];
}

//------------------------------------------------------
// CLI Interaction

- (void)command:(NSString *)theCommand from:(id)sender
{
    shellParseRaw([theCommand cString],0);
}

- (void)showCommandResult:(NSString *)tmp
{
    [dinoCLI putText:tmp];
}

- (void)notifyUser:(NSString *)message returnPrompt:(BOOL)flag
{
    [dinoCLI notifyUser:message returnPrompt:flag];
}

//------------------------------------------------------
// Update GFX window

- (void)swapBuffers
{
    [[dinoGL openGLContext] flushBuffer];
}

- (void)updateStatusBox:(NSString *)text
{
    [statusBox setObjectValue:text];
}

- (void)updateVersionBox:(NSString *)text
{
    [versionBox setObjectValue:text];
}

- (void)resetCurrentContext
{
    [[dinoGL openGLContext] makeCurrentContext];
}

//------------------------------------------------------
// Main menu: File items

- (IBAction)setCurrentDirectory:(id)sender
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
// Main menu: Scene items (And GFX popup menu items)

- (IBAction)autoslab:(id)sender{
   [self command:@"scene autoslab" from:(id)sender]; 
}

- (IBAction)centerCP:(id)sender{
   [self command:@"scene center [$CP]" from:(id)sender]; 
}

- (IBAction)centerCS:(id)sender{
    [self command:@"scene center [$CS]" from:(id)sender]; 
}

- (IBAction)calcDist:(id)sender{
    [self command:@"push [[scene pop]] [[scene pop]]; opr dist; scene message Distance: [pop]" from:(id)sender]; 
}

- (IBAction)calcAngle:(id)sender{
    [self command:@"push [[scene pop]] [[scene pop]] [[scene pop]]; opr angle; scene message Angle:[pop]" from:(id)sender]; 
}

- (IBAction)calcTorsion:(id)sender{
    [self command:@"push [[scene pop]] [[scene pop]] [[scene pop]] [[scene pop]]; opr torsion; scene message Torsion: [pop]" from:(id)sender]; 
}

- (IBAction)showCPMarker:(id)sender{
    [self command:@"scene showcp" from:(id)sender];
}

- (IBAction)hideCPMarker:(id)sender{
    [self command:@"scene hidecp" from:(id)sender];  
}

- (IBAction)toggleDepthCueing:(id)sender{
    [self command:@"scene depthc" from:(id)sender];  
}

- (IBAction)viewLeft:(id)sender{
    [self command:@"scene set view=left" from:(id)sender];
}

- (IBAction)viewRight:(id)sender{
    [self command:@"scene set view=right" from:(id)sender];
}

- (IBAction)viewCenter:(id)sender{
    [self command:@"scene set view=center" from:(id)sender];
}

- (IBAction)showAxis:(id)sender{
    [self command:@"scene set axis=1" from:(id)sender];
}

- (IBAction)hideAxis:(id)sender{
    [self command:@"scene set axis=0" from:(id)sender];
}

- (IBAction)resetAll:(id)sender{
    [self command:@"scene reset all" from:(id)sender];
}

//------------------------------------------------------
// Graphical Control

- (IBAction)rotFromSliderX:(id)sender
{
/*
    int eventType, mask;
    float px, py;

    px = [sender intValue];
    py = 1;

    eventType = CMI_BUTTON_PRESS;
    mask = CMI_BUTTON1_MASK;
    
    eventType = CMI_MOTION;
    mask = CMI_BUTTON1_MASK;

    gui_mouse_input((int)eventType, (int)mask, (int)px, (int)py);
*/
}

- (IBAction)rotFromSliderY:(id)sender
{
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
	[self showCommandResult:@"Initialization of the offscreen context failed."];
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
