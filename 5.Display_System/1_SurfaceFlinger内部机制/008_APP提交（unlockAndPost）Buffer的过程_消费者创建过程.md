# 008_APP提交（unlockAndPost）Buffer的过程_消费者创建过程

标签（空格分隔）： Display SurfaceFlinger

---
```
//Layer.cpp
void Layer::onFirstRef() {
    // Creates a custom BufferQueue for SurfaceFlingerConsumer to use
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
    mProducer = new MonitoredProducer(producer, mFlinger);
    mSurfaceFlingerConsumer = new SurfaceFlingerConsumer(consumer, mTextureName);
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


```
//ConsumerBase.cpp
ConsumerBase::ConsumerBase(const sp<IGraphicBufferConsumer>& bufferQueue, bool controlledByApp) :
        mAbandoned(false),
        mConsumer(bufferQueue) {
   ...
//this是SurfaceFlingerConsumer对象
    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);

//proxy.mConsumerListener 也等于SurfaceFlingerConsumer对象
    sp<IConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);

//mConsumer为BufferQueueConsumer对象,调用connect
    status_t err = mConsumer->consumerConnect(proxy, controlledByApp);
   ...
        mConsumer->setConsumerName(mName);
}
```
* connect操作:
```
status_t BufferQueueConsumer::connect(
        const sp<IConsumerListener>& consumerListener, bool controlledByApp) {
...

//consumerListener就是Proxy
    mCore->mConsumerListener = consumerListener;
    mCore->mConsumerControlledByApp = controlledByApp;

...
}

```





