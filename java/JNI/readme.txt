JNI (java native interface)
Linux是用C语言写的，可以写一个app简单调用open read write来访问驱动程序
Android是用java写的，java怎么访问c函数？

1.加载C库
2.找到函数
  java函数名  映射  C的某个函数名——————名字并不一定完全相同
3.调用函数
