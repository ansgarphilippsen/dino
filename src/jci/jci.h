#ifndef _JCI_H
#define _JCI_H

#ifdef JAVA_GUI

#include <jni.h>

#include "jgui.h"

struct JCI {
  JNIEnv *env;
  JavaVM *jvm;

  jclass jgui;
  jmethodID main;
};

int jciInit(void);

#endif

#endif

