/* StrBuffer.m Copyright (c) 1998-2001 Philippe Mougin.  */
/*   This software is Open Source. See the license.  */  

#import "StrBuffer.h"

@implementation StrBuffer

- addStr:(NSString *)str
{ 
  if ([array count] != 0)
  {
    head = (head+1) % [array count];
    if (head == queue) queue = (queue+1) % [array count];  
    [array replaceObjectAtIndex:head withObject:str];
    [self goToLast];
  } 
  return self;  
}    
 
- (void)dealloc
{
  [array release];
  [super dealloc];
}     

- (void)encodeWithCoder:(NSCoder *)coder
{
  [coder encodeObject:array];
  [coder encodeValueOfObjCType:@encode(typeof(head))   at:&head];
  [coder encodeValueOfObjCType:@encode(typeof(queue))  at:&queue];
  [coder encodeValueOfObjCType:@encode(typeof(cursor)) at:&cursor];
}

- (id)initWithCoder:(NSCoder *)coder
{
  array = [[coder decodeObject] retain];
  [coder decodeValueOfObjCType:@encode(typeof(head))   at:&head];
  [coder decodeValueOfObjCType:@encode(typeof(queue))  at:&queue];
  [coder decodeValueOfObjCType:@encode(typeof(cursor)) at:&cursor];
  return self;
}

- goToFirst
{
  cursor = head;
  return self;
}

- goToLast
{
  cursor = queue;
  return self;
}

- goToNext
{
  if ([array count] != 0)
  {
    if   (cursor == head) cursor = queue;
    else                  cursor = (cursor+1) % [array count];
  }  
  return self;
}

- goToPrevious
{
  if([array count] != 0)
  {
    if   (cursor == queue)  cursor = head;
    else                    cursor = (cursor-1+[array count]) % [array count];
  } 
  return self;
}  
  
- (NSString *)getMostRecentlyInsertedStr
{
  if ([array count] != 0) return [array objectAtIndex:head];
  else                    return(@"");
}

- (NSString *)getStr
{
  if ([array count] != 0) return [array objectAtIndex:cursor];
  else                    return(@"");    
}  

- init {return [self initWithUIntSize:0];}

- initWithUIntSize:(unsigned int)size
{
  [super init];
  array = [[NSMutableArray alloc] initWithCapacity:size];
  head = 0; queue =0;
  for (; size > 0; size--) [array addObject:@""];
  return self;
}  
      
- (int)size {return [array count];}

@end
