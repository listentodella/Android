# SurfaceFlinger内部机制
## APP创建SurfaceFlinger客户端client的过程

### SurfaceFlinger服务入口//`main_surfaceflinger.cpp`
```
int main(int, char**) {
    // When SF is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    // start the thread pool
    sp<ProcessState> ps(ProcessState::self());
    ps->startThreadPool();

    // instantiate surfaceflinger
★    sp<SurfaceFlinger> flinger = new SurfaceFlinger();

#if defined(HAVE_PTHREADS)
    setpriority(PRIO_PROCESS, 0, PRIORITY_URGENT_DISPLAY);
#endif
    set_sched_policy(0, SP_FOREGROUND);

    // initialize before clients can connect
    flinger->init();

    // publish surface flinger
    sp<IServiceManager> sm(defaultServiceManager());

//将sf的服务添加进ServiceManager中
★    sm->addService(String16(SurfaceFlinger::getServiceName()), flinger, false);

    // run in this thread
    flinger->run();

    return 0;
}
```
* 注意类的继承关系
```
↓
class SurfaceFlinger : public BnSurfaceComposer,
                       private IBinder::DeathRecipient,
                       private HWComposer::EventHandler
↓
class BnSurfaceComposer: public BnInterface<ISurfaceComposer> 

class BpSurfaceComposer : public BpInterface<ISurfaceComposer>
```
### client获得sf服务
#### client入口：resize.cpp
```
//由于是sp，注意onfirstRef()
sp<SurfaceComposerClient> client = new SurfaceComposerClient();
↓
SurfaceComposerClient::SurfaceComposerClient()
    : mStatus(NO_INIT), mComposer(Composer::getInstance())
{
}

void SurfaceComposerClient::onFirstRef() {
//sm是ServiceManager的实例化对象？？——待探究

★    sp<ISurfaceComposer> sm(ComposerService::getComposerService());
    if (sm != 0) {
//调用sf服务建立连接（sm里现在已经包含了该服务了）
●        sp<ISurfaceComposerClient> conn = sm->createConnection();
        if (conn != 0) {
            mClient = conn;
            mStatus = NO_ERROR;
        }
    }
}
↓
/*static*/ sp<ISurfaceComposer> ComposerService::getComposerService() {
    ComposerService& instance = ComposerService::getInstance();
    Mutex::Autolock _l(instance.mLock);
    if (instance.mComposerService == NULL) {
//该调用将获得sf服务，并将该服务保存于mComposerService中
★        ComposerService::getInstance().connectLocked();
        assert(instance.mComposerService != NULL);
        ALOGD("ComposerService reconnected");
    }
//返回含有该服务的变量
    return instance.mComposerService;
}
↓
void ComposerService::connectLocked() {
    const String16 name("SurfaceFlinger");
//sf服务将保存在mComposerService中
★    while (getService(name, &mComposerService) != NO_ERROR) {
        usleep(250000);
    }
	...
}
```

### 调用sf服务提供的函数创建client（通过binder远程调用）：`createConnection()`
* mClient指向`SurfaceFlinge`r进程的client对象
```
//调用sf服务建立连接（sm里现在已经包含了该服务了）
//conn最后会转换为BpSurfaceComposerClient类了
sp<ISurfaceComposerClient> conn = sm->createConnection();

将会导致binder调用
----------
ISurfaceComposer.cpp
virtual sp<ISurfaceComposerClient> createConnection()
{
    uint32_t n;
    Parcel data, reply;
    data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
//发起binder远程调用
★    remote()->transact(BnSurfaceComposer::CREATE_CONNECTION, data, &reply);
//从reply中取出IBinder对象（里面包含了client），并将其转换为新的Bpxxx代理类对象——BpSurfaceComposerClient
★    return interface_cast<ISurfaceComposerClient>(reply.readStrongBinder());
}
进而会导致以下函数被调用
----------
SurfaceFlinger.cpp
status_t SurfaceFlinger::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);//提供了多种操作，因此会调用到以下实现
值得注意的是，尽管进入switch-case结构调用到相应的case后，本远程调用并未结束，还会继续函数流程，调用父类的onTransact

status_t err = BnSurfaceComposer::onTransact(code, data, reply, flags);
↓
//ISurfaceComposer.cpp
case CREATE_CONNECTION: {
    CHECK_INTERFACE(ISurfaceComposer, data, reply);
//但是根据多态性，最终还是会调用到子类的实现
//子类的实现调用完成后，会得到一个client对象，赋给IBinder对象，最后会将其返回给调用者
★    sp<IBinder> b = createConnection()->asBinder();
★    reply->writeStrongBinder(b);
    return NO_ERROR;
}
↓
sp<ISurfaceComposerClient> SurfaceFlinger::createConnection()
{
    sp<ISurfaceComposerClient> bclient;
//创建一个client，最后并返回
★    sp<Client> client(new Client(this));
    status_t err = client->initCheck();
    if (err == NO_ERROR) {
        bclient = client;
    }
    return bclient;
}

```
至此，sf服务创建完成，client也创建并获取。

----------
* `Client`对象最后是如何转换成`BpSurfaceComposerClient`对象的
```
class Client : public BnSurfaceComposerClient
```
`ISurfaceComposerClient`的本地实现是`BnSurfaceComposerClient`类，接着它又派生了`Client`类；`ISurfaceComposerClient`的代理类必定是`BpSurfaceComposerClient`。那么通过binder调用最后转换成的代理类也肯定是`BpSurfaceComposerClient`。之后就可以通过这个代理类调用`Client`类的实现了。

现在再看看`onFirstRef()`中
```
sp<ISurfaceComposerClient> conn = sm->createConnection();
```
这里的`conn`其实就可以理解为`BpSurfaceComposerClient conn`了，它已经是一个代理类了

* sp是智能指针，因此会导致`onFirstRef()`被调用
```
sp<SurfaceComposerClient> client = new SurfaceComposerClient();


SurfaceComposerClient::SurfaceComposerClient()
    : mStatus(NO_INIT), mComposer(Composer::getInstance())
{
}

void SurfaceComposerClient::onFirstRef() {
    sp<ISurfaceComposer> sm(ComposerService::getComposerService());
    if (sm != 0) {
★        sp<ISurfaceComposerClient> conn = sm->createConnection();
        if (conn != 0) {
//mClient指向sf进程的client对象
★            mClient = conn;
            mStatus = NO_ERROR;
        }
    }
}
```


> 简化理解：app的BpSurfaceComposer中的createConnection会导致SurfaceFlinger中的createConnection被调用

![createConnection过程](createConnection%E8%BF%87%E7%A8%8B.png)

