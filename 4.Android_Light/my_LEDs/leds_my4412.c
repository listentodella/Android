
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/delay.h>
 
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#include <linux/leds.h>

struct led_desc {
	int gpio;
	char *name;
};

static struct led_desc led_gpios[] = {
	{EXYNOS4212_GPM4(0), "led1"},
	{EXYNOS4212_GPM4(1), "led2"},
	{EXYNOS4212_GPM4(2), "led3"},
	{EXYNOS4212_GPM4(3), "led4"},
};

struct led_classdev_4412 {
	struct led_classdev cdev;
	int gpio;
};


static struct led_classdev_4412 *myled;
static void	 brightness_set_4412(struct led_classdev *led_cdev,
			  enum led_brightness brightness)
{
    struct led_classdev_4412 *dev = (struct led_classdev_4412 *)led_cdev;
    
    led_cdev->brightness = brightness;

    if(brightness != LED_OFF) 
        gpio_set_value(dev->gpio, 0);
    else
        gpio_set_value(dev->gpio, 1);
}


static int leds_init(void)
{
    int i;
    int ret;
    /*1. alloc led_classdev */
    myled = kzalloc(sizeof(struct led_classdev_4412) * ARRAY_SIZE(led_gpios), GFP_KERNEL);
    if (myled == NULL) {
        printk("failed to alloc for myleds\n");
        return -ENOMEM;
    }
    for(i = 0; i < ARRAY_SIZE(led_gpios); i++){

        s3c_gpio_cfgpin(myled[i].gpio, S3C_GPIO_OUTPUT);
        gpio_set_value(myled[i].gpio, 1);

        /*2.set */ 
        myled[i].cdev.max_brightness = 255;
        myled[i].cdev.brightness_set = brightness_set_4412;
        myled[i].cdev.flags = LED_CORE_SUSPENDRESUME;
        myled[i].cdev.brightness = LED_OFF;
        myled[i].cdev.name = led_gpios[i].name;
        //myled[i].cdev.default_trigger = "timer";
        myled[i].gpio = led_gpios[i].gpio;
        /*3. led_classdev_register */
        ret = led_classdev_register(NULL, &myled[i].cdev);
        if (ret) {
            i--;
            while(i >= 0) {
                led_classdev_unregister(&myled[i].cdev);
                i--;
            }
            kfree(myled);
            return -EIO;
        }

    }
    
	return 0;
}

static void leds_exit(void)
{
    int i;
    for(i = 0; i < ARRAY_SIZE(led_gpios); i++){
        led_classdev_unregister(&myled[i].cdev);
    }
    kfree(myled);
}

module_init(leds_init);
module_exit(leds_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("listentodella");


