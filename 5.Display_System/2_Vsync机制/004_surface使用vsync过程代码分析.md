# 004_surface使用vsync过程代码分析

标签（空格分隔）： Vsync Display

---

# `SurfaceFlinger`使用`Vsync`过程

 1. app -> `SurfaceFlinger`线程
 2. `SurfaceFlinger`线程 发送`Vsync`请求给`EventThread for sf`线程
 3. `EventThread`线程 发送`Vsync`请求给 `DispSyncThread`线程
 4. `H/W or S/W`产生的`Vsync`信号 唤醒 `DispSyncThread`线程
 5. `DispSyncThread`线程 -> `EventThread`线程 -> `EventThread for sf`-> `SurfaceFlinger`线程

```seq
app->sf:①发送data
sf->EventThread:②request Vsync
EventThread-->sf:connection
EventThread->DispSyncThread:③发送该请求
Note left of Vysnc信号:H/W
Note right of Vysnc信号:S/W:VsyncThread
Vysnc信号->DispSyncThread:④唤醒
DispSyncThread->EventThread:⑤
EventThread->sf:⑥
Note left of sf:⑦sf处理Vsync
```

![surface对Vsync使用过程](surface%E5%AF%B9Vsync%E4%BD%BF%E7%94%A8%E8%BF%87%E7%A8%8B.jpg)


## 步骤① —— 发送data
首先由app发送数据给`SurfaceFlinger`线程:
```
surface->postAndUnlock()//buffer同步
queueBuffer()//buffer出队
```
## 步骤② —— request Vsync
在 `queueBuffer()` 出队时，会调用 `onFrameAvailable()` 最终通知到 `SurfaceFlinger` 进程,进而`SurfaceFlinger`线程向`EventThread for sf`线程请求`Vsync`信号:
`signalLayerupdate`
```
/frameworks/native/services/surfaceflinger/Layer.cpp
void Layer::onFrameAvailable() {
    android_atomic_inc(&mQueuedFrames);
☆    mFlinger->signalLayerUpdate();
}
↓
↓
↓
//SurfaceFlinger.cpp
void SurfaceFlinger::signalLayerUpdate() {
    mEventQueue.invalidate();
}
↓
//MessageQueue.cpp
void MessageQueue::invalidate() {
#if INVALIDATE_ON_VSYNC
☆    mEvents->requestNextVsync();//定义为 sp<IDisplayEventConnection> mEvents;
#else
    mHandler->dispatchInvalidate();
#endif
}
↓
↓ mEvents 是 BpDisplayEventConnection 代理类对象
↓
/frameworks/native/libs/gui/IDisplayEventConnection.cpp
class BpDisplayEventConnection : public BpInterface<IDisplayEventConnection>
{
public:
    virtual void requestNextVsync() {
        Parcel data, reply;
        data.writeInterfaceToken(IDisplayEventConnection::getInterfaceDescriptor());
        //发起 binder 调用，导致 BnDisplayEventConnection 对应项本地方法调用
☆        remote()->transact(REQUEST_NEXT_VSYNC, data, &reply, IBinder::FLAG_ONEWAY);
    }
}
↓
↓ BnDisplayEventConnection 对应项本地方法调用
↓
status_t BnDisplayEventConnection::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
...
        case REQUEST_NEXT_VSYNC: {
            CHECK_INTERFACE(IDisplayEventConnection, data, reply);
            //调用本地实现
☆            requestNextVsync();
            return NO_ERROR;
        } break;
    }
}
```
`EventThread`类中定义了`Connection`类,它继承了`BnDisplayEventConnection`,并实现了本地方法:
```
/frameworks/native/services/surfaceflinger/EventThread.h
class EventThread : public Thread, private VSyncSource::Callback {
   class Connection : public BnDisplayEventConnection {
☆       virtual void requestNextVsync();    // asynchronous
       ...
}
...
}
↓
↓ 本地方法调用,注意 requestNextVsync(...) 被重载,这里比较好区分,可以通过传参区别;实际上发起调用的对象也不同,也可以用来作区分
↓
//EventThread.cpp
void EventThread::Connection::requestNextVsync() {
    //转而调用 EventThread 提供的方法
    mEventThread->requestNextVsync(this);//定义为 sp<EventThread> mEventThread;
}
↓
↓
↓
//虽然是在 EventThread.cpp 里实现，但是是在 SurfaceFlinger 线程里执行
//发送广播，唤醒 EventThread 线程
void EventThread::requestNextVsync(
        const sp<EventThread::Connection>& connection) {
    Mutex::Autolock _l(mLock);
    if (connection->count < 0) {//count >= 0代表需要得到Vsync信号
        connection->count = 0;
        //发送广播，唤醒 EventThread 线程
☆        mCondition.broadcast();//应该是EventThread.h中定义的 mutable Condition mCondition;
    }
}
```
以上便完成了`SurfaceFlinger`线程接收到app发过来的数据buffer后，向`EventThread`线程发出`Vsync`信号的请求的流程。
`SurfaceFlinger`线程请求`Vsync`信号这一行为可以理解为等到`Vsync`信号来了`EventThread`线程及时通知`SurfaceFlinger`线程。

