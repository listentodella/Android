# APP申请创建Surface的过程

标签（空格分隔）： Display SurfaceFlinger

---

```seq
APP-->Binder: client = new SurfaceComposerClient()
Binder-->SurfaceFlinger: client,派生自BnSurfaceComposerClient
APP-->Binder:client->createSurface() => mClient->createSurface()
Note left of APP: handle
Note left of APP: gbp
Binder-->SurfaceFlinger:client->createSurface()
Note right of SurfaceFlinger:Layer
Note right of SurfaceFlinger:BufferQueueCore
Note right of SurfaceFlinger:BufferQueueSlot[ ]
APP-->Binder:sur = new SurfaceControl()

```
![app申请、创建surface的过程](app%E7%94%B3%E8%AF%B7%E3%80%81%E5%88%9B%E5%BB%BAsurface%E7%9A%84%E8%BF%87%E7%A8%8B.png)

`handle`:`sp<IBinder> handle;`
`gbp`:`sp<IGraphicBufferProducer>`,生产者 代理类`BpGraphicBufferProducer`

## 入口resize.cpp
```
    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();

    sp<SurfaceControl> surfaceControl = client->createSurface(String8("resize"),
            160, 240, PIXEL_FORMAT_RGB_565, 0);

    sp<Surface> surface = surfaceControl->getSurface();
```


### `createSurface`的主要流程
~~由前一篇可知，`mClient`指向`conn`，而这里的`conn`其实就可以理解为`BpSurfaceComposerClient conn`了，它已经是一个代理类了，因此这里的`createSurface`自然是调用的以下的实现~~
* app侧的`mClient`是`BpSurfaceComposerClient`代理类，相对应的，sf侧的`client`(它是在`createConnection`时创建的`sp<Client>`对象)则是`BnSurfaceComposerClient`类(`class Client : public BnSurfaceComposerClient`)
```
//app先调用client->createSurface(),这里的client是sp<SurfaceComposerClient> 对象
//SurfaceComposerClient.cpp
sp<SurfaceControl> SurfaceComposerClient::createSurface(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags)
{
    sp<SurfaceControl> sur;
    if (mStatus == NO_ERROR) {
        //注意此处handle与gbp的创建
☆        sp<IBinder> handle;
☆        sp<IGraphicBufferProducer> gbp;
    //mClient最终会调用到Client.cpp里的实现(通过binder)
    //mClient是 BpSurfaceComposerClient 代理类,调用代理类提供的 createSurface,会发起binder调用,这将会导致 BnSurfaceComposerClient 的onTransact下的createSurface被调用
☆        status_t err = mClient->createSurface(name, w, h, format, flags,
                &handle, &gbp);
        ALOGE_IF(err, "SurfaceComposerClient::createSurface error %s", strerror(-err));
        if (err == NO_ERROR) {
            sur = new SurfaceControl(this, handle, gbp);
        }
    }
    return sur;
}

↓
↓ 通过binder最后调用到
↓

//Client.cpp
status_t Client::createSurface(
        const String8& name,
        uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
        sp<IBinder>* handle,
        sp<IGraphicBufferProducer>* gbp)
{
    /*
     * createSurface must be called from the GL thread so that it can
     * have access to the GL context.
     */
//父类 MessageBase 很重要!!!===>>>>>>>> MessageQueue.h/cpp
☆    class MessageCreateLayer : public MessageBase {
        SurfaceFlinger* flinger;
        Client* client;
        sp<IBinder>* handle;
        sp<IGraphicBufferProducer>* gbp;
        status_t result;
        const String8& name;
        uint32_t w, h;
        PixelFormat format;
        uint32_t flags;
    public:
☆        MessageCreateLayer(SurfaceFlinger* flinger,
                const String8& name, Client* client,
                uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
                sp<IBinder>* handle,
                sp<IGraphicBufferProducer>* gbp)
            : flinger(flinger), client(client),
              handle(handle), gbp(gbp),
              name(name), w(w), h(h), format(format), flags(flags) {
        }
        status_t getResult() const { return result; }
        virtual bool handler() {
            //这里会调用到SurfaceFlinger实现的createLayer()
☆            result = flinger->createLayer(name, client, w, h, format, flags,
                    handle, gbp);
            return true;
        }
    };

☆    sp<MessageBase> msg = new MessageCreateLayer(mFlinger.get(),
            name, this, w, h, format, flags, handle, gbp);
    mFlinger->postMessageSync(msg);
    return static_cast<MessageCreateLayer*>( msg.get() )->getResult();
}

|
▽
//SurfaceFlinger.cpp
status_t SurfaceFlinger::createLayer(
        const String8& name,
        const sp<Client>& client,
        uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
        sp<IBinder>* handle, sp<IGraphicBufferProducer>* gbp)
{
    //ALOGD("createLayer for (%d x %d), name=%s", w, h, name.string());
    if (int32_t(w|h) < 0) {
        ALOGE("createLayer() failed, w or h is negative (w=%d, h=%d)",
                int(w), int(h));
        return BAD_VALUE;
    }

    status_t result = NO_ERROR;
//创建Layer,注意sp!
☆    sp<Layer> layer;

    switch (flags & ISurfaceComposerClient::eFXSurfaceMask) {
        case ISurfaceComposerClient::eFXSurfaceNormal:
		//一般是NormalLayer
★            result = createNormalLayer(client,
                    name, w, h, flags, format,
                    handle, gbp, &layer);
            break;
        case ISurfaceComposerClient::eFXSurfaceDim:
            result = createDimLayer(client,
                    name, w, h, flags,
                    handle, gbp, &layer);
            break;
        default:
            result = BAD_VALUE;
            break;
    }

    if (result == NO_ERROR) {
        addClientLayer(client, *handle, *gbp, layer);
        setTransactionFlags(eTransactionNeeded);
    }
    return result;
}
```

