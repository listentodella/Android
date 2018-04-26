# Java实现_内部机制_Client端
## client端实现
TestServer（此时充当client） ---addService-->>> service_manager（充当
server）
TestClient（此时充当client） ---getService-->>> service_manager（充当server）
TestClient（此时充当client）--sayhello/sayhello_to-->TestServer（充当server）
#### addService/getService
new _ServiceManagerProxy_(obj).addService
new _ServiceManagerProxy_(obj).getService
构造好数据，使用mRemote.transact()发送数据

#### sayhello/sayhello_to
IHelloService.Stub._Proxy_ 最终调用mRemote.transact()发送数据
* Tips：_ServiceManagerProxy_和_Proxy_都有mRemote

#### 统一使用mRemote.transact来发送数据
数据发送三要素：源、目的、数据
源：调用的函数
目的：mRemote（远端）
* 对于addService、getService：
mRemote是一个Java BinderProxy对象，它的mObject指向一个Cpp的BpBinder对象，这个BpBinder的mHandle=0（表示ServiceManager）
* 对于sayhello、sayhello_to：
mRemote是一个Java BinderProxy对象，它的mObject指向一个Cpp的BpBinder对象，这个BpBinder的mHandle=1（来自getService("hello")）

### ServiceManagerProxy中mRemote的构造(用于addService/getService)
猜测：使用0直接构造出一个java BinderProxy对象
getIServiceManager().addService(） / getIServiceManager().getService(）
```
getIServiceManager()
	return ServiceManagerNative.asInterface(BinderInternal.getContextObject());
```
* BinderInternal.getContextObject() // 得到了一个Java BinderProxy对象, 其中mObject指向new BpBinder(0);
   它是一个JNI调用，对应 android_os_BinderInternal_getContextObject() // android_util_Binder.cpp
```
android_os_BinderInternal_getContextObject()
==> sp<IBinder> b = ProcessState::self()->getContextObject(NULL);
	-->getStrongProxyForHandle(0);
		-->b = new BpBinder(handle);  // mHandle = 0

==>return javaObjectForIBinder(env, b);	// b = new BpBinder(0), mHandle = 0
// 使用c代码调用NewObject来创建JAVA BinderProxy对象
	-->object = env->NewObject(gBinderProxyOffsets.mClass, gBinderProxyOffsets.mConstructor);	
	// 设置该对象的mObject = val.get = b = new BpBinder(0)
	-->env->SetLongField(object, gBinderProxyOffsets.mObject, (jlong)val.get());
	-->return object;
```
* ServiceManagerNative.asInterface
	new ServiceManagerProxy(obj); // obj = BinderProxy对象
		mRemote = obj = BinderProxy对象, 其中mObject指向new BpBinder(0);

### hello服务里的mRemote如何构造
* IBinder binder = ServiceManager.getService("hello");
猜测: 它的返回值就是一个java BinderProxy对象, 其中的mObject=new BpBinder(handle) 
```
new ServiceManagerProxy().getService("hello")
-->....
-->IBinder binder = reply.readStrongBinder();
	-->nativeReadStrongBinder  // Parcel.java

nativeReadStrongBinder是一个JNI调用, 对应的代码是 android_os_Parcel_readStrongBinder																
android_os_Parcel_readStrongBinder	
	  // 把java Parce对象转换为c++ Parcel对象
	  // client程序向sevice_manager发出getService请求，
	  // 得到一个回复reply, 它里面含有flat_binder_object
	  // 它被封装成一个c++ Parcel对象
		Parcel* parcel = reinterpret_cast<Parcel*>(nativePtr);
		/* parcel->readStrongBinder()应该是一个 new BpBinder(handle)
		 *  unflatten_binder(ProcessState::self(), *this, &val);
		 *   *out = proc->getStrongProxyForHandle(flat->handle);
		 *      b = new BpBinder(handle); 
		 */
		
  	// 它会创建一个java BinderProxy对象, 其中的mObject=new BpBinder(handle)对象
		return javaObjectForIBinder(env, parcel->readStrongBinder());

* IHelloService svr = IHelloService.Stub.asInterface(binder);
	new IHelloService.Stub.Proxy(obj);  // obj = 步骤a得到的binder
	mRemote = remote;		
```
### mRemote就是一个java BinderProxy对象
看一下mRemote.transact() //Binder.java
transactNativ(code, data, reply, flags)//是一个JNI调用，对应android_os_BinderProxy_transact()
```
android_os_BinderProxy_transact()
{
	//从java BinderProxy对象中把mObject取出，它就是一个BpBinder对象
	IBinder* target = (IBinder *)env->GetLongField(obj, gBinderProxyOffsets.mObject);
	//然后调用BpBinder的transact函数
	status_t err = target->transact(code, *data, reply, flags);
}
```
## 怎么发
对于getService/addService，会得到一个ServiceManagerProxy代理类
对于hello服务，也会得到一个代理类IHelloService.Stub.Proxy
这些代理类中都有一个mRemote成员，它是一个java BinderProxy对象，它的mObject成员指向一个Cpp BpBinder对象
BpBinder中有一个mHandle：
	对于getService、addService，它是0——表示会给ServiceManager进程
	对于hello服务，它来自getService的结果
发送数据时：
	调用mRemote.transact，它会从mObject中取出BpBinder对象，调用它的transact函数，实现了java RPC对Cpp IPC的调用
