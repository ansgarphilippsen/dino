/* Controller */

#import <Cocoa/Cocoa.h>

#import "DinoObject.h"
#import "DinoGLView.h"
#import "CLIView.h"
#import "gui_osx.h"

#include "dino.h"
#include "cmi.h"
#include "shell_raw.h"

@interface Controller : NSObject
{
    IBOutlet DinoGLView *dinoGL;
    IBOutlet NSTextField *statusBox;
    IBOutlet NSTextField *versionBox;
    IBOutlet CLIView *dinoCLI;
    IBOutlet NSOutlineView *dinoOM;
    IBOutlet NSButton *toggleButton;
    
    NSTimer *controlTimer;
    NSMutableDictionary *dataSetList;
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
- (void)showCommandResult:(NSString *)tmp;
- (void)notifyUser:(NSString *)message;
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
// Object menu
- (void)omAddDB:(NSString *)name;
- (void)omDelDB:(NSString *)name;
- (void)omAddObj:(NSString *)name inDB:(NSString *)db;
- (void)omDelObj:(NSString *)name inDB:(NSString *)db;
- (void)omHideObj:(NSString *)name ofDB:(NSString *)db;
- (void)omShowObj:(NSString *)name ofDB:(NSString *)db;
- (IBAction)toggleDinoObject:(id)sender;
// OutlineView data source (for Object menu)
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item;
- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item;
- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item;
- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item;
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item;
- (BOOL)tableView:(NSTableView *)tableView writeRows:(NSArray*)rows toPasteboard:(NSPasteboard*)pboard;
- (NSDragOperation)tableView:(NSTableView*)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)operation;
- (BOOL)tableView:(NSTableView*)tableView acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)operation;
// Graphical Control
- (IBAction)rotFromSliderX:(id)sender;
- (IBAction)rotFromSliderY:(id)sender;
// OffScrenn OpenGL Context
- (int)offScreenContextWidth:(int)width Height:(int)height Accum:(int)af;
- (void)releaseOffScreenContext;

@end
