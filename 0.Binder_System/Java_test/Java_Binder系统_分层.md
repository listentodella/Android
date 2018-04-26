# Binder系统——分层
## 1. 服务层
比如Cpp实现的test_clinet.cpp只是使用sayhello、sayhello_to，并不关心实现
BnHelloService.cpp负责实现业务函数sayhello、sayhello_to
test_server.cpp：addService("hello", new _BnHelloService_())
* 实际上test_client.cpp并不是直接调用BnHelloService.cpp里的实现的！

## 2.RPC层
BpHelloService.cpp
sayhello、sayhello_to{
构造数据
发送数据：Remote()->transact(...);
}
BnHelloService.cpp
onTransact() {
根据数据，调用sayhello、sayhello_to
}
* 那BpHelloService怎么发送数据，BnHelloService怎么收数据、调用到该服务的onTransact函数的呢？

## 3.IPC层————IPCThreadState
### 怎么发数据
remote()得到一个BpBinder对象，最终调用IPCThreadState::self()->transact来发送数据，里面调用ioctl发数据
### 怎么收数据、调用服务的函数
IPCThreadState::self()->joinThreadPool();它是一个循环，在do..while(1)里{调用ioctl读数据,从数据中取出.cookie,转换为BBinder指针，调用它的transact函数,它进而调用onTransact——派生类BnHelloService实现的函数,在onTransact里根据收到的数据调用对应的服务函数 }
* test_server.cpp：addService("hello", new _BnHelloService_())时，注意_BnHelloService_对象派生自BBinder,注册服务时内核的binder_node.cookie=该对象的地址

*********************
在Cpp中，除了IPC层不需要自己写，服务层和RPC层都需要自己写
在Java中，则我们只需要实现服务层
*********************
## 服务层
TestClient.java只使用sayhello、sayhello_to,不关心实现
HelloService.java实现sayhello、sayhello_to
* 和Cpp一样，看似直接调用，实际上也是通过RPC调用

## RPC层
IHelloService.java
	IHelloService.Stub.Proxy
		sayhello、sayhello_to {构造数据、发送数据：mRemote.transact}

IHelloService.java
	IhelloService.Stub
		onTransact {分辨数据、调用sayhello、sayhello_to}
* 看似mRemote.transact直接调用到onTransact，实际上不是

## IPCThreadState（Cpp实现——因此这里涉及到JNI）

