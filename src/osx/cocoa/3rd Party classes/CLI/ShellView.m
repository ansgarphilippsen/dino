/* ShellView.m Copyright (c) 1998-2001 Philippe Mougin.  */
/*   This software is Open Source. See the license.  */  

#import "ShellView.h"
#import "StrBuffer.h"

#define RETURN    @"\015"
#define BACKSPACE @"\177"  // Note SHIFT + BACKSPACE gives "\010"

// Some modifiers we want to test the presence. 
#define MODIF (NSAlphaShiftKeyMask | NSShiftKeyMask | NSControlKeyMask | NSAlternateKeyMask | NSCommandKeyMask | NSHelpKeyMask)
  
static NSDictionary *errorAttributes; // a dictionary of attributes that defines how an error in a command is shown. 
static BOOL useMaxSize;


@implementation ShellView

/////////////////////////////// PRIVATE ////////////////////////////

+ (void) setUseMaxSize:(BOOL)shouldUseMaxSize
{
  useMaxSize = shouldUseMaxSize;
}

- (void) replaceCurrentCommandWith:(NSString *)newCommand   // This method is used when the user browse into the command history
{
  [self setSelectedRange:NSMakeRange(start,[[self string] length])];
  [self insertText:newCommand];
  [self moveToEndOfDocument:self];
  [self scrollRangeToVisible:[self selectedRange]];
  lineEdited = NO; 
}

/////////////////////////////// PUBLIC ////////////////////////////


+ (void)setup // We use +setup instead of the normal +initialize mechanism because of a bug in GNUstep that prevent from allocating objects in +initialize 
{
  static BOOL tooLate = NO;
  if (!tooLate) 
  {
    tooLate = YES;
    errorAttributes = [[NSDictionary alloc] initWithObjectsAndKeys: [NSColor whiteColor],NSForegroundColorAttributeName, [NSColor blackColor], NSBackgroundColorAttributeName, nil];    
    useMaxSize = YES;
   }
}

- (id)commandHandler { return commandHandler;}

- (void) dealloc
{
  //NSLog(@"ShellView dealloc");
  [prompt release];
  [history release];
  if (shouldRetainCommandHandler) [commandHandler release];
  [super dealloc];
}

- (id)initWithFrame:(NSRect)frameRect
{
  return [self initWithFrame:frameRect prompt:@"DINO> " historySize:2000 commandHandler:nil];
}

- (id)initWithFrame:(NSRect)frameRect prompt:(NSString *)thePrompt historySize:(int)theHistorySize commandHandler:(id)theCommandHandler
{
  [[self class] setup];
  if (self = [super initWithFrame:frameRect])
  {
    prompt             = [thePrompt retain];
    history            = [[StrBuffer alloc] initWithUIntSize:theHistorySize];
    parserMode         = NO_DECOMPOSE;
    commandHandler     = [theCommandHandler retain];
    lineEdited         = NO;
    last_command_start = 0;
    shouldRetainCommandHandler = YES;

    [self setFont:[NSFont userFixedPitchFontOfSize:10]];
    [self setSelectedRange:NSMakeRange([[self string] length],0)];
//    [super insertText:prompt];
    start = [[self string] length];
    [self setDelegate:self];   // A CLIView is its own delegate! (see the section implementing delegate methods)
    maxSize = 100000;
    return self;
  }
  return nil;
}

- (void)insertText:(NSString *)aString
{
   [super insertText:aString];
}