## 步骤③ —— 发送请求、⑥ —— `postEvent`
### 步骤③ —— `waitForEvent`
`EventThread`向`DispSyncThread`发出`Vsync`请求
判断所有`connection->count` 是否有 `>= 0`，如果有一个满足表示需要得到`Vsync`信号，则调用`enableVSyncLocked()`.
首先,`SurfaceFlinger`线程在`main_surfaceflinger.cpp`中执行时有：
```
/frameworks/native/services/surfaceflinger/main_surfaceflinger.cpp
int main(int, char**) {
...
    // run in this thread
    //进入查看它是个死循环，在 waitevent，它与 EventThread 通过一个文件句柄传输数据
☆    flinger->run();

    return 0;
}
↓
↓
↓
/frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp
void SurfaceFlinger::run() {
    do {
☆        waitForEvent();
    } while (true);
}
↓
void SurfaceFlinger::waitForEvent() {
    mEventQueue.waitMessage();//定义为 mutable MessageQueue mEventQueue;
}
↓
↓
↓
/frameworks/native/services/surfaceflinger/MessageQueue.cpp
void MessageQueue::waitMessage() {
    do {
☆        IPCThreadState::self()->flushCommands();
☆        int32_t ret = mLooper->pollOnce(-1);// JNI调用
        switch (ret) {
            ...
        }
    } while (true);
}
↓
↓ 使用 binder 进行 IPC 通信
↓
/frameworks/native/libs/binder/IPCThreadState.cpp
void IPCThreadState::flushCommands()
{
    if (mProcess->mDriverFD <= 0)
        return;
    talkWithDriver(false);
}
```

运行`SurfaceFlinger`线程时执行了一个死循环，在 `waitevent()`，它与 `EventThread` 通过一个文件句柄传输数据。
现在来看一下 `EventThread` 线程一直在循环的任务:

