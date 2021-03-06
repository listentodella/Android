# linux led class driver
hardware/libhardware/include/hardware/lights.h

```
echo 255 > sys/class/leds/led1/brightness
    
static ssize_t led_brightness_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)

cat sys/class/leds/led1/brightness

static ssize_t led_max_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)


echo timer > sys/class/leds/led1/trigger
echo 100 > sys/class/leds/led1/delay_on
echo 200 > sys/class/leds/led1/delay_off
```


## How to write driver
* alloc led_classdev
* set:
led_cdev->max_brightness
led_cdev->brightness_set
led_cdev->flags
led_cdev->brightness
led_cdev->name

* register: led_classdev_register

leds_my4412 ... drivers/leds
Makefile:
+ obj-y += leds_my4412.o
+ obj-$(CONFIG_LEDS_CLASS)		+= led-class.o
+ obj-$(CONFIG_LEDS_TRIGGERS)		+= led-triggers.o
+ obj-$(CONFIG_LEDS_TRIGGER_TIMER)	+= ledtrig-timer.o


## HOW TO Blink?     ledtrig-timer.c

echo timer > sys/class/leds/led1/trigger
    led_trigger_store()

echo 100 > sys/class/leds/led1/delay_on
    led_delay_on_store()

echo 200 > sys/class/leds/led1/delay_off
    led_delay_off_store()

# HOW to HAL??? 如何写LIGHTS_HAL
java:frameworks/base/services/java/com/android/server/lights/
JNI:frameworks/base/services/core/jni/com_android_server_lights_LightsService.cpp
HAL:lights.c

a.实现一个名为HMI的hw_module_t的结构体
b.open() ,根据name返回一个设备自定义的结构体
这个设备自定义的结构体的第一个成员是hw_device_t结构体
还可以定义设备相关的成员
c.实现多个light_device_t结构体，每一个对应一个DEVICE
