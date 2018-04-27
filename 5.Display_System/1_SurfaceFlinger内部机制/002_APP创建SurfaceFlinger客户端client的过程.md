# SurfaceFlinger内部机制
## APP创建SurfaceFlinger客户端client的过程
* 获得SurfaceFlinger服务//服务在Main_surfaceflinger.cpp
* 使用它的函数创建client（通过binder远程调用）：createConnection
mClient指向SurfaceFlinger进程的client对象