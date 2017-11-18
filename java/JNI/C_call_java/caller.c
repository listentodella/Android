/**
 * gcc -shared -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/ -fPIC -o cller caller.
 *  -L /usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server -ljvm 表示把该路径下的库链接进去
 *  export LD_LIBRARY= /usr/lib/jvm/java-7-openjdk-amd64/jre/lib/amd64/server
 */

#include <stdio.h>
#include <jni.h>

JNIEnv* create_vm(JavaVM* jvm, JNIEnv** env) {
  JavaVMInitArgs args;
  JavaVMOption options[1];
  args.version = JNI_VERSION_1_6;
  args.nOptions = 1;
  options[0].optionString = "-Djava.class.path=./";
  args.options = options;
  args.ignoreUnrecognized = JNI_FALSE;
  return JNI_CreateJavaVM(&jvm, (void **)&env, &args);
}

int main(int argc, char const *argv[]) {
  JavaVM* jvm;
  JNIEnv* env;
  jclass cls;
  jmethodID mid, mid1, cid;
  jobject jobj;
  jstring jstr;
  jfieldID nameID;
  jfieldID ageID;
  int ret = 0;
  /*1.创建虚拟机*/
  if(create_vm(&jvm, env)){
    printf("can not create jvm\n", );
    return -1;
  }
  /*2.get class*/
  cls = (*env)->FindClass(env, "Prog");
  if(cls == NULL){
    printf("cannot find class\n", );
    ret = -1;
    goto destroy;
  }
  /*
   * 3.create object
   * 如果只是调用静态方法的话可以不创建
   */
   /*
    *3.1 获取构造方法(根据参数选择)
    *3.2 创建参数
    *3.3 创建对象
    */
    cid = (*env)->GetMethodID(env, cls, "<init>", "()V");
    if(cid == NULL){
      printf("cannot get construct method \n", );
      ret = -1;
      goto destroy;
    }

    jobj = (*env)->NewObject(env, cls, cid);
    if(jobj == NULL) {
      ret = -1;
      printf("cannot create object\n", );
      goto destroy;
    }
    /**
     * 读取/设置类中的属性
     * 1. get field id
     * 2. get/set field id
     */
    nameID = (*env)->GetFieldID(env, cls, "name", "Ljava/lang/String;");
    if(nameID == NULL){
      printf("cannot get field name \n", );
      ret = -1;
      goto destroy;
    }
    jstr = (*env)->NewStringUTF(env, "Della");
    (*env)->SetObjectField(env, jobj, nameID, jstr);

    age = (*env)->GetFieldID(env, cls, "age", "Ljava/lang/I;");
    if(ageID == NULL){
      printf("cannot get field age \n", );
      ret = -1;
      goto destroy;
    }
    (*env)->SetIntField(env, jobj, ageID, 18);
  /*
   *4.call method
   *如果有重载，会根据获取的参数选择对应的方法
   *可以通过查看编译出来的class文件获知参数的字段描述符Signature
   *javac Hello.java
   *javap -p -s Hello.class
   */
    /*4.1 get method*/
    mid = (*env)->GetStaticMethodID(env, cls, "main", "([Ljava/lang/String;)V");
    if(mid == NULL) {
      ret = -1;
      printf("cannot get method\n", );
      goto destroy;
    }
    mid1 = (*env)->GetMethodID(env, cls, "sayhelloto", "([Ljava/lang/String;)I");
    if(mid1 == NULL) {
      ret = -1;
      printf("cannot get method\n", );
      goto destroy;
    }
    /*4.2 create params*/
    jstr = (*env)->NewStringUTF(env, "listentodella");
    /*4.3 call method*/
    (*env)->CallStaticVoidMethod(env, cls, mid, NULL);
    int r = (*env)->CallStaticVoidMethod(env, jobj, mid1, jstr);
    printf("ret = %d\n", r);
  /*5.销毁虚拟机*/
destroy:
  (*jvm)->DestroyJavaVM(jvm);
  return ret;
}
