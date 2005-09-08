#import "OMController.h"

#import "Controller.h"
#import "DataObject.h"
#include "gui_osx.h"
#include "gui_ext.h"

static id dinoOMController;

@implementation OMView

/*
- (void)mouseDown:(NSEvent *)theEvent
{
    [NSApp preventWindowOrdering];
}

- (BOOL)shouldDelayWindowOrderingForEvent:(NSEvent *)theEvent
{
    return YES;
}
*/

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

- (NSMenu *)menuForEvent:(NSEvent *)theEvent
{
    int row = [self rowAtPoint:[self convertPoint:[theEvent locationInWindow] fromView:nil]];
    [self selectRow:row byExtendingSelection:NO];
    return [self menu];
}

@end

@implementation OMController

//------------------------------------------------------
// Initialization

+ (id)dinoOMController
{
    if (!dinoOMController)
        dinoOMController=[[self alloc] init];
    return dinoOMController;
}

- (void)awakeFromNib
{
    dinoOMController=self;
	
    dataSetList = [[NSMutableArray alloc] initWithCapacity:2];
	
//    [dinoOM setVerticalMotionCanBeginDrag:YES];
//    [dinoOM registerForDraggedTypes:[NSArray arrayWithObjects:@"DinoObjectType"]];
    
}

- (void)dealloc{
    [dataSetList release];
    [super dealloc];
}

//------------------------------------------------------
// OM Interactions

- (id)dataSetOfName:(NSString *)name{
    NSEnumerator *enumerator = [dataSetList objectEnumerator];
    id aDataSet;
    while (aDataSet = [enumerator nextObject]) {
	if([[aDataSet name] isEqual:name]){ return aDataSet;}
    }
    return nil;
}

- (void)omAddDB:(NSString *)name
{
    DataSet *aDataSet =[[[DataSet alloc] initWithName:name] autorelease];
    [dataSetList addObject:aDataSet];
    [dinoOM reloadData];
}

- (void)omDelDB:(NSString *)name
{
    [dataSetList removeObjectIdenticalTo:[self dataSetOfName:name]];
    [dinoOM reloadData];
}

- (void)omAddObj:(NSString *)name inDB:(NSString *)db
{
    DataObject *anObject = [[[DataObject alloc] initWithName:name inDataSet:db] autorelease];

    [[self dataSetOfName:db] addChildren:anObject];
    [dinoOM reloadData];
    [dinoOM expandItem:[self dataSetOfName:db]];
}

- (void)omDelObj:(NSString *)name inDB:(NSString *)db
{
    [[self dataSetOfName:db] removeChildren:name];
    [dinoOM reloadData];
}

- (void)omHideObj:(NSString *)name ofDB:(NSString *)db
{
    [[[self dataSetOfName:db] childrenOfName:name] setDisplayFlag:NO];
    [dinoOM reloadData];
}

- (void)omShowObj:(NSString *)name ofDB:(NSString *)db
{
    [[[self dataSetOfName:db] childrenOfName:name] setDisplayFlag:YES];
    [dinoOM reloadData];
}

- (IBAction)toggleDinoObject:(id)sender
{
    NSString *obj,*parent;
    int row = [sender selectedRow];
    obj = [[dinoOM itemAtRow:row] name];
    parent = [[dinoOM itemAtRow:row] parentDataSet];

    if(![parent isEqual:@"None"]){
	if([[[self dataSetOfName:parent] childrenOfName:obj] displayFlag]){
	    NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ hide",parent,obj];
	    [[Controller dinoController] command:theCommand from:(id)sender];
	}else if(![[[self dataSetOfName:parent] childrenOfName:obj] displayFlag]){
	    NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ show",parent,obj];
	    [[Controller dinoController] command:theCommand from:(id)sender];
	}
    }else if([parent isEqual:@"None"]){
	NSString *dataSet = obj;
	NSEnumerator *enumerator = [[[self dataSetOfName:dataSet] childrenList ] objectEnumerator];
	id anObject;
	if([[self dataSetOfName:dataSet] displayFlag]){
	    [[self dataSetOfName:dataSet] setDisplayFlag:NO];
	    while ((anObject = [enumerator nextObject])) {
		NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ hide",dataSet,[anObject name]];
		[[Controller dinoController] command:theCommand from:(id)sender];
	    }
	}else if(![[self dataSetOfName:dataSet] displayFlag]){
	    [[self dataSetOfName:dataSet] setDisplayFlag:YES];
	    while ((anObject = [enumerator nextObject])) {
		NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ show",dataSet,[anObject name]];
		[[Controller dinoController] command:theCommand from:(id)sender];
	    }
	}
    }
}

- (IBAction)centerOnTarget:(id)sender
{
    NSString *obj,*parent;
    int row = [dinoOM selectedRow];
    obj = [[dinoOM itemAtRow:row] name];
    parent = [[dinoOM itemAtRow:row] parentDataSet];
    
    if(![parent isEqual:@"None"]){
        NSString *theCommand = [NSString localizedStringWithFormat:@"scene center [.%@.%@]",parent,obj];
        [[Controller dinoController] command:theCommand from:(id)sender];
    }else if([parent isEqual:@"None"]){
        NSString *theCommand = [NSString localizedStringWithFormat:@"scene center [.%@]",obj];
        [[Controller dinoController] command:theCommand from:(id)sender];
    }
    
}


//------------------------------------------------------
// OutlineView data source

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil){
	return [dataSetList count];
    }else{
	return [[item childrenList] count];
    }
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    if(item == nil){
	return YES;
    }else{
	return ([[item childrenList] count] >= 1) ? YES : NO;
    }
    return NO;
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
    if(item == nil){
	return [dataSetList objectAtIndex:index];
    }else{
	if ([item childrenList] != nil) return [[item childrenList] objectAtIndex:index];
    }
    return nil;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    if ([[tableColumn identifier] isEqual:@"dataSetColumn"]){
	return [item name];
    }else if ([[tableColumn identifier] isEqual:@"displayFlagColumn"]) {
	return [NSNumber numberWithBool:[item displayFlag]];
    }
    return nil;
}

//Delegate method
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    if ([[tableColumn identifier] isEqual:@"dataSetColumn"]){
	return NO;
    }else if ([[tableColumn identifier] isEqual:@"displayFlagColumn"]) {
	return YES;
    }
    return NO;
}

/*
//Drag and Drop methods
- (BOOL)tableView:(NSTableView *)tableView writeRows:(NSArray *)rows toPasteboard:(NSPasteboard *)pboard
{
    if ([rows count]==1){
	NSString *draggedObject = [[dinoOM itemAtRow:[rows objectAtIndex:0]] name];
	[pboard declareTypes:[NSArray arrayWithObjects:@"DinoObjectType"] owner:nil];
	[pboard setString:draggedObject forType:@"DinoObjectType"];
	return YES;
    }
    return NO;
}

- (NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id <NSDraggingInfo>)info proposedRow:(int)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    NSArray *typeArray=[[info draggingPasteboard] types];
    if ([typeArray count]==1 && row!=-1){
	if ([[typeArray objectAtIndex:0] isEqualToString:@"DinoObjectType"] && operation==NSTableViewDropAbove){
	    return NSDragOperationMove;
	}
    }
    return NSDragOperationNone;
}

- (BOOL)tableView:(NSTableView*)tableView acceptDrop:(id <NSDraggingInfo>)info row:(int)row dropOperation:(NSTableViewDropOperation)operation
{
    return YES;
}
*/

@end