* 对于`layer`的第一次创建，不能忽略`onFirstRef()`的调用，因为它涉及了消费者`consumer`与生产者`producer`的创建

```
void Layer::onFirstRef() {
    // Creates a custom BufferQueue for SurfaceFlingerConsumer to use
☆    sp<IGraphicBufferProducer> producer;
☆    sp<IGraphicBufferConsumer> consumer;
☆    BufferQueue::createBufferQueue(&producer, &consumer);
    mProducer = new MonitoredProducer(producer, mFlinger);
☆    mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
    mSurfaceFlingerConsumer->setConsumerUsageBits(getEffectiveUsage(0));
    mSurfaceFlingerConsumer->setContentsChangedListener(this);
    mSurfaceFlingerConsumer->setName(mName);

#ifdef TARGET_DISABLE_TRIPLE_BUFFERING
#warning "disabling triple buffering"
    mSurfaceFlingerConsumer->setDefaultMaxBufferCount(2);
#else
    mSurfaceFlingerConsumer->setDefaultMaxBufferCount(3);
#endif

    const sp<const DisplayDevice> hw(mFlinger->getDefaultDisplayDevice());
    updateTransformHint(hw);
}
```
#### `createSurface()`的`binder`调用
* `gbp`与`handle`的创建
回到开头的`createSurface()`,里面首先创建了几个重要的对象,其中就有`gbp`与`handle`的创建(通过`binder`调用实现创建、传递)
```
//SurfaceComposerClient.cpp
sp<SurfaceControl> SurfaceComposerClient::createSurface(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags)
{
    sp<SurfaceControl> sur;
    if (mStatus == NO_ERROR) {
        //注意此处handle与gbp的创建
☆        sp<IBinder> handle;
☆        sp<IGraphicBufferProducer> gbp;
☆        status_t err = mClient->createSurface(name, w, h, format, flags,
                &handle, &gbp);//看见handle需要意识到有binder调用了
...
}
```

