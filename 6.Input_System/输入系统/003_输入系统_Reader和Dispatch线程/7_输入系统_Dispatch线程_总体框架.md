# 7_输入系统_Dispatch线程_总体框架

标签（空格分隔）： Dispatch

---

## `Dispatch`线程框架
* 发什么？
* 发给谁？



`Dispatch`线程和众多app直接各有一个联系`connection`，它是由`wm(WindowsManager`创建的。

### 获得事件：
* 放入队列前先稍加处理:
分类`global/system/user`按键；处理紧急事件（比如来电时，如果按音量键静音）；
* `InputReader`线程放入`mInboundQueue`队列

### 稍加处理
* 对于`global/system`按键
`Dispatch`线程从`InboundQueue`队列取出数据后，同样也要做处理，比如对于`global/system`按键，直接处理，放入`mCommandQueue`,不丢给app了
* 对于`user`按键
放入队列：查找目标app，得到`connection`,把这个输入事件放入到它的`outboundQueue`

### `Dispatch`发送给app
从`outboundQueue`中取出数据，通过`connection`发给app


![Dispatch处理流程](Dispatch%E5%A4%84%E7%90%86%E6%B5%81%E7%A8%8B.png)


