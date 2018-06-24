# 6_Reader线程_简单处理

标签（空格分隔）： Input Reader

---

## Reader线程
### 获得事件
### 简单处理
* 根据`RawEvent`的`type`处理
1. Add Device
2. Remove Device
3. 真正的输入事件
```
void InputReader::processEventsLocked(const RawEvent* rawEvents, size_t count) {
    for (const RawEvent* rawEvent = rawEvents; count;) {
        int32_t type = rawEvent->type;
        size_t batchSize = 1;
        if (type < EventHubInterface::FIRST_SYNTHETIC_EVENT) {
            int32_t deviceId = rawEvent->deviceId;
            while (batchSize < count) {
                if (rawEvent[batchSize].type >= EventHubInterface::FIRST_SYNTHETIC_EVENT
                        || rawEvent[batchSize].deviceId != deviceId) {
                    break;
                }
                batchSize += 1;
            }
#if DEBUG_RAW_EVENTS
            ALOGD("BatchSize: %d Count: %d", batchSize, count);
#endif
            processEventsForDeviceLocked(deviceId, rawEvent, batchSize);
        } else {
            switch (rawEvent->type) {
            case EventHubInterface::DEVICE_ADDED:
                addDeviceLocked(rawEvent->when, rawEvent->deviceId);
                break;
            case EventHubInterface::DEVICE_REMOVED:
                removeDeviceLocked(rawEvent->when, rawEvent->deviceId);
                break;
            case EventHubInterface::FINISHED_DEVICE_SCAN:
                handleConfigurationChangedLocked(rawEvent->when);
                break;
            default:
                ALOG_ASSERT(false); // can't happen
                break;
            }
        }
        count -= batchSize;
        rawEvent += batchSize;
    }
}

```
### add Devices
`EventHub`中已经对`mDevices`进行过add了，而`InputReader`里又add了一次，这么做是为了分层。
`InputReader`下：
```
.mDevices -> input_Device {
    mID//可以通过它找到EventHub.mDevice.Device
    mMappers//对上报的事件，用mMappers处理
    //对于键盘，它有一个keyboardInputMapper
}
```
`EventHub`下：
```
.mDevices -> Device {
    fd
    identify//通过ioctl获取一些设备信息
    KeyMap{.kl, .kcm}
}
```
可见，`EventHub`的`Device`主要用来读取事件，而`InputReader`下的`mDevices.input_Device`只要用来处理事件。
### Remove Devices
过程和add类似
### 真正的输入事件
```
void InputReader::processEventsForDeviceLocked(int32_t deviceId,
        const RawEvent* rawEvents, size_t count) {
    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex < 0) {
        ALOGW("Discarding event for unknown deviceId %d.", deviceId);
        return;
    }

    InputDevice* device = mDevices.valueAt(deviceIndex);
    if (device->isIgnored()) {
        //ALOGD("Discarding event for ignored deviceId %d.", deviceId);
        return;
    }

★    device->process(rawEvents, count);
}
```
对于不同的输入设备，有不同的`process`，例如对键盘：
```
void KeyboardInputMapper::process(const RawEvent* rawEvent) {
    switch (rawEvent->type) {
    case EV_KEY: {
        int32_t scanCode = rawEvent->code;
        int32_t usageCode = mCurrentHidUsage;
        mCurrentHidUsage = 0;
    
        if (isKeyboardOrGamepadKey(scanCode)) {
            int32_t keyCode;
            uint32_t flags;
            //传入底层上报的scancode，得到上层用的keyCode
★            if (getEventHub()->mapKey(getDeviceId(), scanCode, usageCode, &keyCode, &flags)){
                keyCode = AKEYCODE_UNKNOWN;
                flags = 0;
            } 
★            processKey(rawEvent->when, rawEvent->value != 0, keyCode, scanCode, flags);
        }
        break;
    }
    case EV_MSC: {
        if (rawEvent->code == MSC_SCAN) {
            mCurrentHidUsage = rawEvent->value;
        }
        break;
    }
    case EV_SYN: {
        if (rawEvent->code == SYN_REPORT) {
            mCurrentHidUsage = 0;
        }
    }
    }
}


void KeyboardInputMapper::processKey(nsecs_t when, bool down, int32_t keyCode,
        int32_t scanCode, uint32_t policyFlags) {
...
    //构造args
    NotifyKeyArgs args(when, getDeviceId(), mSource, policyFlags,
            down ? AKEY_EVENT_ACTION_DOWN : AKEY_EVENT_ACTION_UP,
            AKEY_EVENT_FLAG_FROM_SYSTEM, keyCode, scanCode, newMetaState, downTime);
    //丢给Dispatch线程处理
    getListener()->notifyKey(&args);
}
```

## 总结

![Reader线程简单分析](Reader%E7%BA%BF%E7%A8%8B%E7%AE%80%E5%8D%95%E5%88%86%E6%9E%90.png)

