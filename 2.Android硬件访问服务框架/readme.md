# Android硬件访问服务框架
## 通过JNI直接访问
* 如果我们开发的硬件比较独特，只有我们自己写的应用程序来访问它，可以使用这种方法来实现Android app访问硬件。
Android app 通过loadLibrary加载C库，C库具有图示的几个功能：

![Android驱动开发_硬件访问服务](Android%E9%A9%B1%E5%8A%A8%E5%BC%80%E5%8F%91_%E7%A1%AC%E4%BB%B6%E8%AE%BF%E9%97%AE%E6%9C%8D%E5%8A%A1.png)

但是当有多个app访问同一个硬件时就会造成冲突，比如屏幕、声卡。
## SystemServer
```
Server:服务器，提供服务。Server提供Service——Server里有很多Service
这些app想要访问硬件时，将请求发给SystemServer，由它统一管理。
```
硬件访问服务
1. loadLibrary 加载C库
2. JNI_Onload 在C库的JNI_Onload函数里注册本地方法
	分别调用各个硬件的函数来注册本地方法
3. SystemServer对每个硬件:
	使用本地方法构造Service
	addService
4. app怎么使用
	获得服务：getService
	使用服务：执行Service方法


![硬件访问服务框架](%E7%A1%AC%E4%BB%B6%E8%AE%BF%E9%97%AE%E6%9C%8D%E5%8A%A1%E6%A1%86%E6%9E%B6.png)


```
Onload.cpp:
里面有很多诸如
register_android_server_xxxService(env);

实际上这些注册服务的函数都各自对应一个文件，用于注册本地方法
com_android_server_xxxService.cpp
这些文件可以成为JNI文件。

```
HAL：如果硬件操作很简单，可以直接在JNI文件里实现；但是当硬件毕竟复杂时，建议放在HAL。当本地接口改动了，只要修改hal层的文件就行了，然后更新so文件，JNI那边不需要改动，编译、烧写也会很方便；如果通过JNI直接访问，这种一旦有改动，就需要编译整个系统。此外不是所有厂商都愿意公开硬件操作，他们只提供so文件。
* 容易修改
* 保密

## 怎么实现硬件访问服务
1. JNI和HAL
	com_android_server_LedService.cpp注册JNI本地方法
	hal_led.c:open read write，将会被cpp加载
2. 修改onload.cpp
	调用上面的cpp里实现的函数
3.修改SystemServer.java：
	new LedService —— 构造服务
	.addService —— 添加服务
4. LedService.java
	调用本地方法
5.ILedService.java
	给app使用
```
仿照其他aidl文件，编写ILedService.aidl文件，然后修改makefile，执行mmm frameworks/base 命令就可以生成java文件了。

1️⃣ILedService.aidl==》ILedService.java
app使用：
	ILedService iLedService;
	iLedService = ILedService.Stub.asInterface(ServiceManager.getService("led"));
						
	iLedService.ledCtrl(0, 1);

2️⃣LedService.java
修改SystemServer.java，在里面仿照添加addService("led", new LedService());

3️⃣com_android_server_ledService.cpp
==>用于注册本地方法，供ledService.java使用

修改onload.cpp，添加register_android_server_LedService(env);

4️⃣编写app(以下最好谷歌）
包含什么?
拷贝出 out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/classes.jar

怎么包含?

编译错误?

```



