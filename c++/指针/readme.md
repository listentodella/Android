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
android源码里就有例子：
```
prebuilts\ndk\9\platforms\android-19\arch-x86\usr\include\rs\cpp\util
RefBase.h
StrongPointer.h
TypeHelpers.h
```