* 在app调用`createSurface`时,`mClient->createSurface()`会导致`BpSurfaceComposerClient`中通过发起`binder`读操作读取`reply`,则由`BnSurfaceComposerClient`中发起`binder`写操作设置`reply`,这里的`reply`里就保存着`gbp`和`handle`

```
//发起binder读操作
//ISurfaceComposerClient.cpp
class BpSurfaceComposerClient : public BpInterface<ISurfaceComposerClient>
{
public:
    BpSurfaceComposerClient(const sp<IBinder>& impl)
        : BpInterface<ISurfaceComposerClient>(impl) {
    }

    virtual status_t createSurface(const String8& name, uint32_t w,
            uint32_t h, PixelFormat format, uint32_t flags,
            sp<IBinder>* handle,
            sp<IGraphicBufferProducer>* gbp) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
        data.writeString8(name);
        data.writeInt32(w);
        data.writeInt32(h);
        data.writeInt32(format);
        data.writeInt32(flags);
        remote()->transact(CREATE_SURFACE, data, &reply);
        //binder读操作
☆        *handle = reply.readStrongBinder();
☆        *gbp = interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());
        return reply.readInt32();
    }
    ...
}
↓
↓
↓
//Client.cpp( class Client : public BnSurfaceComposerClient )
status_t Client::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
	...
		//这里的code自然是CREATE_SURFACE
★     return BnSurfaceComposerClient::onTransact(code, data, reply, flags);
}
↓
↓
↓
//发起binder写操作
//ISurfaceComposerClient.cpp
status_t BnSurfaceComposerClient::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
switch(code) {
    case CREATE_SURFACE: {
        ...
        sp<IBinder> handle;
        sp<IGraphicBufferProducer> gbp;
        //这里将会调用sf的client->createSurface,即Client.cpp里的createSurface
☆        status_t result = createSurface(name, w, h, format, flags,
                &handle, &gbp);
        //binder写操作
☆        reply->writeStrongBinder(handle);
☆        reply->writeStrongBinder(gbp->asBinder());
        ...
        return NO_ERROR;
    } break;
...
}
↓
↓最后又会调用到子类的实现
↓
//Client.cpp
status_t Client::createSurface(
        const String8& name,
        uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
        sp<IBinder>* handle,
        sp<IGraphicBufferProducer>* gbp)
{
    ...
}
```
以上便是`createSurface()`大致的`binder`调用流程。

#### 创建`Layer`的过程：
```
//Client.cpp
status_t Client::createSurface(
        const String8& name,
        uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
        sp<IBinder>* handle,
        sp<IGraphicBufferProducer>* gbp)
{
    /*
     * createSurface must be called from the GL thread so that it can
     * have access to the GL context.
     */
//父类 MessageBase 很重要!!!===>>>>>>>> MessageQueue.h/cpp
☆    class MessageCreateLayer : public MessageBase {
        SurfaceFlinger* flinger;
        Client* client;
        sp<IBinder>* handle;
        sp<IGraphicBufferProducer>* gbp;
        status_t result;
        const String8& name;
        uint32_t w, h;
        PixelFormat format;
        uint32_t flags;
    public:
☆        MessageCreateLayer(SurfaceFlinger* flinger,
                const String8& name, Client* client,
                uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
                sp<IBinder>* handle,
                sp<IGraphicBufferProducer>* gbp)
            : flinger(flinger), client(client),
              handle(handle), gbp(gbp),
              name(name), w(w), h(h), format(format), flags(flags) {
        }
        status_t getResult() const { return result; }
        virtual bool handler() {
            //这里会调用到SurfaceFlinger实现的createLayer()
☆            result = flinger->createLayer(name, client, w, h, format, flags,
                    handle, gbp);
            return true;
        }
    };

☆    sp<MessageBase> msg = new MessageCreateLayer(mFlinger.get(),
            name, this, w, h, format, flags, handle, gbp);
    mFlinger->postMessageSync(msg);
    return static_cast<MessageCreateLayer*>( msg.get() )->getResult();
}

|
▽
//SurfaceFlinger.cpp
status_t SurfaceFlinger::createLayer(
        const String8& name,
        const sp<Client>& client,
        uint32_t w, uint32_t h, PixelFormat format, uint32_t flags,
        sp<IBinder>* handle, sp<IGraphicBufferProducer>* gbp)
{
    //ALOGD("createLayer for (%d x %d), name=%s", w, h, name.string());
    if (int32_t(w|h) < 0) {
        ALOGE("createLayer() failed, w or h is negative (w=%d, h=%d)",
                int(w), int(h));
        return BAD_VALUE;
    }

    status_t result = NO_ERROR;
//创建Layer,注意sp!
☆    sp<Layer> layer;

    switch (flags & ISurfaceComposerClient::eFXSurfaceMask) {
        case ISurfaceComposerClient::eFXSurfaceNormal:
		//一般是NormalLayer
★            result = createNormalLayer(client,
                    name, w, h, flags, format,
                    handle, gbp, &layer);
            break;
        case ISurfaceComposerClient::eFXSurfaceDim:
            result = createDimLayer(client,
                    name, w, h, flags,
                    handle, gbp, &layer);
            break;
        default:
            result = BAD_VALUE;
            break;
    }

    if (result == NO_ERROR) {
        addClientLayer(client, *handle, *gbp, layer);
        setTransactionFlags(eTransactionNeeded);
    }
    return result;
}
```

