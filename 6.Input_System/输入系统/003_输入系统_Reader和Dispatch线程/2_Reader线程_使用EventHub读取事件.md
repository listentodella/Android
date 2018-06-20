# 2_Reader线程_使用EventHub读取事件

标签（空格分隔）： Input EventHub Reader

---
## `Reader`线程 主要任务
* 使用`EventHub`读取事件
* 简单处理
* 传给`Dispatch`线程

### 使用`EventHub`读取事件
```
InputReader.cpp
void InputReader::loopOnce() {
    int32_t oldGeneration;
    int32_t timeoutMillis;
    bool inputDevicesChanged = false;
    Vector<InputDeviceInfo> inputDevices;
    { // acquire lock
        AutoMutex _l(mLock);

        oldGeneration = mGeneration;
        timeoutMillis = -1;

        uint32_t changes = mConfigurationChangesToRefresh;
        if (changes) {
            mConfigurationChangesToRefresh = 0;
            timeoutMillis = 0;
            refreshConfigurationLocked(changes);
        } else if (mNextTimeout != LLONG_MAX) {
            nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
            timeoutMillis = toMillisecondTimeoutDelay(now, mNextTimeout);
        }
    } // release lock
//使用EventHub来读取事件，存放于mEventBuffer中
★    size_t count = mEventHub->getEvents(timeoutMillis, mEventBuffer, EVENT_BUFFER_SIZE);

    { // acquire lock
        AutoMutex _l(mLock);
        mReaderIsAliveCondition.broadcast();

        if (count) {
            processEventsLocked(mEventBuffer, count);
        }

        if (mNextTimeout != LLONG_MAX) {
            nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
            if (now >= mNextTimeout) {
#if DEBUG_RAW_EVENTS
                ALOGD("Timeout expired, latency=%0.3fms", (now - mNextTimeout) * 0.000001f);
#endif
                mNextTimeout = LLONG_MAX;
                timeoutExpiredLocked(now);
            }
        }

        if (oldGeneration != mGeneration) {
            inputDevicesChanged = true;
            getInputDevicesLocked(inputDevices);
        }
    } // release lock

    // Send out a message that the describes the changed input devices.
    if (inputDevicesChanged) {
        mPolicy->notifyInputDevicesChanged(inputDevices);
    }

    // Flush queued events out to the listener.
    // This must happen outside of the lock because the listener could potentially call
    // back into the InputReader's methods, such as getScanCodeState, or become blocked
    // on another thread similarly waiting to acquire the InputReader lock thereby
    // resulting in a deadlock.  This situation is actually quite plausible because the
    // listener is actually the input dispatcher, which calls into the window manager,
    // which occasionally calls into the input reader.
    mQueuedListener->flush();
}

```

* `Reader`线程获得事件，用以下结构体表示，以及其成员`type`所包含的事件有：
```
/*
 * A raw event as retrieved from the EventHub.
 */
struct RawEvent {
    nsecs_t when;
    int32_t deviceId;
    int32_t type;
    int32_t code;
    int32_t value;
};

enum {
    // Sent when a device is added.
    DEVICE_ADDED = 0x10000000,
    // Sent when a device is removed.
    DEVICE_REMOVED = 0x20000000,
    // Sent when all added/removed devices from the most recent scan have been reported.
    // This event is always sent at least once.
    FINISHED_DEVICE_SCAN = 0x30000000,

    FIRST_SYNTHETIC_EVENT = DEVICE_ADDED,
};

type:
DEVICE_ADDED
DEVICE_REMOVED
FINISHED_DEVICE_SCAN
EV_KEY
EV_ABS
EV_REL
...
```
而对于底层输入事件上报的数据类型
```
struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

type:
EV_KEY
EV_ABS
EV_REL
...
```
可见上层对底层事件进行了扩展。
对于InputReader.cpp，它通过`mEventHub->getEvents(...)`获得`RawEvent`类型数据，底层驱动则是，例如键盘，产生了数据，驱动程序就会上报数据`input_event`，上层即可获取`input_event`并将其包装成`RawEvent`。
对于设备的拔插，`EventHub`使用`inotify`检测`/dev/input`路径；
对于数据，使用`epoll`监测有无数据
#### 细节
##### 使用`inofity`监测
```
参考EventHub构造函数
int fd1 = inotify_init("/dev/input", ...);
```
##### 对于该路径下已存在的节点
```
参考EventHub.cpp下的scanDevicesLocked()的作用
fd2 = open("/dev/input/event0", ...)
fd3 = open("/dev/input/event1", ...)
```
##### 使用`epoll_wait`监测这些fds
参考EventHub.cpp下的scanDevicesLocked()的作用，同时它也处于`getEvents`的循环里
##### 读取fd，构造`RawEvent`//`getEvents`的循环里
* 当`epoll_wait`返回时，说明有数据产生了，如果返回的文件句柄恰好等于`fd1`，则说明该路径下有设备拔插了（因为有节点创建或删除），并且`open`或`close`该节点，将其加入或移出`epoll_wait`
```
RawEvent.type == DEVICE_ADDED 设备插入
RawEvent.type == DEVICE_REMOVED 设备移除
```
* 如果`epoll_wait`返回了其他的文件句柄时，则表明输入设备有数据，读出`input_event`，构造`RawEvent`：
```
RawEvent.type == EV_KEY等其他对应的事件
```