- (void)notifyUser:(NSString *)notification returnPrompt:(BOOL)flag
{
    NSString *command = [[self string] substringFromIndex:start];
    NSRange selectedRange = [self selectedRange];
  
    [self setSelectedRange:NSMakeRange(start,[[self string] length])];
    if(flag){
	int delta = [prompt length] + [notification length] + 2;
	[self insertText:[NSString stringWithFormat:@"\n%@\n%@%@",notification,prompt,command]];
	[self setFont:[NSFont boldSystemFontOfSize:10] range:NSMakeRange(start,[notification length]+1)];
	start += delta;
	[self setSelectedRange:NSMakeRange(selectedRange.location+delta,selectedRange.length)];
    }else{
	int delta = [notification length] + 2;
	[self insertText:[NSString stringWithFormat:@"\n%@\n",notification]];
	[self setFont:[NSFont boldSystemFontOfSize:10] range:NSMakeRange(start,[notification length]+1)];
	start += delta;
	[self setSelectedRange:NSMakeRange(selectedRange.location+delta,selectedRange.length)];
	
    }
}

- (void)keyDown:(NSEvent *)theEvent  // Called by the AppKit when the user press a key.
{
  
  //NSLog(@"key = %@",[theEvent characters]);
  //NSLog(@"modifierFlags = %x",[theEvent modifierFlags]);
  
  if ([theEvent type] != NSKeyDown) [super keyDown:theEvent];
 
  // Is the current insertion point valid ?
  if ([self selectedRange].location < start 
      && !([theEvent modifierFlags] & NSShiftKeyMask 
           && (   [[theEvent characters] characterAtIndex:0] == NSLeftArrowFunctionKey 
               || [[theEvent characters] characterAtIndex:0] == NSRightArrowFunctionKey
               || [[theEvent characters] characterAtIndex:0] == NSUpArrowFunctionKey
               || [[theEvent characters] characterAtIndex:0] == NSDownArrowFunctionKey)))
  {
    [self moveToEndOfDocument:self];
    [self scrollRangeToVisible:[self selectedRange]];  
  }

  if ([[theEvent characters] isEqualToString:RETURN] && !([theEvent modifierFlags] & NSControlKeyMask)) 
  {
    // *RETURN* : The user has entered a command
    NSString *command = [[self string] substringFromIndex:start];
    long overflow;

    if (useMaxSize && (overflow = [[self string] length] - maxSize) > 0)
    {
      overflow = overflow + maxSize / 3;
      [self replaceCharactersInRange:NSMakeRange(0,overflow) withString:@""];
      start = start - overflow;
    }    

    last_command_start = start;
    if ([command length] > 0 && ![command isEqualToString:[history getMostRecentlyInsertedStr]])
      [history addStr:command];
    [history goToLast];
    [self moveToEndOfDocument:self];
    [self insertText:@"\n"];
    [commandHandler command:command from:self]; // The command handler is notified
    [self insertText:prompt];
    [self scrollRangeToVisible:[self selectedRange]];
    start = [[self string] length];
    lineEdited = NO;
  }
  else if ([[theEvent charactersIgnoringModifiers] isEqualToString:RETURN] && [theEvent modifierFlags] & NSControlKeyMask)
  {
    [self insertText:@"\n"];
  }
  else if ([[theEvent characters] characterAtIndex:0] == NSLeftArrowFunctionKey)
  {
    if  ([theEvent modifierFlags] & NSControlKeyMask)  
      // *CONTROL + LEFT ARROW* : go to start of current command
      [self setSelectedRange:NSMakeRange(start,0)];
    else if ([theEvent modifierFlags] & NSShiftKeyMask || [self selectedRange].location > start)
      [super keyDown:theEvent];
  }
  else if ([[theEvent characters] characterAtIndex:0] == NSRightArrowFunctionKey && ([theEvent modifierFlags] & NSControlKeyMask))
  {
    // *CONTROL + RIGHT ARROW* : go to end of current command
    [self moveToEndOfDocument:self];
  } 
  else if ([[theEvent characters] characterAtIndex:0] == NSUpArrowFunctionKey && !([theEvent modifierFlags] & MODIF) )
  {
    // *UP ARROW without any of the modifier key defined in MODIF*
    // if we are on the first line of current command ==> replace current command by the previous one (history)
    //                                           else ==> apply the normal text edition behaviour.
    unsigned loc = [self selectedRange].location;
    [self moveUp:self];  // principle: to see where we are (i.e. where the insertion point is), temporary move the insertion point up  
                         // one line then see if we are still on the current command or not.
    if ([self selectedRange].location > start && [self selectedRange].location != loc)
    {
      // we are still on the current command, so move down one line then apply the normal text behaviour.
      [self moveDown:self];
      [super keyDown:theEvent]; 
    }
    else if ([self selectedRange].location >= start-[prompt length] && [self selectedRange].location != loc)
    {
      // we are on the promt, so move to the start of the current command (the insertion point should not be on the prompt)
      [self setSelectedRange:NSMakeRange(start,0)];
    }
    else
    {
      // get previous command in history
      
      if (lineEdited) // if the current command has been edited by the user, save it in the history.
      {
        NSString *command = [[self string] substringFromIndex:start];
        if ([command length] > 0) { [history addStr:command]; [history goToPrevious]; }
      }
      [self replaceCurrentCommandWith:[[history goToPrevious] getStr]]; 
    }
  }
  else if ([[theEvent characters] characterAtIndex:0] == NSUpArrowFunctionKey && ([theEvent modifierFlags] & NSControlKeyMask))
  {
    // *CONTROL + UP ARROW* : get previous command in history
    [self replaceCurrentCommandWith:[[history goToPrevious] getStr]];
  }
  else if ([[theEvent characters] characterAtIndex:0] == NSDownArrowFunctionKey && !([theEvent modifierFlags] & MODIF) )
  {
    // *DOWN ARROW without any of the modifier key defined in MODIF*
    // if we are on the last line of current command ==> replace current command by the next one (history)
    //                                          else ==> apply the normal text edition behaviour.
    unsigned loc = [self selectedRange].location;
    
    [self moveDown:self];
    if ([self selectedRange].location != loc)
    {
      [self moveUp:self];
      [super keyDown:theEvent]; 
    }
    else
    {
      if (lineEdited)
      {
        NSString *command = [[self string] substringFromIndex:start];
        if ([command length] > 0)  [history addStr:command];
      }
      else
        [history goToNext];
      
      [self replaceCurrentCommandWith:[history getStr]];
    }
  }
  else if ([[theEvent characters] characterAtIndex:0] == NSDownArrowFunctionKey && ([theEvent modifierFlags] & NSControlKeyMask))
  {
    // *CONTROL + DOWN ARROW* : get next command in history 
    [self replaceCurrentCommandWith:[[history goToNext] getStr]];
  }
  else if ([[theEvent characters] isEqualToString:BACKSPACE] && ([theEvent modifierFlags] & NSControlKeyMask))
  {
    // *CONTROL + BACKSPACE : delete current command*
    [self setSelectedRange:NSMakeRange(start,[[self string] length])];
    [self delete:self];
  }
  else if ([[theEvent characters] characterAtIndex:0] == NSF10FunctionKey && ([theEvent modifierFlags] & NSControlKeyMask))
  {
    // *CONTROL + F10* : switch the parser modes
    if (parserMode == NO_DECOMPOSE)
    {
      [self notifyUser:@"When pasting command, newline is now interpreted as subcommands separator" returnPrompt:YES];
      parserMode = DECOMPOSE;
    }
    else
    {
      [self notifyUser:@"When pasting command, newline is now NOT interpreted as subcommands separator" returnPrompt:YES];
      parserMode = NO_DECOMPOSE;
    }
  }
  else
    [super keyDown:theEvent]; 
}

