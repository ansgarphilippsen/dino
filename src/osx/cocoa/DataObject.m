#import "DataObject.h"

@implementation DataObject

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
    childrenList = [[NSMutableArray alloc] initWithCapacity:5];
    return self;
}

- (void)dealloc{
    [childrenList release];
    [super dealloc];
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

- (void)addChildren:(DataObject *)anObject{
    [childrenList addObject:anObject];  
}

- (void)removeChildren:(NSString *)s{
    [childrenList removeObjectIdenticalTo:[self childrenOfName:s]];
}

- (DataObject *)childrenOfName:(NSString *)s{
    NSEnumerator *enumerator = [[self childrenList] objectEnumerator];
    id aDinoObject;
    while (aDinoObject = [enumerator nextObject]) {
	if([[aDinoObject name] isEqual:s]){ return aDinoObject;}
    }
    return nil;
}

- (NSMutableArray *)childrenList{
    return childrenList;
}

- (NSString *)parentDataSet{
    return @"None";
}

@end

