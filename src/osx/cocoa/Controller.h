/* Controller */

#import <Cocoa/Cocoa.h>

#import "DinoGLView.h"
#import "CLIView.h"
#import "gui_osx.h"

#include "dino.h"
#include "cmi.h"
#include "shell_raw.h"

@interface Controller : NSObject
{
    IBOutlet DinoGLView *dinoGL;
    IBOutlet CLIView *dinoCLI;
    IBOutlet NSTextField *statusBox;

    NSTimer *controlTimer;
    NSOpenGLContext *offScreenContext;
    void *memPointer;
}

// Initializatiion
+ (id)dinoController;
- (void)awakeFromNib;
- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)timerControl;
// CLI Interaction
- (void)command:(NSString *)theCommand from:(id)sender;
- (void)showCommandResult:(const char *)tmp;
- (void)notifyUser:(NSString *)message;
// Update Display
- (void)swapBuffers;
- (void)updateStatusBox:(NSString *)text;
- (void)resetCurrentContext;
// Main Menu Item
- (IBAction)setWorkingDirectory:(id)sender;
- (IBAction)runScript:(id)sender;
// Custom Menu Item
- (IBAction)autoslab:(id)sender;
- (IBAction)centerCP:(id)sender;
- (IBAction)centerCS:(id)sender;
- (IBAction)calcDist:(id)sender;
- (IBAction)calcAngle:(id)sender;
- (IBAction)calcTorsion:(id)sender;
// OffScrenn OpenGL Context
- (int)offScreenContextWidth:(int)width Height:(int)height Accum:(int)af;
- (void)releaseOffScreenContext;

@end
