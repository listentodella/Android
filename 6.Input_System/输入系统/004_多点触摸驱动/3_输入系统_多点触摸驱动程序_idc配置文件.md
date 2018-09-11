# 3_输入系统_多点触摸驱动程序_idc配置文件

	之前在驱动程序中，将设备的名字准确设置了一下，是因为 Android 会根据它找到相应的配置文件。
	`inputreader`线程会监测`/dev/input/`路径，只要在这个路径下有新的节点生成，就会读入`idc`	等文件。对于TP，一般只需要`idc`文件，而系统首先会去`/system/usr/idc/*`，其次是`/data/system/devices/idc/*`下查找`idc`文件，找到一个符合的就会立即返回。
  例如`/system/usr/idc/ft5x0x_ts.idc`，打开之后发现内容会有很多。那么哪些内容才是必须的呢？

## `deviceType`
```
touch.deviceType = touchscreen
```

仅有这一句配置时，tp 也可以使用。针对 `touch`的`deviceType`有：
* touchscreen
触摸屏, 覆盖在显示器上, 可以直接操作各种图标
* touchpad
触摸板, 不是覆盖在显示器上, 需要在LCD上显示一个光标以便定位，可以理解为笔记本上的触摸板，可以控制光标，点击事件
* pointer
跟`touchPad`类似, 多一些手势功能("Indirect Multi-touch Pointer Gestures")
* default
由系统自己确定。

那么是否可以直接在驱动里配置，省略掉这个`idc`文件呢？可以根据`idc`文件被加载的地方，往前搜索，毕竟上层在配置前也是要读取底层的设置的。
```
set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
```

如果配置了这一项，就不需要`idc`配置文件了。

