#ifdef JAVA_GUI

#include "jci.h"

#define USER_CLASSPATH "../jgui/"

struct JCI jci;

/*
  starts the JVM
*/

/*
void main() {
  jciInit();
}
*/

int jciInit() 
{
  JavaVMInitArgs vm_args;
  JavaVMOption options[4];
  jint res;

  JNINativeMethod nm[1];
  
  options[0].optionString = "-Djava.compiler=NONE";
  options[1].optionString = "-Djava.class.path=./jgui.jar";
  options[2].optionString = "-Djava.library.path=/usr/java/jre/lib/i386:/usr/java/jre/lib/i386/client:/usr/java/jre/lib/i386/native_threads:.";
  options[3].optionString = "-verbose:class";
  
  vm_args.version = JNI_VERSION_1_2;
  vm_args.options = options;
  vm_args.nOptions = 4;
  vm_args.ignoreUnrecognized = JNI_TRUE;
  
  res = JNI_CreateJavaVM(&jci.jvm, 
			 (void **)&jci.env,
			 &vm_args);
  if (res < 0) {
    fprintf(stderr, "Can't create Java VM (error code %d)\n",(int)res);
    return -1;
  }

  jci.jgui = (*jci.env)->FindClass(jci.env,"JGUI");

  if(jci.jgui == 0) {
    fprintf(stderr,"Can't find JGUI class\n");
    return -1;
  }

  jci.main = (*jci.env)->GetStaticMethodID(jci.env,
					   jci.jgui,
					   "main",
					   "([Ljava/lang/String;)V");

  if(jci.main==0) {
    fprintf(stderr,"Can't find JGUI.main()\n");
    return -1;
  }

  nm[0].name="rawCommand";
  nm[0].signature="([B)V";
  nm[0].fnPtr=Java_JGUI_rawCommand;

  if((*jci.env)->RegisterNatives(jci.env,jci.jgui,nm,1)<0) {
    fprintf(stderr,"Can't register native methods\n");
    return -1;
  }

  (*jci.env)->CallStaticVoidMethod(jci.env,jci.jgui,jci.main,0);
  (*jci.jvm)->DetachCurrentThread(jci.jvm);

  return 0;
}

JNIEXPORT void JNICALL Java_JGUI_rawCommand
  (JNIEnv *env, jclass cls, jbyteArray b)
{
}

#endif
