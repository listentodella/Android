# App申请（lock）Buffer的过程_分配Buffer

标签（空格分隔）： SurfaceFlinger Display

---
### 获得`Surface`中的`Buffer`
* ~~查看`mSlots`中有无空余项~~（分析`dequeue`发现并未进行此操作）
* 若无，向生产者申请
```
SurfaceFlinger进程中中进行：
查看自己的mSlots中有无空余项
若无，向Gralloc HAL（匿名共享Ashm）申请，得到一个fd
返回fd给app
```

* app获得fd'，mmap获得地址，通过Gralloc HAL来mmap
以上主要步骤都是在APP进程中进行，因此fd是通过Binder驱动传回给APP
## 关键点
* 由SurfaceFlinger分配buffer
* 由SurfaceFlinger返回fd给app
* app mmap

### app操作-----分配
* app向发出buffer申请，`SurfaceFlinger`负责分配alloc buffer
* app请求返回fd，然后再mmap，`SurfaceFlinger`则负责返回一个fd
```
//Surface.cpp
status_t Surface::lock(ANativeWindow_Buffer* outBuffer, ARect* inOutDirtyBounds) {
    ...
    //app向SurfaceFlinger发出buffer申请，导致对方分配alloc buffer
    //向SurfaceFlinger请求返回fd，然后在App这边mmap
    status_t err = dequeueBuffer(&out, &fenceFd);
    ...
}
↓
↓
↓
//Surface.cpp
int Surface::dequeueBuffer(android_native_buffer_t** buffer, int* fenceFd) {
...
    //app向SurfaceFlinger发出buffer申请，导致对方分配alloc buffer
    //这里就是sf响应它的申请，将可用下标返回到buf，值得注意的是这里的buf是int,为mSlots的下标
☆    status_t result = mGraphicBufferProducer->dequeueBuffer(&buf, &fence, swapIntervalZero,
            reqW, reqH, reqFormat, reqUsage);
...
    //向SurfaceFlinger请求返回fd，然后在App这边mmap
    //sf响应它的请求，
    if ((result & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
        result = mGraphicBufferProducer->requestBuffer(buf, &gbuf);
        ...
        if (fence->isValid()) {
            //surfaceflinger这边返回一个fd，注意这里的dup
☆           *fenceFd = fence->dup();
...
}
```
### SurfaceFlinger操作-----分配
* 对应`dequeueBuffer`
* 值得注意的是映射出来的地址仅供`SurfaceFlinger`调用
* `mSlots[]` 构造其中的某一项，含有`GraphicBuffer`,它又含有`handle`，又含有fd，base（它是mmap的结果）
```
//Surface.cpp
int Surface::dequeueBuffer(android_native_buffer_t** buffer, int* fenceFd) {
...
    //app向SurfaceFlinger发出buffer申请，导致对方分配alloc buffer
    //这里就是sf响应它的申请,值得注意的是这里的buf是int,为mSlots的下标
☆    status_t result = mGraphicBufferProducer->dequeueBuffer(&buf, &fence, swapIntervalZero,
            reqW, reqH, reqFormat, reqUsage);
...
}
↓
↓SurfaceFlinger分配buffer
↓
//framework/native../BufferQueueProducer.cpp
status_t BufferQueueProducer::dequeueBuffer(int *outSlot,
        sp<android::Fence> *outFence, bool async,
        uint32_t width, uint32_t height, uint32_t format, uint32_t usage) {
...
    //检查有无空余项，如果有，将可用下标赋值给found，作为mSlots[]的下标
    int found;
☆    status_t status = waitForFreeSlotThenRelock("dequeueBuffer", async,
            &found, &returnFlags);
    ...
    *outSlot = found;//将可用下标返回给app

    //是否需要分配Buffer
    if (returnFlags & BUFFER_NEEDS_REALLOCATION) {
        status_t error;
        BQ_LOGV("dequeueBuffer: allocating a new buffer for slot %d", *outSlot);
    //mCore 是 BufferQueueCore 对象,mAllocator 是 GraphicBufferAlloc 对象
    //调用 GraphicBufferAlloc 类提供的方法分配buffer
☆        sp<GraphicBuffer> graphicBuffer(mCore->mAllocator->createGraphicBuffer(
                    width, height, format, usage, &error));
...
}
↓
↓
↓
//frameworks/native/libs/gui/GraphicBufferAlloc.cpp
sp<GraphicBuffer> GraphicBufferAlloc::createGraphicBuffer(uint32_t w, uint32_t h, PixelFormat format, uint32_t usage, status_t* error) {
    //new GraphicBuffer 对象，这个过程可以理解为分配(创建)buffer
☆    sp<GraphicBuffer> graphicBuffer(new GraphicBuffer(w, h, format, usage));
    status_t err = graphicBuffer->initCheck();
    *error = err;
...
    return graphicBuffer;
}
↓
↓
↓
//frameworks/native/libs/ui/GraphicBuffer.cpp
GraphicBuffer::GraphicBuffer(uint32_t w, uint32_t h,
        PixelFormat reqFormat, uint32_t reqUsage)
    : BASE(), mOwner(ownData), mBufferMapper(GraphicBufferMapper::get()),
      mInitCheck(NO_ERROR), mId(getUniqueId())
{
...
    //调用Gralloc HAL模块来分配Buffer
☆    mInitCheck = initSize(w, h, reqFormat, reqUsage);
}
↓
↓
↓
GraphicBuffer.cpp
//调用Gralloc HAL模块来分配Buffer
status_t GraphicBuffer::initSize(uint32_t w, uint32_t h, PixelFormat format,
        uint32_t reqUsage)
{
//打开HAL
//这里的get()在头文件中实现:static inline GraphicBufferAllocator& get() { return getInstance(); }
//可能调用/hardware/libhardware/modules/gralloc/gralloc.cpp下的gralloc_device_open(...)
    GraphicBufferAllocator& allocator = GraphicBufferAllocator::get();

//分配：通过Ashmem分配buffer，得到fd；使用fd构造handle，因此handle中将会含有fd，mmap得到vaddr供SurfaceFlinger使用
☆    status_t err = allocator.alloc(w, h, format, reqUsage, &handle, &stride);
    ...
}
↓
↓
↓
GraphicBufferAllocator.cpp
status_t GraphicBufferAllocator::alloc(uint32_t w, uint32_t h, PixelFormat format,
        int usage, buffer_handle_t* handle, int32_t* stride)
{
...
    //这里应该是调用HAL层了
☆    err = mAllocDev->alloc(mAllocDev, w, h, format, usage, handle, stride);
...
        list.add(*handle, rec);
...
}
↓
↓//分配：通过Ashmem分配buffer，得到fd；使用fd构造handle，因此handle中将会含有fd，mmap得到vaddr供SurfaceFlinger使用
↓
/hardware/libhardware/modules/gralloc/gralloc.cpp
dev->device.alloc   = gralloc_alloc
static int gralloc_alloc(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
...
    if (usage & GRALLOC_USAGE_HW_FB) {//有硬件合成时？
        err = gralloc_alloc_framebuffer(dev, size, usage, pHandle);
    } else {//无硬件合成
☆        err = gralloc_alloc_buffer(dev, size, usage, pHandle);
    }
...

    *pStride = stride;
    return 0;
}

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int /*usage*/, buffer_handle_t* pHandle)
{
...
    //通过Ashmem分配buffer，得到fd
☆    fd = ashmem_create_region("gralloc-buffer", size);
    ...
    //使用fd构造handle
    if (err == 0) {
☆        private_handle_t* hnd = new private_handle_t(fd, size, 0);
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
☆        err = mapBuffer(module, hnd);
        if (err == 0) {
☆            *pHandle = hnd;
        }
    }
...
}

```
## 总结
以上便是 SurfaceFlinger 侧响应app侧的请求，进而分配 buffer的大致流程：
* app向发出buffer申请，`SurfaceFlinger`负责分配alloc buffer
* app请求返回fd，然后再mmap，`SurfaceFlinger`则负责返回一个fd
sf侧会先判断是否要进行分配，如果条件满足，就会调用到HAL层的方法，如果是如软件合成 就调用 gralloc_alloc_buffer()，它既会分配 buffer 后 mmap得到一个fd
* 由SurfaceFlinger分配buffer
* 由SurfaceFlinger返回fd(dup)给app，但是app得到的是(dup)
* app获得fd'，mmap获得地址，通过Gralloc HAL来mmap

##### App侧的对应关系
app侧每一个Surface也都有`mSlots[ ]`，BufferSlot { GraphicBuffer { handle { fd base
```
status_t result = mGraphicBufferProducer->dequeueBuffer(&buf, &fence, swapIntervalZero,
        reqW, reqH, reqFormat, reqUsage);

buf即是mSlots[ ]下标
result即是SurfaceFlinger侧判断是否要分配Buffer时的returnFlags
```
