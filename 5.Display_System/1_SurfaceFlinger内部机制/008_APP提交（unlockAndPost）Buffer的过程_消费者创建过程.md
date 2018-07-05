# 008_APP提交（unlockAndPost）Buffer的过程_消费者创建过程

标签（空格分隔）： Display SurfaceFlinger

---
## 消费者创建过程
buffer 一般都要经历从创建到使用的过程，因此是生产者创建buffer，消费者使用buffer。
前面分析了buffer是如何创建的，接下来分析如何使用buffer。
在那之前，先分析一下消费者创建的过程：
### `ConsumerListener`的创建
来看一下 `ConsumerListener`的用途：
> ConsumerListener is the interface through which the BufferQueue notifies the consumer of events that the consumer may wish to react to.  Because the consumer will generally have a mutex that is locked during calls from the consumer to the BufferQueue, these calls from the BufferQueue to the consumer MUST be called only when the BufferQueue mutex is NOT locked.

```
/frameworks/native/services/surfaceflinger/Layer.cpp
void Layer::onFirstRef() {
    // Creates a custom BufferQueue for SurfaceFlingerConsumer to use
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
    mProducer = new MonitoredProducer(producer, mFlinger);
    //消费者的创建
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
↓
↓ 先调用 SurfaceFlingerConsumer 的构造函数
↓
/frameworks/native/services/surfaceflinger/SurfaceFlingerConsumer.h
class SurfaceFlingerConsumer : public GLConsumer {
...
    SurfaceFlingerConsumer(const sp<IGraphicBufferConsumer>& consumer,
            uint32_t tex)
            //创建消费者
☆        : GLConsumer(consumer, tex, GLConsumer::TEXTURE_EXTERNAL, false, false),
          mTransformToDisplayInverse(false)
    {}
...
}
↓
↓ 再调用 GLConsumer 的构造函数
↓
//GLConsumer.cpp
GLConsumer::GLConsumer(const sp<IGraphicBufferConsumer>& bq, uint32_t tex,
        uint32_t texTarget, bool useFenceSync, bool isControlledByApp) :
    //注意第二个参数的布尔值为 false
☆    ConsumerBase(bq, isControlledByApp),
    ...
{
    ST_LOGV("GLConsumer");

    memcpy(mCurrentTransformMatrix, mtxIdentity,
            sizeof(mCurrentTransformMatrix));

    mConsumer->setConsumerUsageBits(DEFAULT_USAGE_FLAGS);
}
↓
↓ 再调用 ConsumerBase 的构造函数
↓
//ConsumerBase.cpp
ConsumerBase::ConsumerBase(const sp<IGraphicBufferConsumer>& bufferQueue, bool controlledByApp) :
        mAbandoned(false),
        mConsumer(bufferQueue) {
    // Choose a name using the PID and a process-unique ID.
    mName = String8::format("unnamed-%d-%d", getpid(), createProcessUniqueId());

    //ctor == constructor
    //dtor ==  destructor
    // Note that we can't create an sp<...>(this) in a ctor that will not keep a
    // reference once the ctor ends, as that would cause the refcount of 'this'
    // dropping to 0 at the end of the ctor.  Since all we need is a wp<...>
    // that's what we create.
//this是 SurfaceFlingerConsumer 对象,不要忘了本次构造是 SurfaceFlingerConsumer 发起的
☆    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);

//proxy.mConsumerListener 也等于 SurfaceFlingerConsumer 对象
//因为 proxy.mConsumerListener 由 listener 赋值得到,而 listener 是由 SurfaceFlingerConsumer 通过 static_cast 转换得来的
☆    sp<IConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);

//mConsumer 为 BufferQueueConsumer 对象,调用connect
☆    status_t err = mConsumer->consumerConnect(proxy, controlledByApp);
   ...
        mConsumer->setConsumerName(mName);
}
↓
↓//将得到的 listener 赋值给 mConsumerListener
↓
/frameworks/native/libs/gui/BufferQueue.cpp
BufferQueue::ProxyConsumerListener::ProxyConsumerListener(
        const wp<ConsumerListener>& consumerListener):
☆        mConsumerListener(consumerListener) {}
```

