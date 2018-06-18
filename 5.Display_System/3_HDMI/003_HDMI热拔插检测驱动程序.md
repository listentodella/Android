# 003_HDMI热拔插检测驱动程序

标签（空格分隔）： HDMI HPD

---

### 怎么写
* 构造`file_operations`
* 注册`fops`到内核

### .read = s5p_hpd_read,
直接返回状态
### .poll = s5p_hpd_poll,
APP_poll调用会导致内核_poll:sys_poll被调用，如果s5p_hpd_poll返回值为0，则休眠，反之返回给app

```
static unsigned int s5p_hpd_poll(struct file *file, poll_table * wait)
{
	poll_wait(file, &hpd_struct.waitq, wait);

	if (atomic_read(&poll_state) != -1)
		return POLLIN | POLLRDNORM;//返回到app

	return 0;//休眠，休眠在某个队列上
}
因此poll_state用来表示热拔插状态是否发生变化
```
但如果休眠了谁来唤醒？
APP有两种方法获得`switch`的`state`
#### 第一种方法：
HPD引脚中断服务程序，从队列（`&hpd_struct.waitq`）中把app唤醒
* `open("/dev/HPD")`
* `poll`得到“状态发生了变化”
* `read`确定是接上、取下

#### 第二种方法：
使用`switch`系统主动上报数据

### HPD引脚还有wakeup功能


## 源码分析
### 入口
注册平台driver
### probe
* 注册`miscdevice`(含有`fops`)
* 注册中断
* `switch_dev_register`

## 使用`/dev/HPD`情景分析（方法一）
### app:open("/dev/HPD")
导致driver的open
```
static int s5p_hpd_open(struct inode *inode, struct file *file)
{
//这里设置成1，意味着第一次poll的时候，可以直接返回一个值（POLLIN | POLLRDNORM）
	atomic_set(&poll_state, 1);

	return 0;
}
```
### app:poll
最终导致driver：poll->假设休眠，由中断唤醒
```
static unsigned int s5p_hpd_poll(struct file *file, poll_table * wait)
{
	poll_wait(file, &hpd_struct.waitq, wait);

	if (atomic_read(&poll_state) != -1)
		return POLLIN | POLLRDNORM;

	return 0;
}


static irqreturn_t s5p_hpd_irq_handler(int irq, void *dev_id)
{
	irqreturn_t ret = IRQ_HANDLED;

	/* check HDMI status */
	if (atomic_read(&hdmi_status)) {
		/* HDMI on */
		HPDIFPRINTK("HDMI HPD interrupt\n");
		ret = s5p_hpd_irq_hdmi(irq);
		HPDIFPRINTK("HDMI HPD interrupt - end\n");
	} else {
		/* HDMI off */
		HPDIFPRINTK("EINT HPD interrupt\n");
		ret = s5p_hpd_irq_eint(irq);
		HPDIFPRINTK("EINT HPD interrupt - end\n");
	}

	return ret;
}

static int s5p_hpd_irq_hdmi(int irq)
{
	u8 flag;
	int ret = IRQ_HANDLED;
	HPDIFPRINTK("\n");

	/* read flag register */
	flag = s5p_hdmi_reg_intc_status();

	if (s5p_hdmi_reg_get_hpd_status())
		s5p_hdmi_reg_intc_clear_pending(HDMI_IRQ_HPD_PLUG);
	else
		s5p_hdmi_reg_intc_clear_pending(HDMI_IRQ_HPD_UNPLUG);

	s5p_hdmi_reg_intc_enable(HDMI_IRQ_HPD_UNPLUG, 0);
	s5p_hdmi_reg_intc_enable(HDMI_IRQ_HPD_PLUG, 0);

	/* is this our interrupt? */
	if (!(flag & (1 << HDMI_IRQ_HPD_PLUG | 1 << HDMI_IRQ_HPD_UNPLUG))) {
		printk(KERN_WARNING "%s() flag is wrong : 0x%x\n",
		       __func__, flag);
		ret = IRQ_NONE;

		goto out;
	}

	if (flag == (1 << HDMI_IRQ_HPD_PLUG | 1 << HDMI_IRQ_HPD_UNPLUG)) {
		HPDIFPRINTK("HPD_HI && HPD_LO\n");

		if (last_hpd_state == HPD_HI && s5p_hdmi_reg_get_hpd_status())
			flag = 1 << HDMI_IRQ_HPD_UNPLUG;
		else
			flag = 1 << HDMI_IRQ_HPD_PLUG;
	}

	if (flag & (1 << HDMI_IRQ_HPD_PLUG)) {
		HPDIFPRINTK("HPD_HI\n");

		s5p_hdmi_reg_intc_enable(HDMI_IRQ_HPD_UNPLUG, 1);
		if (atomic_read(&hpd_struct.state) == HPD_HI)
			return IRQ_HANDLED;

		atomic_set(&hpd_struct.state, HPD_HI);
		atomic_set(&poll_state, 1);

		last_hpd_state = HPD_HI;
		wake_up_interruptible(&hpd_struct.waitq);

	} else if (flag & (1 << HDMI_IRQ_HPD_UNPLUG)) {
		HPDIFPRINTK("HPD_LO\n");
#if defined(CONFIG_SAMSUNG_WORKAROUND_HPD_GLANCE) &&\
	!defined(CONFIG_SAMSUNG_MHL_9290) &&\
	!defined(CONFIG_MHL_SII9234)
		call_sched_mhl_hpd_handler();
#endif

		s5p_hdcp_stop();

		s5p_hdmi_reg_intc_enable(HDMI_IRQ_HPD_PLUG, 1);
		if (atomic_read(&hpd_struct.state) == HPD_LO)
			return IRQ_HANDLED;

		atomic_set(&hpd_struct.state, HPD_LO);
		atomic_set(&poll_state, 1);

		last_hpd_state = HPD_LO;
#ifdef CONFIG_HDMI_CONTROLLED_BY_EXT_IC
		schedule_delayed_work(&ext_ic_control_dwork ,
				msecs_to_jiffies(1000));
#endif

		wake_up_interruptible(&hpd_struct.waitq);
	}

	schedule_work(&hpd_work);

 out:
	return IRQ_HANDLED;
}


```
### app:read


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






