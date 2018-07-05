# 009_APP提交（unlockAndPost）Buffer的过程_提交过程

标签（空格分隔）： Display SurfaceFlinger

---
## 通知过程：生产者->消费者->Layer->SurfaceFlinger


sp<GraphicBuffer>           mLockedBuffer;
sp<GraphicBuffer>           mPostedBuffer;
## 通知消费者
#### 这里是生产者
当app申请的buffer准备好，填充之后就可以调用`unlockAndPost()`来提交buffer
* `unlockAsync()`
前面申请buffer的时候出现过`lockAsync()`,这里的`unlockAsync()`就是与之相对的,主要作用还是用于同步
```
//resize.cpp
{
    ...
    ANativeWindow_Buffer outBuffer;
    surface->lock(&outBuffer, NULL);
    ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
    android_memset16((uint16_t*)outBuffer.bits, 0xF800, bpr*outBuffer.height);
☆    surface->unlockAndPost();
    ...
}
↓
↓
↓
//Surface.cpp
status_t Surface::unlockAndPost()
{
    if (mLockedBuffer == 0) {
        ALOGE("Surface::unlockAndPost failed, no locked buffer");
        return INVALID_OPERATION;
    }

    int fd = -1;
    //mLockedBuffer 是个 sp<GraphicBuffer>
☆    status_t err = mLockedBuffer->unlockAsync(&fd);
    ALOGE_IF(err, "failed unlocking buffer (%p)", mLockedBuffer->handle);

☆    err = queueBuffer(mLockedBuffer.get(), fd);
    ALOGE_IF(err, "queueBuffer (handle=%p) failed (%s)",
            mLockedBuffer->handle, strerror(-err));

    mPostedBuffer = mLockedBuffer;
    mLockedBuffer = 0;
    return err;
}
↓   
↓//对于 status_t err = mLockedBuffer->unlockAsync(&fd);
↓
//GraphicBuffer.cpp
status_t GraphicBuffer::unlockAsync(int *fenceFd)
{
    //getBufferMapper()得到的一个 GraphicBufferMapper 对象
    status_t res = getBufferMapper().unlockAsync(handle, fenceFd);
    return res;
}
↓
↓ //mAllocMod 是 gralloc_module_t，就会调用到HAL层的实现
↓
//GraphicBufferMapper.cpp
status_t GraphicBufferMapper::unlockAsync(buffer_handle_t handle, int *fenceFd)
{
...

    if (mAllocMod->common.module_api_version >= GRALLOC_MODULE_API_VERSION_0_3) {
        err = mAllocMod->unlockAsync(mAllocMod, handle, fenceFd);
    } else {
        *fenceFd = -1;
        err = mAllocMod->unlock(mAllocMod, handle);
    }
...
}
```
以上便完成了`unlockAsync()`的工作。

* `queueBuffer()`
同步工作完成后，就可以将 buffer 进行入队、出队了。
```
☆    err = queueBuffer(mLockedBuffer.get(), fd);
↓
↓
↓
//Surface.cpp
int Surface::queueBuffer(android_native_buffer_t* buffer, int fenceFd) {
...
//生产者代理 BpGraphicBufferProducer 对象 mGraphicBufferProducer 的queueBuffer，导致binder远程调用
☆    status_t err = mGraphicBufferProducer->queueBuffer(i, input, &output);
    if (err != OK)  {
        ALOGE("queueBuffer: error queuing buffer to SurfaceTexture, %d", err);
    }
    uint32_t numPendingBuffers = 0;
    uint32_t hint = 0;
    output.deflate(&mDefaultWidth, &mDefaultHeight, &hint,
            &numPendingBuffers);

    // Disable transform hint if sticky transform is set.
    if (mStickyTransform == 0) {
        mTransformHint = hint;
    }

    mConsumerRunningBehind = (numPendingBuffers >= 2);

    return err;
}
↓
↓ BpGraphicBufferProducer,将会导致 binder 调用
↓
/frameworks/native/libs/gui/IGraphicBufferProducer.cpp
virtual status_t queueBuffer(int buf,
        const QueueBufferInput& input, QueueBufferOutput* output) {
    Parcel data, reply;
    data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
    data.writeInt32(buf);
    data.write(input);
    //发起binder调用
☆    status_t result = remote()->transact(QUEUE_BUFFER, data, &reply);
    if (result != NO_ERROR) {
        return result;
    }
    memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
    result = reply.readInt32();
    return result;
}
↓
↓ BnGraphicBufferProducer::onTransact() 对应项调用
↓
/frameworks/native/libs/gui/IGraphicBufferProducer.cpp
case QUEUE_BUFFER: {
    CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
    int buf = data.readInt32();
    QueueBufferInput input(data);
    QueueBufferOutput* const output =
            reinterpret_cast<QueueBufferOutput *>(
                    reply->writeInplace(sizeof(QueueBufferOutput)));
    //继承 BnGraphicBufferProducer 的只有 BufferQueueProducer,自然是调用 BufferQueueProducer 提供的方法了
☆    status_t result = queueBuffer(buf, input, output);
    reply->writeInt32(result);
    return NO_ERROR;
} break;
```
以上便是生产者`queueBuffer()`的binder调用大致流程，`queueBuffer()`(应该是出队)完成之后，一次buffer的`post`工作就算是完成了。

