# SurfaceFlinger内部机制（一）
## APP与SurfaceFlinger的数据结构
* 每个app可以有多个surface，每个surface可以有多个buffer
* app向SurfaceFlinger申请
```
layer，用来表示app的surface;client表示app
生产者：mProducer
消费者：mSurfaceFlingerConsumer
二者有同一个mCore（BufferQueueCore）,它有一个mSlots（BufferSlot[64]数组），这个bufferslot每一项中有一个mGraphicBuffer,用来表示buffer
```
![SurfaceFlinger数据结构](SurfaceFlinger%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84.png)


SurfaceFlinger里肯定有结构体表示App————Client
肯定也有结构体表示Surface————Layer
肯定也有结构体表示Surface里的buffer————sp<GraphicBuffer>mGraphicBuffer

每个app都有SurfaceComposerClient来与SurfaceFinger连接
sp<SurfaceComposerClient> client = new SurfaceComposerClient();
其成员mClient指向SurfaceFlinger里的Client
每一个Surface都对应有一个SurfaceControl
Surface里有一个数组BufferSlot mSlots[64]

