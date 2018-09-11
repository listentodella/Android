/*************************************************************************
	> File Name: multitouch.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年08月30日 星期四 21时39分29秒
 ************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>

#define MULTITOUCH_ADDR 0x70
#define TS_NAME "h233_tp"

#define MAX_ID 10 //according to the device hardware
#define MAX_X 800
#define MAX_Y 480

#define TP_IRQ 123

struct input_dev *ts_dev;
static struct work_struct ts_work;

static irqreturn_t ts_interrupt(int irq, void *dev_id)
{
    /* 
     * get data of points, and report
     * but i2c bus is slow, so we cannot do this work in irq func
     */
    // use work_queue, and kernel thread will schedule it
    schedule_work(&ts_work);

    return IRQ_HANDLED;
}

static void ts_work_func(struct work_struct *work)
{
    // read i2c device, get data of points, and report
    //read
    //get
    //report
    if () {// with no points
	input_mt_sync(ts_dev);
	input_sync(ts_dev);
	return;
    }

    for () {// for each point
	input_report_abs(ts_dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts_dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts_dev, ABS_MT_TRACKING_ID, id);
	input_mt_sync(ts_dev);
    }
    input_sync(ts_dev);
}

static int __devinit multitouch_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    // input system
    // allocate inpu_dev
    ts_dev = input_allocate_device();

    // set
    // could generate what classes of events
    set_bit(EV_SYN, ts_dev->evbit);
    set_bit(EV_ABS, ts_dev->evbit);
    set_bit(INPUT_PROP_DIRECT, ts_dev->propbit);// could without idc config file

    // could generate what kinds of these classes
    set_bit(ABS_MT_TRACKING_ID, ts_dev->absbit);
    set_bit(ABS_MT_POSITION_X, ts_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, ts_dev->absbit);

    // the range of these events 
    input_set_abs_params(ts_dev, ABS_MT_TRACKING_ID, 0, MAX_ID, 0, 0);
    input_set_abs_params(ts_dev, ABS_MT_POSITION_X, 0, MAX_X, 0, 0);
    input_set_abs_params(ts_dev, ABS_MT_POSITION__Y, 0, MAX_Y, 0, 0);

    //android will find config file according to its name
    ts_dev->name = TS_NAME;
    
    // register
    input_register_device(ts_dev);
    
    // hardware about
    INIT_WORK(&ts_work, ts_work_func);
    request_irq(TP_IRQ, ts_interrupt,
	    IRQ_TYPE_EDGE_FALLING, "h233_multitouch", ts_dev);

    return 0;
}


static int __devexit multitouch_remove(struct i2c_client *client)
{
    free_irq(TP_IRQ, ts_dev);
    cancel_work_sync(&ts_work);

    input_unregister_device(ts_dev);
    input_free_device(ts_dev);

    return 0;
}

static const struct i2c_device_id multitouch_id_table[] = {
    {"h233_multitouch", 0},
    {  }
};

static int multitouch_detect(struct i2c_client *client, struct i2c_board_info *info)
{
    /* 
     * if this function can be called, it illustrates that the device with the addr exist really
     * however, some devices cannot be distinguished such as IC_A with 0x50, IC_B could be 0x50, too
     * so we need read or write the i2c device further to distinguish them
     *
     * the detect func is to distinguish them, and set info->type */
    printk("%s: addr = 0x%x\n", __func__, client->addr);

    // to distinguish them
    strlcpy(info->type, "h233_multitouch", I2C_NAME_SIZE);

    return 0;
    //once returned 0, a new i2c device will be created
    // i2c_new_device(adapter, &info);//info->type was set above...
}


static const unsigned short addr_list[] = {MULTITOUCH_ADDR, I2C_CLIENT_END};

// 1. allocate / set i2c_driver
static struct i2c_driver multitouch_driver = {
    .class = I2C_CLASS_HWMON,// to find device through i2c adapter
    .driver = {
	.name = "h233",
	.owner = THIS_MODULE,
    },
    .probe = multitouch_probe,
    .remove = __devexit_p(multitouch_remove),
    .id_table = multitouch_id_table,
    .detect = multitouch_detect,//to detect this device exists really
    .address_list = addr_list,// the address of these devices
};


static int multitouch_drv_init(void)
{
    // 2. register i2c_driver
    i2c_add_driver(&multitouch_driver);

    return 0;
}
module_init(multitouch_drv_init);

static void multitouch_drv_exit(void)
{
    i2c_del_driver(&multitouch_driver);
}
module_exit(multitouch_drv_exit);

MODULE_LICENSE("GPL");
