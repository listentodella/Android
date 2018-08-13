# 10_输入系统_使用GlobalKey一键启动程序

标签（空格分隔）： Input

---
## 流程

参考文章:
Android 两种注册、发送广播的区别
http://www.jianshu.com/p/ea5e233d9f43

【Android】动态注册广播接收器 
http://blog.csdn.net/etzmico/article/details/7317528

Android初学习 - 在BroadcastReceiver中启动Activity的问题 
http://blog.csdn.net/cnmilan/article/details/50617802

* 对于global key，系统会根据global_keys.xml发送消息给某个组件
```
frameworks/base/core/res/res/xml/global_keys.xml

<!-- Example format: keyCode = keycode to handle globally. component = component which will handle this key. -->
<key keyCode="KEYCODE_TV" component="com.example.a93713.app_0001_leddemo/.MyBroadcastReceiver" />
```
* app应该注册广播消息的接收者
编写`BroadcastReceiver`派生类，实现消息处理函数
注册派生类
* 然后在该组件中启动app



## Test
手工发广播
最后一个参数是包名/.类名
```
am broadcast -a android.intent.action.BUTTON -n com.example.a93713.app_0001_leddemo/.MyBroadcastReceiver
```


修改完global_keys.xml后需要重新编译img才能够生效
```
mmm frameworks/base/core/res
```
它会生成 `framework-res.apk`, 复制（或者adb push）到单板`/system/framework/`:
注意如果是用NFS形式，需要重新`mount /system`分区：
```
cat /proc/mounts 可以看到权限是 r
mount -o remount,rw /system
cat /proc/mounts 可以看到权限是 rw
```
然后再 `mount NFS`:
```
ifconfig eth0 192.168.1.100
busybox mount -t nfs -o nolock,vers=2 192.168.1.123:/work/nfs_root /data/nfs

cp framework-res.apk /system/framework/  
```

修改驱动程序对应的kl文件(对于TV键不需要修改, scancode 377就对应TV键)
```
cp /system/usr/keylayout/Generic.kl /data/system/devices/keylayout/InputEmulatorFrom100ask_net.kl
```

```
insmod InputEmulator.ko
```

模拟上报按键:
```
sendevent /dev/input/event5 1 377 1
sendevent /dev/input/event5 1 377 0
sendevent /dev/input/event5 0 0 0
```

也可以不使用驱动而使用以下命令模拟按键:
```
input keyevent TV    
```


