/* OMController */

#import <Cocoa/Cocoa.h>

@interface OMController : NSObject
{
    IBOutlet NSOutlineView *dinoOM;
    IBOutlet NSButton      *toggleButton;

    NSMutableArray         *dataSetList;
}

// Initializatiion
+ (id)dinoOMController;
- (void)awakeFromNib;
- (void)dealloc;
// OM Interactions
- (id)dataSetOfName:(NSString *)name;
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
//Delegate method
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item;
/*
//Drag and Drop methods
- (BOOL)tableView:(NSTableView *)tableView writeRows:(NSArray *)rows toPasteboard:(NSPasteboard *)pboard;
- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)operation;
- (BOOL)tableView:(NSTableView*)tableView acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)operation;
*/

@end