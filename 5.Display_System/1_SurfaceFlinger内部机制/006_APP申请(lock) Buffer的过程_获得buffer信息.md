# 006_APP申请(lock) Buffer的过程-获得buffer信息

标签（空格分隔）： Display SurfaceFlinger

---
* 如果发现某一个`slot`里的buffer需要重新分配，那么需要通过远程调用去获得buffer的信息
```
//frameworks/native/.../Surface.cpp
int Surface::dequeueBuffer(android_native_buffer_t** buffer, int* fenceFd) {
...
//分配完buffer之后，看一下这个buffer是否需要重新分配
if ((result & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
☆    result = mGraphicBufferProducer->requestBuffer(buf, &gbuf);
...
}
↓//注意 sp<IGraphicBufferProducer> mGraphicBufferProducer;
↓//mGraphicBufferProducer 是一个 BpGraphicBufferProducer 代理类
↓
//frameworks/native/.../IGraphicBufferProducer.cpp
virtual status_t requestBuffer(int bufferIdx, sp<GraphicBuffer>* buf) {
    Parcel data, reply;
    data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
    data.writeInt32(bufferIdx);
    //发起binder远程调用,导致 BnGraphicBufferProducer 相应项被调用
☆    status_t result =remote()->transact(REQUEST_BUFFER, data, &reply);
    if (result != NO_ERROR) {
        return result;
    }
    bool nonNull = reply.readInt32();
    if (nonNull) {
    //通过binder驱动读，读fd'
        *buf = new GraphicBuffer();
☆        result = reply.read(**buf);
        if(result != NO_ERROR) {
            (*buf).clear();
            return result;
        }
    }
    result = reply.readInt32();
    return result;
}
```
对应本地操作为:`BnGraphicBufferProducer`里的`onTransact`被调用
```
//frameworks/native/.../IGraphicBufferProducer.cpp
status_t BnGraphicBufferProducer::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
    case REQUEST_BUFFER: {
        CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
        int bufferIdx   = data.readInt32();
        sp<GraphicBuffer> buffer;
        //导致生产者 BufferQueueProducer 实现的方法被调用
☆        int result = requestBuffer(bufferIdx, &buffer);
        reply->writeInt32(buffer != 0);
        if (buffer != 0) {
            //写给binder驱动,写fd
☆            reply->write(*buffer);
        }
        reply->writeInt32(result);
        return NO_ERROR;
    } break;
...
}
↓
↓
↓
BufferQueueProducer.cpp
status_t BufferQueueProducer::requestBuffer(int slot, sp<GraphicBuffer>* buf) {
    ATRACE_CALL();
    BQ_LOGV("requestBuffer: slot %d", slot);
    Mutex::Autolock lock(mCore->mMutex);

    if (mCore->mIsAbandoned) {
        BQ_LOGE("requestBuffer: BufferQueue has been abandoned");
        return NO_INIT;
    }

    if (slot < 0 || slot >= BufferQueueDefs::NUM_BUFFER_SLOTS) {
        BQ_LOGE("requestBuffer: slot index %d out of range [0, %d)",
                slot, BufferQueueDefs::NUM_BUFFER_SLOTS);
        return BAD_VALUE;
    } else if (mSlots[slot].mBufferState != BufferSlot::DEQUEUED) {
        BQ_LOGE("requestBuffer: slot %d is not owned by the producer "
                "(state = %d)", slot, mSlots[slot].mBufferState);
        return BAD_VALUE;
    }

    mSlots[slot].mRequestBufferCalled = true;
    *buf = mSlots[slot].mGraphicBuffer;
    return NO_ERROR;
}
```

