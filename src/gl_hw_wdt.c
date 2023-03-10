#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>

#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_platform.h>
#include <linux/fb.h>
#include <linux/err.h>
#include <linux/slab.h>

#define HW_DWI_INT_NS 1000000000 //  feed dog period,100ms = 100000000ns
static struct hrtimer hw_wdt_timer;
static ktime_t hw_wdt_timeout;


static int enable = 1;//When the package is installed, the watchdog will be opened by default

static ssize_t wdt_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    if ((buf[0] == '1' || buf[0] == 1) && enable == 0) {
        enable = 1;
        hrtimer_start(&hw_wdt_timer, hw_wdt_timeout, HRTIMER_MODE_REL);
        printk("enable hardware watchdog\n");
    } else if ((buf[0] == '0' || buf[0] == 0) && enable == 1) {
        enable = 0;
        printk("disable hardware watchdog\n");
    }

    return 0;
}

static struct file_operations hw_wdt_fops = {
    .owner = THIS_MODULE,
    // .open  = wdt_open,
    .write = wdt_write,
};

static  struct miscdevice  g_tmisc = {
    .minor 	=	MISC_DYNAMIC_MINOR,
    .name 	=	"hw_wdt",
    .fops 	=	&hw_wdt_fops,
};

static unsigned int feed_dog_gpio = 0xff;
static unsigned int dog_en_gpio = 0xff;
static unsigned int feed_dog_interval = HW_DWI_INT_NS;
static enum hrtimer_restart hw_wdt_timeout_handle(struct hrtimer *timer)
{
    enum hrtimer_restart ret = HRTIMER_RESTART;
    static int gpio_status = 0;
    static int a = 0;

    a++;
    if (a > 10) {
        a = 0;
    }

    ktime_t current_time = ktime_get();

    hrtimer_forward(&hw_wdt_timer, current_time, hw_wdt_timeout);

    gpio_status = !gpio_status;
    gpiod_set_value(gpio_to_desc(feed_dog_gpio), gpio_status);

    if (enable) {
        ret = HRTIMER_RESTART;
    } else {
        ret = HRTIMER_NORESTART;
    }


    return ret;
}


static int hw_wdt_probe(struct platform_device *pdev)
{
    printk("hw_wdt enter probe\n");


    if (!of_property_read_u32(pdev->dev.of_node, "feed_dog_gpio", &feed_dog_gpio)) {
        printk("feed dog gpio = gpio%d \n", feed_dog_gpio);
        if (gpio_request(feed_dog_gpio, "feed_dog") != 0) {
            printk("gpio_request gpio%d error\n", feed_dog_gpio);
        }

        if (gpiod_direction_output(gpio_to_desc(feed_dog_gpio), 1) != 0) {
            printk("gpio_direction_output gpio%d error\n", feed_dog_gpio);
        }
        gpiod_export(gpio_to_desc(feed_dog_gpio), 1);
    }

    if (!of_property_read_u32(pdev->dev.of_node, "dog_en_gpio", &dog_en_gpio)) {
        printk("watchdog enable gpio = gpio%d \n", dog_en_gpio);
        if (gpio_request(dog_en_gpio, "dog_en") != 0) {
            printk("gpio_request gpio%d error\n", dog_en_gpio);
        }

        if (gpiod_direction_output(gpio_to_desc(dog_en_gpio), 1) != 0) {
            printk("gpio_direction_output gpio%d error\n", dog_en_gpio);
        }
        gpiod_export(gpio_to_desc(dog_en_gpio), 1);
    }
    if (!of_property_read_u32(pdev->dev.of_node, "feed_dog_interval", &feed_dog_interval)) {
        printk("feed watchdog interval %d ns\n", feed_dog_interval);
        hw_wdt_timeout = ktime_set(0, feed_dog_interval);
        hrtimer_init(&hw_wdt_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        hw_wdt_timer.function = &hw_wdt_timeout_handle;
    }


    hrtimer_start(&hw_wdt_timer, hw_wdt_timeout, HRTIMER_MODE_REL);

    misc_register(&g_tmisc);
    return 0;
}

static int hw_wdt_remove(struct platform_device *pdev)
{
    printk("enter remove\n");
    hrtimer_cancel(&hw_wdt_timer);
    misc_deregister(&g_tmisc);
}

static const struct of_device_id of_hw_wdt_match[] = {
    { .compatible = "hw_wdt", },
    {},
};

static struct platform_driver hw_wdt_driver = {
    .probe		= hw_wdt_probe,
    .remove		= hw_wdt_remove,
    .driver		= {
        .name	= "hw_wdt",
        .of_match_table = of_hw_wdt_match,
    },
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("deng");
MODULE_DESCRIPTION("gl hardware watchdog");
MODULE_VERSION("0.1");

module_platform_driver(hw_wdt_driver);

