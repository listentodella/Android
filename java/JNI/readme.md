# JNI (java native interface)
Linux是用C语言写的，可以写一个app简单调用open read write来访问驱动程序
***
Android是用java写的，java怎么访问c函数？
## 1.加载C库
###  System.loadLibrary
## 2.找到函数
###  java函数名  映射  C的某个函数名——————名字并不一定完全相同
###     1.隐式建立
###     2.显式建立
          JNI_OnLoad->加载C库时就会调用这个函数
          RegisterNative(...)
## 3.调用函数

### 编译与运行：
     javac JNIDemo.java
     gcc -shared -I/usr/lib/jvm/java-1.7.0-openjdk-amd64/include/ -fPIC -o libnative.so native.c
     export LD_LIBRARY_PATH=...
     java JNIDemo

### JNI字段描述符：
#### 1.工具生成 javac xxx.java javah -jni xxx.java ====>>> xxx.h 该头文件里既有对应的字段描述，也有隐式建立方式的C语言的函数名

#### 2.查表自己写



## 4.java和c库传递数据
### 传递基本数据类型-----直接使用、直接返回
### 传递字符串-----需要借助运行环境进行转换
### 传递数组

***
# C调用Java
## 步骤
     1.创建虚拟机
     2.获得class
     3.实例化对象
      3.1获得构造方法（方法名为"<init>"）
      3.2 构造参数
      3.3 调用方法
     4.调用方法
      4.1 获得方法
      4.2 构造参数
      4.3 调用方法
## 读取/设置类中的属性
      1.获得属性ID
      2.读取/设置