* 对于`layer`的第一次创建，不能忽略`onFirstRef()`的调用，它涉及了消费者`consumer`与生产者`producer`的创建

```
void Layer::onFirstRef() {
    // Creates a custom BufferQueue for SurfaceFlingerConsumer to use
☆    sp<IGraphicBufferProducer> producer;
☆    sp<IGraphicBufferConsumer> consumer;
☆    BufferQueue::createBufferQueue(&producer, &consumer);
☆    mProducer = new MonitoredProducer(producer, mFlinger);
☆    mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
    mSurfaceFlingerConsumer->setConsumerUsageBits(getEffectiveUsage(0));
    mSurfaceFlingerConsumer->setContentsChangedListener(this);
    mSurfaceFlingerConsumer->setName(mName);

#ifdef TARGET_DISABLE_TRIPLE_BUFFERING
#warning "disabling triple buffering"
    mSurfaceFlingerConsumer->setDefaultMaxBufferCount(2);
#else
    mSurfaceFlingerConsumer->setDefaultMaxBufferCount(3);
#endif

    const sp<const DisplayDevice> hw(mFlinger->getDefaultDisplayDevice());
    updateTransformHint(hw);
}
```

* 在创建生产者和消费者之前，来看一下`sp<IGraphicBufferAlloc>& allocator`：
###### `mSlots`
```
对于 mSlots： BufferQueueCore.h 里有定义:
    BufferQueueDefs::SlotsType mSlots;
而 BufferQueueDefs::SlotsType 则在 BufferQueueDefs.h中有定义:
    typedef BufferSlot SlotsType[NUM_BUFFER_SLOTS];//(NUM_BUFFER_SLOTS = 64)
而 BufferSlot 则是一个结构体了

这样定义相当于：BufferSlot mSlots[64]
```
###### `mAllocator`
```
先看看在 BufferQueueCore.h 的定义
// mAllocator is the connection to SurfaceFlinger that is used to allocate new GraphicBuffer objects.
    sp<IGraphicBufferAlloc> mAllocator;

IGraphicBufferAlloc里提供了Bpxxx代理类，后续binder调用就会通过它来实现
```
* `mSlots`与`mAllocator`的创建
```
BufferQueue::createBufferQueue(&producer, &consumer);
//native/.../BufferQueue.cpp
void BufferQueue::createBufferQueue(sp<IGraphicBufferProducer>* outProducer,
        sp<IGraphicBufferConsumer>* outConsumer,
        const sp<IGraphicBufferAlloc>& allocator) {
    ...
    //里面有 mAllocator 、 BufferSlot mSlots[64]
☆    sp<BufferQueueCore> core(new BufferQueueCore(allocator));
    ...
}
↓
↓
↓
BufferQueueCore::BufferQueueCore(const sp<IGraphicBufferAlloc>& allocator) :
☆    mAllocator(allocator),//将创建的IGraphicBufferAlloc对象赋值给它
...
    mConsumerListener(),
...
    mConnectedProducerListener(),
☆    mSlots(),//创建好的BufferSlot 数组
    mQueue(),
...
{
    if (allocator == NULL) {
        //这里会获取surfaceflinger服务
        sp<ISurfaceComposer> composer(ComposerService::getComposerService());
        //调用surfaceflinger服务提供的函数
☆        mAllocator = composer->createGraphicBufferAlloc();
        if (mAllocator == NULL) {
            BQ_LOGE("createGraphicBufferAlloc failed");
        }
    }
}
↓
↓// class GraphicBufferAlloc : public BnGraphicBufferAlloc
↓
sp<IGraphicBufferAlloc> SurfaceFlinger::createGraphicBufferAlloc()
{
    sp<GraphicBufferAlloc> gba(new GraphicBufferAlloc());
    return gba;
}

```
至此，`sp<BufferQueueCore> core`对象里就含有`mSlots`与`mAllocator`
* 消费者与生产者的创建
```
//native/.../BufferQueue.cpp
void BufferQueue::createBufferQueue(sp<IGraphicBufferProducer>* outProducer,
        sp<IGraphicBufferConsumer>* outConsumer,
        const sp<IGraphicBufferAlloc>& allocator) {
    ...
//.mCore指向上面创建的core，mSlots引用上面创建的mSlots
☆    sp<IGraphicBufferProducer> producer(new BufferQueueProducer(core));
☆    sp<IGraphicBufferConsumer> consumer(new BufferQueueConsumer(core));

    //将创建好的消费者、生产者返回
    *outProducer = producer;
    *outConsumer = consumer;
}
↓
↓core里就含有mSlots
↓
//BufferQueueProducer.cpp，生产者的创建
BufferQueueProducer::BufferQueueProducer(const sp<BufferQueueCore>& core) :
    mCore(core),//mCore指向上面创建的core
    mSlots(core->mSlots),//mSlots引用上面创建的mSlots
    mConsumerName(),
    mStickyTransform(0) {}

//BufferQueueConsumer.cpp，消费者的创建
BufferQueueConsumer::BufferQueueConsumer(const sp<BufferQueueCore>& core) :
    mCore(core),//mCore指向上面创建的core
    mSlots(core->mSlots),//mSlots引用上面创建的mSlots
    mConsumerName() {}
```
可见(同一个`Layer`)创建的消费者、生产者都指向同一个`sp<BufferQueueCore> core`和 `BufferSlot mSlots[64]`。

