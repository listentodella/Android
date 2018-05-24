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

* app获得fd'，mmap获得地址，通过Gralloc HAL来mmp
以上主要步骤都是在APP进程中进行，因此fd是通过Binder驱动传回给APP
## 关键点
* 由SurfaceFlinger分配buffer
* 由SurfaceFlinger返回fd给app
* app mmap

### app操作-----分配
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
### SurfaceFlinger操作-----分配
* 对应`dequeueBuffer`
* 值得注意的是映射出来的地址仅供`SurfaceFlinger`调用
* `mSlots[]` 构造其中的某一项，含有`GraphicBuffer`,它又含有`handle`，又含有fd，base（它是mmap的结果）
```
//framework/native../BufferQueueProducer.cpp
//有无空余项
status_t status = waitForFreeSlotThenRelock("dequeueBuffer", async,
        &found, &returnFlags);

//是否需要分配Buffer
if (returnFlags & BUFFER_NEEDS_REALLOCATION) {
    status_t error;
    BQ_LOGV("dequeueBuffer: allocating a new buffer for slot %d", *outSlot);
//mCore是BufferQueueCore对象
//mAllocator是GraphicBufferAlloc对象
    sp<GraphicBuffer> graphicBuffer(mCore->mAllocator->createGraphicBuffer(
                width, height, format, usage, &error));
====>>>//frameworks/native/.../Surface.cpp
sp<GraphicBuffer> graphicBuffer(new GraphicBuffer(w, h, format, usage));
=====>>>>//frameworks/native/.../GraphicBuffer.cpp
GraphicBuffer::GraphicBuffer(uint32_t w, uint32_t h, 
        PixelFormat reqFormat, uint32_t reqUsage)
    : BASE(), mOwner(ownData), mBufferMapper(GraphicBufferMapper::get()),
      mInitCheck(NO_ERROR), mId(getUniqueId())
{
...
    mInitCheck = initSize(w, h, reqFormat, reqUsage);
}
//调用Gralloc HAL模块来分配Buffer
status_t GraphicBuffer::initSize(uint32_t w, uint32_t h, PixelFormat format,
        uint32_t reqUsage)
{
//打开HAL
    GraphicBufferAllocator& allocator = GraphicBufferAllocator::get();
//分配：通过Ashmem分配buffer，得到fd；使用fd构造handle，因此handle中将会含有fd，mmap得到vaddr供SurfaceFlinger使用
    status_t err = allocator.alloc(w, h, format, reqUsage, &handle, &stride);
```

##### App侧的对应关系
app侧每一个Surface也都有`mSlots[ ]`，BufferSlot { GraphicBuffer { handle { fd base
```
status_t result = mGraphicBufferProducer->dequeueBuffer(&buf, &fence, swapIntervalZero,
        reqW, reqH, reqFormat, reqUsage);

buf即是mSlots[ ]下标
result即是SurfaceFlinger侧判断是否要分配Buffer时的returnFlags
```