- (void)paste:(id)sender
{
  NSPasteboard *pb = [NSPasteboard pasteboardByFilteringTypesInPasteboard:[NSPasteboard  generalPasteboard]];
  
  if ([pb availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]] == NSStringPboardType)
    [self putCommand:[pb stringForType:NSStringPboardType]];
}

- (void)putCommand:(NSString *)command
{   
    NSCharacterSet *separatorSet;
    NSScanner      *scanner = [NSScanner scannerWithString:command];
    NSString       *subCommand;

    separatorSet =[NSCharacterSet characterSetWithCharactersInString:((parserMode == DECOMPOSE) ? @"\n" : @"")];
    [scanner setCharactersToBeSkipped:[NSCharacterSet characterSetWithCharactersInString:@""]]; // because Scanners skip whitespace and newline characters by default
            
    if ([self selectedRange].location < start)
      [self moveToEndOfDocument:self];
              
    if ([scanner scanUpToCharactersFromSet:separatorSet intoString:&subCommand])
      [self insertText:subCommand];
            
    while (![scanner isAtEnd])
    { 
      [scanner scanString:@"\n" intoString:NULL];
      subCommand = [[self string] substringFromIndex:start];
      last_command_start = start;
      if ([subCommand length] > 0)  [history addStr:subCommand];
      [self moveToEndOfDocument:self];
      [self insertText:@"\n"];
      [self scrollRangeToVisible:[self selectedRange]];
      [commandHandler command:subCommand from:self]; // notify the command handler
      // NSLog(@"puting command : %@",subCommand);
      [self insertText:prompt];
      start = [[self string] length];
      lineEdited = NO;
      subCommand = @"";  
      [scanner scanUpToCharactersFromSet:separatorSet intoString:&subCommand];
      if ([subCommand length] > 0)  [self insertText:subCommand];
      [self scrollRangeToVisible:[self selectedRange]];    
    } 
} 

