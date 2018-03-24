# Android灯光系统
## 总体框架
```
	App			Java
-------------    ^		提供java执行C函数的接口
	JNI			cpp
-------------	 \/		访问HAL
	HAL					封装硬件的操作细节
-------------
	Driver		C		访问硬件

```
```
	别人的app
-----------------
 Android现有的JNI
-----------------
	自己的HAL
-----------------
	自己的Driver
-----------------
	硬件
```
```
light系统
==================

APP:电池灯app、通知灯app、背光灯app
-----------------------
JNI：com_android_server_lights_LightsService.cpp
-----------------------
HAL:自己写lights.c
-----------------------
驱动：led_lights.c
```
### 怎么写
* 看功能：
	电池灯：
	通知灯：硬件上是同一个，功能上颜色、闪烁
	背光灯：调节LCD亮度
* 写驱动：
	硬件上电池灯、通知灯是RGB 3个灯在一起，操作这3个led即可
	背光灯一般则是通过调节PWM
```
	1.对于RGB三个LED实现亮、灭、闪烁三个功能
	2.对于背光灯，设置PWM
```
* HAL:
	把硬件的LED再次划分为不同的逻辑灯
```
	1.实现hw_module_t
	2.实现open，根据name返回不同的light_device_t结构体
		针对背光灯、电池灯、通知灯，实现对应的set_light函数，如set_light_backlight set_light_battery.etc=>再去操作驱动
```
* JNI：com_android_server_lights_LightsService.cpp
```
	1.获得hw_module_t
	2.根据不同的name，调用module->methods,返回不同的light_device_t
	name:背光灯、电池灯、通知灯
```


