/* DinoObject */

#import <Foundation/Foundation.h>

@interface DataObject : NSObject {

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
    NSMutableArray      *childrenList;
    BOOL                *displayFlag;
}

- (id)initWithName:(NSString *)s;
- (void)dealloc;
- (void)setName:(NSString *)s;
- (NSString *)name;
- (BOOL *)displayFlag;
- (void)setDisplayFlag:(BOOL *)flag;
- (void)addChildren:(DataObject *)anObject;
- (void)removeChildren:(NSString *)name;
- (DataObject *)childrenOfName:(NSString *)s;
- (NSMutableArray *)childrenList;
- (NSString *)parentDataSet;

@end