```
/frameworks/native/services/surfaceflinger/EventThread.cpp
bool EventThread::threadLoop() {
    DisplayEventReceiver::Event event;
    Vector< sp<EventThread::Connection> > signalConnections;
//1.向 DispSyncThread 发出 Vsync 请求
//2.等待 Vsync 信号
☆    signalConnections = waitForEvent(&event);

...
//步骤⑥
☆        status_t err = conn->postEvent(event);
...
    return true;
}
↓ 根据源码英文注释了解到, waitForEvent() 返回的条件:
↓ 1. 接收到一次 Vsync 事件
↓ 2. 在 waiting 时至少有一个 connection 希望获取 Vsync 事件
/frameworks/native/services/surfaceflinger/EventThread.cpp
Vector< sp<EventThread::Connection> > EventThread::waitForEvent(
        DisplayEventReceiver::Event* event)
{
    ...
        // find out connections waiting for events
        size_t count = mDisplayEventConnections.size();
        for (size_t i=0 ; i<count ; i++) {
            sp<Connection> connection(mDisplayEventConnections[i].promote());
            if (connection != NULL) {
                bool added = false;
                if (connection->count >= 0) {
                    // we need vsync events because at least
                    // one connection is waiting for it
//这里根据之前设置的 count 值来判断是否需要设置标志
★★★                    waitForVSync = true;
                    if (timestamp) {
                        // we consume the event only if it's time
                        // (ie: we received a vsync event)
                        if (connection->count == 0) {
                            // fired this time around
                            connection->count = -1;
                            signalConnections.add(connection);
                            added = true;
                        } else if (connection->count == 1 ||
                                (vsyncCount % connection->count) == 0) {
                            // continuous event, and time to report it
                            signalConnections.add(connection);
                            added = true;
                        }
                    }
                }

                if (eventPending && !timestamp && !added) {
                    // we don't have a vsync event to process
                    // (timestamp==0), but we have some pending
                    // messages.
                    signalConnections.add(connection);
                }
            } else {
                // we couldn't promote this reference, the connection has
                // died, so clean-up!
                mDisplayEventConnections.removeAt(i);
                --i; --count;
            }
        }

        // Here we figure out if we need to enable or disable vsyncs
        if (timestamp && !waitForVSync) {
            // we received a VSYNC but we have no clients
            // don't report it, and disable VSYNC events
            disableVSyncLocked();
        } else if (!timestamp && waitForVSync) {
            // we have at least one client, so we want vsync enabled
            // (TODO: this function is called right after we finish
            // notifying clients of a vsync, so this call will be made
            // at the vsync rate, e.g. 60fps.  If we can accurately
            // track the current state we could avoid making this call
            // so often.)
//根据上面设置的标志判断是否需要 enableVSync
//目前(直到8.0仍然，后续是否会有措施未知) 该调用频率根据帧率决定，所以还是很频繁的
★★★         enableVSyncLocked();
        }
...

                // Nobody is interested in vsync, so we just want to sleep.
                // h/w vsync should be disabled, so this will wait until we
                // get a new connection, or an existing connection becomes
                // interested in receiving vsync again.
★★★ 休眠               mCondition.wait(mLock);

...
    return signalConnections;
}
↓
↓ 设置 Callback,收到 Vsync 信号后处理
↓
/frameworks/native/services/surfaceflinger/EventThread.cpp
void EventThread::enableVSyncLocked() {
    if (!mUseSoftwareVSync) {
        // never enable h/w VSYNC when screen is off
        if (!mVsyncEnabled) {
            mVsyncEnabled = true;
//给 DispSyncThread 设置 Callback, 当 DispSyncThread 收到 Vsync 信号后就会调用这里设置的 Callback 函数
//sp<VSyncSource> mVSyncSource,但它是个纯虚类
//该类下的DispSync 对象会周期性地调用 onDispSyncEvent()
☆            mVSyncSource->setCallback(static_cast<VSyncSource::Callback*>(this));
            mVSyncSource->setVSyncEnabled(true);
        }
    }
    mDebugVsyncEnabled = true;
    sendVsyncHintOnLocked();
}
↓
/frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp
virtual void setCallback(const sp<VSyncSource::Callback>& callback) {
    Mutex::Autolock lock(mMutex);
    mCallback = callback;
}
↓ 会周期性地调用它，进而调用到 Callback
/frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp
virtual void onDispSyncEvent(nsecs_t when) {
        sp<VSyncSource::Callback> callback;
        {
            Mutex::Autolock lock(mMutex);
            callback = mCallback;

            if (mTraceVsync) {
                mValue = (mValue + 1) % 2;
                ATRACE_INT(mVsyncEventLabel.string(), mValue);
            }
        }

        if (callback != NULL) {
            //最终导致 EventThread 的 onVsyncEvent() 被调用？
            callback->onVSyncEvent(when);
        }
}
```
以上分析了`SurfaceFlinger`线程、`DispSyncThread`线程和`EventThread`线程之间对于`Vsync`信号进行`waitForEvent()`的工作，下面来看一下它们对于`Vsync`信号的`postEvent()`工作。
### 步骤⑥ —— `postEvent`
```
/frameworks/native/services/surfaceflinger/EventThread.cpp
bool EventThread::threadLoop() {
        // dispatch events to listeners...
//步骤⑥
☆        status_t err = conn->postEvent(event);
...
    return true;
}
↓
↓
/frameworks/native/services/surfaceflinger/EventThread.cpp
status_t EventThread::Connection::postEvent(
        const DisplayEventReceiver::Event& event) {
    ssize_t size = DisplayEventReceiver::sendEvents(mChannel, &event, 1);
    return size < 0 ? status_t(size) : status_t(NO_ERROR);
}
```
以上就是对于一次`Vysnc`信号请求从wait到post的流程。

