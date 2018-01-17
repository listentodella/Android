# Android消息处理机制
## 1.Android消息处理机制：Handler，MessageQueue，Looper，Thread
### 线程概念：一个应用程序运行时它的主体被称为进程，一个进程内部可以有多个线程，线程共享进程的资源
### 线程间通信：
0.创建MessageQueue：Looper.prepare()
1.使用Handler构造、发送消息
1.1 new Handler
1.2 Handler.sendMessage, sendEmptyMessageAtTime, sendMessageDelayed...
2.使用Looper循环处理消息
2.1 从MessageQueue中取出
2.2 执行它的处理函数:msg.target.dispatchMessage