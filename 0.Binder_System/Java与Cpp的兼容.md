# Java与Cpp的兼容
java server <==> c++ client
```
logcat  TestService:* TestServer:* TestClient:* HelloService:* GoodbyeService:* *:S &
CLASSPATH=/mnt/android_fs/TestServer.jar app_process / TestServer &
./test_client hello 
./test_client hello 100ask.taobao.com

./test_client goodbye 
./test_client goodbye 100ask.taobao.com
```

c++ server <==> java client
```
logcat  TestService:* TestServer:* TestClient:* HelloService:* GoodbyeService:* *:S &
./test_server &
CLASSPATH=/mnt/android_fs/TestClient.jar app_process / TestClient hello
CLASSPATH=/mnt/android_fs/TestClient.jar app_process / TestClient hello weidongshan

CLASSPATH=/mnt/android_fs/TestClient.jar app_process / TestClient goodbye
CLASSPATH=/mnt/android_fs/TestClient.jar app_process / TestClient goodbye weidongshan
```

## 要点：保证双方传输的数据一致
sayhello_to
client ==> server
0
IHelloService
name

server ==> client
0 /* no exception */
cnt



	第一次:
	git clone https://github.com/weidongshan/APP_0005_Binder_JAVA_App.git
	
	更新:
	git pull origin
	
	取出指定版本:
	git checkout v3  // 增加goodbye服务


	第一次:
	git clone https://github.com/weidongshan/APP_0003_Binder_C_App.git
	
	更新:
	git pull origin
	
	取出指定版本:
	git checkout v7  // 兼容APP_0005_Binder_JAVA_App


	第一次:
	git clone https://github.com/weidongshan/APP_0004_Binder_CPP_App.git
	
	更新:
	git pull origin
	
	取出指定版本:
	git checkout v4  // 兼容APP_0005_Binder_JAVA_App
