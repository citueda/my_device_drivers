#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/io.h>

MODULE_AUTHOR("Ryuichi Ueda");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;

static volatile uint32_t *gpio_base = NULL;

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	char c;
	if(copy_from_user(&c,buf,sizeof(char)))
		return -EFAULT;

	if(c == '0')
		gpio_base[10] = (1 << 25) & 0xffffffff; //10: GPCLR0
	else if(c == '1')
		gpio_base[7] = (1 << 25) & 0xffffffff; //7: GPSET0

        return 1;
}

static struct file_operations led_fops =
{
	owner   : THIS_MODULE,
	write   : led_write,
};

static int __init init_mod(void)
{
	int retval;

	const uint32_t rpi_gpio_base = 0x3f000000 + 0x200000;//レジスタのベース+ GPIOのオフセット
	const uint8_t gpio_size = 0xC0;

	const int led = 25;
	const int index = 0 + led/10;//0: GPFSEL0, 25: GPIO NO
	const int shift = (led%10)*3;
	const uint32_t mask = ~(0x7 << shift);

	gpio_base = ioremap_nocache(rpi_gpio_base, gpio_size); //0xC0: gpio size
	gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift); //0x1: GPF_OUTPUT

	retval =  alloc_chrdev_region(&dev, 0, 1, "led");
	if(retval < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));

	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, MKDEV(MAJOR(dev),0), 1);
	if(retval < 0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:0\n",MAJOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE,"myled");
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	device_create(cls, NULL, MKDEV(MAJOR(dev),0), NULL, "myled0");

	return 0;
}

static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	device_destroy(cls, MKDEV(MAJOR(dev),0));
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
	iounmap(gpio_base);
}

module_init(init_mod);
module_exit(cleanup_mod);
