/*************************************************************************
  > File Name: multitouch.c
  > Author: H233
  > Mail: 937138688@qq.com
  > Created Time: 2018年08月30日 星期四 21时39分29秒
 ************************************************************************/
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/mach/irq.h>

#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#define MULTITOUCH_ADDR (0x70 >> 1)//0x38
#define TS_NAME "ft5x0x_ts"

#define MAX_ID 15 //according to the device hardware
#define MAX_X 800
#define MAX_Y 480

#define TP_IRQ gpio_to_irq(EXYNOS4_GPX1(6))

struct input_dev *ts_dev;
struct struct i2c_client *mtp_client;
static struct work_struct ts_work;

struct mtp_event {
    int x;
    int y;
    int id;  
};

static struct mtp_event mtp_events[16];
static int mtp_points;

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


static int mtp_ft5x0x_i2c_rxdata(struct i2c_client *client, char *rxdata, int length) {
    int ret;
    struct i2c_msg msgs[] = {
	{
	    .addr	= this_client->addr,
	    .flags	= 0,
	    .len	= 1,
	    .buf	= rxdata,
	},
	{
	    .addr	= this_client->addr,
	    .flags	= I2C_M_RD,
	    .len	= length,
	    .buf	= rxdata,
	},
    };

    ret = i2c_transfer(mtp_client->adapter, msgs, 2);
    if (ret < 0)
	pr_err("%s: i2c read error: %d\n", __func__, ret);

    return ret;
}

#if 0
static int mtp_ft5x0x_i2c_txdata(char *txdata, int length)
{
    int ret;
    struct i2c_msg msg[] = {
	{
	    .addr	= this_client->addr,
	    .flags	= 0,
	    .len	= length,
	    .buf	= txdata,
	},
    };

    ret = i2c_transfer(this_client->adapter, msg, 1);
    if (ret < 0)
	pr_err("%s: i2c write error: %d\n", __func__, ret);

    return ret;
}

static int mtp_ft5x0x_write_reg(u8 addr, u8 val)
{
    u8 buf[4];
    int ret;

    buf[0] = addr;
    buf[1] = val;
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
	pr_err("write 0x%02x to reg (0x%02x) failed, %d", addr, val, ret);
	return -1;
    }

    return 0;
}
#endif

static int mtp_ft5x0x_read_data(void)
{
    u8 buf[32] = { 0 };
    int ret;

    ret = ft5x0x_i2c_rxdata(mtp_client, buf, 31);
    if (ret < 0) {
	printk("%s: read touch data failed, %d\n", __func__, ret);
	return ret;
    }


    mtp_points = buf[2] & 0x0f;

    /* if (!mtp_points) {
       printk("no points!\n");
       input_mt_sync(ts_dev);
       input_sync(ts_dev);
    //ft5x0x_ts_release(ts);
    return 1;
    } */

    switch (mtp_points) {
	case 5:
	    mtp_events[4].x = (s16)(buf[0x1b] & 0x0F)<<8 | (s16)buf[0x1c];
	    mtp_events[4].y = (s16)(buf[0x1d] & 0x0F)<<8 | (s16)buf[0x1e];
	    mtp_events[4].id = buf[0x1d] >> 4;
	case 4:
	    mtp_events[3].x = (s16)(buf[0x15] & 0x0F)<<8 | (s16)buf[0x16];
	    mtp_events[3].y = (s16)(buf[0x17] & 0x0F)<<8 | (s16)buf[0x18];
	    mtp_events[3].id = buf[0x17] >> 4;
	case 3:
	    mtp_events[2].x = (s16)(buf[0x0f] & 0x0F)<<8 | (s16)buf[0x10];
	    mtp_events[2].y = (s16)(buf[0x11] & 0x0F)<<8 | (s16)buf[0x12];
	    mtp_events[2].id = buf[0x11] >> 4;
	case 2:
	    mtp_events[1].x = (s16)(buf[0x09] & 0x0F)<<8 | (s16)buf[0x0a];
	    mtp_events[1].y = (s16)(buf[0x0b] & 0x0F)<<8 | (s16)buf[0x0c];
	    mtp_events[1].id = buf[0x0b] >> 4;
	case 1:
	    mtp_events[0].x = (s16)(buf[0x03] & 0x0F)<<8 | (s16)buf[0x04];
	    mtp_events[0].y = (s16)(buf[0x05] & 0x0F)<<8 | (s16)buf[0x06];
	    mtp_events[0].id = buf[0x05] >> 4;
	    break;
	case 0:
	default:
	    //printk("%s: invalid touch data, %d\n", __func__, event->touch_point);
	    return 0;
    }

    //event->pressure = 200;

    return 0;
}


static void ts_work_func(struct work_struct *work)
{
    int i;
    int ret;
    // read i2c device, get data of points, and report
    //read
    ret = mtp_ft5x0x_read_data();
    if (ret != 0) {
	return;
    }
    //get
    //report
    if (!mtp_points) {// with no points
	input_mt_sync(ts_dev);
	input_sync(ts_dev);
	return;
    }

    for (i = 0; i < mtp_points; i++) {// for each point
	input_report_abs(ts_dev, ABS_MT_POSITION_X, mtp_events[i].x);
	input_report_abs(ts_dev, ABS_MT_POSITION_Y, mtp_events[i].y);
	input_report_abs(ts_dev, ABS_MT_TRACKING_ID, mtp_events[i].id);
	input_mt_sync(ts_dev);
    }
    input_sync(ts_dev);
}

static int __devinit multitouch_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    // input system
    mtp_client = client;
    // allocate input_dev
    ts_dev = input_allocate_device();

    // set
    // could generate what classes of events
    set_bit(EV_SYN, ts_dev->evbit);
    set_bit(EV_ABS, ts_dev->evbit);

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
    u8 buf[32] = { 0 };
    int ret;
    /* 
     * if this function can be called, it illustrates that the device with the addr exist really
     * however, some devices cannot be distinguished such as IC_A with 0x50, IC_B could be 0x50, too
     * so we need read or write the i2c device further to distinguish them
     *
     * the detect func is to distinguish them, and set info->type */
    printk("%s: addr = 0x%x\n", __func__, client->addr);

    //to confirm wheather the device exists or not
    buf[0] = 0xa3;//vendor id
    ret = ft5x0x_i2c_rxdata(client, buf, 1);
    if (ret < 0) {
	printk("%s: read data failed, not real device, %d\n", __func__, ret);
	return ret;
    }
    
    if (buf[0] != 0x55) {
	printk("no right device!, vendor id = 0x%x\n", buf[0]);
	return ret;
    }

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
