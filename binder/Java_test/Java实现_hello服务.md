# Java实现_hello服务
* 定义接口：aidl文件————相对于Cpp的.h文件
sayhello
sayhello_to
编译生成IHelloService.java
该文件声明有接口，定义了Stub类，定义了Proxy类

* server端：
定义：HelloService类，继承于Stub
实现sayhello、sayhello_to
```
TestServer.java
main()
{
addService;
while(1){
读取、解析、调用执行、返回 ===== 整个循环都不用自己写 
}
```
* client端
代理类已有，直接使用即可
```
TestClient.java
main()
{
getService;
调用sayhello、sayhello_to
}
```

******
## 定义接口
* 写IHelloServic.aidl文件，上传、编译，得到IHElloService.java
	里面有Stub：onTransact，它会分辨收到的数据然后调用sayhello、sayhello_to
	Proxy：提供有sayhello、sayhello_to两个函数，它们会构造数据然后发送给server
1. 把IHelloService.aidl放入frameworks/base/core/java/android/os
2. 修改frameworks/base/Android.mk 添加一行
```
core/java/android/os/IVibratorService.aidl\
core/java/android/os/IHelloService.aidl\
```
3. mmm frameworks/base
4. 它会自动生成IHelloService.java文件
```
out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/src/core/java/android/os/IHelloService.java
```
* 实现服务类：HelloService.java
	在里面实现sayhello、sayhello_to（因为生成的Stub里并没有实现）
* 实现app：TestServer、TestClient
TestServer：addService，循环
TestClient：getService，调用sayhello、sayhello_to（来自Proxy）

## Test
```
logcat TestServer:* TestClient:* HelloService:* *:S &
CLASSPATH=/mnt/android_fs/TestServer.jar app_process / TestServer &
CLASSPATH=/mnt/android_fs/TestClient.jar app_process / TestClient hello
CLASSPATH=/mnt/android_fs/TestClient.jar app_process / TestClient hello h233
```