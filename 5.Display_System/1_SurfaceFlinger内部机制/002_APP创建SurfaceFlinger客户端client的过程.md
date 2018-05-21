# SurfaceFlinger内部机制
## APP创建SurfaceFlinger客户端client的过程
### 获得SurfaceFlinger服务//服务在`Main_surfaceflinger.cpp`
```
int main(int, char**) {
    // When SF is launched in its own process, limit the number of
    // binder threads to 4.
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    // start the thread pool
    sp<ProcessState> ps(ProcessState::self());
    ps->startThreadPool();

    // instantiate surfaceflinger
    sp<SurfaceFlinger> flinger = new SurfaceFlinger();

#if defined(HAVE_PTHREADS)
    setpriority(PRIO_PROCESS, 0, PRIORITY_URGENT_DISPLAY);
#endif
    set_sched_policy(0, SP_FOREGROUND);

    // initialize before clients can connect
    flinger->init();

    // publish surface flinger
    sp<IServiceManager> sm(defaultServiceManager());
    sm->addService(String16(SurfaceFlinger::getServiceName()), flinger, false);

    // run in this thread
    flinger->run();

    return 0;
}
```
### 使用它的函数创建client（通过binder远程调用）：`createConnection()`
* mClient指向SurfaceFlinger进程的client对象
```
sp<ISurfaceComposerClient> conn = sm->createConnection();

将会导致binder调用
----------
ISurfaceComposer.cpp
virtual sp<ISurfaceComposerClient> createConnection()
{
    uint32_t n;
    Parcel data, reply;
    data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
    remote()->transact(BnSurfaceComposer::CREATE_CONNECTION, data, &reply);
    return interface_cast<ISurfaceComposerClient>(reply.readStrongBinder());
}
进而会导致以下函数被调用
----------
SurfaceFlinger.cpp
status_t SurfaceFlinger::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);//提供了多种操作，因此会调用到以下实现

sp<ISurfaceComposerClient> SurfaceFlinger::createConnection()
{
    sp<ISurfaceComposerClient> bclient;
    sp<Client> client(new Client(this));
    status_t err = client->initCheck();
    if (err == NO_ERROR) {
        bclient = client;
    }
    return bclient;
}

```

----------

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
        sp<ISurfaceComposerClient> conn = sm->createConnection();
        if (conn != 0) {
            mClient = conn;
            mStatus = NO_ERROR;
        }
    }
}
```

> 简化理解：app的BpSurfaceComposer中的createConnection会导致SurfaceFlinger中的createConnection被调用

![createConnection过程](createConnection%E8%BF%87%E7%A8%8B.png)

