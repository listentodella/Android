# 12_输入系统_APP获得并处理输入事件流程

标签（空格分隔）： Input

---

## app对fd/InputChannel的注册过程：
* new `WindowInputEventReceiver` | 派生自`InputEventReceiver` --> 进入c++ `NativeInputEventReceiver` | 派生自`LooperCallback`
* `.onInputEvent` | `.dispatchInputEvent` ==== `.consumeEvents` | `.handleEvent`
 调用流程则是  <-- | <-- | <--
```
frameworks/base/core/java/android/view/ViewRootImpl.java

public void setView(View view, WindowManager.LayoutParams attrs, View panelParentView) {
        synchronized (this) {
... 
                if (mInputChannel != null) {
                    if (mInputQueueCallback != null) {
                        mInputQueue = new InputQueue();
                        mInputQueueCallback.onInputQueueCreated(mInputQueue);
                    }
                    //利用 mInputChannel 构造
★                    mInputEventReceiver = new WindowInputEventReceiver(mInputChannel,
                            Looper.myLooper());
                }
...
}
}
↓
↓
↓
final class WindowInputEventReceiver extends InputEventReceiver {
        public WindowInputEventReceiver(InputChannel inputChannel, Looper looper) {
            super(inputChannel, looper);
        }
//当收到输入事件后，该函数会被调用
        @Override
★        public void onInputEvent(InputEvent event) {
            enqueueInputEvent(event, this, 0, true);
        }

        @Override
        public void onBatchedInputEventPending() {
            if (mUnbufferedInputDispatch) {
                super.onBatchedInputEventPending();
            } else {
                scheduleConsumeBatchedInput();
            }
        }

        @Override
        public void dispose() {
            unscheduleConsumeBatchedInput();
            super.dispose();
        }
    }

```
来看一下父类`InputEventReceiver`，当创建`WindowInputEventReceiver`实例化对象时，父类的构造函数自然也会被调用哪个。
```
frameworks/base/core/java/android/view/InputEventReceiver.java
public abstract class InputEventReceiver {
 ....
    /**
     * Called when an input event is received.
     * The recipient should process the input event and then call {@link #finishInputEvent}
     * to indicate whether the event was handled.  No new input events will be received
     * until {@link #finishInputEvent} is called.
     *
     * @param event The input event that was received.
     */
★    public void onInputEvent(InputEvent event) {
        finishInputEvent(event, false);
    }

...

    // Called from native code.
    @SuppressWarnings("unused")
★    private void dispatchInputEvent(int seq, InputEvent event) {
        mSeqMap.put(event.getSequenceNumber(), seq);
        onInputEvent(event);
    }
...
}


    // Called from native code.
    @SuppressWarnings("unused")
    private void dispatchInputEvent(int seq, InputEvent event) {
        mSeqMap.put(event.getSequenceNumber(), seq);
//调用子类的函数
★        onInputEvent(event);
    }

```
再来看一下父类的构造函数，它调用了一个`nativeInit`
```
    public InputEventReceiver(InputChannel inputChannel, Looper looper) {
        if (inputChannel == null) {
            throw new IllegalArgumentException("inputChannel must not be null");
        }
        if (looper == null) {
            throw new IllegalArgumentException("looper must not be null");
        }

        mInputChannel = inputChannel;
        mMessageQueue = looper.getQueue();
        //调用c++
★        mReceiverPtr = nativeInit(new WeakReference<InputEventReceiver>(this),
                inputChannel, mMessageQueue);

        mCloseGuard.open("dispose");
    }
↓
↓
↓
../base/core/jni/android_view_InputEventReceiver.cpp    
static jlong nativeInit(JNIEnv* env, jclass clazz, jobject receiverWeak,
        jobject inputChannelObj, jobject messageQueueObj) {
    sp<InputChannel> inputChannel = android_view_InputChannel_getInputChannel(env,
            inputChannelObj);
    if (inputChannel == NULL) {
        jniThrowRuntimeException(env, "InputChannel is not initialized.");
        return 0;
    }

    sp<MessageQueue> messageQueue = android_os_MessageQueue_getMessageQueue(env, messageQueueObj);
    if (messageQueue == NULL) {
        jniThrowRuntimeException(env, "MessageQueue is not initialized.");
        return 0;
    }

★    sp<NativeInputEventReceiver> receiver = new NativeInputEventReceiver(env,
            receiverWeak, inputChannel, messageQueue);
    //这里会将 inputChannel 里含有的fd 注册给 Looper
★    status_t status = receiver->initialize();
    if (status) {
        String8 message;
        message.appendFormat("Failed to initialize input event receiver.  status=%d", status);
        jniThrowRuntimeException(env, message.string());
        return 0;
    }

    receiver->incStrong(gInputEventReceiverClassInfo.clazz); // retain a reference for the object
    return reinterpret_cast<jlong>(receiver.get());
}
↓
↓
↓
class NativeInputEventReceiver : public LooperCallback {
public:
    NativeInputEventReceiver(JNIEnv* env,
            jobject receiverWeak, const sp<InputChannel>& inputChannel,
            const sp<MessageQueue>& messageQueue);

    status_t initialize();
    void dispose();
    status_t finishInputEvent(uint32_t seq, bool handled);
    // 这个函数又会调用到java
★    status_t consumeEvents(JNIEnv* env, bool consumeBatches, nsecs_t frameTime,
            bool* outConsumedBatch);
...
};

status_t NativeInputEventReceiver::consumeEvents(JNIEnv* env,
        bool consumeBatches, nsecs_t frameTime, bool* outConsumedBatch) {
...
//调用java提供的 dispatchInputEvent 方法
★    env->CallVoidMethod(receiverObj.get(), gInputEventReceiverClassInfo.dispatchBatchedInputEventPending);
                    if (env->ExceptionCheck()) {
                        ALOGE("Exception dispatching batched input events.");
                        mBatchedInputEventPending = false; // try again later
                    }
...
}
```

## 获取和处理流程
app启动以后，一定会有一个循环，一直在等待、读取
```
/system/core.../libutils/Looper.cpp
Looper.cpp -> pollAll() --> pollOnce() --> pollInner() --> epoll_wait()

epoll_wait()会返回出各种fd
requestIndex = mRequests.indexOfKey(fd);//根据fd取出request
pushResponse(...) 将 request构造成一个reponse然后push到某个缓冲里
```
Invoke all repsonse callbacks，将从 `response`里取出`request`，再取出`callback`，然后调用其`handleEvent`,接着便会调用到上面提到的注册流程里的调用流程