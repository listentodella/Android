/**
 * gcc -shared -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/ -fPIC -o libnative.so native.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>

/*C函数里永远比java多这2个参数*/
jint c_hello1(JNIEnv *env, jobject cls, jint m)
{
  printf("Hello world! val = %d\n", m);
  return 100;
}

jstring JNICALL c_hello2(JNIEnv *env, jobject cls, jstring str )
{
  // printf("this is c : %s\n", str);
  // return "return from C";
  const jbyte *cstr;
  cstr = (*env)->GetStringUTFChars(env, str, NULL);
  if(cstr == NULL) {
    return NULL;
  }
  printf("get string from java:%s\n", str);
  (*env)->ReleaseStringUTFChars(env, str, cstr);

  return (*env)->NewStringUTF(env, "return from C");
}

jint c_hello3(JNIEnv *env, jobject cls, jintArray arr)
{
  jint *carr;
  jint i, sum = 0;
  carr = (*env)->GetArrayElements(env, arr, NULL);
  if (carr == NULL) {
    return 0;
  }
  for(i = 0; i < (*env)->GetArrayLength(env, arr); i++) {
    sum += carr[i];
  }
  (*env)->ReleaseIntArrayElements(env, arr, carr, 0);

  return sum;
}

jintArray c_hello4(JNIEnv *env, jobject cls, jintArray arr)
{
  jint *carr;
  jint *oarr;
  jintArray rarr;
  jint i, n = 0;
  carr = (*env)->GetArrayElements(env, arr, NULL);
  if (carr == NULL) {
    return 0;
  }
  n = (*env)->GetArrayLength(env, arr);
  oarr = malloc(sizeof(jint) * n);
  if(oarr == NULL) {
    (*env)->ReleaseIntArrayElements(env, arr, carr, 0);
    return 0;
  }

  for(i = 0; i < n; i++) {
    oarr[i] = carr[n-1-i];
  }
  (*env)->ReleaseIntArrayElements(env, arr, carr, 0);

  /*create jintArray*/
  rarr = (*env)->NewIntArray(env, n);
  if(rarr == NULL) {
    return 0;
  }

  (*env)->SetIntArrayRegion(env, rarr, 0, n, oarr);
  free(oarr);

  return rarr;
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
static const JNINativeMethod methods[] = {
  {"hello1", "(I)I", (void *)c_hello1},
  {"hello2", "(Ljava/lang/String;)Ljava/lang/String;", (void *)c_hello2},
  {"hello3", "([I)I", (void *)c_hello3},
  {"hello4", "([I)[I", (void *)c_hello4},
  // {"sd", "()V", (void *)c_bye},
  // {"sdf", "()V", (void *)c_hello},
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
  cls = (*env)->FindClass(env, "JNIDemo");
  if (cls == NULL) {
    return JNI_ERR;
  }

  /*建立映射关系*/
  if((*env)->RegisterNative(env, cls, methods, 1) < 0) {
    return JNI_ERR;
  }

  return JNI_VERSION_1_4;
}
