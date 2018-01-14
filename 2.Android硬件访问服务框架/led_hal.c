
/*1.实现一个名为HMI的hw_module_t结构体*/

/*2.实现一个open函数，它根据name返回一个设备自定义的结构体——led_device_t*/

/*实现这个自定义结构体——led_device_t*/

/*参考文件hardware/libhardware/modules/vibrator/vibrator.c*/

#define LOG_TAG "LedHal"

#include <led_hal.h>
#include <cutils/log.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


static int fd;

static int led_close(struct hw_device_t *device)
{
  close(fd);
  return 0;
}

static int led_open(struct led_device_t *dev)
{
  fd = open("/dev/leds", O_RDWR);
  ALOGI("ledOpen: %d", fd);
  if(fd >= 0)
    return 0;
  else
    return -1;
}

static int led_ctrl(struct led_device_t *dev, int which, int status)
{
  int ret = ioctl(fd, status, which);
  ALOGI("led_ctrl: %d, %d, %d", which, status, ret);
  return ret;
}


static struct led_device_t led_dev = {
  .common = {
    .close = led_close,
  },
  .led_open = led_open,
  .led_ctrl = led_ctrl,
};

static int led_device_open(onst hw_module_t* module, const char* id __unused,
                      hw_device_t** device __unused)
{
  *device = &led_dev;
  return 0;
}

static struct hw_module_t led_module_methods = {
  .open = led_device_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
  .id = "led",
  .methods = &led_module_methods,
};