###### `BufferQueueProducer::queueBuffer()`分析
接下来分析一下`BufferQueueProducer::queueBuffer()`的详细工作：
* mConsumerListener 是 Proxy
* listener->onFrameAvailable(),从下往上进行通知
```
//frameworks/native/libs/gui/BufferQueueProducer.cpp
status_t BufferQueueProducer::queueBuffer(int slot,
        const QueueBufferInput &input, QueueBufferOutput *output) {
    ...
    sp<IConsumerListener> listener;
...
    mSlots[slot].mFence = fence;
    mSlots[slot].mBufferState = BufferSlot::QUEUED;
    ++mCore->mFrameCounter;
    mSlots[slot].mFrameNumber = mCore->mFrameCounter;

//构造BufferItem对象
    BufferItem item;
    item.mAcquireCalled = mSlots[slot].mAcquireCalled;
    item.mGraphicBuffer = mSlots[slot].mGraphicBuffer;
    item.mCrop = crop;
    item.mTransform = transform & ~NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY;
    item.mTransformToDisplayInverse =
            bool(transform & NATIVE_WINDOW_TRANSFORM_INVERSE_DISPLAY);
    item.mScalingMode = scalingMode;
    item.mTimestamp = timestamp;
    item.mIsAutoTimestamp = isAutoTimestamp;
    item.mFrameNumber = mCore->mFrameCounter;
    item.mSlot = slot;
    item.mFence = fence;
    item.mIsDroppable = mCore->mDequeueBufferCannotBlock || async;

    mStickyTransform = stickyTransform;

//根据流程可以发现，不管是否为空最后都会被Push back, mQueue 这部分工作可以理解为 BufferQueueCore 对于这个 slot 的管理，通过 FIFO ?
    // mQueue is a FIFO of queued buffers used in synchronous mode.
        if (mCore->mQueue.empty()) {
            // When the queue is empty, we can ignore mDequeueBufferCannotBlock
            // and simply queue this buffer
//push back
            mCore->mQueue.push_back(item);
//mConsumerListener是Proxy
            listener = mCore->mConsumerListener;
        } else {
           ...
            } else {
//push back
                mCore->mQueue.push_back(item);
                listener = mCore->mConsumerListener;
            }
        }
       ...
    } // Autolock scope

    // Call back without lock held
    if (listener != NULL) {
//从下往上进行通知，这里开始通知！
☆        listener->onFrameAvailable();
    }

    return NO_ERROR;
}

```

### 分析通知流程 `onFrameAvailable()`
## 进入消费者
先来回顾一下之前的创建消费者时创建的`ConsumerListener`,有以下调用:
```
//ConsumerBase.cpp
ConsumerBase::ConsumerBase(const sp<IGraphicBufferConsumer>& bufferQueue, bool controlledByApp) :
        mAbandoned(false),
        mConsumer(bufferQueue) {
...
//this 是 SurfaceFlingerConsumer 对象, 不要忘了本次构造是 SurfaceFlingerConsumer 发起的
☆    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);

    //就是这个proxy!!!
    //调用 ProxyConsumerListener 构造函数
☆    sp<IConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);

...
}
↓将 listener 赋值给 mConsumerListener
BufferQueue::ProxyConsumerListener::ProxyConsumerListener(
        const wp<ConsumerListener>& consumerListener):
        mConsumerListener(consumerListener) {}
```

关于`ProxyConsumerListener`的含义有以下介绍:
> ProxyConsumerListener is a ConsumerListener implementation that keeps a weak
reference to the actual consumer object.  It forwards all calls to that
consumer object so long as it exists.
This class exists to avoid having a circular reference between the
BufferQueue object and the consumer object.  The reason this can't be a weak
reference in the BufferQueue class is because we're planning to expose the
consumer side of a BufferQueue as a binder interface, which doesn't support
weak references.

```
/frameworks/native/include/gui/BufferQueue.h
class BufferQueue {
public:
...
☆    class ProxyConsumerListener : public BnConsumerListener {
    public:
☆        ProxyConsumerListener(const wp<ConsumerListener>& consumerListener);
        virtual ~ProxyConsumerListener();
        virtual void onFrameAvailable();
        virtual void onBuffersReleased();
        virtual void onSidebandStreamChanged();
    private:
        // mConsumerListener is a weak reference to the IConsumerListener.  This is
        // the raison d'etre of ProxyConsumerListener.
☆        wp<ConsumerListener> mConsumerListener;
    };
...
}
```