这里的`binder`读写操作，由于传入的参数是`GraphicBuffer`对象，这个类有实现自己的`flatten()`与`unflatten()`，因此有：
### 写操作 Parcel.cpp里的`flatten`被重载如下：
```
//frameworks/native/libs/ui/GraphicBuffer.cpp
status_t GraphicBuffer::flatten(void*& buffer, size_t& size, int*& fds, size_t& count) const {
    size_t sizeNeeded = GraphicBuffer::getFlattenedSize();
    if (size < sizeNeeded) return NO_MEMORY;

    size_t fdCountNeeded = GraphicBuffer::getFdCount();
    if (count < fdCountNeeded) return NO_MEMORY;

    int32_t* buf = static_cast<int32_t*>(buffer);
    buf[0] = 'GBFR';
    buf[1] = width;
    buf[2] = height;
    buf[3] = stride;
    buf[4] = format;
    buf[5] = usage;
    buf[6] = static_cast<int32_t>(mId >> 32);
    buf[7] = static_cast<int32_t>(mId & 0xFFFFFFFFull);
    buf[8] = 0;
    buf[9] = 0;

    if (handle) {
        buf[8] = handle->numFds;
        buf[9] = handle->numInts;
        native_handle_t const* const h = handle;
        memcpy(fds,     h->data,             h->numFds*sizeof(int));
        memcpy(&buf[10], h->data + h->numFds, h->numInts*sizeof(int));
    }

    buffer = reinterpret_cast<void*>(static_cast<int*>(buffer) + sizeNeeded);
    size -= sizeNeeded;
    if (handle) {
        fds += handle->numFds;
        count -= handle->numFds;
    }

    return NO_ERROR;
}
```
### 读操作 Parcel.cpp里`unflatten`被重载如下
* 使用fd'构造handle
* `status_t err = mBufferMapper.registerBuffer(handle);`（mmap(fd')获得虚拟地址vaddr,得到的vaddr会放在`handle->base`中）

```
//frameworks/native/.../GraphicBuffer.cpp
status_t GraphicBuffer::unflatten(
        void const*& buffer, size_t& size, int const*& fds, size_t& count) {
    if (size < 8*sizeof(int)) return NO_MEMORY;

    int const* buf = static_cast<int const*>(buffer);
    if (buf[0] != 'GBFR') return BAD_TYPE;

    const size_t numFds  = buf[8];
    const size_t numInts = buf[9];

    const size_t sizeNeeded = (10 + numInts) * sizeof(int);
    if (size < sizeNeeded) return NO_MEMORY;

    size_t fdCountNeeded = 0;
    if (count < fdCountNeeded) return NO_MEMORY;

    if (handle) {
        // free previous handle if any
        free_handle();
    }

    if (numFds || numInts) {
        width  = buf[1];
        height = buf[2];
        stride = buf[3];
        format = buf[4];
        usage  = buf[5];
        native_handle* h = native_handle_create(numFds, numInts);
        memcpy(h->data,          fds,     numFds*sizeof(int));
        memcpy(h->data + numFds, &buf[10], numInts*sizeof(int));
        handle = h;
    } else {
        width = height = stride = format = usage = 0;
        handle = NULL;
    }

    mId = static_cast<uint64_t>(buf[6]) << 32;
    mId |= static_cast<uint32_t>(buf[7]);

    mOwner = ownHandle;

    if (handle != 0) {
        status_t err = mBufferMapper.registerBuffer(handle);
        if (err != NO_ERROR) {
            width = height = stride = format = usage = 0;
            handle = NULL;
            ALOGE("unflatten: registerBuffer failed: %s (%d)",
                    strerror(-err), err);
            return err;
        }
    }

    buffer = reinterpret_cast<void const*>(static_cast<int const*>(buffer) + sizeNeeded);
    size -= sizeNeeded;
    fds += numFds;
    count -= numFds;

    return NO_ERROR;
}
```
以上便是app侧申请buffer，`SurfaceFlinger`侧响应请求，分配完buffer、fd后，app侧获取buffer和fd的binder调用流程。

## lockAsync
`lock(...)`先完成`dequeueBuffer(...)`后，接下来的主要工作就是`lockAsync()`了
`lockAsync()`与`lock()`大体作用相同，区别主要在在于前者是异步的，后者可能会被阻塞（例如等待h/w完成渲染或者cpu caches需要同步），它应当在一个buffer在被使用前被调用。
* `lockAsync`直接返回`handle->base`(即上面mmap得到的地址）
```
//Surface.cpp
status_t Surface::lock(
        ANativeWindow_Buffer* outBuffer, ARect* inOutDirtyBounds)
{
    ...
    sp<GraphicBuffer> backBuffer(GraphicBuffer::getSelf(out));
    ...

☆    status_t res = backBuffer->lockAsync(
        GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN,
        newDirtyRegion.bounds(), &vaddr, fenceFd);
...
    outBuffer->bits   = vaddr;
}
↓
↓ backBuffer 是 sp<GraphicBuffer> 指针
↓
//native/.../GraphicBuffer.cpp
status_t GraphicBuffer::lockAsync(uint32_t usage, const Rect& rect, void** vaddr, int fenceFd)
{
    if (rect.left < 0 || rect.right  > this->width ||
        rect.top  < 0 || rect.bottom > this->height) {
        ALOGE("locking pixels (%d,%d,%d,%d) outside of buffer (w=%d, h=%d)",
                rect.left, rect.top, rect.right, rect.bottom,
                this->width, this->height);
        return BAD_VALUE;
    }
    //跟进一下即可发现 getBufferMapper() 返回的是一个 GraphicBufferMapper() 对象
☆    status_t res = getBufferMapper().lockAsync(handle, usage, rect, vaddr, fenceFd);
    return res;
}
↓
↓//这里应该会调用到HAL层
↓//mAllocMod 在 GraphicBufferMapper.h 中定义为 gralloc_module_t const *
GraphicBufferMapper.cpp
status_t GraphicBufferMapper::lockAsync(buffer_handle_t handle,
        int usage, const Rect& bounds, void** vaddr, int fenceFd)
{
    ATRACE_CALL();
    status_t err;

    if (mAllocMod->common.module_api_version >= GRALLOC_MODULE_API_VERSION_0_3) {
        err = mAllocMod->lockAsync(mAllocMod, handle, usage,
                bounds.left, bounds.top, bounds.width(), bounds.height(),
                vaddr, fenceFd);
    } else {
        sync_wait(fenceFd, -1);
        close(fenceFd);
        err = mAllocMod->lock(mAllocMod, handle, usage,
                bounds.left, bounds.top, bounds.width(), bounds.height(),
                vaddr);
    }

    ALOGW_IF(err, "lockAsync(...) failed %d (%s)", err, strerror(-err));
    return err;
}
```
这里的`lockAsync()`或者`lock()`就可以到 `/hardware/libhardware/include/hardware/gralloc.h .cpp` 看一下说明。
由于HAL层涉及到硬件，因此可能具体实现都不太相同,
* 对于软件合成：
```
int gralloc_lock(gralloc_module_t const* /*module*/,
        buffer_handle_t handle, int /*usage*/,
        int /*l*/, int /*t*/, int /*w*/, int /*h*/,
        void** vaddr)
{
    // this is called when a buffer is being locked for software
    // access. in thin implementation we have nothing to do since
    // not synchronization with the h/w is needed.
    // typically this is used to wait for the h/w to finish with
    // this buffer if relevant. the data cache may need to be
    // flushed or invalidated depending on the usage bits and the
    // hardware.

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    *vaddr = (void*)hnd->base;
    return 0;
}
```
* 具体到硬件合成的，可以参考一下`/hardware/qcom/display/msm8084/libgralloc/mapper.cpp`的实现:
```
int gralloc_lock(gralloc_module_t const* module,
                 buffer_handle_t handle, int usage,
                 int /*l*/, int /*t*/, int /*w*/, int /*h*/,
                 void** vaddr)
{
    private_handle_t* hnd = (private_handle_t*)handle;
    int err = gralloc_map_and_invalidate(module, handle, usage);
    if(!err)
        *vaddr = (void*)hnd->base;
    return err;
}
↓ //这里实现同步
static int gralloc_map_and_invalidate (gralloc_module_t const* module,
                                       buffer_handle_t handle, int usage)
{
    if (!module || private_handle_t::validate(handle) < 0)
        return -EINVAL;

    int err = 0;
    private_handle_t* hnd = (private_handle_t*)handle;
    if (usage & (GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK)) {
        if (hnd->base == 0) {
            // we need to map for real
            pthread_mutex_t* const lock = &sMapLock;
            pthread_mutex_lock(lock);
            err = gralloc_map(module, handle);
            pthread_mutex_unlock(lock);
        }
        if (hnd->flags & private_handle_t::PRIV_FLAGS_USES_ION) {
            //Invalidate if reading in software. No need to do this for the
            //metadata buffer as it is only read/written in software.
            IMemAlloc* memalloc = getAllocator(hnd->flags) ;
            err = memalloc->clean_buffer((void*)hnd->base,
                                         hnd->size, hnd->offset, hnd->fd,
                                         CACHE_INVALIDATE);
            if (usage & GRALLOC_USAGE_SW_WRITE_MASK) {
                // Mark the buffer to be flushed after cpu read/write
                hnd->flags |= private_handle_t::PRIV_FLAGS_NEEDS_FLUSH;
            }
        }
    } else {
        hnd->flags |= private_handle_t::PRIV_FLAGS_DO_NOT_FLUSH;
    }
    return err;
}
```
这里便大致介绍了`lockAsync()`调用流程，及其作用，实现