再来看`onFirstRef`的后续：
```
void Layer::onFirstRef() {
...
☆    mProducer = new MonitoredProducer(producer, mFlinger);
☆    mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
...
}
↓将创建好的生产者、消费者传入
↓
//mProducer指向前面创建的producer, mFlinger则指向surfacefinger
MonitoredProducer::MonitoredProducer(const sp<IGraphicBufferProducer>& producer,
        const sp<SurfaceFlinger>& flinger) :
    mProducer(producer),
    mFlinger(flinger) {}

//mConsumer指向前面创建的consumer(尽管不是直接就实现的，但是跟进一下GLConsumer的构造函数很容易)
SurfaceFlingerConsumer(const sp<IGraphicBufferConsumer>& consumer,
        uint32_t tex)
    : GLConsumer(consumer, tex, GLConsumer::TEXTURE_EXTERNAL, false, false),
      mTransformToDisplayInverse(false)
{}
```
至此 生产者、消费者也创建完成，一个`Layer`的创建也算差不多完成了。

### Tips：关于`gbp`
`gbp`指向`mProducer`这个`MonitoredProducer`对象，而`gbp->asBinder()`则指向了`mProducer`的`mProducer`成员，它实际上是一个`BufferQueueProducer`对象——生产者，从`binder`调用的代码就可以看出。
```
*gbp = interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());
```
###### 以APP侧`resize.cpp`为入口分析：
* APP侧`sp<SurfaceComposerClient> client = new SurfaceComposerClient()`（注意`sp`）完成后,其对象`mClient`是`BpSurfaceComposerClient`代理类(`ISurfaceComposerClient.cpp`),指向SF的client

