# 006_APP申请(lock) Buffer的过程-获得buffer信息

标签（空格分隔）： Display SurfaceFlinger

---
* 如果发现某一个`slot`里的buffer需要重新分配，那么需要通过远程调用去获得buffer的信息
```
//frameworks/native/.../Surface.cpp
if ((result & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
    result = mGraphicBufferProducer->requestBuffer(buf, &gbuf);
 
```

```
//frameworks/native/.../IGraphicBufferProducer.cpp
virtual status_t requestBuffer(int bufferIdx, sp<GraphicBuffer>* buf) {
    Parcel data, reply;
    data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
    data.writeInt32(bufferIdx);
    status_t result =remote()->transact(REQUEST_BUFFER, data, &reply);
    if (result != NO_ERROR) {
        return result;
    }
    bool nonNull = reply.readInt32();
    if (nonNull) {
    //通过binder驱动读，读fd'
        *buf = new GraphicBuffer();
        result = reply.read(**buf);
        if(result != NO_ERROR) {
            (*buf).clear();
            return result;
        }
    }
    result = reply.readInt32();
    return result;
}
```
对应本地操作为:`BnGraphicBufferProducer`
里的`onTransact`被调用
```
//frameworks/native/.../IGraphicBufferProducer.cpp
case REQUEST_BUFFER: {
CHECK_INTERFACE(IGraphicBufferProducer, data, reply);
    int bufferIdx   = data.readInt32();
    sp<GraphicBuffer> buffer;
    //生产者BufferQueueProducer实现的
    int result = requestBuffer(bufferIdx, &buffer);
    reply->writeInt32(buffer != 0);
    if (buffer != 0) {
    //写给binder驱动,写fd
        reply->write(*buffer);
    }
    reply->writeInt32(result);
    return NO_ERROR;
```
### 写操作 Parcel.cpp里的`flatten`被重载如下：
```
//frameworks/native/.../GraphicBuffer.cpp
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
* `status_t err = mBufferMapper.registerBuffer(handle);`（mmap(fd')获得虚拟地址vaddr,得到的vaddr会放在handle->base中）

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

## lockAsync
* `lockAsync`直接返回`handle->base`(即上面mmap得到的地址）
```
//Surface.cpp
sp<GraphicBuffer> backBuffer(GraphicBuffer::getSelf(out));

...

status_t res = backBuffer->lockAsync(
    GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN,
    newDirtyRegion.bounds(), &vaddr, fenceFd);
    

...

outBuffer->bits   = vaddr;

```

```
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
    status_t res = getBufferMapper().lockAsync(handle, usage, rect, vaddr, fenceFd);
    return res;
}

```

