#import <Foundation/Foundation.h>

@interface DinoObject : NSObject {

    NSString    *name;
    NSString    *parentDataSet;
    BOOL        *displayFlag;
}

- (id)initWithName:(NSString *)s inDataSet:(NSString *)p;
- (void)setName:(NSString *)s;
- (NSString *)name;
- (void)setParentDataSet:(NSString *)p;
- (NSString *)parentDataSet;
- (BOOL *)displayFlag;
- (void)setDisplayFlag:(BOOL *)flag;
- (id)childrenList;

@end

@interface DinoDataSet : NSObject {

    NSString            *name;
    NSMutableDictionary *childrenList;
    BOOL                *displayFlag;
}

- (id)initWithName:(NSString *)s;
- (void)setName:(NSString *)s;
- (NSString *)name;
- (BOOL *)displayFlag;
- (void)setDisplayFlag:(BOOL *)flag;
- (void)addChildren:(DinoObject *)anObject withKey:(NSString *)name;
- (void)removeChildren:(NSString *)name;
- (DinoObject *)childrenForKey:(NSString *)name;
- (NSMutableDictionary *)childrenList;
- (NSString *)parentDataSet;

@end


