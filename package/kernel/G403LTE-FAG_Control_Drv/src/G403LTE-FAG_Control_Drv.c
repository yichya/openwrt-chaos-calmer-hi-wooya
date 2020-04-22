#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/uaccess.h>

#define RESET_N_LOW 		0
#define RESET_N_HIGH 		1
#define PWRKEY_LOW 		2
#define PWRKEY_HIGH 		3

volatile unsigned long *GPIO1_MODE;
volatile unsigned long *GPIO_CTRL_0;		// GPIO0到GPIO31方向控制寄存器
volatile unsigned long *GPIO_POL_0;			// GPIO0到GPIO31极性控制寄存器，即数据是否反转
volatile unsigned long *GPIO_DATA_0;		// GPIO0到GPIO31数据寄存器
volatile unsigned long *GPIO_DSET_0;		// GPIO0到GPIO31数据设置寄存器

static struct class *uc20_control_drv_class;

/*
** RESET_N ---- GPIO#1
** PWRKEY ---- GPIO#3
*/
static int uc20_control_drv_open(struct inode *inode, struct file *file)
{
	/* 让GPIO#1、GPIO#3输出高电平 */
	*GPIO_DATA_0 |= (1<<1)|(1<<3);

	return 0;
}

static long uc20_control_drv_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case RESET_N_LOW:	// GPIO#1设置为0
			*GPIO_DATA_0 &= ~(1<<1);
			break;
		case RESET_N_HIGH:	// GPIO#1设置为1
			*GPIO_DATA_0 |= (1<<1);
			break;
		case PWRKEY_LOW:	// GPIO#3设置为0
			*GPIO_DATA_0 &= ~(1<<3);
			break;
		case PWRKEY_HIGH:	// GPIO#3设置为1
			*GPIO_DATA_0 |= (1<<3);
			break;
		default:
			break;
	}
	
	return 0;
}

/* 1.分配、设置一个file_operations结构体 */
static struct file_operations uc20_control_drv_fops = {
	.owner   			= THIS_MODULE,    				/* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.open    			= uc20_control_drv_open,
	.unlocked_ioctl	= uc20_control_drv_unlocked_ioctl,
};

int major;
static int __init uc20_control_drv_init(void)
{
	/* 2.注册 */
	major = register_chrdev(0, "uc20_control_drv", &uc20_control_drv_fops);

	/* 3.自动创建设备节点 */
	/* 创建类 */
	uc20_control_drv_class = class_create(THIS_MODULE, "uc20_control_drv");
	/* 类下面创建设备节点 */
	device_create(uc20_control_drv_class, NULL, MKDEV(major, 0), NULL, "uc20_control_drv");		// /dev/uc20_control_drv

	/* 4.硬件相关的操作 */
	/* 映射寄存器的地址 */
	GPIO1_MODE = (volatile unsigned long *)ioremap(0x10000060, 4);
	GPIO_CTRL_0 = (volatile unsigned long *)ioremap(0x10000600, 4);
	GPIO_POL_0 = (volatile unsigned long *)ioremap(0x10000610, 4);
	GPIO_DATA_0 = (volatile unsigned long *)ioremap(0x10000620, 4);
	GPIO_DSET_0 = (volatile unsigned long *)ioremap(0x10000630, 4);

	/* 设置相应管脚用于GPIO(I2S) */
	/*
	** RESET_N ---- GPIO#1
	** PWRKEY ---- GPIO#3
	*/
	*GPIO1_MODE |= (0x1<<6);

	/* 将GPIO#1、GPIO#3设置为输出 */
	*GPIO_CTRL_0 = (1<<1)|(1<<3);

	/* 让GPIO#1、GPIO#3输出高电平 */
	*GPIO_DATA_0 |= (1<<1)|(1<<3);

	return 0;
}

static void __exit uc20_control_drv_exit(void)
{
	unregister_chrdev(major, "uc20_control_drv");
	device_destroy(uc20_control_drv_class, MKDEV(major, 0));
	class_destroy(uc20_control_drv_class);
	iounmap(GPIO1_MODE);
	iounmap(GPIO_CTRL_0);
	iounmap(GPIO_POL_0);
	iounmap(GPIO_DATA_0);
	iounmap(GPIO_DSET_0);
}

module_init(uc20_control_drv_init);
module_exit(uc20_control_drv_exit);

MODULE_LICENSE("GPL");

