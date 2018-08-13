# 11_输入系统_APP跟输入系统建立联系_InputChannel和Connection

标签（空格分隔）： Input

---

App与Input系统如何建立联系？核心就是`socketpair`.

#### `WindowManagerService`与`InputDispatcher`的任务:
对于每一个app1，2...，`WindowManagerSerivce`都有一个相对应的`WindowState`来表示它们。假设现在新建一个app3,那么它需要调用`addToDisplay`通过`Binder`调用`WindowManagerService`的`addWindow`,它完成以下任务：
* 创建一个结构体 `WindowState`
* 创建 `socketpair`得到fd0和fd1（fd1会返回给app3），fd0则会被封装为一个`InputChannel`。
这个`InputChannel`会放入`WindowState`，同时也会`registerInputChannel`到`InputDispatcher`。
```
./frameworks/.../InputDispatcher.h
这里的复数说明了，connection可能不止一个

class InputState {
...
// All registered connections mapped by channel file descriptor.
    KeyedVector<int, sp<Connection> > mConnectionsByFd;
...
}
```
`registerInputChannel`的任务：
* 创建`Connection`，它含有`.InputChnannel`，它又含有`.fd`
* 然后将这个`Connection`放到`KeyedVector`

这样，当`InputDispatcher`得到数据后，可以从`KeyedVector`找出某一个`Connection`,然后把输入事件放到`fd0`里，接着app就可以从`fd1`里得到数据了。

#### app的任务
app3得到`fd1`后，将其封装为一个`InputChannel`，然后再封装为`WindowInputEventReceiver`，最终把`fd1`放入到`Looper`，使用`epoll`等待数据。

#### app获得数据

```seq
InputDispatcher->WindowManagerService:
WindowManagerService->app:
Note left of InputDispatcher:Connection
Note left of WindowManagerService:WindowState
Note left of InputDispatcher:fd0
Note left of WindowManagerService:.fd0 socketpair .fd1
Note right of app:fd1
```
![app获取数据流程](app%E8%8E%B7%E5%8F%96%E6%95%B0%E6%8D%AE%E6%B5%81%E7%A8%8B.jpg)


----------
需要注意的是，`InputDispathcer`、`WMS`和`InputReader`线程，都处于另一个进程里————`SystemServer`进程中。因此这三个线程间的通讯是不需要通过`Binder`的；但是app想跟它们通讯时，就需要通过`Binder`或者事先建立好的`sockerpair`。

### 源码分析
app入口：
`ActivityThread.java`->`handleResumeActivity(...)`
`IWindowManager.java`、`IWindowSession.java`（编译时在out目录下自动生成。。。,与`Binder`相关）

`android_view_InputChannel.cpp`

#### app获得socketpair的fd1过程
`fd0`会通过`registerInputChannel`告诉`Input`系统，而`fd1`则会通过`Binder`告诉app。
`IWindowSession.java`下的`mWindowSession.addToDisplay(...)`

```
frameworks/base/core/java/android/view/ViewRootImpl.java
public void setView(View view, WindowManager.LayoutParams attrs, View panelParentView) {
...
★    res = mWindowSession.addToDisplay(mWindow, mSeq, mWindowAttributes,
                                getHostVisibility(), mDisplay.getDisplayId(),
                                mAttachInfo.mContentInsets, mInputChannel);
...
}
↓
↓ IWindowSession.java // 在out目录下生成
↓out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/src/core/java/android/view/IWindowSession.java
↓
@Override public int addToDisplay(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outContentInsets, android.view.InputChannel outInputChannel) throws android.os.RemoteException
{
    android.os.Parcel _data = android.os.Parcel.obtain();
    android.os.Parcel _reply = android.os.Parcel.obtain();
    int _result;
    try {
        _data.writeInterfaceToken(DESCRIPTOR);
        _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
        _data.writeInt(seq);
        if ((attrs!=null)) {
            _data.writeInt(1);
            attrs.writeToParcel(_data, 0);
        }
        else {
            _data.writeInt(0);
        }
        _data.writeInt(viewVisibility);
        _data.writeInt(layerStackId);
//导致binder调用 WindowManagerService线程(IWindowSession.java)本地的 onTransact() 的addToDisplay
★        mRemote.transact(Stub.TRANSACTION_addToDisplay, _data, _reply, 0);
        _reply.readException();
        _result = _reply.readInt();
        if ((0!=_reply.readInt())) {
            outContentInsets.readFromParcel(_reply);
        }
        if ((0!=_reply.readInt())) {
★            outInputChannel.readFromParcel(_reply);
        }
    }
    finally {
        _reply.recycle();
        _data.recycle();
    }
    return _result;
}
↓
onTransact(){
...
case TRANSACTION_addToDisplay:
{
    data.enforceInterface(DESCRIPTOR);
    android.view.IWindow _arg0;
    _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
    int _arg1;
    _arg1 = data.readInt();
    android.view.WindowManager.LayoutParams _arg2;
    if ((0!=data.readInt())) {
        _arg2 = android.view.WindowManager.LayoutParams.CREATOR.createFromParcel(data);
    }
    else {
        _arg2 = null;
    }
    int _arg3;
    _arg3 = data.readInt();
    int _arg4;
    _arg4 = data.readInt();
    android.graphics.Rect _arg5;
    _arg5 = new android.graphics.Rect();
    android.view.InputChannel _arg6;
    _arg6 = new android.view.InputChannel();
    //最终会调用到addWindow(Session.java提供
    ) / WindowManagerService.java
★    int _result = this.addToDisplay(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
    reply.writeNoException();
    reply.writeInt(_result);
    if ((_arg5!=null)) {
        reply.writeInt(1);
        _arg5.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
    }
    else {
        reply.writeInt(0);
    }
    if ((_arg6!=null)) {
        reply.writeInt(1);

★        _arg6.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
    }
    else {
        reply.writeInt(0);
    }
    return true;
}
....
}



```