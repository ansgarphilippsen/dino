/* Controller */

#import <Cocoa/Cocoa.h>

#import "DinoGLView.h"
#import "CLIView.h"
#import "gui_osx.h"

#include "shell.h"

@interface Controller : NSObject
{
    IBOutlet CLIView *dinoCLI;
    IBOutlet DinoGLView *dinoGL;
    IBOutlet NSTextField *statusBox;
    NSTimer *controlTimer;
}

// Initializatiion
+ (id)dinoController;
- (void)awakeFromNib;
- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)timerControl;
// CLI Interaction
- (void)command:(NSString *)theCommand from:(id)sender;
- (void)putText:(unsigned char *)text;
// Mouse Control 

// Update Display
- (void)updateDisplay;
- (void)swapBuffers;
- (void)updateStatusBox:(NSString *)text;

@end
