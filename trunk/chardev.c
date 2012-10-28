#include "humble.h"

#define DEVICE_NAME "hcontrol"
#define DEVICE_BASE_MINOR  0
#define DEVICE_MINOR_COUNT 1

static dev_t g_dev;
static struct class *g_class;
static struct cdev g_cdev;

static int g_open_count = 0;


static ssize_t device_read(struct file *filp, char __user *buffer,
                           size_t size, loff_t *offset)
{
	PRdebug("Read");
	return 0;
}

static ssize_t device_write(struct file *filp, const char __user *buffer,
                            size_t size, loff_t *offset)
{
	PRdebug("Write");
	return size;
}

/*
 *  Only one process can hold the device file opened. Any other will recieve
 *  EBUSY error.
 *
 *  Also we get() the module to prohibit its unloading while the file is opened.
 */

static int device_open(struct inode *node, struct file *filp)
{
	if (g_open_count > 0) {
		PRdebug("Open busy");
		return -EBUSY;
	}
	++g_open_count;
	try_module_get(THIS_MODULE);
	PRdebug("Opened");
	return 0;
}

static int device_release(struct inode *node, struct file *filp)
{
	--g_open_count;
	module_put(THIS_MODULE);
	PRdebug("Closed");
	return 0;
}

static struct file_operations g_device_fops = {
	.owner   = THIS_MODULE,
	.read    = device_read,
	.write   = device_write,
	.open    = device_open,
	.release = device_release
};


int humble_devfile_startup_once(void)
{
	int err;

	err = alloc_chrdev_region(&g_dev, DEVICE_BASE_MINOR,
	                          DEVICE_MINOR_COUNT, DEVICE_NAME);
	if (err) goto out;

	cdev_init(&g_cdev, &g_device_fops);
	g_cdev.owner = THIS_MODULE;

	err = cdev_add(&g_cdev, g_dev, DEVICE_MINOR_COUNT);
	if (err) goto clear_region;

	g_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (!g_class) {
		err = -ENOMEM;
		goto clear_cdev;
	}

	if (IS_ERR(device_create(g_class, NULL, g_dev, NULL, DEVICE_NAME))) {
		err = -ENOMEM;
		goto clear_class;
	}

	goto out;

clear_class:
	class_destroy(g_class);
clear_cdev:
	cdev_del(&g_cdev);
clear_region:
	unregister_chrdev_region(g_dev, DEVICE_MINOR_COUNT);
out:
	return err;
}

void humble_devfile_cleanup_once(void)
{
	device_destroy(g_class, g_dev);
	class_destroy(g_class);
	cdev_del(&g_cdev);
	unregister_chrdev_region(g_dev, DEVICE_MINOR_COUNT);
}
