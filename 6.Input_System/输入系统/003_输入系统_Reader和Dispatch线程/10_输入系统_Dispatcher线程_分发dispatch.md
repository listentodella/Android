# 10_输入系统_Dispatcher线程_分发dispatch

标签（空格分隔）： Input

---

首先`InputReader`线程从输入设备得到输入事件，并稍作处理（紧急事件），然后将其放到队列，唤醒`InputDisaptcher`线程，然后它先处理，并将user分给app。
`Dispatcher`线程里包含一个`mConnectionByFd`，它则包含了许多`Connection`，每个`Connection`都对应一个app。值得注意的是，`Connection`含有一个`fd0`，而app含有一个`fd1`（通过`dup`得到），它们通过`socketpair`通讯。

## 分发流程(以按键为例)
* 查找目标：
向`WindowManagerService`查询当前`window`，获得对应的`Connection`
* 把输入事件放入`Connection`的队列`outboundQueue`
* 从队列中逐个把事件写入`fd`
取出，构造出`InputMsg`发送

## 源码分析
### 入口
```
/frameworks/native/services/inputflinger/InputDispatcher.cpp
bool InputDispatcher::dispatchKeyLocked(nsecs_t currentTime, KeyEntry* entry,
        DropReason* dropReason, nsecs_t* nextWakeupTime) {
...
// Identify targets.
    Vector<InputTarget> inputTargets;
    //查找目标，构造 inputTargets
★    int32_t injectionResult = findFocusedWindowTargetsLocked(currentTime,
            entry, inputTargets, nextWakeupTime);
    if (injectionResult == INPUT_EVENT_INJECTION_PENDING) {
        return false;
    }

    setInjectionResultLocked(entry, injectionResult);
    if (injectionResult != INPUT_EVENT_INJECTION_SUCCEEDED) {
        return true;
    }

    addMonitoringTargetsLocked(inputTargets);

    // Dispatch the key.
    //分发, 利用上面构造好的 inputTargets 
★    dispatchEventLocked(currentTime, entry, inputTargets);
    return true;

...
}
```
### 查找目标
```
/frameworks/native/services/inputflinger/InputDispatcher.cpp
int32_t InputDispatcher::findFocusedWindowTargetsLocked(nsecs_t currentTime,
        const EventEntry* entry, Vector<InputTarget>& inputTargets, nsecs_t* nextWakeupTime) {
    int32_t injectionResult;
    String8 reason;

    // If there is no currently focused window and no focused application
    // then drop the event.
    //该变量表示目标窗口，它由wms来设置
    if (mFocusedWindowHandle == NULL) {
        if (mFocusedApplicationHandle != NULL) {
            injectionResult = handleTargetsNotReadyLocked(currentTime, entry,
                    mFocusedApplicationHandle, NULL, nextWakeupTime,
                    "Waiting because no window has focus but there is a "
                    "focused application that may eventually add a window "
                    "when it finishes starting up.");
            goto Unresponsive;
        }

        ALOGI("Dropping event because there is no focused window or focused application.");
        injectionResult = INPUT_EVENT_INJECTION_FAILED;
        goto Failed;
    }

    // Check permissions.
    if (! checkInjectionPermission(mFocusedWindowHandle, entry->injectionState)) {
        injectionResult = INPUT_EVENT_INJECTION_PERMISSION_DENIED;
        goto Failed;
    }

    // Check whether the window is ready for more input.
    reason = checkWindowReadyForMoreInputLocked(currentTime,
            mFocusedWindowHandle, entry, "focused");
    if (!reason.isEmpty()) {
        injectionResult = handleTargetsNotReadyLocked(currentTime, entry,
                mFocusedApplicationHandle, mFocusedWindowHandle, nextWakeupTime, reason.string());
        goto Unresponsive;
    }

    // Success!  Output targets.
    //使用 目标窗口 mFocusedWindowHandle 构造出一个 InputTarget
    injectionResult = INPUT_EVENT_INJECTION_SUCCEEDED;
★    addWindowTargetLocked(mFocusedWindowHandle,
            InputTarget::FLAG_FOREGROUND | InputTarget::FLAG_DISPATCH_AS_IS, BitSet32(0),
            inputTargets);

    // Done.
Failed:
Unresponsive:
    nsecs_t timeSpentWaitingForApplication = getTimeSpentWaitingForApplicationLocked(currentTime);
    updateDispatchStatisticsLocked(currentTime, entry,
            injectionResult, timeSpentWaitingForApplication);
#if DEBUG_FOCUS
    ALOGD("findFocusedWindow finished: injectionResult=%d, "
            "timeSpentWaitingForApplication=%0.1fms",
            injectionResult, timeSpentWaitingForApplication / 1000000.0);
#endif
    return injectionResult;
}
↓
↓
↓
void InputDispatcher::addWindowTargetLocked(const sp<InputWindowHandle>& windowHandle,
        int32_t targetFlags, BitSet32 pointerIds, Vector<InputTarget>& inputTargets) {
    inputTargets.push();

    const InputWindowInfo* windowInfo = windowHandle->getInfo();
    InputTarget& target = inputTargets.editTop();
★    target.inputChannel = windowInfo->inputChannel;
    target.flags = targetFlags;
    target.xOffset = - windowInfo->frameLeft;
    target.yOffset = - windowInfo->frameTop;
    target.scaleFactor = windowInfo->scaleFactor;
    target.pointerIds = pointerIds;
}
```

