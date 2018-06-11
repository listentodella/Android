# 004_surface使用vsync过程代码分析

标签（空格分隔）： Vsync Display

---

# `SurfaceFlinger`使用`Vsync`过程

 1. app -> sf
 2. sf 发送`Vsync`请求给`EventThread`(sf)
 3. `EventThread` 发送Vsync请求 给 `DispSyncThread`
 4. `H/W or S/W 产生的Vsync 唤醒 `DispSyncThread`
 5. `DispSyncThread` -> `EventThread` -> sf
 
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


## 步骤①
```
surface->postAndUnlock
queueBuffer
```
## 步骤②
`signalLayerupdate`
```
//SurfaceFlinger.cpp
void SurfaceFlinger::signalLayerUpdate() {
    mEventQueue.invalidate();
}
//MessageQueue.cpp
void MessageQueue::invalidate() {
#if INVALIDATE_ON_VSYNC
    mEvents->requestNextVsync();
#else
    mHandler->dispatchInvalidate();
#endif
}

//EventThread.cpp
void EventThread::Connection::requestNextVsync() {
    mEventThread->requestNextVsync(this);
}
//虽然是在EventThread.cpp里实现，但是是在SurfaceFlinger线程里执行
//发送广播，唤醒EventThread线程
void EventThread::requestNextVsync(
        const sp<EventThread::Connection>& connection) {
    Mutex::Autolock _l(mLock);
    if (connection->count < 0) {//count >= 0代表需要得到Vsync信号
        connection->count = 0;
        mCondition.broadcast();
    }
}
```
## 步骤③`waitforEvent`、⑥
* `向DispSyncThread`发出Vsync请求
判断所有`connection->count` 是否有 >= 0，如果有一个满足表示需要得到Vsync信号，则调用`enableVSyncLocked()`
```
bool EventThread::threadLoop() {
    DisplayEventReceiver::Event event;
    Vector< sp<EventThread::Connection> > signalConnections;
//1.向DispSyncThread发出Vsync请求
//2.等待Vsync信号
    signalConnections = waitForEvent(&event);

...
//步骤⑥
        status_t err = conn->postEvent(event);
...
    return true;
}


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


//EventThread.cpp
void EventThread::enableVSyncLocked() {
    if (!mUseSoftwareVSync) {
        // never enable h/w VSYNC when screen is off
        if (!mVsyncEnabled) {
            mVsyncEnabled = true;
            //给DispSyncThread设置callback，当DispSyncThread收到Vsync信号后就会调用这里设置的callback函数
            mVSyncSource->setCallback(static_cast<VSyncSource::Callback*>(this));
            mVSyncSource->setVSyncEnabled(true);
        }
    }
    mDebugVsyncEnabled = true;
    sendVsyncHintOnLocked();
}

```
## 步骤④
调用`SurfaceFlinger::onVsyncRecevied()`
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
        mHwc.mEventHandler.onVSyncReceived(0, next_vsync);
    }

    return true;
}

//无论是硬件还是软件Vsync都会调用它发送Vsync信号
//SurfaceFlinger.cpp
void SurfaceFlinger::onVSyncReceived(int type, nsecs_t timestamp) {
    bool needsHwVsync = false;

    { // Scope for the lock
        Mutex::Autolock _l(mHWVsyncLock);
        if (type == 0 && mPrimaryHWVsyncEnabled) {
            needsHwVsync = mPrimaryDispSync.addResyncSample(timestamp);
        }
    }

    if (needsHwVsync) {
        enableHardwareVsync();
    } else {
        disableHardwareVsync(false);
    }
}

//DispSync.cpp
bool DispSync::addResyncSample(nsecs_t timestamp) {
...
    updateModelLocked();
...
    return mPeriod == 0 || mError > kErrorThreshold;
}

void DispSync::updateModelLocked() {
...
        mThread->updateModel(mPeriod, mPhase);
...
}

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
        mCond.signal();
    }
...
}
```
## 步骤⑤
* Listener(`EventThread`发现`connection->coun`t >= 0时，向`DispSyncThrea`d注册callback=>callback变成Listener)
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
//收集callback函数
        callbackInvocations = gatherCallbackInvocationsLocked(now);
//调用callback=>最终导致EventThread的onVsyncEvent()被调用
        if (callbackInvocations.size() > 0) {
            fireCallbackInvocations(callbackInvocations);
...
}

EventThread.cpp
//通过广播唤醒EventThread::threadLoop线程(因为该线程在waitForEvent)
void EventThread::onVSyncEvent(nsecs_t timestamp) {
    Mutex::Autolock _l(mLock);
    mVSyncEvent[0].header.type = DisplayEventReceiver::DISPLAY_EVENT_VSYNC;
    mVSyncEvent[0].header.id = 0;
    mVSyncEvent[0].header.timestamp = timestamp;
    mVSyncEvent[0].vsync.count++;
    mCondition.broadcast();
}

//唤醒EventThread线程,向sf或app发出信号
//postEvent会通过connection的fd发送数据写给sf,sf通过得到的数据调用对应的函数
status_t EventThread::Connection::postEvent(
        const DisplayEventReceiver::Event& event) {
    ssize_t size = DisplayEventReceiver::sendEvents(mChannel, &event, 1);
    return size < 0 ? status_t(size) : status_t(NO_ERROR);
}
```
## 步骤⑦ SF对Vsync的处理
```
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