## 步骤④ —— 唤醒 `DispSyncThread` 线程，即收到`Vsync`信号
调用`SurfaceFlinger::onVSyncReceived()`。
`VsyncThread`线程用于产生`Vsync`信号。因为实际上`Vsync`信号应该是和硬件相关的，所以最开始的构造函数走的是`HWComposer()`而不是直接走`VsyncThread`的构造函数。这里分析的是软件产生的`Vsync`信号。
```
HWComposer.cpp
bool HWComposer::VSyncThread::threadLoop() {
    { // scope for lock
        Mutex::Autolock _l(mLock);
        while (!mEnabled) {
            mCondition.wait(mLock);
        }
    }

    const nsecs_t period = mRefreshPeriod;
    const nsecs_t now = systemTime(CLOCK_MONOTONIC);
    nsecs_t next_vsync = mNextFakeVSync;
    nsecs_t sleep = next_vsync - now;
    if (sleep < 0) {
        // we missed, find where the next vsync should be
        sleep = (period - ((now - next_vsync) % period));
        next_vsync = now + sleep;
    }
    mNextFakeVSync = next_vsync + period;
    //计算休眠时间
    struct timespec spec;
    spec.tv_sec  = next_vsync / 1000000000;
    spec.tv_nsec = next_vsync % 1000000000;

    int err;
    do {
    //休眠
        err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
    } while (err<0 && errno == EINTR);

    if (err == 0) {
☆        mHwc.mEventHandler.onVSyncReceived(0, next_vsync);
    }

    return true;
}
↓
↓ //无论是硬件还是软件产生的 Vsync 都会调用它发送 Vsync 信号
↓
//SurfaceFlinger.cpp
void SurfaceFlinger::onVSyncReceived(int type, nsecs_t timestamp) {
    bool needsHwVsync = false;

    { // Scope for the lock
        Mutex::Autolock _l(mHWVsyncLock);
        if (type == 0 && mPrimaryHWVsyncEnabled) {
            //无论是硬件还是软件产生的 Vsync 都会调先用它发送 Vsync 信号
☆            needsHwVsync = mPrimaryDispSync.addResyncSample(timestamp);
        }
    }

    if (needsHwVsync) {
        enableHardwareVsync();
    } else {
        disableHardwareVsync(false);
    }
}
↓
//DispSync.cpp
bool DispSync::addResyncSample(nsecs_t timestamp) {
...
    updateModelLocked();
...
    return mPeriod == 0 || mError > kErrorThreshold;
}
↓
//DispSync.cpp
void DispSync::updateModelLocked() {
...
        mThread->updateModel(mPeriod, mPhase);
...
}
↓
//DispSync.cpp
class DispSyncThread: public Thread {
public:

    DispSyncThread():
            mStop(false),
            mPeriod(0),
            mPhase(0),
            mWakeupLatency(0) {
    }

    virtual ~DispSyncThread() {}
//要注意虽然是在DispSync.cpp里实现，属于DispSyncThread类，但其调用者是VsyncThread线程
    void updateModel(nsecs_t period, nsecs_t phase) {
        Mutex::Autolock lock(mMutex);
        mPeriod = period;
        mPhase = phase;
☆唤醒        mCond.signal();
    }
...
}
```
以上就是`VsyncThread`线程收到`Vysnc`信号后(不管是软件产生还是硬件产生)，唤醒`DispSyncThread`的流程。

