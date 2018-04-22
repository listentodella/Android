# C++实现程序：复用代码，简洁
## C语言
### service_manager：管理服务
### test_server.c：提供服务
* sayhello()
* sayhelloto()
### test_client.c：使用服务
* sayhello()
* sayhelloto()

## C++ 程序结构改进
共有的收发函数:
binder.c
库

IHelloService.h —— I 表示 Interface
```
void sayhello();
int sayhello_to(char *);
```
server: BnHelloService.c　—— B 代表 binder，n 表示 native
```
#include "IHelloService.h"
void sayhello() 
{
    printf()
}
int sayhello_to 
{
    printf()
}
onTransact() 
{
    解析数据， 调用sayhello or sayhello_to
}
```


client: BpHelloService.c —— p 代表 proxy

```
#include "IHelloService.h"
void sayhello() 
{
    构造、 发送数据
}
void sayhello_to() 
{
    构造、 发送数据
}
```


test_server.c
```
main()
{
	while(1) {
	接收数据;
	onTransact();
	}
}
```
test_client.c
```
main()
{
	sayhello();
	sayhello_to();
}
```

## Binder服务的Cpp实现
* 定义接口类
```
IHelloService:
virtual sayhello()
virtual sayhello_to()
则客户端和服务端都要实现这两个函数
```
* server端实现
```
BnHelloService:IHelloService
sayhello()
sayhello_to()
onTransact()
```
* client端实现
```
BpHelloService:IHelloService
sayhello()
sayhello_to()
```
* 实现server
```
main() 
{
    addService()
    while (1) {
        读数据， 解析数据， 调用函数
    }
}
```
* 实现client
```
main() 
{
    svr = getService()
    svr - > sayhello()
    svr - > sayhello_to()
}
```

## How To Test???

### 编译:
* a. 文件放入frameworks/testing/APP_0004_Binder_CPP_App
* b. enviroment
```
    cd /work/android-5.0.2/
    . setenv
    lunch //选择23. full_tiny4412-eng
```
* c. mmm
```
    cd frameworks/testing/APP_0004_Binder_CPP_App
    mmm .   
```

### 测试:
* a. 重新编译内核让它支持NFS（即更新Android的bootimage）
```
    make menuconfig
    <*>   NFS client support                                                        | |
    [*]     NFS client support for NFS version 3                                    | |
    [*]       NFS client support for the NFSv3 ACL protocol extension               | |
    [*]     NFS client support for NFS version 4                                    | |
    [*]       NFS client support for NFSv4.1 (EXPERIMENTAL) 

    make zImage, 并使用新的zImage启动单板
```

* b. mount nfs：这里使用的是Android系统，因此需要busybox
```
   su
   ifconfig eth0 192.168.1.100
   busybox mount -t nfs -o nolock,vers=2 192.168.1.123:/work/nfs_root /mnt
```   

* c. 执行 test_server, test_client (cpp or c)
```
   ./test_server &
   logcat HelloService:* *:S &
   ./test_client hello
   ./test_client hello weidongshan

```
*****************
## 添加服务
BnGoodbyeService.cpp
.saygoodbye
.saygoodbye_to
BpGoodbyeService.cpp
.saygoodbye
.saygoodbye_to


