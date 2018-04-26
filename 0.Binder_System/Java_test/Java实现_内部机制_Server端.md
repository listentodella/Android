# Java实现_内部机制_Client端
## server怎么读到数据
使用app_process来启动server进程,它会先创建子线程：
```
AppRuntime::onStarted()
-->proc->startThreadPool();
	-->spawnPooledThread(true);
		-->sp<Thread> t = new PoolThread(isMain);
		-->t->run(name.string());
		// 它会创建子线程, 并执行threadLoop
		IPCThreadState::self()->joinThreadPool(mIsMain)
			--->do{}while(...)
```
## 怎么调用某服务的RPC层的onTransact函数
### 在addService时设置 ._ptr_ 和 ._cookie_
ServiceManager.addService("hello", new HelloService());
##### 分析：
* new HelloService()是JAVA对象
* 处理数据时把._cookie_转换成_BBinder_对象, 它是c++对象
所以: addService中肯定会把JAVA对象转换成一个_BBinder_派生类对象，存在._cookie_里

##### 结论
* addService会通过JNI调用Cpp函数：
	创建一个BBinder派生类JavaBBinder对象，它的._mObject_指向Java对象：new HelloService(),它含有onTransact函数
	把这个对象存入._cookie_（最终存入binder驱动中，该服务对应的binder_node.cookie）

* server进程从驱动中读到数据，里面含有._cookie_
	把它转换为_BBinder_对象
	调用它的transact函数
	它会调用到派生类JavaBBinder中定义的onTransact函数

* JavaBBinder中定义的onTransact函数(Cpp)
	它通过JNI调用java Binder的execTransact方法，
	然后调用Binder派生类IHelloService.Stub中定义的onTransact函数（Java）

* IHelloService.Stub中定义的onTransact函数（Java）
	分析数据
	调用sayhello、sayhello_to

##### 源码阅读
* ServiceManager.addService("hello", new HelloService());
```
ServiceManagerProxy.addService;
	//data是Parcel.java对象
	data.writeStrongBinder(service);
		//它是一个JNI调用，对应android_os_Parcel_writeStrongBinder(Cpp)
		nativeWriteStrongBinder(mNativePtr, val);//val = service = new HelloService()
```
		
* android_os_Parcel_writeStrongBinder(Cpp)
它会构造一个JavaBBinder对象，._mObject_ = new HelloService() Java对象
然后让._cookie_ = JavaBBinder对象（Cpp）
```
// 把Java Parcel转换为c++ Parcel
Parcel* parcel = reinterpret_cast<Parcel*>(nativePtr);

// .cookie = ibinderForJavaObject(env, object)得到一个JavaBBinder对象
parcel->writeStrongBinder(ibinderForJavaObject(env, object))
```
* ibinderForJavaObject(env, object)//object = new HelloService()
把一个Java对象（new HelloService()）转换为cpp IBinder对象
``` 
JavaBBinderHolder* jbh = (JavaBBinderHolder*)env->GetLongField(obj, gBinderOffsets.mObject);
return jbh != NULL ? jbh->get(env, obj) : NULL;
			
	b = new JavaBBinder(env, obj); // obj = new HelloService() 
	         mObject = new HelloService() 

```
* 从驱动中得到了._cookie_，它是一个JavaBBinder对象
	调用它的transact函数，导致JavaBBinder对象的onTransact被调用
```
JavaBBinder::onTransact (调用java里的某个函数)

		// mObject指向 HelloService对象
		// gBinderOffsets.mExecTransact指向: java Binder类中的execTransact方法
		// 调用HelloService(派生自Binder)对象中的execTransact方法
    jboolean res = env->CallBooleanMethod(mObject, gBinderOffsets.mExecTransact,
        code, reinterpret_cast<jlong>(&data), reinterpret_cast<jlong>(reply), flags);

```
* java Binder execTransact（） 
public class HelloService extends IHelloService.Stub 
	Stub的父类android.os.Binder中定义了execTransact方法//frameworks/base/core/java/android/os/Binder.java
```
res = onTransact(code, data, reply, flags);
调用HelloService中的onTransact方法(来自IHelloService.Stub)
	    分辨数据
	    调用sayhello/sayhello_to
```


### 读取到的数据里含有._ptr_ 和 ._cookie_，它会把_cookie_转换为_BBinder_对象，调用它的onTransact函数
```
joinThreadPool(){
	do{
		result = getAndExecuteCommand();
			result = talkWithDriver();
			result = executeCommand(cmd);
				case BR_TRANSACTION:
					sp<BBinder> b((BBinder *)tr.cookie);
					error = b->transact(tr.code, buffer, &reply, tr.flags);

}while()
}
```