## 步骤⑤ —— `DispSyncThread`唤醒`EventThread`
* `DispSyncThread` 线程传递到`EventThread` 线程
`DispSyncThread` 线程被唤醒后，需要先唤醒`EventThread`线程。
`Listener`(`EventThread`发现`connection->count >= 0`时，向`DispSyncThread`注册`Callback`=>callback变成Listener)
```
DispSync.cpp
virtual bool threadLoop() {
...
//计算最近的EventThread(Listener)的时间
        nextEventTime = computeNextEventTimeLocked(now);
...
//休眠
        if (now < targetTime) {
            err = mCond.waitRelative(mMutex, targetTime - now);
...
//收集Callback函数
        callbackInvocations = gatherCallbackInvocationsLocked(now);
//调用Callback=>最终导致 EventThread 的 onVsyncEvent() 被调用
        if (callbackInvocations.size() > 0) {
            fireCallbackInvocations(callbackInvocations);
...
}

void fireCallbackInvocations(const Vector<CallbackInvocation>& callbacks) {
    for (size_t i = 0; i < callbacks.size(); i++) {
        callbacks[i].mCallback->onDispSyncEvent(callbacks[i].mEventTime);
    }
}
↓
↓  调用 EventThread 的 onVsyncEvent()
↓
EventThread.cpp
void EventThread::onVSyncEvent(nsecs_t timestamp) {
    Mutex::Autolock _l(mLock);
    mVSyncEvent[0].header.type = DisplayEventReceiver::DISPLAY_EVENT_VSYNC;
    mVSyncEvent[0].header.id = 0;
    mVSyncEvent[0].header.timestamp = timestamp;
    mVSyncEvent[0].vsync.count++;
    //通过广播唤醒 EventThread::threadLoop 线程(因为有需求的线程在 waitForEvent )
    mCondition.broadcast();
}
```
唤醒 `EventThread`后，最后传递到`SurfaceFlinger`：
```
//唤醒 EventThread 线程,向sf或app发出信号
//postEvent 会通过 connection 的fd发送数据写给sf,sf通过得到的数据调用对应的函数
status_t EventThread::Connection::postEvent(
        const DisplayEventReceiver::Event& event) {
    ssize_t size = DisplayEventReceiver::sendEvents(mChannel, &event, 1);
    return size < 0 ? status_t(size) : status_t(NO_ERROR);
}
```
以上就是`DispSyncThread` 线程唤醒`EventThread` 线程的流程。

## 步骤⑦ `SurfaceFlinger`对`Vsync`的处理
回顾一下创建`EventThread for sf`时
```
//SurfaceFlinger.cpp
void SurfaceFlinger::init() {
...
☆    sp<VSyncSource> sfVsyncSrc = new DispSyncSource(&mPrimaryDispSync,
            sfVsyncPhaseOffsetNs, true, "sf");
    mSFEventThread = new EventThread(sfVsyncSrc);
☆    mEventQueue.setEventThread(mSFEventThread);
...
}
↓
↓
/frameworks/native/services/surfaceflinger/MessageQueue.cpp
void MessageQueue::setEventThread(const sp<EventThread>& eventThread)
{
    mEventThread = eventThread;
    mEvents = eventThread->createEventConnection();
    mEventTube = mEvents->getDataChannel();
    mLooper->addFd(mEventTube->getFd(), 0, Looper::EVENT_INPUT,
☆            MessageQueue::cb_eventReceiver, this);
}
↓
↓
//MessageQueue.cpp
int MessageQueue::cb_eventReceiver(int fd, int events, void* data) {
    MessageQueue* queue = reinterpret_cast<MessageQueue *>(data);
    return queue->eventReceiver(fd, events);
}

int MessageQueue::eventReceiver(int /*fd*/, int /*events*/) {
...
#if INVALIDATE_ON_VSYNC
                mHandler->dispatchInvalidate();
#else
                mHandler->dispatchRefresh();
#endif
                break;
            }
        }
    }
    return 1;
}

void MessageQueue::Handler::dispatchInvalidate() {
    if ((android_atomic_or(eventMaskInvalidate, &mEventMask) & eventMaskInvalidate) == 0) {
        mQueue.mLooper->sendMessage(this, Message(MessageQueue::INVALIDATE));
    }
}

void MessageQueue::Handler::handleMessage(const Message& message) {
    switch (message.what) {
        case INVALIDATE:
            android_atomic_and(~eventMaskInvalidate, &mEventMask);
            mQueue.mFlinger->onMessageReceived(message.what);
            break;
        case REFRESH:
            android_atomic_and(~eventMaskRefresh, &mEventMask);
            mQueue.mFlinger->onMessageReceived(message.what);
            break;
        case TRANSACTION:
            android_atomic_and(~eventMaskTransaction, &mEventMask);
            mQueue.mFlinger->onMessageReceived(message.what);
            break;
    }
}

//SurfaceFlinger.cpp
void SurfaceFlinger::onMessageReceived(int32_t what) {
    ATRACE_CALL();
    switch (what) {
    case MessageQueue::TRANSACTION:
        handleMessageTransaction();
        break;
    ★case MessageQueue::INVALIDATE:
        handleMessageTransaction();
        handleMessageInvalidate();
        signalRefresh();
        break;
    case MessageQueue::REFRESH:
        handleMessageRefresh();
        break;
    }
}

```