### `connect`操作:
将 `mCore`(`BufferQueueCore`对象)的 `mConsumerListener`成员与刚刚创建好的listener连接
```
//ConsumerBase.cpp
ConsumerBase::ConsumerBase(const sp<IGraphicBufferConsumer>& bufferQueue, bool controlledByApp) :
        mAbandoned(false),
        mConsumer(bufferQueue) {
...
    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);
    sp<IConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);

    1. //mConsumer 为 BufferQueueConsumer 对象
    2. //我个人认为 mConsumer 为 IGraphicBufferConsumer 对象
    //调用 connect 将 BpConsumerListener 代理类 proxy 与 mConsumer 连接起来
☆    status_t err = mConsumer->consumerConnect(proxy, controlledByApp);
   ...
        mConsumer->setConsumerName(mName);
}
```
#### 若理解`mConsumer` 为 `BufferQueueConsumer` 对象
↓
↓//因为mConsumer 为 BufferQueueConsumer 对象，所以调用 BufferQueueConsumer 的
↓
```
/frameworks/native/libs/gui/BufferQueueConsumer.cpp
status_t BufferQueueConsumer::connect(
        const sp<IConsumerListener>& consumerListener, bool controlledByApp) {
...

//consumerListener就是Proxy
    mCore->mConsumerListener = consumerListener;
    mCore->mConsumerControlledByApp = controlledByApp;

...
}
```

#### 若理解`mConsumer` 为 `IGraphicBufferConsumer` 对象
↓
↓//因为mConsumer 为 IGraphicBufferConsumer 对象(应该是BpGraphicBufferConsumer代理类)，所以调用 IGraphicBufferConsumer 的
↓
```
///frameworks/native/libs/gui/IGraphicBufferConsumer.cpp

virtual status_t consumerConnect(const sp<IConsumerListener>& consumer, bool controlledByApp) {
    Parcel data, reply;
    data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
    data.writeStrongBinder(consumer->asBinder());
    data.writeInt32(controlledByApp);
    //发起远程调用
☆    status_t result = remote()->transact(CONSUMER_CONNECT, data, &reply);
    if (result != NO_ERROR) {
        return result;
    }
    return reply.readInt32();
}
↓
↓ 导致 BnGraphicBufferConsumer 本地方法 onTransact()调用
↓
case CONSUMER_CONNECT: {
    CHECK_INTERFACE(IGraphicBufferConsumer, data, reply);
    sp<IConsumerListener> consumer = IConsumerListener::asInterface( data.readStrongBinder() );
    bool controlledByApp = data.readInt32();
☆    status_t result = consumerConnect(consumer, controlledByApp);
    reply->writeInt32(result);
    return NO_ERROR;
} break;
↓
↓ BufferQueueConsumer 是 BnGraphicBufferConsumer 的子类
↓
/frameworks/native/include/gui/BufferQueueConsumer.h
class BufferQueueConsumer : public BnGraphicBufferConsumer {
...
    virtual status_t consumerConnect(const sp<IConsumerListener>& consumer,
            bool controlledByApp) {
☆        return connect(consumer, controlledByApp);
    }
...
}
↓
↓ 再调用 BufferQueueConsumer 提供的 connect() 方法
↓
/frameworks/native/libs/gui/BufferQueueConsumer.cpp
status_t BufferQueueConsumer::connect(
        const sp<IConsumerListener>& consumerListener, bool controlledByApp) {
...

    //consumerListener 就是 Proxy
    //这样就完成了 mCore 的 mConsumerListener 的创建
    mCore->mConsumerListener = consumerListener;
    mCore->mConsumerControlledByApp = controlledByApp;

...
}
```
`connect()`完成之后再通过`binder`写回去`BpGraphicBufferConsumer`。

## 总结
创建消费者只是调用了一句
```
mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
```
但它要完成的工作不光是创建消费者`Consumer`，同时也要创建一个`listener`，并告知`BufferQueueCore`对象`mCore`，这样才能实现以后的同步、管理。
