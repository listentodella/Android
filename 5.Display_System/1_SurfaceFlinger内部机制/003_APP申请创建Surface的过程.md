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
`handle`:`sp<IBinder> handle;`
`gbp`:`sp<IGraphicBufferProducer>`,生产者 代理类`BpGraphicBufferProducer`

```
//ISurfaceComposerClient.cpp
*gbp = interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());//读
```
* 通过`binder`读取`reply`，由`BnSurfaceComposerClient`中设置`gbp`的返回值
```
//ISurfaceComposerClient.cpp
status_t BnSurfaceComposerClient::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
switch(code) {
    case CREATE_SURFACE: {
        ...
        sp<IBinder> handle;
        sp<IGraphicBufferProducer> gbp;
        status_t result = createSurface(name, w, h, format, flags,
                &handle, &gbp);
        reply->writeStrongBinder(handle);
        reply->writeStrongBinder(gbp->asBinder());//写
        ...
        return NO_ERROR;
    } break;
...
}
```

### 分析：
* APP侧`client = new SurfaceComposerClient()`,其对象`mClient`是`BpSurfaceComposerClient`代理类,指向SF的client
* SurfaceFlinger侧的client派生自`BnSurfaceComposerClient`
* 当APP侧的`client->createSurface()`调用时会导致`mClient->createSurface()`被调用，通过`binder`远程调用SF的`client->createSurface()`.对于APP的每一个`Surface`创建一个`Layer`，`Layer`里有生产者和消费者，它们指向同一个`BufferQueueCore`,它含有`BufferQueueSlot[]`

## 创建`Layer`的过程：
* 创建生产者和消费者
``` Cpp
BufferQueue::createBufferQueue(&producer, &consumer);
//native/.../BufferQueue.cpp
void BufferQueue::createBufferQueue(sp<IGraphicBufferProducer>* outProducer,
        sp<IGraphicBufferConsumer>* outConsumer,
        const sp<IGraphicBufferAlloc>& allocator) {
//里面有mAllocator、BufferSlot mSlots[64]
    sp<BufferQueueCore> core(new BufferQueueCore(allocator));
//.mCore指向上面创建的Core，mSlots引用上面创建的mSlots
    sp<IGraphicBufferProducer> producer(new BufferQueueProducer(core));
//.mCore .mSlots引用，同上
    sp<IGraphicBufferConsumer> consumer(new BufferQueueConsumer(core));

    *outProducer = producer;
    *outConsumer = consumer;
}
```

```
mProducer = new MonitoredProducer(producer, mFlinger);
//.mProducer指向前面创建的producer



mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
//.mConsumer指向前面创建的consumer
 
```
`gbp`指向`mProducer`这个`MonitoredProducer`对象，而`gbp->asBinder()`则指向了`mProducer`的`mProducer`成员，它实际上是一个`BufferQueueProducer`对象——生产者

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