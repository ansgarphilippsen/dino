/*************************************************
  main.m

  initialize the components of DINO:

    - COM
    - GUI

**************************************************/

#import <Cocoa/Cocoa.h>
#include <stdio.h>

#include "dino.h"
#include "cmi.h"
#include "gui_ext.h"
#include "gfx.h"

int main(int argc, const char *argv[])
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    if([[[NSFileManager defaultManager] currentDirectoryPath] isEqual:@"/"]){
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: NSHomeDirectory()];
    }

    dinoParseArgs(argc,argv);
    cmiInit();
    gfxInit();
    if(dinoMain(argc, argv)<0) return -1;

    [pool release];
    return NSApplicationMain(argc, argv);
}
