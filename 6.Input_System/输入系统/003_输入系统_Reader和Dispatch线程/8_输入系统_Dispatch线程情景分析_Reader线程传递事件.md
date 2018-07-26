# 8_输入系统_Dispatch线程情景分析_Reader线程传递事件

标签（空格分隔）： Dispatch Reader

---

* global key:
按下按键，启动某个app
可以自己指定，修改`frameworks/base/core/res/res/Global_keys.xml`
假设它是`AKEYCODE_TV`
* system key:
比如音量键`AKEYCODE_VOLUME_DOWN`
* user key：
其他按键，比如ABCD...`AKEYCODE_A`

`Reader`线程把驱动上报的`scancode`根据`.kl`文件转换成`keycode`

## `Dispatch`情景分析
### 三种按键的处理过程
`Reader`线程从驱动收到扫描码`scancode`之后，根据`.kl`文件转换得到按键码`keycode`，然后构造出一个`args`参数，最后`notifyKey(&args)`告诉它的`listener`
```
//frameworks/native/services/inputflinger/InputReader.cpp
{
...
    NotifyKeyArgs args(when, getDeviceId(), mSource, policyFlags,
            down ? AKEY_EVENT_ACTION_DOWN : AKEY_EVENT_ACTION_UP,
            AKEY_EVENT_FLAG_FROM_SYSTEM, keyCode, scanCode, newMetaState, downTime);
    //getListener()得到的是一个 InputDispatcher 的实例化对象
 ★   getListener()->notifyKey(&args);
...
}
↓
↓
↓
//frameworks/native/services/inputflinger/InputDispatcher.cpp
void InputDispatcher::notifyKey(const NotifyKeyArgs* args) {
...

//使用传进来的参数构造 keyEvent
★    KeyEvent event;
    event.initialize(args->deviceId, args->source, args->action,
            flags, keyCode, args->scanCode, metaState, 0,
            args->downTime, args->eventTime);
//放入队列之前稍加处理
//值得注意的是，第二个参数正如其注释，是一个 引用 类型，因此后续会被用到
//实际上是在下面构造 newEntry 的时候传给它了
★    mPolicy->interceptKeyBeforeQueueing(&event, /*byref*/ policyFlags);

    bool needWake;
    { // acquire lock
        mLock.lock();

        if (shouldSendKeyToInputFilterLocked(args)) {
            mLock.unlock();

            policyFlags |= POLICY_FLAG_FILTERED;
            if (!mPolicy->filterInputEvent(&event, policyFlags)) {
                return; // event was consumed by the filter
            }

            mLock.lock();
        }

        int32_t repeatCount = 0;
//注意这里的 policyFlags 是一个引用
        KeyEntry* newEntry = new KeyEntry(args->eventTime,
                args->deviceId, args->source, policyFlags,
                args->action, flags, keyCode, args->scanCode,
                metaState, repeatCount, args->downTime);
//放入 mInboundQueue 队列中去
★        needWake = enqueueInboundEventLocked(newEntry);
        mLock.unlock();
    } // release lock

    if (needWake) {
        mLooper->wake();//如果有必要就唤醒 Dispatch 线程
    }
}
↓
↓
↓
//frameworks/base/services/core/jni/com_android_server_input_InputManagerService.cpp
void NativeInputManager::interceptKeyBeforeQueueing(const KeyEvent* keyEvent,
        uint32_t& policyFlags) {
    // Policy:
    // - Ignore untrusted events and pass them along.
    // - Ask the window manager what to do with normal events and trusted injected events.
    // - For normal events wake and brighten the screen if currently off or dim.
    if (mInteractive) {
        policyFlags |= POLICY_FLAG_INTERACTIVE;
    }
    if ((policyFlags & POLICY_FLAG_TRUSTED)) {
        nsecs_t when = keyEvent->getEventTime();
        JNIEnv* env = jniEnv();
        jobject keyEventObj = android_view_KeyEvent_fromNative(env, keyEvent);
        jint wmActions;
        if (keyEventObj) {
        //这里会调用感到 java 里的同名函数( phoneWindowManager.java的同名函数)
        //根据返回值设置 policyFlags，这里决定了是否要(PASS_TO_USER)
★        wmActions = env->CallIntMethod(mServiceObj,
                    gServiceClassInfo.interceptKeyBeforeQueueing,
                    keyEventObj, policyFlags);
            if (checkAndClearExceptionFromCallback(env, "interceptKeyBeforeQueueing")) {
                wmActions = 0;
            }
            android_view_KeyEvent_recycle(env, keyEventObj);
            env->DeleteLocalRef(keyEventObj);
        } else {
            ALOGE("Failed to obtain key event object for interceptKeyBeforeQueueing.");
            wmActions = 0;
        }
    //根据返回结果设置这个 flags
★        handleInterceptActions(wmActions, when, /*byref*/ policyFlags);
    } else {
        if (mInteractive) {
            policyFlags |= POLICY_FLAG_PASS_TO_USER;
        }
    }
}
↓
↓
↓
void NativeInputManager::handleInterceptActions(jint wmActions, nsecs_t when,
        uint32_t& policyFlags) {
// 是否需要返回给 User
    if (wmActions & WM_ACTION_PASS_TO_USER) {
        policyFlags |= POLICY_FLAG_PASS_TO_USER;
    } else {
#if DEBUG_INPUT_DISPATCHER_POLICY
        ALOGD("handleInterceptActions: Not passing key to user.");
#endif
    }
}



/frameworks/base/policy/src/com/android/internal/policy/impl/PhoneWindowManager.java
public int interceptKeyBeforeQueueing(KeyEvent event, int policyFlags) {
        if (!mSystemBooted) {
            // If we have not yet booted, don't let key events do anything.
            return 0;
        }
        //这个变量表明 Android系统与用户是否处于交互状态
        //一般灭屏后就可认为不是交互状态了
   ★     final boolean interactive = (policyFlags & FLAG_INTERACTIVE) != 0;
        final boolean down = event.getAction() == KeyEvent.ACTION_DOWN;
        final boolean canceled = event.isCanceled();
        final int keyCode = event.getKeyCode();
...
// 需要设置为 PASS_TO_USER
        if (interactive || (isInjected && !isWakeKey)) {
            // When the device is interactive or the key is injected pass the
            // key to the application.
            result = ACTION_PASS_TO_USER;
            isWakeKey = false;
        } else if (!interactive && shouldDispatchInputWhenNonInteractive()) {
// If we're currently dozing with the screen on and the keyguard showing, pass the key
// to the application but preserve its wake key status to make sure we still move
// from dozing to fully interactive if we would normally go from off to fully
// interactive.
            result = ACTION_PASS_TO_USER;
        } else {
...
        }

        // If the key would be handled globally, just return the result, don't worry about special key processing.
        // 对于 global key 直接返回 result：ACTION_PASS_TO_USER
 ★       if (mGlobalKeyManager.shouldHandleGlobalKey(keyCode, event)) {
            if (isWakeKey) {
                mPowerManager.wakeUp(event.getEventTime());
            }
            return result;
        }
...
}


//如果这个 keyCode 在 mKeyMapping 内，那么它就是 global key，反之不是
//这个 mKeyMapping 会在 private void loadGlobalKeys(Context context) 函数里，通过xml解析后设置
boolean shouldHandleGlobalKey(int keyCode, KeyEvent event) {
        return mKeyMapping.get(keyCode) != null;
    }

```
Tips:流程分析入口
```
//frameworks/base/services/core/jni/com_android_server_input_InputManagerService.cpp
static jlong nativeInit(JNIEnv* env, jclass clazz,
        jobject serviceObj, jobject contextObj, jobject messageQueueObj) {
...

★    NativeInputManager* im = new NativeInputManager(contextObj, serviceObj,
            messageQueue->getLooper());
...
}
↓
↓
↓
NativeInputManager::NativeInputManager(jobject contextObj,
        jobject serviceObj, const sp<Looper>& looper) :
        mLooper(looper), mInteractive(true) {
...

 ★   sp<EventHub> eventHub = new EventHub();
 ★   mInputManager = new InputManager(eventHub, this, this);
}
↓
↓
↓
//frameworks/native/services/inputflinger/InputManager.cpp
InputManager::InputManager(
        const sp<EventHubInterface>& eventHub,
        const sp<InputReaderPolicyInterface>& readerPolicy,
        const sp<InputDispatcherPolicyInterface>& dispatcherPolicy) {
    //这就是为什么 getListener()得到的是一个 InputDispatcher 的实例化对象    
    mDispatcher = new InputDispatcher(dispatcherPolicy);
    //注意其第三个参数，是一个 listener
★    mReader = new InputReader(eventHub, readerPolicy, mDispatcher);
    initialize();
}
↓
//frameworks/native/services/inputflinger/InputReader.cpp
InputReader::InputReader(const sp<EventHubInterface>& eventHub,
        const sp<InputReaderPolicyInterface>& policy,
        const sp<InputListenerInterface>& listener);
```
因此最开始，三种按键都是通过调用它的`getListener()`获得`inputDispatcher`实例化对象，再调用`notifyKey()`，此时还属于`InputReader`线程范围内。


#### global key:
比如`AKEYCODE_TV`,调用 `PhoneWindowManager.java`，对于global key按键，返回 `PASS_TO_USER`
#### system key:
比如音量键`AKEYCODE_VOLUME_DOWN`,,调用 `PhoneWindowManager.java`，对于系统按键，分类处理：
* 可以直接处理的紧急事件，就处理，设置返回值 `! PASS_TO_USER`
* 否则还是要 `PASS_TO_USER `
#### user key:
比如`AKEYCODE_A`,调用 `PhoneWindowManager.java`，对于普通按键，返回 `PASS_TO_USER`