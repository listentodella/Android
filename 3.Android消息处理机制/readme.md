# Android消息处理机制
## 1.Android消息处理机制：Handler，MessageQueue，Looper，Thread
### 线程概念：一个应用程序运行时它的主体被称为进程，一个进程内部可以有多个线程，线程共享进程的资源
### 线程间通信：
* 0.创建MessageQueue：Looper.prepare()
* 1.使用Handler构造、发送消息
1.1 new Handler
1.2 Handler.sendMessage, sendEmptyMessageAtTime, sendMessageDelayed...
* 2.使用Looper循环处理消息
2.1 从MessageQueue中取出Message  
2.2 执行它的处理函数:msg.target.dispatchMessage


## ArrayMap
### get:根据k，得到v
* 1.根据 k 得到 Hash 值 H
* 2.在mHashes(这个数组从小到大排列）使用折半查找法，找到跟H值相等的所有项（因为Hash值有可能相同）：假设有H3 H4
* 3.再次比较k值，mArray[2*2] == k（✖） mArray[3*2] == k（✔）
* 4.返回index = 3
```
	index = indexofKey(k);
	return mArray[index*2+1]
```

###  put:把键值对放进去
index = indexofKey(k)
a:index >= 0:表示已经有(k,v'),只需要修改v'=>v
b:index < 0：index = ~index，表示要新添加(k,v)
1.有必要的话数组扩容
2.如果index是在数组的中间，移动数据
3.赋值
```
	mHashes[index] = H;
	mArray[index*2]=k;
	mArray[index*2+1]=v;
```