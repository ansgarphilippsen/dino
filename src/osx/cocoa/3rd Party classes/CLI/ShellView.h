/* ShellView.h Copyright (c) 1998-2001 Philippe Mougin.  */
/*   This software is Open Source. See the license.  */  

#import <AppKit/AppKit.h>

@class StrBuffer;

typedef enum {DECOMPOSE,NO_DECOMPOSE} T_parser_mode;

@protocol ShellViewCommandHandler <NSObject>
- (void)command:(NSString *)command from:(id)sender;
@end

@interface ShellView : NSTextView
{
  NSString *prompt;                              
  long start;                         // The start of the current command (i.e. the command beign edited) 
                                      // in term of its character position in the whole NSTextView.
  
  id<ShellViewCommandHandler> commandHandler; // The object that is notified when a command is entered.     
  StrBuffer *history;                 // The history of entered command.
  T_parser_mode parserMode;           // (parsermode == DECOMPOSE)  ==> When some text is "pasted" in by the user, each line
                                      //                                of this text is considered to be an independent command.
                                      // (parsermode == NO_DECOMPOSE)  ==> the entire "pasted" text forms only one command. 
  
  BOOL lineEdited;                    // Did the user edit the current command ?
  long last_command_start;            // The start of the last entered command (its charactere position).
  BOOL shouldRetainCommandHandler;       
  long maxSize;                       // The maximum size of the shellView content in term of character. -1 means no limit.
                                      // When the limit is reached, the oldest contents will be deleted to make room.
                                      // This limit is just an aproximation. At some points in time the size will
                                      // be bigger, at some other points it will be smaller. This is more like a hint.
                                      // This is used because it has been noted that if we let the content of the view
                                      // becoming too big then the user interface slow down considerably (with the new NSText).  
}

- (id)commandHandler;
- (void) dealloc;
- (id)initWithFrame:(NSRect)frameRect;
- (id)initWithFrame:(NSRect)frameRect prompt:(NSString *)thePrompt historySize:(int)theHistorySize commandHandler:(id)theCommandHandler;
- (void)keyDown:(NSEvent *)theEvent;
- (void) notifyUser:(NSString *)notification;
- (void)paste:(id)sender;
- (void)putCommand:(NSString *)command;
- (void)putText:(NSString *)text;
- (void)setCommandHandler:handler;
- (void)setShouldRetainCommandHandler:(BOOL)shouldRetain;
- (BOOL)shouldRetainCommandHandler; 
- (void)showError:(NSRange)range;
- (BOOL)textView:(NSTextView *)aTextView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString;

@end
