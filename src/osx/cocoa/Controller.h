/* Controller */

#import <Cocoa/Cocoa.h>

#import "DinoGLView.h"
#import "CLIView.h"
#import "gui_osx.h"

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
// Menu Item
- (IBAction)setWorkingDirectory:(id)sender;
- (IBAction)runScript:(id)sender;
// CLI Interaction
- (void)command:(NSString *)theCommand from:(id)sender;
- (void)commandResult:(const char *)tmp;
- (void)notifyUser:(const char *)message;
// Update Display
- (void)swapBuffers;
- (void)updateStatusBox:(NSString *)text;
- (void)resetCurrentContext;
// OffScrenn OpenGL Context
- (int)offScreenContextWidth:(int)width Height:(int)height Accum:(int)af;
- (void)releaseOffScreenContext;

@end
