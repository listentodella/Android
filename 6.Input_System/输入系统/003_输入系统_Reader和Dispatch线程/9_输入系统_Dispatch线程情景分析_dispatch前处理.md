# 9_输入系统_Dispatch线程情景分析_dispatch前处理

标签（空格分隔）： Dispatch Input

---

从`mInboundQueue`中取出数据并稍加处理

`getListener()->notifyKey(&args)`先得到监听者，然后调用`notifyKey()`，接着调用`mPolicy->interceptKeyBeforeQueueing(...)`稍加处理，无论处理结果如何，都会把结果放入`mInboundQueue`队列中（通过`enqueueInboundEventLocked(...)`?），最后便执行`mLooper->wake()`唤醒`Dispatch`线程。

* `Dispatch`线程
```
//frameworks/native/services/inputfinger/InputReader.cpp
bool InputDispatcherThread::threadLoop() {
    mDispatcher->dispatchOnce();
    return true;
}
↓
↓
↓
void InputDispatcher::dispatchOnce() {
    nsecs_t nextWakeupTime = LONG_LONG_MAX;
    { // acquire lock
        AutoMutex _l(mLock);
        mDispatcherIsAliveCondition.broadcast();

        // Run a dispatch loop if there are no pending commands.
        // The dispatch loop might enqueue commands to run afterwards.
        //如果没有cmd，生成cmd
★        if (!haveCommandsLocked()) {//判断 mCommandQueue 是否为空
            dispatchOnceInnerLocked(&nextWakeupTime);
        }

        // Run all pending commands if there are any.
        // If any commands were run then force the next poll to wake up immediately.
        //执行cmd
★        if (runCommandsLockedInterruptible()) {
            nextWakeupTime = LONG_LONG_MIN;
        }
    } // release lock

    // Wait for callback or timeout or wake.  (make sure we round up, not down)
    nsecs_t currentTime = now();
    int timeoutMillis = toMillisecondTimeoutDelay(currentTime, nextWakeupTime);
    //如果 nextWakeupTime == LONG_LONG_MIN，意味着会立即再次执行 dispatchOnce(...)
    mLooper->pollOnce(timeoutMillis);
}

```
命令队列为空时：
* 从 `mInbounfQueue`中取出时间
* 用它来生成命令，放入命令队列`mCommandQueue`，或者直接丢弃(对于`!PASS_TO_USER`的事件)
* 命令队列有数据时，执行命令（也有可能直接到这一步），对于GlobalKey，发广播；对于SystemKey，直接处理；UserKey，不处理
* 对于经过处理的事件，对于GlobalKey，丢弃；对于SystemKey，丢弃；UserKey，找到target，`dispatch`它

