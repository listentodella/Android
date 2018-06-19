# 1_Reader_Dispatch线程启动分析

标签（空格分隔）： Input Reader Dispatch

---
### 几个重要的类与对象
`InputReaderThread`和`InputDispatchThread`，
各自的实例化对象`mReaderThread`和`mDispatchThread`，它们都只是创建线程、实现循环操作；
在循环操作中，分别执行`InputReader`的实例化对象`mReader`与`InputDispatcher`的实例化对象`mDispatcher`的成员函数。
另外一个重要的`EventHub`的`mEventHub`，它处理着多种输入设备。

![几个类之间的关系](%E5%87%A0%E4%B8%AA%E7%B1%BB%E4%B9%8B%E9%97%B4%E7%9A%84%E5%85%B3%E7%B3%BB.jpg)


对于上层调用者来说，不希望看到上面的复杂结构，通过`InputManager mInputManager`。
以上都是`cpp`实现，对于`java`而言同样也不想看到这些结构，通过`NativeInputManager im`。

所有的源头都从`SystemServer.java`开始.
