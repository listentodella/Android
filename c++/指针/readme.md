# 智能指针
## 1.智能指针
```
person5.cpp 崩溃分析：
第一次test_func调用完后，析构函数已经执行，空间已经被释放
第二次test_func调用时，该对象并不知道p已经被销毁，仍然去执行，必然崩溃
```

* 引用计数
每有一个对象引用，就给该计数加1；每当有对象不再引用它，技术就减1；当不再有引用时，析构函数进行释放。
```
person6.cpp 流程分析
$ ./a.exe
Person()
sp(Person *other)
before call test_func: 1
sp(const sp &other)
In call test_func: 2
just a test func
~sp()
after call test_func: 1
sp(const sp &other)
In call test_func: 2
just a test func
~sp()
after call test_func: 1
~sp()
~Person()

main函数执行new时，建立第一个引用对象，计数为1
第一次test_func调用时，传入other引用，增加一次计数，为2
然后第一次test_func调用结束，局部变量被回收，调用析构函数，计数减1，为1
第二次test_func调用时，重复第一次的操作
最后当main函数结束调用时，new的对象需要释放，调用sp的析构函数，计数减1，为0，便可以调用Person的析构函数了
```
* 缺点：++ -- 并非原子操作，可能会被打断，那样就有潜在的风险
## 轻量级指针
android源码里就有例子，具有原子操作：
```
prebuilts\ndk\9\platforms\android-19\arch-x86\usr\include\rs\cpp\util
RefBase.h
StrongPointer.h
TypeHelpers.h
```
* 缺点
1. 只是针对引用计数线程安全而已
2. 如果a b互相引用，永远都无法释放，因为count无法归零，造成内存泄露

### 强指针与弱指针
* 强指针/强引用：A指向B,A决定B的生死（增加了对方的引用计数值，交叉引用后会导致内存泄露）
* 弱指针/弱引用：A指向B,A不能决定B的生死（避免了上面内存泄露的缺点）

![强指针与弱指针](%E5%BC%BA%E6%8C%87%E9%92%88%E4%B8%8E%E5%BC%B1%E6%8C%87%E9%92%88.png)

### 强指针和弱指针的实现与使用
* 头文件
```
system\core\include\cutils
system\core\include\utils
```
* cpp
```
system\core\libutils
```