### 分发
```
void InputDispatcher::dispatchEventLocked(nsecs_t currentTime,
        EventEntry* eventEntry, const Vector<InputTarget>& inputTargets) {
#if DEBUG_DISPATCH_CYCLE
    ALOGD("dispatchEventToCurrentInputTargets");
#endif

    ALOG_ASSERT(eventEntry->dispatchInProgress); // should already have been set to true

    pokeUserActivityLocked(eventEntry);

    for (size_t i = 0; i < inputTargets.size(); i++) {
        const InputTarget& inputTarget = inputTargets.itemAt(i);

        ssize_t connectionIndex = getConnectionIndexLocked(inputTarget.inputChannel);
        if (connectionIndex >= 0) {
        //取出 Connection，并将输入事件放到  prepareDispatchCycleLocked 队列里
★            sp<Connection> connection = mConnectionsByFd.valueAt(connectionIndex);
★             prepareDispatchCycleLocked(currentTime, connection, eventEntry, &inputTarget);
        } else {
#if DEBUG_FOCUS
            ALOGD("Dropping event delivery to target with channel '%s' because it "
                    "is no longer registered with the input dispatcher.",
                    inputTarget.inputChannel->getName().string());
#endif
        }
    }
}
↓
↓
↓
void InputDispatcher::prepareDispatchCycleLocked(nsecs_t currentTime,
        const sp<Connection>& connection, EventEntry* eventEntry, const InputTarget* inputTarget) {
#if DEBUG_DISPATCH_CYCLE
    ALOGD("channel '%s' ~ prepareDispatchCycle - flags=0x%08x, "
            "xOffset=%f, yOffset=%f, scaleFactor=%f, "
            "pointerIds=0x%x",
            connection->getInputChannelName(), inputTarget->flags,
            inputTarget->xOffset, inputTarget->yOffset,
            inputTarget->scaleFactor, inputTarget->pointerIds.value);
#endif

    // Skip this event if the connection status is not normal.
    // We don't want to enqueue additional outbound events if the connection is broken.
    if (connection->status != Connection::STATUS_NORMAL) {
#if DEBUG_DISPATCH_CYCLE
        ALOGD("channel '%s' ~ Dropping event because the channel status is %s",
                connection->getInputChannelName(), connection->getStatusLabel());
#endif
        return;
    }

    // Split a motion event if needed.
    //触摸屏相关的
    if (inputTarget->flags & InputTarget::FLAG_SPLIT) {
        ALOG_ASSERT(eventEntry->type == EventEntry::TYPE_MOTION);

        MotionEntry* originalMotionEntry = static_cast<MotionEntry*>(eventEntry);
        if (inputTarget->pointerIds.count() != originalMotionEntry->pointerCount) {
            MotionEntry* splitMotionEntry = splitMotionEvent(
                    originalMotionEntry, inputTarget->pointerIds);
            if (!splitMotionEntry) {
                return; // split event was dropped
            }
#if DEBUG_FOCUS
            ALOGD("channel '%s' ~ Split motion event.",
                    connection->getInputChannelName());
            logOutboundMotionDetailsLocked("  ", splitMotionEntry);
#endif
            enqueueDispatchEntriesLocked(currentTime, connection,
                    splitMotionEntry, inputTarget);
            splitMotionEntry->release();
            return;
        }
    }

    // Not splitting.  Enqueue dispatch entries for the event as is.
★    enqueueDispatchEntriesLocked(currentTime, connection, eventEntry, inputTarget);
}
↓
↓
↓
void InputDispatcher::enqueueDispatchEntriesLocked(nsecs_t currentTime,
        const sp<Connection>& connection, EventEntry* eventEntry, const InputTarget* inputTarget) {
    bool wasEmpty = connection->outboundQueue.isEmpty();

    // Enqueue dispatch entries for the requested modes.
    //它会根据 FLAG 来决定是否将事件放到队列里去
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_HOVER_EXIT);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_OUTSIDE);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_HOVER_ENTER);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_IS);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER);

    // If the outbound queue was previously empty, start the dispatch cycle going.
    if (wasEmpty && !connection->outboundQueue.isEmpty()) {
    //把队列里的事件取出来，写到文件句柄fd里
★        startDispatchCycleLocked(currentTime, connection);
    }
}
↓
↓
↓
void InputDispatcher::enqueueDispatchEntryLocked(
        const sp<Connection>& connection, EventEntry* eventEntry, const InputTarget* inputTarget,
        int32_t dispatchMode) {
    int32_t inputTargetFlags = inputTarget->flags;
    //它会根据 FLAG 来决定是否将事件放到队列里去
    if (!(inputTargetFlags & dispatchMode)) {
        return;
    }
    inputTargetFlags = (inputTargetFlags & ~InputTarget::FLAG_DISPATCH_MASK) | dispatchMode;

    // This is a new event.
    // Enqueue a new dispatch entry onto the outbound queue for this connection.
    DispatchEntry* dispatchEntry = new DispatchEntry(eventEntry, // increments ref
            inputTargetFlags, inputTarget->xOffset, inputTarget->yOffset,
            inputTarget->scaleFactor);

    // Apply target flags and update the connection's input state.
    switch (eventEntry->type) {
    //按键类
    case EventEntry::TYPE_KEY: {
        KeyEntry* keyEntry = static_cast<KeyEntry*>(eventEntry);
        dispatchEntry->resolvedAction = keyEntry->action;
        dispatchEntry->resolvedFlags = keyEntry->flags;

        if (!connection->inputState.trackKey(keyEntry,
                dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags)) {
#if DEBUG_DISPATCH_CYCLE
            ALOGD("channel '%s' ~ enqueueDispatchEntryLocked: skipping inconsistent key event",
                    connection->getInputChannelName());
#endif
            delete dispatchEntry;
            return; // skip the inconsistent event
        }
        break;
    }

    case EventEntry::TYPE_MOTION: {
        MotionEntry* motionEntry = static_cast<MotionEntry*>(eventEntry);
        if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_OUTSIDE) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_OUTSIDE;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_HOVER_EXIT) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_HOVER_EXIT;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_HOVER_ENTER) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_HOVER_ENTER;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_CANCEL;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_DOWN;
        } else {
            dispatchEntry->resolvedAction = motionEntry->action;
        }
        if (dispatchEntry->resolvedAction == AMOTION_EVENT_ACTION_HOVER_MOVE
                && !connection->inputState.isHovering(
                        motionEntry->deviceId, motionEntry->source, motionEntry->displayId)) {
#if DEBUG_DISPATCH_CYCLE
        ALOGD("channel '%s' ~ enqueueDispatchEntryLocked: filling in missing hover enter event",
                connection->getInputChannelName());
#endif
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_HOVER_ENTER;
        }

        dispatchEntry->resolvedFlags = motionEntry->flags;
        if (dispatchEntry->targetFlags & InputTarget::FLAG_WINDOW_IS_OBSCURED) {
            dispatchEntry->resolvedFlags |= AMOTION_EVENT_FLAG_WINDOW_IS_OBSCURED;
        }

        if (!connection->inputState.trackMotion(motionEntry,
                dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags)) {
#if DEBUG_DISPATCH_CYCLE
            ALOGD("channel '%s' ~ enqueueDispatchEntryLocked: skipping inconsistent motion event",
                    connection->getInputChannelName());
#endif
            delete dispatchEntry;
            return; // skip the inconsistent event
        }
        break;
    }
    }

    // Remember that we are waiting for this dispatch to complete.
    if (dispatchEntry->hasForegroundTarget()) {
        incrementPendingForegroundDispatchesLocked(eventEntry);
    }

    // Enqueue the dispatch entry.
    //从connection中得到一个队列，并将 dispatchEntry（某个输入事件） 放入队列尾部中去
★    connection->outboundQueue.enqueueAtTail(dispatchEntry);
    traceOutboundQueueLengthLocked(connection);
}
↓
↓
↓
void InputDispatcher::startDispatchCycleLocked(nsecs_t currentTime,
        const sp<Connection>& connection) {
#if DEBUG_DISPATCH_CYCLE
    ALOGD("channel '%s' ~ startDispatchCycle",
            connection->getInputChannelName());
#endif

    while (connection->status == Connection::STATUS_NORMAL
            && !connection->outboundQueue.isEmpty()) {
    //从队列 outboundQueue 头部取出输入事件 dispatchEntry
★        DispatchEntry* dispatchEntry = connection->outboundQueue.head;
        dispatchEntry->deliveryTime = currentTime;

        // Publish the event.
        status_t status;
        EventEntry* eventEntry = dispatchEntry->eventEntry;
        switch (eventEntry->type) {
        case EventEntry::TYPE_KEY: {
            KeyEntry* keyEntry = static_cast<KeyEntry*>(eventEntry);

            // Publish the key event，发送出去
★            status = connection->inputPublisher.publishKeyEvent(dispatchEntry->seq,
                    keyEntry->deviceId, keyEntry->source,
                    dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags,
                    keyEntry->keyCode, keyEntry->scanCode,
                    keyEntry->metaState, keyEntry->repeatCount, keyEntry->downTime,
                    keyEntry->eventTime);
            break;
        }

        case EventEntry::TYPE_MOTION: {
            MotionEntry* motionEntry = static_cast<MotionEntry*>(eventEntry);

            PointerCoords scaledCoords[MAX_POINTERS];
            const PointerCoords* usingCoords = motionEntry->pointerCoords;

            // Set the X and Y offset depending on the input source.
            float xOffset, yOffset, scaleFactor;
            if ((motionEntry->source & AINPUT_SOURCE_CLASS_POINTER)
                    && !(dispatchEntry->targetFlags & InputTarget::FLAG_ZERO_COORDS)) {
                scaleFactor = dispatchEntry->scaleFactor;
                xOffset = dispatchEntry->xOffset * scaleFactor;
                yOffset = dispatchEntry->yOffset * scaleFactor;
                if (scaleFactor != 1.0f) {
                    for (uint32_t i = 0; i < motionEntry->pointerCount; i++) {
                        scaledCoords[i] = motionEntry->pointerCoords[i];
                        scaledCoords[i].scale(scaleFactor);
                    }
                    usingCoords = scaledCoords;
                }
            } else {
                xOffset = 0.0f;
                yOffset = 0.0f;
                scaleFactor = 1.0f;

                // We don't want the dispatch target to know.
                if (dispatchEntry->targetFlags & InputTarget::FLAG_ZERO_COORDS) {
                    for (uint32_t i = 0; i < motionEntry->pointerCount; i++) {
                        scaledCoords[i].clear();
                    }
                    usingCoords = scaledCoords;
                }
            }

            // Publish the motion event.
            status = connection->inputPublisher.publishMotionEvent(dispatchEntry->seq,
                    motionEntry->deviceId, motionEntry->source,
                    dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags,
                    motionEntry->edgeFlags, motionEntry->metaState, motionEntry->buttonState,
                    xOffset, yOffset,
                    motionEntry->xPrecision, motionEntry->yPrecision,
                    motionEntry->downTime, motionEntry->eventTime,
                    motionEntry->pointerCount, motionEntry->pointerProperties,
                    usingCoords);
            break;
        }

        default:
            ALOG_ASSERT(false);
            return;
        }

        // Check the result.
        if (status) {
            if (status == WOULD_BLOCK) {
                if (connection->waitQueue.isEmpty()) {
                    ALOGE("channel '%s' ~ Could not publish event because the pipe is full. "
                            "This is unexpected because the wait queue is empty, so the pipe "
                            "should be empty and we shouldn't have any problems writing an "
                            "event to it, status=%d", connection->getInputChannelName(), status);
                    abortBrokenDispatchCycleLocked(currentTime, connection, true /*notify*/);
                } else {
                    // Pipe is full and we are waiting for the app to finish process some events
                    // before sending more events to it.
#if DEBUG_DISPATCH_CYCLE
                    ALOGD("channel '%s' ~ Could not publish event because the pipe is full, "
                            "waiting for the application to catch up",
                            connection->getInputChannelName());
#endif
                    connection->inputPublisherBlocked = true;
                }
            } else {
                ALOGE("channel '%s' ~ Could not publish event due to an unexpected error, "
                        "status=%d", connection->getInputChannelName(), status);
                abortBrokenDispatchCycleLocked(currentTime, connection, true /*notify*/);
            }
            return;
        }

        // Re-enqueue the event on the wait queue.
        connection->outboundQueue.dequeue(dispatchEntry);
        traceOutboundQueueLengthLocked(connection);
        connection->waitQueue.enqueueAtTail(dispatchEntry);
        traceWaitQueueLengthLocked(connection);
    }
}
↓
↓
↓
status_t InputPublisher::publishKeyEvent(
        uint32_t seq,
        int32_t deviceId,
        int32_t source,
        int32_t action,
        int32_t flags,
        int32_t keyCode,
        int32_t scanCode,
        int32_t metaState,
        int32_t repeatCount,
        nsecs_t downTime,
        nsecs_t eventTime) {
#if DEBUG_TRANSPORT_ACTIONS
    ALOGD("channel '%s' publisher ~ publishKeyEvent: seq=%u, deviceId=%d, source=0x%x, "
            "action=0x%x, flags=0x%x, keyCode=%d, scanCode=%d, metaState=0x%x, repeatCount=%d,"
            "downTime=%lld, eventTime=%lld",
            mChannel->getName().string(), seq,
            deviceId, source, action, flags, keyCode, scanCode, metaState, repeatCount,
            downTime, eventTime);
#endif

    if (!seq) {
        ALOGE("Attempted to publish a key event with sequence number 0.");
        return BAD_VALUE;
    }
//构造 msg
    InputMessage msg;
    msg.header.type = InputMessage::TYPE_KEY;
    msg.body.key.seq = seq;
    msg.body.key.deviceId = deviceId;
    msg.body.key.source = source;
    msg.body.key.action = action;
    msg.body.key.flags = flags;
    msg.body.key.keyCode = keyCode;
    msg.body.key.scanCode = scanCode;
    msg.body.key.metaState = metaState;
    msg.body.key.repeatCount = repeatCount;
    msg.body.key.downTime = downTime;
    msg.body.key.eventTime = eventTime;
    //发送
★    return mChannel->sendMessage(&msg);
}
↓
↓
↓
status_t InputChannel::sendMessage(const InputMessage* msg) {
    size_t msgLength = msg->size();
    ssize_t nWrite;
    do {
    //将 msg 写到文件句柄fd0里去，则app那边很快就可以通过fd1得到输入事件
★        nWrite = ::send(mFd, msg, msgLength, MSG_DONTWAIT | MSG_NOSIGNAL);
    } while (nWrite == -1 && errno == EINTR);

    if (nWrite < 0) {
        int error = errno;
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ error sending message of type %d, errno=%d", mName.string(),
                msg->header.type, error);
#endif
        if (error == EAGAIN || error == EWOULDBLOCK) {
            return WOULD_BLOCK;
        }
        if (error == EPIPE || error == ENOTCONN || error == ECONNREFUSED || error == ECONNRESET) {
            return DEAD_OBJECT;
        }
        return -error;
    }

    if (size_t(nWrite) != msgLength) {
#if DEBUG_CHANNEL_MESSAGES
        ALOGD("channel '%s' ~ error sending message type %d, send was incomplete",
                mName.string(), msg->header.type);
#endif
        return DEAD_OBJECT;
    }

#if DEBUG_CHANNEL_MESSAGES
    ALOGD("channel '%s' ~ sent message of type %d", mName.string(), msg->header.type);
#endif
    return OK;
}

```