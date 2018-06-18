# 005_HDMI热拔插检测驱动程序_HPD引脚的两种功能

标签（空格分隔）： Display HDMI HPD

---

HPD引脚，可以设置为HPD功能，或者wakeup(Eint）功能

`hdmi_status`:
ON：正在使用HDMI
OFF：不在使用

`hpd_struct.hpd_switch.state`
1：HDMI接上（并不代表在使用）
0：HDMI拔出

处理HDMI拔插：
```
if(hdmi_status == ON){//正在使用
    if(HPD引脚 == 1){//误触发
        可以不处理，
        也可以处理
    } else{ //正在使用，却拔掉了
        清除工作（关闭HDMI接口），
        设置状态为0，并且上报
    }
} else {//未使用HDMI
    设置state，上报
}
```

```
static irqreturn_t s5p_hpd_irq_handler(int irq, void *dev_id)
{
	irqreturn_t ret = IRQ_HANDLED;

	/* check HDMI status */
	if (atomic_read(&hdmi_status)) {
		/* HDMI on */
		HPDIFPRINTK("HDMI HPD interrupt\n");
	//引脚被配置为HPD功能
		ret = s5p_hpd_irq_hdmi(irq);
		HPDIFPRINTK("HDMI HPD interrupt - end\n");
	} else {
		/* HDMI off */
		HPDIFPRINTK("EINT HPD interrupt\n");
	//引脚被配置为wakeup（或者说eint外部中断功能）
		ret = s5p_hpd_irq_eint(irq);
		HPDIFPRINTK("EINT HPD interrupt - end\n");
	}

	return ret;
}

```
该引脚可以配置为HPD功能，或者外部中断（wakeup/eint）功能
它们都可以在HDMI接入/拔出时产生中断，却别在于：
* 一开始：无人使用HDMI设备，可以设置成wakeup/eint，万一休眠，可以唤醒系统
* 如果已经使用了，后续有app使用HDMI设备，可以把这个已经设置成HPD功能，可抗干扰
```
static int s5p_hpd_probe(struct platform_device *pdev)
{
。。。
    if (pdata->int_src_hdmi_hpd)
		hpd_struct.int_src_hdmi_hpd =
		    (void (*)(void))pdata->int_src_hdmi_hpd;//设置成hpd
	if (pdata->int_src_ext_hpd)
		hpd_struct.int_src_ext_hpd =
		    (void (*)(void))pdata->int_src_ext_hpd;//设置成外部中断

。。。
//开始时认为没有使用，故设置为外部中断方式
    hpd_struct.int_src_ext_hpd();
。。。
}

```

