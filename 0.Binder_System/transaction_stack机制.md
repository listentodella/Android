# transaction_stack
client与server之间的通讯只有BC_TRANSACTION,BR_TRANSACTION, BC_REPLY,BR_REPLY
但是binder_proc和binder_thread都有todo链表

## 发给谁？
handle只表明了进程，那么传输过程中是发给了进程还是线程？
$ 一般放在binder_proc.todo,唤醒等待于binder_proc.wait的空闲进程
$ 对于双向传输(用transaction_stack来判断)，则放在binder_thread.todo里，唤醒该进程
* ①BC_TRANSACTION----test_client：TR通过from_parent入栈
```
会调用到binder_transaction()
a.一开始非“双向传输”，所以数据将放在test_server的binder_proc.todo
b.入栈：test_client.binder_thread.transaction_stack-->from_parent
.from = test_client
.to_proc = test_server
.to_thread = test_server
c.数据放入test_server.binder_proc.todo,唤醒test_server.binder_proc.wait上的线程
```
* ② 收到BR_TRANSACTION----test_server: TR通过to_parent入栈
```
binder_thread_read()
a.从test_server.binder_proc.todo链表中取出数据,处理,返回给用户空间
b.入栈：test_server.binder_thread.transaction_stack-->to_parent
.from = test_client
.to_proc = test_server
.to_thread = test_server
★同一个binder_transaction，通过.from_parent放入发送者的栈，通过.to_parent放入接收者的栈
```
## 回给谁？
没有handle表明目的进程，必定在某个地方记录“发送者”——这个地方就是transaction_stack

* ③发送BC_REPLY----test_server: TR通过to_parent出栈,TR通过from_parent出栈
```
binder_transction()
a.回给谁？从栈中取出一个结构体（transaction_stack）可知要回复给谁
.from = test_client
.to_proc = test_server
.to_thread = test_server
b.test_server出栈：test_server.binder_thread.transaction_stack = NULL
c.数据copy_from_user=>test_client
d.test_client出栈：test_client.binder_thread.transaction_stack = NULL
e.放入todo,唤醒
```

* ④收到BR_REPLY----test_client：不涉及栈
binder_thread_read(), 返回给用户空间，不涉及栈

## 双向传输
现p1提供了s1服务，p2提供了s2服务，p3提供了s3服务
p1有t1,t1',t1''...线程，p2有t2,t2',t2''...线程，p3有t3,t3',t3''...线程

假设 t1现在要使用s2服务，会向t2线程发送数据，t2线程的处理过程中，t2要用到t3，在t3处理过程中，要反过来调用t1。
即t1-->t2-->t3-->发给谁？
问：t3把数据放到p1的binder_proc.todo，让p1使用新线程t1'来处理，还是把数据放到t1.binder_thread.todo，让t1来处理？

双向传输：
p1的t1---》发出服务请求
p1《----处理过程中（用到s1服务）
所以用t1来处理s1服务

### 情景分析
t1发送TR1--->t2，发送TR2--->t3，发送TR3---> ???
* BC_TRANSACTION
t1.sp->TR1.from_parent->NULL
* BR_TRANSACTION
t2.sp->TR1.to_parent->NULL
* BC_TRANSACTION
t2.sp->TR2.from_parent->TR1.to_parent->NULL
* BR_TRANSACTION
t3.sp->TR2.to_parent->NULL
* BC_TRANSACTION,发给谁？t1,因为t3要用到p1的服务，p1下t1在休眠等待
t3.sp->TR3.from_parent->TR2.to_parent->NULL
```
	t3	TR2
tmp = thread->transaction_stack;

while (tmp) {
	if (tmp->from && tmp->from->proc == target_proc)
		target_thread = tmp->from;
	tmp = tmp->from_parent;
}
```
* 收到t3的BR_TRANSACTION
t1.sp->TR3.to_parent->TR1.from_parent->NULL
* 发出BC_REPLY
出栈：t1.sp->TR1.from_parent->NULL
出栈：t3.sp->TR2.to_parent->NULL
* 收到BR_REPLY
处理TR2，发出BC_REPLY：t3.sp->TR2.to_parent->NULL
从t3.sp中取出TR2，TR2.from=t2,所以回复t2
出栈：t3.sp->NULL  t2.sp->TR1.to_parent
* 收到BR_REPLY
处理TR1，发出BC_REPLY，从t2.sp中取出栈顶，TR1.from=t1,所以回复t1
出栈：t2.sp=NULL  t1.sp=NULL
* 收到BR_REPLY
处理完毕











