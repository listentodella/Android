/**
 * gcc -shared -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/ -fPIC -o libnative.so native.c
 */
#include <stdio.h>
#include <jni.h>

/*C函数里永远比java多这2个参数*/
void c_hello(JNIEnv *env, jobject cls)
{
  printf("Hello world!\n");
}


#if 0
typedef struct {
  char *name;     /*Java里调用的函数名*/
  char *signature; /*JNI字段描述符，用来表示Java里调用的函数的参数和*/
  void *fnPtr;    /*C语言实现的本地函数*/
}JNINativeMethod;
#endif
/*()及其里面的代表参数，V代表void返回值类型，其他的对应关系可以查表
  也可以用工具生成 javah -jni xxx.java(要先编译java源文件)
*/
jint ledOpen(JNIEnv *env, jobject cls)
{
  return 0;
}

void ledClose(JNIEnv *env, jobject cls)
{

}

jint ledCtrl(JNIEnv *env, jobject cls, jint which, jint status)
{
  return 0;
}


static const JNINativeMethod methods[] = {
  {"ledOpen", "()I", (void *)ledOpen},
  {"ledClose", "()V", (void *)ledClose},
  {"ledCtrl", "(II)I", (void *)ledCtrl},
};


/*System.loadLibrary*/
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved)
{
  JNIEnv *env;
  jclass cls;

  if ((*jvm)->GetEnv(jvm, (void **)&env, JNI_VERSION_1_4)) {
    return JNI_ERR; /* JNI version not supported */
  }
  cls = (*env)->FindClass(env, "com/example/hardlibrary/HardControl");
  if (cls == NULL) {
    return JNI_ERR;
  }

  /*建立映射关系*/
  if((*env)->RegisterNatives(env, cls, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
    return JNI_ERR;
  }

  return JNI_VERSION_1_4;
}