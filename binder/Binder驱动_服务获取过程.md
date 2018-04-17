# Binder驱动——服务获取过程
## 注册过程
	用户态：
	1️⃣ 构造数据：name = "hello" 、flat_binder_node

|type|flags|binder|cookie|
|---|---|---|---|
| | |进程相关||
	2️⃣ 用ioctl发送

test_server ----------------------------------

	内核态：
	3️⃣把数据放到service_manager的todo链表
	4️⃣构造结构体：binder_node（给源进程创建）、binder_ref（给目的进程创建）


.binder_proc结构体

	5️⃣唤醒service_manager




	用户态：
	6️⃣调用ADD_SERVICE函数
	
	 ⑦在svclist创建一项

service_manger -----------------------------------






## 获取过程
* test_client
** 用户态：
	1.构造数据：name = "hello 
	2.ioctl发送数据


** 内核态：
	3.根据汉德勒=0，找到service_manager，把数据放入它的todo链表
	4.唤醒它


* service_manager
** 用户态：
	6.取出数据，得到hello
	7.在svclist链表里根据“hello”找到一项，得知handle为1
	8.yong ioctl把handle发给驱动

** 内核态
	返回  数据
	在refs_by_desc树中，根据handle为1找到binder_ref，进而找到hello服务的binder_node
	为test_client创建binder_ref，把handle=1放入test_client的todo链表
	唤醒test_client