- (void)putText:(NSString *)text
{
  [self moveToEndOfDocument:self];
  [self insertText:text];
  start = [[self string] length];
  [self scrollRangeToVisible:[self selectedRange]];
}

- (void)setCommandHandler:handler
{  
  if (shouldRetainCommandHandler)
  {
    [handler retain];
    [commandHandler release];
  }
  commandHandler = handler;
}

- (void)setShouldRetainCommandHandler:(BOOL)shouldRetain
{
  if (shouldRetainCommandHandler == YES && shouldRetain == NO)
    [commandHandler release];
  else if (shouldRetainCommandHandler == NO && shouldRetain == YES) 
    [commandHandler retain];
  shouldRetainCommandHandler = shouldRetain;
}

- (BOOL)shouldRetainCommandHandler { return shouldRetainCommandHandler;}

- (void)showError:(NSRange)range
{
  NSTextStorage *theTextStore = [self textStorage];
 
  range.location += last_command_start;
  
  // The folowing instruction gives an better visual result.
  // Note that for it to work, showError:, the current method, must be called before outputing any error message
  // due to the use of the text's length in the test. 
  if (range.location + range.length >= [[self string] length] && range.length > 1) range.length--;

  if ([self shouldChangeTextInRange:range replacementString:nil]) 
  { 
    [theTextStore beginEditing];
    [theTextStore addAttributes:errorAttributes range:range];
    [theTextStore endEditing]; 
    [self didChangeText]; 
  }
}

// I implement this method, inherited from NSTextView,in order to prevent 
// the "smart delete" to happend on parts of the prompt (in practice, this 
// has been seen to happen when the prompt ends whith whithespace) 
- (NSRange)smartDeleteRangeForProposedRange:(NSRange)proposedCharRange 
{
  NSRange r = [super smartDeleteRangeForProposedRange:proposedCharRange]; 
  
  if (proposedCharRange.location >= start && r.location < start) 
  {
    r.length   = r.length - (start - r.location) ;
    r.location = start;
  }
  
  return r;   
}

///////////////////////// Delegate methods /////////////////


// Since a CLIView is his own delegate, it receives the NSTextView(its super class) delegate calls.

- (BOOL)textView:(NSTextView *)aTextView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString
{
  // policy: do not accept a modification outside the current command.
    
  if (replacementString && affectedCharRange.location < start)
  {
      NSBeep();
      return NO;
  }
  else
  {
    lineEdited = YES;
    return YES;
  }
}

@end
