/*
 *input emulator * 
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/input.h>


static struct input_dev *input_emulator_dev;

static int input_emulator_init(void)
{

	int i;
	//1.分配一个input_dev结构体
	input_emulator_dev = input_allocate_device();;
	//2.设置
	//2.1 能产生哪类事件
	set_bit(EV_KEY, input_emulator_dev->evbit);
	set_bit(EV_REP, input_emulator_dev->evbit);//重复，可以达到按住不放，一直产生该键的效果
	
	//2.2 能产生这类操作的哪些事件
    //set can generate all keys
	for (i = 0; i < BITS_TO_LONGS(KEY_CNT); i++)
	    input_emulator_dev->keybit[i] = ~0UL;

	//2.3 为android构造一些设备信息
	input_emulator_dev->name = "InputEmulatorFromh233";
	input_emulator_dev->id.bustype = 1;
	input_emulator_dev->id.vendor  = 0x1234;
	input_emulator_dev->id.product = 0x5678;
	input_emulator_dev->id.version = 1;

	//3.注册
	input_register_device(input_emulator_dev);

	
	return 0;
}
module_init(input_emulator_init);

static void input_emulator_exit(void)
{
	input_unregister_device(input_emulator_dev);
	input_free_device(input_emulator_dev);
}
module_exit(input_emulator_exit);

MODULE_LICENSE("GPL");
