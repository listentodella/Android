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
            case EventHubInterface::DEVICE_ADDED://设备插入
                addDeviceLocked(rawEvent->when, rawEvent->deviceId);
                break;
            case EventHubInterface::DEVICE_REMOVED://设备拔出
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
```
void InputReader::addDeviceLocked(nsecs_t when, int32_t deviceId) {
    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex >= 0) {
        ALOGW("Ignoring spurious device added event for deviceId %d.", deviceId);
        return;
    }

    InputDeviceIdentifier identifier = mEventHub->getDeviceIdentifier(deviceId);
    uint32_t classes = mEventHub->getDeviceClasses(deviceId);
    int32_t controllerNumber = mEventHub->getDeviceControllerNumber(deviceId);
//创建设备
 ★   InputDevice* device = createDeviceLocked(deviceId, controllerNumber, identifier, classes);
    device->configure(when, &mConfig, 0);
    device->reset(when);

    if (device->isIgnored()) {
        ALOGI("Device added: id=%d, name='%s' (ignored non-input device)", deviceId,
                identifier.name.string());
    } else {
        ALOGI("Device added: id=%d, name='%s', sources=0x%08x", deviceId,
                identifier.name.string(), device->getSources());
    }
//添加设备
 ★   mDevices.add(deviceId, device);
    bumpGenerationLocked();
}
```

`EventHub`中已经对`mDevices`进行过add了，而`InputReader`里又add了一次，这么做是为了分层。
`InputReader`下有一个`mDevices`成员，它包含了`input_Device`：
```
.mDevices -> input_Device {
    mID//可以通过它找到EventHub.mDevice.Device
    mMappers//对上报的事件，用mMappers处理
    //对于键盘，它有一个keyboardInputMapper
}
```
`EventHub`下也有一个`mDevices`成员，它包含了很多`Device`:
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
// device 是 InputDevice 实例化对象
★    device->process(rawEvents, count);
}
↓
↓
↓
void InputDevice::process(const RawEvent* rawEvents, size_t count) {
    // Process all of the events in order for each mapper.
    // We cannot simply ask each mapper to process them in bulk because mappers may
    // have side-effects that must be interleaved.  For example, joystick movement events and
    // gamepad button presses are handled by different mappers but they should be dispatched
    // in the order received.
    size_t numMappers = mMappers.size();
    for (const RawEvent* rawEvent = rawEvents; count--; rawEvent++) {
#if DEBUG_RAW_EVENTS
        ALOGD("Input event: device=%d type=0x%04x code=0x%04x value=0x%08x when=%lld",
                rawEvent->deviceId, rawEvent->type, rawEvent->code, rawEvent->value,
                rawEvent->when);
#endif

        if (mDropUntilNextSync) {
            if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT) {
                mDropUntilNextSync = false;
#if DEBUG_RAW_EVENTS
                ALOGD("Recovered from input event buffer overrun.");
#endif
            } else {
#if DEBUG_RAW_EVENTS
                ALOGD("Dropped input event while waiting for next input sync.");
#endif
            }
        } else if (rawEvent->type == EV_SYN && rawEvent->code == SYN_DROPPED) {
            ALOGI("Detected input event buffer overrun for device %s.", getName().string());
            mDropUntilNextSync = true;
            reset(rawEvent->when);
        } else {
            for (size_t i = 0; i < numMappers; i++) {
            //把 mMappers 一个个取出来，调用对应的 process 函数
★                InputMapper* mapper = mMappers[i];
★                mapper->process(rawEvent);
            }
        }
    }
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
    //丢给 Dispatch 线程处理
    getListener()->notifyKey(&args);
}
```
如何将从底层得到的`scancode`转换为上层用的`keyCode`:
```
//EventHub.cpp
status_t EventHub::mapKey(int32_t deviceId, int32_t scanCode, int32_t usageCode,
        int32_t* outKeycode, uint32_t* outFlags) const {
    AutoMutex _l(mLock);
    Device* device = getDeviceLocked(deviceId);

    if (device) {
        // Check the key character map first.
        //先用 kcm 文件映射
        sp<KeyCharacterMap> kcm = device->getKeyCharacterMap();
        if (kcm != NULL) {
            if (!kcm->mapKey(scanCode, usageCode, outKeycode)) {
                *outFlags = 0;
                return NO_ERROR;
            }
        }

        // Check the key layout next.
        //如果没有 kcm 则用 kl 文件映射
        if (device->keyMap.haveKeyLayout()) {
            if (!device->keyMap.keyLayoutMap->mapKey(
                    scanCode, usageCode, outKeycode, outFlags)) {
                return NO_ERROR;
            }
        }
    }

    *outKeycode = 0;
    *outFlags = 0;
    return NAME_NOT_FOUND;
}

```

## 总结


![Reader线程简单分析](Reader%E7%BA%BF%E7%A8%8B%E7%AE%80%E5%8D%95%E5%88%86%E6%9E%90.png)

