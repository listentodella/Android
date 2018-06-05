# 009_APP提交（unlockAndPost）Buffer的过程_提交过程

标签（空格分隔）： Display SurfaceFlinger

---
## 通知过程：生产者->消费者->Layer->SurfaceFlinger


## 通知消费者
#### 这里是生产者
```
//Surface.cpp
status_t Surface::unlockAndPost()
{
    if (mLockedBuffer == 0) {
        ALOGE("Surface::unlockAndPost failed, no locked buffer");
        return INVALID_OPERATION;
    }

    int fd = -1;
    status_t err = mLockedBuffer->unlockAsync(&fd);
    ALOGE_IF(err, "failed unlocking buffer (%p)", mLockedBuffer->handle);

    err = queueBuffer(mLockedBuffer.get(), fd);
    ALOGE_IF(err, "queueBuffer (handle=%p) failed (%s)",
            mLockedBuffer->handle, strerror(-err));

    mPostedBuffer = mLockedBuffer;
    mLockedBuffer = 0;
    return err;
}

```
```
//Surface.cpp
int Surface::queueBuffer(android_native_buffer_t* buffer, int fenceFd) {
...
    
//生产者代理对象的queueBuffer，导致binder远程调用
    status_t err = mGraphicBufferProducer->queueBuffer(i, input, &output);
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

```
* mConsumerListener是Proxy
* listener->onFrameAvailable(),从下往上进行通知
```
//BufferQueueProducer.cpp
status_t BufferQueueProducer::queueBuffer(int slot,
        const QueueBufferInput &input, QueueBufferOutput *output) {
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

//根据流程可以发现，不管是否为空最后都会被Push back
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
//从下往上进行通知
        listener->onFrameAvailable();
    }

    return NO_ERROR;
}

```
### 分析通知流程
## 进入消费者
#### 
* `listener` 是Proxy，`.mConsumerListener`是`SurfaceFlingerConsumer`对象
```
//ConsumerBase.cpp
ConsumerBase::ConsumerBase(const sp<IGraphicBufferConsumer>& bufferQueue, bool controlledByApp) :
        mAbandoned(false),
        mConsumer(bufferQueue) {
...
    wp<ConsumerListener> listener = static_cast<ConsumerListener*>(this);
    
//就是这个proxy!!!
    sp<IConsumerListener> proxy = new BufferQueue::ProxyConsumerListener(listener);
    
...
}
```

* `mConsumerListener`是`SurfaceFlingerConsumer`对象
* 调用`listener->onFrameAvailable()`进入消费者
```
//BufferQueue.cpp
void BufferQueue::ProxyConsumerListener::onFrameAvailable() {
//mConsumerListener是SurfaceFlingerConsumer对象
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
void ConsumerBase::onFrameAvailable() {
    CB_LOGV("onFrameAvailable");

    sp<FrameAvailableListener> listener;
    { // scope for the lock
        Mutex::Autolock lock(mMutex);
//是Layer对象
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