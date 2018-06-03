# APP 提交(unlockAndPost) Buffer的过程_框架

标签（空格分隔）： Display SurfaceFlinger

---

```seq
App->SurfaceFlinger: binder
Note left of App:createSurface
Note right of SurfaceFlinger:client
Note left of App:SurfaceControl：含有生产者代理
Note right of SurfaceFlinger:Layer:生产者、消费者
Note left of App:getSurface，得到Surface，则含有生产者代理、mSlots
App->SurfaceFlinger: dequeueBuffer
Note right of SurfaceFlinger:向ashmem申请buffer
Note left of App:填充数据到vaddr后，上交给SF
App->SurfaceFlinger: unlockAndPost
```
* `SurfaceControl`里的生产者代理指向`Layer`里的生产者
* 生产者、消费者里都含有`mCore`、`mSlots`(引用)
* 两个`mCore`都指向同一个`BufferQueueCore`对象
* 两个`mSlots`也都引用同一个`BufferSlot`数组
* `Surface`对象 通过`lock`申请buffer，进而通过生产者代理发出`dequeueBuffer`请求
* 右边`SurfaceFlinger`向`ashmem`申请buffer，得到fd，mmap(fd)
* `SurfaceFlinger`向app通过binder返回fd,也会填入某个`mSlots[xx]`
* app得到fd'
* app 得到vaddr = mmap(fd')，会填入某个`mSlots[xx]`

# unlockAndPost 猜测
## 通知消费者
## 消费者通知Layer
## Layer通知client
## client通知SurfaceFlinger


