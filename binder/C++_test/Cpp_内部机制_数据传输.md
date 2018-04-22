# Cpp_内部机制_数据传输:ProcessState & IPCThreadState
test_server:
1. open
mmap
2. addService:ioctl
3. while(1){
read data;:ioctl
parse data;
process data;
reply;:ioctl
}

## test_server.cpp
### 
c 主线程open、mmap后会得到一个fd，那么之后的多个子线程可以共享到主线程的fd及其他资源
cpp 主线程open、mmap，以某个类的对象来操作————ProcessState，单例模式，每个进程只有一个ProcessState对象，ProcessState::self()来创建对象。

IPCThreadState,单例模式，每个线程有一个对象，IPCThreadState::self()来得到这个对象
*****
ProcessState：
```
gProcess = new ProcessState时，open，mmap
fd存在mDriverFD
```
IPCThreadState：
```
mProcess = ProcessState::self();//里面含有mDriverFD
IPCThreadState::self()->joinThreadPool();//主线程，会创建一个循环
do {
	read,
	parse,
	process,
	reply
}
do {
    processPendingDerefs();
    // now get the next command to be processed, waiting if necessary
    result = getAndExecuteCommand();//调用ioctl

    if (result < NO_ERROR && result != TIMED_OUT && result != -ECONNREFUSED && result != -EBADF) {
        ALOGE("getAndExecuteCommand(fd=%d) returned unexpected error %d, aborting",
              mProcess->mDriverFD, result);
        abort();
    }
    
    // Let this thread exit the thread pool if it is no longer
    // needed and it is not the main process thread.
    if(result == TIMED_OUT && !isMain) {
        break;
    }
} while (result != -ECONNREFUSED && result != -EBADF);
```
```
ProcessState::self()->startThreadPool();//创建子线程，最终也执行IPCThreadState::self()->joinThreadPool()
do {
	read,
	parse,
	process,
	reply
}

ProcessState::self()->startThreadPool();
	-->void ProcessState::spawnPooledThread(bool isMain)
		-->sp<Thread> t = new PoolThread(isMain);
			-->virtual bool threadLoop()
			    {
			        IPCThreadState::self()->joinThreadPool(mIsMain);
			        return false;
			    }
```
## addService
对于不同的服务，构造_flat_binder_object_结构体，里面的_.binder_/_.cookie_对于不同的服务，它们的值不一样
```
sm->addService(String16("hello"), new BnHelloService());
	==>data.writeStrongBinder(service);//service = new BnHelloService()
		==> flatten_binder(ProcessState::self(), val, this);//val = service = new BnHelloService()
			==> flat_binder_object obj;//参数binder = val = service = new BnHelloService()
				==>IBinder *local = binder->localBinder();//= this = new BnHelloService()
					obj.type = BINDER_TYPE_BINDER;
				    obj.binder = reinterpret_cast<uintptr_t>(local->getWeakRefs());
				    obj.cookie = reinterpret_cast<uintptr_t>(local);//new BnHelloService()
```
## server如何分辨client想使用哪一个服务
## 以及怎么调用到HelloService所提供的函数
server收到的数据里含有_flat_binder_object_结构体,它可以根据_.binder_/_.cookie_分析client想使用哪一个服务
把_.cookie_转换为BnXXX对象，然后调用它的函数:
```
void IPCThreadState::joinThreadPool(bool isMain)
==>status_t IPCThreadState::getAndExecuteCommand()
	==>status_t IPCThreadState::getAndExecuteCommand()
		==>status_t IPCThreadState::talkWithDriver(bool doReceive)
		==>status_t IPCThreadState::executeCommand(int32_t cmd)
			==>case BR_TRANSACTION:
		//根据cookie构造了一个BBinder指针，实际上是指向某个BnXXX对象
				sp<BBinder> b((BBinder*)tr.cookie);
		//然后调用它的transact函数
				error = b->transact(tr.code, buffer, &reply, tr.flags);
					==> err = onTransact(code, data, reply, flags);//就会调用到BnXXX里实现的onTransact，它就会根据code值来调用不同的函数
例如：
status_t BnHelloService::onTransact( uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags)
{
    /*解析数据,调用sayhello or sayhello_to*/
    switch (code) {
        case HELLO_SVR_CMD_SAYHELLO: {
            sayhello();
            return NO_ERROR;
        } break;

        case HELLO_SVR_CMD_SAYHELLO_TO: {
            /*从data中取出参数*/
            int32_t policy = data.readInt32();
            String16 name16 = data.readString16();
            String8 name8(name16);

            int cnt = sayhello_to(name8.string());

            /*把返回值写入reply传出去*/
            reply->writeInt32(cnt);
            return NO_ERROR;
        } break;
        
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

```
*****

#### 线程特有数据
ThreadSpecificData
每个线程都有自己的IPCThreadState对象，该对象是线程特有，各自不同，它应该存在线程的局部空间里。
linux通过k = pthread_key_create()创建一个k,每个线程通过pthread_setspecific(k, v1)设置不一样的value，以后就可以使用v = pthread_getspecific(k)读出自己的v。