# 004_HDMI热拔插检测驱动程序_switch系统简介

标签（空格分隔）： HDMI HPD Switch

---
#switch_class.c switch.c uevent.c ??
## 使用`switch`系统（方法二）
### `switch`系统简介
* 构造`switch_dev`
```
.name = "hdmi",
.state = 1 or 0
```
* `switch_dev_register`
创建：class_create("switch")
`/sys/class/switch`
在类下创建设备：device_create()
`/sys/class/switch/hdmi`
device_create_file(...state...)
`/sys/class/switch/hdmi/state`
device_create_file(...name...)
`/sys/class/switch/hdmi/name`
> 可以读这些虚拟文件得到name和state

### 使用`switch`系统（传统）
* 中断程序
设置`switch_dev`的`state`
* app 
读`/sys/class/switch/hdmi/state`获得状态
但是只用这种方法频繁去读，比较耗费资源

#### `switch`系统设置`state`的过程：
中断->`schedule_work`->`switch_set_state`
#####方法一
`sdev->state = state`
#####方法二
###### 构造`envp`:
```
envp[0] = "SWITCH_NAME = hdmi"
envp[1] = "SWITCH_STATE = 1（或者0）"
envp[2] = "SWITCH_TIME = xxx"
```
###### 调用`kobject_uevent_env(..., KOBJ_CHANGE, envp)`发送给user（app）
* 使用网络发送`envp`
* 或者使用`call_usermoehelper`，最终会`/sbin/hotplug`
```
call_usermoehelper(argv[0], argv, env->envp, UMH_WAIT_EXEC)
```

### 使用`switch`系统（改进，也是Android采用的方法）
* app:设置网络，读数据，若无数据则休眠
* 驱动：
中断->`schedule_work`->`switch_set_state`->发送网络数据
* app 被唤醒，读、处理





