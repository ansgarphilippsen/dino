/* Controller */

#import <Cocoa/Cocoa.h>

#import "DinoGLView.h"
#import "CLIView.h"

@interface Controller : NSObject
{
    IBOutlet DinoGLView    *dinoGL;
    IBOutlet NSTextField   *statusBox;
    IBOutlet NSTextField   *versionBox;
    IBOutlet CLIView       *dinoCLI;
    
    NSTimer             *controlTimer;
    NSOpenGLContext     *offScreenContext;
    void                *memPointer;
}

// Initializatiion
+ (id)dinoController;
- (void)awakeFromNib;
- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)timerControl;
- (void)dealloc;
// CLI Interaction
- (void)command:(NSString *)theCommand from:(id)sender;
- (void)showCommandResult:(NSString *)tmp;
- (void)notifyUser:(NSString *)message returnPrompt:(BOOL)flag;
// Update Display
- (void)swapBuffers;
- (void)updateStatusBox:(NSString *)text;
- (void)updateVersionBox:(NSString *)text;
- (void)resetCurrentContext;
// Main menu: File items
- (IBAction)setCurrentDirectory:(id)sender;
- (IBAction)runScript:(id)sender;
// Main menu: Scene items (And GFX popup menu items)
- (IBAction)autoslab:(id)sender;
- (IBAction)centerCP:(id)sender;
- (IBAction)centerCS:(id)sender;
- (IBAction)calcDist:(id)sender;
- (IBAction)calcAngle:(id)sender;
- (IBAction)calcTorsion:(id)sender;
- (IBAction)showCPMarker:(id)sender;
- (IBAction)hideCPMarker:(id)sender;
- (IBAction)toggleDepthCueing:(id)sender;
- (IBAction)viewLeft:(id)sender;
- (IBAction)viewRight:(id)sender;
- (IBAction)viewCenter:(id)sender;
- (IBAction)showAxis:(id)sender;
- (IBAction)hideAxis:(id)sender;
- (IBAction)resetAll:(id)sender;
// Graphical Control
- (IBAction)rotFromSliderX:(id)sender;
- (IBAction)rotFromSliderY:(id)sender;
// OffScreen OpenGL Context
- (int)offScreenContextWidth:(int)width Height:(int)height Accum:(int)af;
- (void)releaseOffScreenContext;

@end
