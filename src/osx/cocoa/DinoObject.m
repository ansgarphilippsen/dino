#import "DinoObject.h"

@implementation DinoObject

- (id)initWithName:(NSString *)s inDataSet:(NSString *)p{
    [super init];
    [self setName:s];
    [self setParentDataSet:p];
    [self setDisplayFlag:YES];
    return self;
}

- (void)setName:(NSString *)s{
    [s retain];
    [name release];
    name = s;
}

- (NSString *)name{
    return name;
}

- (void)setParentDataSet:(NSString *)p{
    [p retain];
    [parentDataSet release];
    parentDataSet = p;
}

- (NSString *)parentDataSet{
    return parentDataSet;
}

- (void)setDisplayFlag:(BOOL *)flag{
    displayFlag = flag;
}

- (BOOL *)displayFlag{
    return displayFlag;
}

- (id)childrenList{
    return nil;
}

@end

@implementation DinoDataSet

- (id)initWithName:(NSString *)s{
    [super init];
    [self setName:s];
    [self setDisplayFlag:YES];
    childrenList = [[NSMutableDictionary alloc] initWithCapacity:5];
    return self;
}

- (void)setName:(NSString *)s{
    [s retain];
    [name release];
    name = s;
}

- (NSString *)name{
    return name;
}

- (void)setDisplayFlag:(BOOL *)flag{
    displayFlag = flag;
}

- (BOOL *)displayFlag{
    return displayFlag;
}

- (void)addChildren:(DinoObject *)anObject withKey:(NSString *)s{
    [childrenList setObject:anObject forKey:s];  
}

- (void)removeChildren:(NSString *)s{
    [childrenList removeObjectForKey:s];
}

- (DinoObject *)childrenForKey:(NSString *)s{
    return [childrenList objectForKey:s];
}

- (NSMutableDictionary *)childrenList{
    return childrenList;
}

- (NSString *)parentDataSet{
    return @"None";
}

@end

