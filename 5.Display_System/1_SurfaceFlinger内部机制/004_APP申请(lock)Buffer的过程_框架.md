# APP申请(lock)Buffer的过程_框架

标签（空格分隔）： SurfaceFlinger

---

```seq
App->SurfaceFlinger:创建client
App->SurfaceFlinger:创建surafce
Note left of App:.GraphicBufferProducer
Note left of App:.mSlot [64]
Note right of SurfaceFlinger:Layer:生产者、消费者
App->SurfaceFlinger:获得Surface中的Buffer
```

![app申请buffer大致框架](app%E7%94%B3%E8%AF%B7buffer%E5%A4%A7%E8%87%B4%E6%A1%86%E6%9E%B6.png)

* `GraphicBufferProducer`指向`Layer`中的生产者，生产者中含有`mCore`和`mSlots [64]`（它是一个`BufferSlot`数组）

### 获得`Surface`中的`Buffer`
* 查看`mSlots`中有无空余项
* 若无，向生产者申请
```
SurfaceFlinger进程中中进行：
查看自己的mSlots中有无空余项
若无，向Gralloc HAL（匿名共享Ashm）申请，得到一个fd
返回fd给app
```

* app获得fd'，mmap获得地址，通过Gralloc HAL来mmp
以上主要步骤都是在APP进程中进行，因此fd是通过Binder驱动传回给APP

## 关键点
* 由SurfaceFlinger分配buffer
* 由SurfaceFlinger返回fd给app
* app mmap

```
frameworks/native/libs/gui/Surface.cpp
status_t Surface::lock(
        ANativeWindow_Buffer* outBuffer, ARect* inOutDirtyBounds)
{
    if (mLockedBuffer != 0) {
        ALOGE("Surface::lock failed, already locked");
        return INVALID_OPERATION;
    }
    //获得显示器等信息
    if (!mConnectedToCpu) {
        int err = Surface::connect(NATIVE_WINDOW_API_CPU);
        if (err) {
            return err;
        }
        // we're intending to do software rendering from this point
        setUsage(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
    }

    ANativeWindowBuffer* out;
    int fenceFd = -1;
    status_t err = dequeueBuffer(&out, &fenceFd);
    ALOGE_IF(err, "dequeueBuffer failed (%s)", strerror(-err));
    if (err == NO_ERROR) {
        sp<GraphicBuffer> backBuffer(GraphicBuffer::getSelf(out));
        const Rect bounds(backBuffer->width, backBuffer->height);

        Region newDirtyRegion;
        if (inOutDirtyBounds) {
            newDirtyRegion.set(static_cast<Rect const&>(*inOutDirtyBounds));
            newDirtyRegion.andSelf(bounds);
        } else {
            newDirtyRegion.set(bounds);
        }

        // figure out if we can copy the frontbuffer back
        const sp<GraphicBuffer>& frontBuffer(mPostedBuffer);
        const bool canCopyBack = (frontBuffer != 0 &&
                backBuffer->width  == frontBuffer->width &&
                backBuffer->height == frontBuffer->height &&
                backBuffer->format == frontBuffer->format);

        if (canCopyBack) {
            // copy the area that is invalid and not repainted this round
            const Region copyback(mDirtyRegion.subtract(newDirtyRegion));
            if (!copyback.isEmpty())
                copyBlt(backBuffer, frontBuffer, copyback);
        } else {
            // if we can't copy-back anything, modify the user's dirty
            // region to make sure they redraw the whole buffer
            newDirtyRegion.set(bounds);
            mDirtyRegion.clear();
            Mutex::Autolock lock(mMutex);
            for (size_t i=0 ; i<NUM_BUFFER_SLOTS ; i++) {
                mSlots[i].dirtyRegion.clear();
            }
        }


        { // scope for the lock
            Mutex::Autolock lock(mMutex);
            int backBufferSlot(getSlotFromBufferLocked(backBuffer.get()));
            if (backBufferSlot >= 0) {
                Region& dirtyRegion(mSlots[backBufferSlot].dirtyRegion);
                mDirtyRegion.subtract(dirtyRegion);
                dirtyRegion = newDirtyRegion;
            }
        }

        mDirtyRegion.orSelf(newDirtyRegion);
        if (inOutDirtyBounds) {
            *inOutDirtyBounds = newDirtyRegion.getBounds();
        }

        void* vaddr;
        status_t res = backBuffer->lockAsync(
                GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN,
                newDirtyRegion.bounds(), &vaddr, fenceFd);

        ALOGW_IF(res, "failed locking buffer (handle = %p)",
                backBuffer->handle);

        if (res != 0) {
            err = INVALID_OPERATION;
        } else {
            mLockedBuffer = backBuffer;
            outBuffer->width  = backBuffer->width;
            outBuffer->height = backBuffer->height;
            outBuffer->stride = backBuffer->stride;
            outBuffer->format = backBuffer->format;
            outBuffer->bits   = vaddr;
        }
    }
    return err;
}
```
* app向发出buffer申请，`SurfaceFlinger`负责分配alloc buffer
* app请求返回fd，然后再mmap，`SurfaceFlinger`则负责返回一个fd
```
status_t err = dequeueBuffer(&out, &fenceFd);
//app向SurfaceFlinger发出buffer申请，导致对方分配alloc buffer
status_t result = mGraphicBufferProducer->dequeueBuffer(&buf, &fence, swapIntervalZero,
        reqW, reqH, reqFormat, reqUsage);

//向SurfaceFlinger请求返回fd，然后在App这边mmap
if ((result & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
    result = mGraphicBufferProducer->requestBuffer(buf, &gbuf);
 
```
```
sp<GraphicBuffer> backBuffer(GraphicBuffer::getSelf(out));
```

```
status_t res = backBuffer->lockAsync(
        GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN,
        newDirtyRegion.bounds(), &vaddr, fenceFd);
...
    outBuffer->bits   = vaddr;//这就是最后要填充内容的地址
```