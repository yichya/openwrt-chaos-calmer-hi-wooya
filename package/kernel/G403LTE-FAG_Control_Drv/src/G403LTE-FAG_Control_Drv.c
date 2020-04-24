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
volatile unsigned long *GPIO_CTRL_0;		// GPIO0��GPIO31������ƼĴ���
volatile unsigned long *GPIO_POL_0;			// GPIO0��GPIO31���Կ��ƼĴ������������Ƿ�ת
volatile unsigned long *GPIO_DATA_0;		// GPIO0��GPIO31���ݼĴ���
volatile unsigned long *GPIO_DSET_0;		// GPIO0��GPIO31�������üĴ���

static struct class *uc20_control_drv_class;

/*
** RESET_N ---- GPIO#1
** PWRKEY ---- GPIO#3
*/
static int uc20_control_drv_open(struct inode *inode, struct file *file)
{
	/* ��GPIO#1��GPIO#3����ߵ�ƽ */
	*GPIO_DATA_0 |= (1<<1)|(1<<3);

	return 0;
}

static long uc20_control_drv_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case RESET_N_LOW:	// GPIO#1����Ϊ0
			*GPIO_DATA_0 &= ~(1<<1);
			break;
		case RESET_N_HIGH:	// GPIO#1����Ϊ1
			*GPIO_DATA_0 |= (1<<1);
			break;
		case PWRKEY_LOW:	// GPIO#3����Ϊ0
			*GPIO_DATA_0 &= ~(1<<3);
			break;
		case PWRKEY_HIGH:	// GPIO#3����Ϊ1
			*GPIO_DATA_0 |= (1<<3);
			break;
		default:
			break;
	}
	
	return 0;
}

/* 1.���䡢����һ��file_operations�ṹ�� */
static struct file_operations uc20_control_drv_fops = {
	.owner   			= THIS_MODULE,    				/* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
	.open    			= uc20_control_drv_open,
	.unlocked_ioctl	= uc20_control_drv_unlocked_ioctl,
};

int major;
static int __init uc20_control_drv_init(void)
{
	/* 2.ע�� */
	major = register_chrdev(0, "uc20_control_drv", &uc20_control_drv_fops);

	/* 3.�Զ������豸�ڵ� */
	/* ������ */
	uc20_control_drv_class = class_create(THIS_MODULE, "uc20_control_drv");
	/* �����洴���豸�ڵ� */
	device_create(uc20_control_drv_class, NULL, MKDEV(major, 0), NULL, "uc20_control_drv");		// /dev/uc20_control_drv

	/* 4.Ӳ����صĲ��� */
	/* ӳ��Ĵ����ĵ�ַ */
	GPIO1_MODE = (volatile unsigned long *)ioremap(0x10000060, 4);
	GPIO_CTRL_0 = (volatile unsigned long *)ioremap(0x10000600, 4);
	GPIO_POL_0 = (volatile unsigned long *)ioremap(0x10000610, 4);
	GPIO_DATA_0 = (volatile unsigned long *)ioremap(0x10000620, 4);
	GPIO_DSET_0 = (volatile unsigned long *)ioremap(0x10000630, 4);

	/* ������Ӧ�ܽ�����GPIO(I2S) */
	/*
	** RESET_N ---- GPIO#1
	** PWRKEY ---- GPIO#3
	*/
	*GPIO1_MODE |= (0x1<<6);

	/* ��GPIO#1��GPIO#3����Ϊ��� */
	*GPIO_CTRL_0 = (1<<1)|(1<<3);

	/* ��GPIO#1��GPIO#3����ߵ�ƽ */
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

