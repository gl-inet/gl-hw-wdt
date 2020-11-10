#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h> 
#include <linux/hrtimer.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>

#define HW_WDI	2//watchdog gpio WDI
#define HW_DWI_INT_NS 1000000000 //  feed dog period,100ms = 100000000ns
static struct hrtimer hw_wdt_timer;
static ktime_t hw_wdt_timeout;


static int enable = 1;//驱动加载时，默认开启喂狗功能，待应用层打开时关闭
static int __init hw_wdt_drv_init(void);
static void __exit hw_wdt_drv_exit(void);
// static int wdt_open(struct inode *inode, struct file *file)
// {
// 	enable = 0;//关闭定时器喂狗功能
// 	gpio_direction_output(HW_WDT_CTL,0);
// 	return 0;
// }
 
// static unsigned int wdt_status=0x00;//记录引脚状态，每调用一次write时进行"非"操作一次
 
static ssize_t wdt_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	if((buf[0] == '1' || buf[0] == 1) && enable == 0) {
		enable = 1;
		hrtimer_start(&hw_wdt_timer, hw_wdt_timeout, HRTIMER_MODE_REL);
		printk("enable hardware watchdog\n");
	}
	else if((buf[0] == '0' || buf[0] == 0) && enable == 1) {
		enable = 0;
		printk("disable hardware watchdog\n");
	}

	return 0;
}
 
static struct file_operations hw_wdt_fops={
	.owner = THIS_MODULE,
	// .open  = wdt_open,
	.write = wdt_write,
};

static enum hrtimer_restart hw_wdt_timeout_handle(struct hrtimer* timer)
{
	enum hrtimer_restart ret = HRTIMER_RESTART;
	static int gpio_status = 0;
	static int a = 0;

	a++;
	if(a > 10) {
	 a=0;
	}

	ktime_t current_time = ktime_get();

	hrtimer_forward(&hw_wdt_timer, current_time, hw_wdt_timeout);

	gpio_status = !gpio_status;
	gpio_set_value(HW_WDI, gpio_status);
//	gpiod_set_value(gpio_to_desc(HW_WDI),gpio_status);
	
	if(enable) {
		ret = HRTIMER_RESTART;
	}
	else {
		ret = HRTIMER_NORESTART;
	}
	

	return ret;
}


static int major=0;
static struct class *drv_class;
 
static int __init hw_wdt_drv_init(void)
{
	printk("hw_wdt enter probe\n");

	hw_wdt_timeout = ktime_set(0, HW_DWI_INT_NS);
	hrtimer_init(&hw_wdt_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hw_wdt_timer.function = &hw_wdt_timeout_handle;
	
/*	if( gpio_request(HW_WDI, "hw_wdi") != 0 ){
		printk("gpio_request gpio%d error\n",HW_WDI);
		return 0;
	}
	
	if( gpio_direction_output(HW_WDI, 1) != 0 ){
		printk("gpio_direction_output gpio%d error\n",HW_WDI);
		gpio_free(HW_WDI);
		return 0;
	}
*/
	hrtimer_start(&hw_wdt_timer, hw_wdt_timeout, HRTIMER_MODE_REL);

	// devno = MKDEV(major,minor);
	major = register_chrdev(major,"hw_wdt",&hw_wdt_fops);
	drv_class = class_create(THIS_MODULE,"hw_wdt");
	device_create(drv_class,NULL,MKDEV(major,0),NULL,"hw_wdt");
	return 0;
}
 
static void __exit hw_wdt_drv_exit(void)
{
	printk("enter remove\n");
	hrtimer_cancel(&hw_wdt_timer);
//	gpio_free(HW_WDI);
	device_destroy(drv_class,MKDEV(major,0));
	class_destroy(drv_class);
	unregister_chrdev(major,"hw_wdt");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("deng");
MODULE_DESCRIPTION("gl hardware watchdog");
MODULE_VERSION("0.1");

module_init(hw_wdt_drv_init);
module_exit(hw_wdt_drv_exit);
 
