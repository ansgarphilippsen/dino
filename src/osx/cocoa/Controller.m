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

    dataSetList = [[NSMutableArray alloc] initWithCapacity:2];
    [[dinoOM tableColumnWithIdentifier:@"displayFlagColumn"] setDataCell:[toggleButton cell]];
    [dinoOM setVerticalMotionCanBeginDrag:YES];
    [dinoOM registerForDraggedTypes:[NSArray arrayWithObjects:@"DinoObjectType"]];
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
    [dataSetList release];
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
// Object menu

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
    DinoDataSet *aDataSet =[[[DinoDataSet alloc] initWithName:name] autorelease];
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
    DinoObject *anObject = [[[DinoObject alloc] initWithName:name inDataSet:db] autorelease];
    
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
	    [self command:theCommand from:(id)sender];
	}else if(![[[self dataSetOfName:parent] childrenOfName:obj] displayFlag]){
	    NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ show",parent,obj];
	    [self command:theCommand from:(id)sender];
	}
    }else if([parent isEqual:@"None"]){
	NSString *dataSet = obj;
	NSEnumerator *enumerator = [[[self dataSetOfName:dataSet] childrenList ] objectEnumerator];
	id anObject;
	if([[self dataSetOfName:dataSet] displayFlag]){
	    [[self dataSetOfName:dataSet] setDisplayFlag:NO];
	    while ((anObject = [enumerator nextObject])) {
		NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ hide",dataSet,[anObject name]];
		[self command:theCommand from:(id)sender];
	    }
	}else if(![[self dataSetOfName:dataSet] displayFlag]){
	    [[self dataSetOfName:dataSet] setDisplayFlag:YES];
	    while ((anObject = [enumerator nextObject])) {
		NSString *theCommand = [NSString localizedStringWithFormat:@".%@.%@ show",dataSet,[anObject name]];
		[self command:theCommand from:(id)sender];
	    }
	}
    }
}

//------------------------------------------------------
// OutlineView data source (for Object menu)

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