```
SurfaceComposerClient::SurfaceComposerClient()
    : mStatus(NO_INIT), mComposer(Composer::getInstance())
{
}

void SurfaceComposerClient::onFirstRef() {
    sp<ISurfaceComposer> sm(ComposerService::getComposerService());
    if (sm != 0) {
        sp<ISurfaceComposerClient> conn;
        if (conn != 0) {
/*
class SurfaceComposerClient : public RefBase {
...
    sp<ISurfaceComposerClient>  mClient;//ISurfaceComposerClient.cpp
...
}
*/
☆            mClient = conn;
            mStatus = NO_ERROR;
        }
    }
}
```

* SurfaceFlinger侧的client派生自`BnSurfaceComposerClient`
* 当APP侧的`client->createSurface()`(`SurfaceComposerClient`对象)调用时会导致`mClient->createSurface()`被调用，通过`binder`远程调用SF的`client->createSurface()`.对于APP的每一个`Surface`创建一个`Layer`，`Layer`里有生产者和消费者，它们指向同一个`BufferQueueCore`,它含有`BufferQueueSlot[]`

```
sp<SurfaceControl> SurfaceComposerClient::createSurface(
        const String8& name,
        uint32_t w,
        uint32_t h,
        PixelFormat format,
        uint32_t flags)
{
    sp<SurfaceControl> sur;
    if (mStatus == NO_ERROR) {
☆        sp<IBinder> handle;
☆        sp<IGraphicBufferProducer> gbp;
☆        status_t err = mClient->createSurface(name, w, h, format, flags,
                &handle, &gbp);//看见handle需要意识到有binder调用了
        ALOGE_IF(err, "SurfaceComposerClient::createSurface error %s", strerror(-err));
        if (err == NO_ERROR) {
☆            sur = new SurfaceControl(this, handle, gbp);
        }
    }
    return sur;
}
|
▽
//mClient->createSurface()调用到 ISurfaceComposerClient.cpp里的
virtual status_t createSurface(const String8& name, uint32_t w,
            uint32_t h, PixelFormat format, uint32_t flags,
            sp<IBinder>* handle,
            sp<IGraphicBufferProducer>* gbp) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
        data.writeString8(name);
        data.writeInt32(w);
        data.writeInt32(h);
        data.writeInt32(format);
        data.writeInt32(flags);
☆        remote()->transact(CREATE_SURFACE, data, &reply);//导致binder调用
        *handle = reply.readStrongBinder();
☆        *gbp = interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());
        return reply.readInt32();
}
```

#### 总结
app->client
surfaceControl-->Layer
.gbp-->.mProducer(MonitoredProducer)的 .mProduer(BufferQueueProducer)
它们都遵循同一套接口`IGraphicsBufferProducer`->`BnGraphicBufferProducer`->`BufferQueueProduer`
`IGraphicsBufferProducer`->`BpGraphicBufferProducer`--`gbp`是实例化对象

### 如何得到Surface
```
sp<Surface> surface = surfaceControl->getSurface();

sp<Surface> SurfaceControl::getSurface() const
{
    Mutex::Autolock _l(mLock);
    if (mSurfaceData == 0) {
        // This surface is always consumed by SurfaceFlinger, so the
        // producerControlledByApp value doesn't matter; using false.
        mSurfaceData = new Surface(mGraphicBufferProducer, false);
    }
    return mSurfaceData;
}
```
`mGraphicBufferProducer`肯定等于前面的生产者`gbp`---参考`SurfaceControl`其构造函数