## 情景分析
### 不传给user —— `policyFlags = !PASS_TO_USER`
一开始命令队列为空，那么要执行`dispatchOnceInnerLocked(...)`：
```
//frameworks/native/services/inputfinger/InputReader.cpp
void InputDispatcher::dispatchOnceInnerLocked(nsecs_t* nextWakeupTime) {
    nsecs_t currentTime = now();

    // Reset the key repeat timer whenever normal dispatch is suspended while the
    // device is in a non-interactive state.  This is to ensure that we abort a key
    // repeat if the device is just coming out of sleep.
    if (!mDispatchEnabled) {
        resetKeyRepeatLocked();
    }

    // If dispatching is frozen, do not process timeouts or try to deliver any new events.
    if (mDispatchFrozen) {
#if DEBUG_FOCUS
        ALOGD("Dispatch frozen.  Waiting some more.");
#endif
        return;
    }

    // Optimize latency of app switches.
    // Essentially we start a short timeout when an app switch key (HOME / ENDCALL) has
    // been pressed.  When it expires, we preempt dispatch and drop all other pending events.
    bool isAppSwitchDue = mAppSwitchDueTime <= currentTime;
    if (mAppSwitchDueTime < *nextWakeupTime) {
        *nextWakeupTime = mAppSwitchDueTime;
    }

    // Ready to start a new event.
    // If we don't already have a pending event, go grab one.
    if (! mPendingEvent) {
        if (mInboundQueue.isEmpty()) {
            if (isAppSwitchDue) {
                // The inbound queue is empty so the app switch key we were waiting
                // for will never arrive.  Stop waiting for it.
                resetPendingAppSwitchLocked(false);
                isAppSwitchDue = false;
            }

            // Synthesize a key repeat if appropriate.
            if (mKeyRepeatState.lastKeyEntry) {
                if (currentTime >= mKeyRepeatState.nextRepeatTime) {
                    mPendingEvent = synthesizeKeyRepeatLocked(currentTime);
                } else {
                    if (mKeyRepeatState.nextRepeatTime < *nextWakeupTime) {
                        *nextWakeupTime = mKeyRepeatState.nextRepeatTime;
                    }
                }
            }

            // Nothing to do if there is no pending event.
            if (!mPendingEvent) {
                return;
            }
        } else {
            // Inbound queue has at least one entry.
★            mPendingEvent = mInboundQueue.dequeueAtHead();
            traceInboundQueueLengthLocked();
        }

        // Poke user activity for this event.
        if (mPendingEvent->policyFlags & POLICY_FLAG_PASS_TO_USER) {
            pokeUserActivityLocked(mPendingEvent);
        }

        // Get ready to dispatch the event.
        resetANRTimeoutsLocked();
    }

    // Now we have an event to dispatch.
    // All events are eventually dequeued and processed this way, even if we intend to drop them.
    ALOG_ASSERT(mPendingEvent != NULL);
    bool done = false;
    //是否丢弃，丢弃原因
★    DropReason dropReason = DROP_REASON_NOT_DROPPED;
    if (!(mPendingEvent->policyFlags & POLICY_FLAG_PASS_TO_USER)) {
        dropReason = DROP_REASON_POLICY;//因为策略而丢弃
    } else if (!mDispatchEnabled) {
        dropReason = DROP_REASON_DISABLED;
    }

    if (mNextUnblockedEvent == mPendingEvent) {
        mNextUnblockedEvent = NULL;
    }

    switch (mPendingEvent->type) {
    case EventEntry::TYPE_CONFIGURATION_CHANGED: {
        ConfigurationChangedEntry* typedEntry =
                static_cast<ConfigurationChangedEntry*>(mPendingEvent);
        done = dispatchConfigurationChangedLocked(currentTime, typedEntry);
        dropReason = DROP_REASON_NOT_DROPPED; // configuration changes are never dropped
        break;
    }

    case EventEntry::TYPE_DEVICE_RESET: {
        DeviceResetEntry* typedEntry =
                static_cast<DeviceResetEntry*>(mPendingEvent);
        done = dispatchDeviceResetLocked(currentTime, typedEntry);
        dropReason = DROP_REASON_NOT_DROPPED; // device resets are never dropped
        break;
    }
//假设是按键类型
★    case EventEntry::TYPE_KEY: {
        KeyEntry* typedEntry = static_cast<KeyEntry*>(mPendingEvent);
        if (isAppSwitchDue) {
            if (isAppSwitchKeyEventLocked(typedEntry)) {
                resetPendingAppSwitchLocked(true);
                isAppSwitchDue = false;
            } else if (dropReason == DROP_REASON_NOT_DROPPED) {
                dropReason = DROP_REASON_APP_SWITCH;
            }
        }
        if (dropReason == DROP_REASON_NOT_DROPPED
                && isStaleEventLocked(currentTime, typedEntry)) {
            dropReason = DROP_REASON_STALE;
        }
        if (dropReason == DROP_REASON_NOT_DROPPED && mNextUnblockedEvent) {
            dropReason = DROP_REASON_BLOCKED;
        }
        //如果 flag 表明不会 PASS_TO_USER，则 done == true
★        done = dispatchKeyLocked(currentTime, typedEntry, &dropReason, nextWakeupTime);
        break;
    }

    case EventEntry::TYPE_MOTION: {
        MotionEntry* typedEntry = static_cast<MotionEntry*>(mPendingEvent);
        if (dropReason == DROP_REASON_NOT_DROPPED && isAppSwitchDue) {
            dropReason = DROP_REASON_APP_SWITCH;
        }
        if (dropReason == DROP_REASON_NOT_DROPPED
                && isStaleEventLocked(currentTime, typedEntry)) {
            dropReason = DROP_REASON_STALE;
        }
        if (dropReason == DROP_REASON_NOT_DROPPED && mNextUnblockedEvent) {
            dropReason = DROP_REASON_BLOCKED;
        }
        done = dispatchMotionLocked(currentTime, typedEntry,
                &dropReason, nextWakeupTime);
        break;
    }

    default:
        ALOG_ASSERT(false);
        break;
    }

    if (done) {//如果 done 为 true
        if (dropReason != DROP_REASON_NOT_DROPPED) {
            dropInboundEventLocked(mPendingEvent, dropReason);
        }

        releasePendingEventLocked();
        //表明处理完一个按键之后会立即处理下一个按键
        //注意这个值会通过指针被返回出去，并在后续流程被使用，mLooper->pollOnce(...)，可知不会休眠，意味着会立即再次执行 dispatchOnce(...)
        *nextWakeupTime = LONG_LONG_MIN;  // force next poll to wake up immediately
    }
}





//如果是按键类型
bool InputDispatcher::dispatchKeyLocked(nsecs_t currentTime, KeyEntry* entry,
        DropReason* dropReason, nsecs_t* nextWakeupTime) {
    // Preprocessing.
    if (! entry->dispatchInProgress) {//一开始并不会是处理过程中，所以会走这个分支
        if (entry->repeatCount == 0
                && entry->action == AKEY_EVENT_ACTION_DOWN
                && (entry->policyFlags & POLICY_FLAG_TRUSTED)
                && (!(entry->policyFlags & POLICY_FLAG_DISABLE_KEY_REPEAT))) {
            if (mKeyRepeatState.lastKeyEntry
                    && mKeyRepeatState.lastKeyEntry->keyCode == entry->keyCode) {
                // We have seen two identical key downs in a row which indicates that the device
                // driver is automatically generating key repeats itself.  We take note of the
                // repeat here, but we disable our own next key repeat timer since it is clear that
                // we will not need to synthesize key repeats ourselves.
                entry->repeatCount = mKeyRepeatState.lastKeyEntry->repeatCount + 1;
//当长时间一直按住时，由Android系统处理按键的重复上报，而不是由底层上报
★                resetKeyRepeatLocked();
                mKeyRepeatState.nextRepeatTime = LONG_LONG_MAX; // don't generate repeats ourselves
            } else {
                // Not a repeat.  Save key down state in case we do see a repeat later.
                resetKeyRepeatLocked();
                mKeyRepeatState.nextRepeatTime = entry->eventTime + mConfig.keyRepeatTimeout;
            }
            mKeyRepeatState.lastKeyEntry = entry;
            entry->refCount += 1;
        } else if (! entry->syntheticRepeat) {
            resetKeyRepeatLocked();
        }

        if (entry->repeatCount == 1) {
            entry->flags |= AKEY_EVENT_FLAG_LONG_PRESS;
        } else {
            entry->flags &= ~AKEY_EVENT_FLAG_LONG_PRESS;
        }

        entry->dispatchInProgress = true;

        logOutboundKeyDetailsLocked("dispatchKey - ", entry);
    }

    // Handle case where the policy asked us to try again later last time.
    //一开始的状态是 Unkown，所以这里并不会先执行
    if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER) {
        if (currentTime < entry->interceptKeyWakeupTime) {
            if (entry->interceptKeyWakeupTime < *nextWakeupTime) {
                *nextWakeupTime = entry->interceptKeyWakeupTime;
            }
            return false; // wait until next wakeup
        }
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN;
        entry->interceptKeyWakeupTime = 0;
    }

    // Give the policy a chance to intercept the key.
    //一开始是Unknown
★    if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN) {
        if (entry->policyFlags & POLICY_FLAG_PASS_TO_USER) {
            CommandEntry* commandEntry = postCommandLocked(
                    & InputDispatcher::doInterceptKeyBeforeDispatchingLockedInterruptible);
            if (mFocusedWindowHandle != NULL) {
                commandEntry->inputWindowHandle = mFocusedWindowHandle;
            }
            commandEntry->keyEntry = entry;
            entry->refCount += 1;
            return false; // wait for the command to run
        } else {
            entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_CONTINUE;
        }
    } else if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_SKIP) {
        if (*dropReason == DROP_REASON_NOT_DROPPED) {
            *dropReason = DROP_REASON_POLICY;
        }
    }

    // Clean up if dropping the event.
    //如果是 !PASS_TO_USER，最后会在这里返回true
★    if (*dropReason != DROP_REASON_NOT_DROPPED) {
        setInjectionResultLocked(entry, *dropReason == DROP_REASON_POLICY
                ? INPUT_EVENT_INJECTION_SUCCEEDED : INPUT_EVENT_INJECTION_FAILED);
        return true;
    }

    // Identify targets.
    Vector<InputTarget> inputTargets;
    int32_t injectionResult = findFocusedWindowTargetsLocked(currentTime,
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
    dispatchEventLocked(currentTime, entry, inputTargets);
    return true;
}

```
### `policyFlags == PASS_TO_USER`
`Global Key`:

