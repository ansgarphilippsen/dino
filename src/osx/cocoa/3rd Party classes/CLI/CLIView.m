/* CLIView.m Copyright (c) 1998-2001 Philippe Mougin.  */
/*   This software is Open Source. See the license.  */  

/*

The organisation of the view hierarchy:

      ----- CLIView --------------------------------------
      |                                                  |
      | ----- NSScrollView ----------------------------  |
      | |                                             |  |
      | |  ----- ShellView -------------------------  |  |
      | |  | prompt>                               |  |  |
      | |  |                                       |  |  |
      | |  |                                       |  |  |
      | |  |                                       |  |  |
      | |  |                                       |  |  |
      | |  |                                       |  |  |
      | |  -----------------------------------------  |  |
      | |                                             |  |
      | -----------------------------------------------  |
      |                                                  |
      ----------------------------------------------------

A CLIView has one subview: a NSScrollView.
 
This NSScrollView has a ShellView as document view.

The ShellView is the view that displays the prompt, receive the keyboard events from the user, display
the commands entered by the user and the results of those commands etc.  

*/

#import "CLIView.h"
#import "ShellView.h"

@interface CLIView(CLIViewPrivate)
- (ShellView *)shellView;
@end

@implementation CLIView

- (id)commandHandler {return [[self shellView] commandHandler];}

- (void)encodeWithCoder:(NSCoder *)coder
{
  id sub; 
  BOOL shouldRetainCommandHandler = [self shouldRetainCommandHandler] ;
  id _commandHandler = [self commandHandler];
  
  sub = [[[self subviews] objectAtIndex:0] retain];   // I can't encode the shellView since its superclass
                                                      // (NSTextView) doesn't 
                                                      // comforms to NSCoding (note: this is true under 
                                                      // OpenStep, it seems to have changed with OSX) 
  [sub removeFromSuperview];                          // So I remove it (actually I remove it and its 
                                                      // superview (the NSScrollView)) before encoding 
                                                      // the view hierarchy

  [super encodeWithCoder:coder];                      // This will encode the view hierarchy
  [coder encodeValueOfObjCType:@encode(BOOL) at:&shouldRetainCommandHandler];
  [coder encodeConditionalObject:_commandHandler];
  [self addSubview:sub];                              // I reinstall the NSScrollView in the view hierarchy
  [sub release];
}

- (float)fontSize
{ return [[[self shellView] font] pointSize]; }

- (id) _init  // construction and configuration of the view hierarchy
{
    NSScrollView *scrollview =[[[NSScrollView alloc] initWithFrame:[self bounds]] autorelease];
    NSSize contentSize = [scrollview contentSize];
    ShellView *shellView; 

    [scrollview setBorderType:NSNoBorder];
    [scrollview setHasVerticalScroller:YES];
    [scrollview setHasHorizontalScroller:NO];
    [scrollview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable]; 

    shellView = [[[ShellView alloc] initWithFrame:NSMakeRect(0, 0,[scrollview contentSize].width, [scrollview contentSize].height)] autorelease];
    [shellView setMinSize:(NSSize){0.0, contentSize.height}];
    [shellView setMaxSize:(NSSize){1e7, 1e7}];
    [shellView setVerticallyResizable:YES];
    [shellView setHorizontallyResizable:NO];
    [shellView setAutoresizingMask:NSViewWidthSizable ]; 
    [[shellView textContainer] setWidthTracksTextView:YES];

    [scrollview setDocumentView:shellView];
    [self addSubview:scrollview];
    
    { /* This should go in the +initialize method but this is not possible due to 
         a bug in GNUstep that prevent creating object in +initialize */
         
      NSMutableDictionary *registrationDict = [NSMutableDictionary dictionary];

      [registrationDict setObject:@"10" forKey:@"fontSize"];
      [[NSUserDefaults standardUserDefaults] registerDefaults:registrationDict];
    }
    [self setFontSize:[[NSUserDefaults standardUserDefaults] integerForKey:@"fontSize"]];
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
  BOOL shouldRetainCommandHandler;

  self = [super initWithCoder:coder];
  [self _init];
  [coder decodeValueOfObjCType:@encode(BOOL) at:&shouldRetainCommandHandler];
  [self setShouldRetainCommandHandler:shouldRetainCommandHandler];
  [self setCommandHandler:[coder decodeObject]];
  [self setAutoresizesSubviews:YES];
  return self;
}

- (id)initWithFrame:(NSRect)frameRect
{
  if (self = [super initWithFrame:frameRect])
  {
    [self _init];
    [self setAutoresizesSubviews:YES];    
    return self;
  }
  return nil;
}

- (void) notifyUser:(NSString *)message {[[self shellView] notifyUser:message];}

- (void)putCommand:(NSString *)command { [[self shellView] putCommand:command];}

- (void)putText:(NSString *)text { [[self shellView] putText:text];}

- (void)setCommandHandler:handler { [[self shellView] setCommandHandler:handler];}

- (void)setFontSize:(int)theSize { [[self shellView] setFont:[NSFont userFixedPitchFontOfSize:theSize]];}

- (void)setShouldRetainCommandHandler:(BOOL)shouldRetain { [[self shellView] setShouldRetainCommandHandler:shouldRetain];}

- (BOOL)shouldRetainCommandHandler { return [[self shellView] shouldRetainCommandHandler]; }

- (void)showError:(NSRange)range { [[self shellView] showError:range];}


// Private

- (ShellView *)shellView
{
  return [[[self subviews] objectAtIndex:0] documentView];
}

@end

