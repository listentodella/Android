#ifndef ANDROID_LED_INTERFACE_H
#define ANDROID_LED_INTERFACE_H


#include <stdint.h>
#include <sys/cdefs.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer_defs.h>

__BEGIN_DECLS

struct led_device_t {
  struct hw_device_t common;
  int (*led_open)(struct led_device_t* dev);
  int (*led_ctrl)(struct led_device_t* dev, int which, int status);
};

__END_DECLS
#endif