System Key
user Key
只要flag 是 `PASS_TO_USER`
```
// Give the policy a chance to intercept the key.
if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN) {
        CommandEntry* commandEntry = postCommandLocked(
        //这个就是下面要执行的cmd
 ★               & InputDispatcher::doInterceptKeyBeforeDispatchingLockedInterruptible);
        if (mFocusedWindowHandle != NULL) {
            commandEntry->inputWindowHandle = mFocusedWindowHandle;
        }
        commandEntry->keyEntry = entry;
        entry->refCount += 1;
        return false; // wait for the command to run
    }
...
}
```
`false`导致不会像上面那样设置一个不休眠的流程，而且命令队列`mCommandQueue`这时也不为空了:

```
bool InputDispatcher::runCommandsLockedInterruptible() {
    if (mCommandQueue.isEmpty()) {//此时不为空
        return false;
    }

    do {
        CommandEntry* commandEntry = mCommandQueue.dequeueAtHead();

        Command command = commandEntry->command;
//其实是 doInterceptKeyBeforeDispatchingLockedInterruptible(...)
   ★     (this->*command)(commandEntry); // commands are implicitly 'LockedInterruptible'

        commandEntry->connection.clear();
        delete commandEntry;
    } while (! mCommandQueue.isEmpty());
    return true;
}


void InputDispatcher::doInterceptKeyBeforeDispatchingLockedInterruptible(
        CommandEntry* commandEntry) {
    KeyEntry* entry = commandEntry->keyEntry;

    KeyEvent event;
    initializeKeyEvent(&event, entry);

    mLock.unlock();
//调用 phoneWindowManager.java 同名函数
    nsecs_t delay = mPolicy->interceptKeyBeforeDispatching(commandEntry->inputWindowHandle,
            &event, entry->policyFlags);

    mLock.lock();
    
    //根据返回结果，设置解析结果，下同
    if (delay < 0) {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_SKIP;
    } else if (!delay) {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_CONTINUE;
    } else {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER;
        entry->interceptKeyWakeupTime = now() + delay;
    }
    entry->release();
}

```
对于`Global Key`，java调用：
如果是`GlobalKey`，那么返回-1，则如上流程会设置为`INTERCEPT_KEY_RESULT_SKIP`，自然就不会上传给用户（app）了
```
public long interceptKeyBeforeDispatching(...){
...
    if(mGlobalKeyManager.handleGlobalKey(mContext, keyCode, event)){
        return -1;
    }
...
}
根据 Global_Key.xml 发送广播给某个组件
```
* 对于`System Key`：
直接处理，return -1，也意味着会设置为`INTERCEPT_KEY_RESULT_SKIP`，不会上传给app
* 对于`User Key`:
return 0，设置为`INTERCEPT_KEY_RESULT_CONTINUE`，再次执行`dispatchOnceInnerLocked(...)`，最终找到目标app，让app来处理
```
 // Identify targets.
    Vector<InputTarget> inputTargets;
    //找到app
★    int32_t injectionResult = findFocusedWindowTargetsLocked(currentTime,
            entry, inputTargets, nextWakeupTime);
    if (injectionResult == INPUT_EVENT_INJECTION_PENDING) {
        return false;
    }
...

// Dispatch the key.
//发给app
★    dispatchEventLocked(currentTime, entry, inputTargets);

```

![Dispatch处理流程_修订版](Dispatch%E5%A4%84%E7%90%86%E6%B5%81%E7%A8%8B_%E4%BF%AE%E8%AE%A2%E7%89%88.jpg)