而在`BufferQueueProducer::queueBuffer(...)`时又有以下:
```
    sp<IConsumerListener> listener;
...
    if (mCore->mQueue.empty()) {
           // When the queue is empty, we can ignore mDequeueBufferCannotBlock
           // and simply queue this buffer
           mCore->mQueue.push_back(item);
//在上面的分析中,mConsumerListener 又指向了 ConsumerListener 对象，这个对象又由 SurfaceFlingerConsumer 对象转换得到
☆           listener = mCore->mConsumerListener;
       }
...
```

因此可以理解为这里的`listener` 是`ProxyConsumerListener`对象,`.mConsumerListener`是`SurfaceFlingerConsumer`对象:
* `mConsumerListener`是`SurfaceFlingerConsumer`对象
* 调用`listener->onFrameAvailable()`进入消费者

```
listener->onFrameAvailable();
↓
↓ 是否经过binder调用？
↓
//BufferQueue.cpp
void BufferQueue::ProxyConsumerListener::onFrameAvailable() {

//promote 是 wp的一个函数,里面调用了attemptIncStrong,返回了一个对象的强引用,可以看看RefBase
//mConsumerListener 是 SurfaceFlingerConsumer 对象
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != NULL) {
//进入消费者
        listener->onFrameAvailable();
    }
}
```
#### 这里是消费者
* `SurfaceFlingerConsumer`对象相应的`onFrameAvailable()`操作，继承自其父类`ConsumerBase`
```
class SurfaceFlingerConsumer : public GLConsumer
class GLConsumer : public ConsumerBase

void ConsumerBase::onFrameAvailable() {
    CB_LOGV("onFrameAvailable");

    sp<FrameAvailableListener> listener;
    { // scope for the lock
        Mutex::Autolock lock(mMutex);
//promote()后是 Layer 对象
        listener = mFrameAvailableListener.promote();
    }

    if (listener != NULL) {
        CB_LOGV("actually calling onFrameAvailable");
//进入Layer
        listener->onFrameAvailable();
    }
}
```
## 进入Layer
#### 这里是 `Layer`
```
//Layer.cpp
void Layer::onFrameAvailable() {
    android_atomic_inc(&mQueuedFrames);
//进入SurfaceFlinger
    mFlinger->signalLayerUpdate();
}
```
## 进入SurfaceFlinger
#### 这里是SurfaceFlinger
* 会导致另外线程被唤醒
```
void SurfaceFlinger::signalLayerUpdate() {
    mEventQueue.invalidate();
}
```
以上便是通知的大致流程。

## 总结
`unlockAndPost()`主要完成的工作：
* `unlockAsync()`实现buffer的同步
* `dequeueBuffer()`实现buffer出队
* `onFrameAvailable()`实现通知过程：生产者->消费者->Layer->SurfaceFlinger


########Tips：关于 onFrameAvailable()的 binder 调用
分析的不准确，待完善：
```
listener->onFrameAvailable();
↓
↓ BpConsumerListener对应项调用
↓
/frameworks/native/libs/gui/IConsumerListener.cpp
class BpConsumerListener : public BpInterface<IConsumerListener>
{
...
    virtual void onFrameAvailable() {
        Parcel data, reply;
        data.writeInterfaceToken(IConsumerListener::getInterfaceDescriptor());
        //发起binder调用
☆        remote()->transact(ON_FRAME_AVAILABLE, data, &reply, IBinder::FLAG_ONEWAY);
    }
...
}
↓
↓ BnConsumerListener::onTransact() 对应项调用
↓
/frameworks/native/libs/gui/IConsumerListener.cpp
status_t BnConsumerListener::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case ON_FRAME_AVAILABLE:
            CHECK_INTERFACE(IConsumerListener, data, reply);
            //调用本地实现
☆            onFrameAvailable();
            return NO_ERROR;
...
}
↓
↓ BnConsumerListener::onTransact() 对应项调用本地实现
↓ ProxyConsumerListener 继承了 BnConsumerListener
/frameworks/native/include/gui/BufferQueue.h
class BufferQueue {
public:
...
☆    class ProxyConsumerListener : public BnConsumerListener {
    public:
☆        ProxyConsumerListener(const wp<ConsumerListener>& consumerListener);
        virtual ~ProxyConsumerListener();
        virtual void onFrameAvailable();
        virtual void onBuffersReleased();
        virtual void onSidebandStreamChanged();
    private:
        // mConsumerListener is a weak reference to the IConsumerListener.  This is
        // the raison d'etre of ProxyConsumerListener.
☆        wp<ConsumerListener> mConsumerListener;
    };
...
}
//BufferQueue.cpp
void BufferQueue::ProxyConsumerListener::onFrameAvailable() {
//mConsumerListener 是 SurfaceFlingerConsumer 对象
    sp<ConsumerListener> listener(mConsumerListener.promote());
    if (listener != NULL) {
//进入消费者
        listener->onFrameAvailable();
    }
}
```
之后再进行上面分析的通知流程。
