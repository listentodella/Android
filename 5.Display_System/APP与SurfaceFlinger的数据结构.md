# APP与SurfaceFlinger的数据结构
* 每个app可以有多个surface，每个surface可以有多个buffer
* app向SurfaceFlinger申请
```
layer，用来表示app的surface;client表示app
生产者：mProducer
消费者：mSurfaceFlingerConsumer
二者有同一个mCore（BufferQueueCore）,它有一个mSlots（BufferSlot[64]数组），这个bufferslot每一项中有一个mGraphicBuffer,用来表示buffer
```