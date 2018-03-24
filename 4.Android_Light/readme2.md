# 灯光使用方式
* brightness
0 ~ 255
0	255

* color
	RGB

* blink
	onMs、offMs
	定时器：点亮led，设置超时时间为onMs，当超时时间到，熄灭led，再次设置超时时间为offMs，当超时时间到就点亮led……如此循环

## 怎么写驱动
1.分配led_classdev
2.设置
```
led_cdev->max_brightness;
led_cdev->brightness;
led_cdev->flags;
led_cdev->brightness_set()
```
3.注册：led_classdev_register