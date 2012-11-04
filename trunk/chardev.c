#include "humble.h"

#define DEVICE_NAME "hcontrol"
#define DEVICE_BASE_MINOR  0
#define DEVICE_MINOR_COUNT 1

static dev_t g_dev;
static struct class *g_class;
static struct cdev g_cdev;

static int g_open_count = 0;

#define OUTPUT_BUFFER_SIZE 40
#define INPUT_BUFFER_SIZE 512

static char obuffer[OUTPUT_BUFFER_SIZE];
static char ibuffer[INPUT_BUFFER_SIZE];
static ssize_t obuffer_len;

#define print_out(args...) \
    (obuffer_len = snprintf(obuffer, OUTPUT_BUFFER_SIZE, args), \
     obuffer_len = (obuffer_len >= 0 ? obuffer_len + 1 : 0) )

static inline int output_error(int code)
{
	return print_out("E%d\n", code);
}

static ssize_t device_read(struct file *filp, char __user *buffer,
                           size_t size, loff_t *offset)
{
	ssize_t bytes_read = 0;

	PRdebug("Read @ %d: %s\n", size, obuffer);

	if (obuffer[0] == '\0') {
		return 0;
	}
	if (size >= obuffer_len) {
		bytes_read = obuffer_len;
		bytes_read -= copy_to_user(buffer, obuffer, obuffer_len);
	}
	return bytes_read;
}

static void handle_hiding(void)
{
	int err;
	u64 ino;

	PRdebug("Hide\n");

	if (ibuffer[1] != ' ' || ibuffer[2] != '/') {
		PRdebug("Invalid hiding format: %s\n", ibuffer);
		output_error(-EINVAL);
		return;
	}
	err = humble_hide_file(ibuffer + 2, &ino);
	if (!err) {
		PRdebug("Hidden %s\n", ibuffer + 2);
		print_out("%lld\n", ino);
	} else {
		PRdebug("Failed hiding %s: %d\n", ibuffer + 2, err);
		output_error(err);
	}
}

static void handle_unhiding(void)
{
	int err;
	u64 ino;

	PRdebug("Unhide\n");

	if (sscanf(ibuffer + 1, "%lld", &ino) != 1) {
		PRdebug("Invalid unhiding format: %s\n", ibuffer);
		output_error(-EINVAL);
		return;
	}
	err = humble_unhide_file(ino);
	if (!err) {
		PRdebug("Unhidden #%lld\n", ino);
		print_out("%lld\n", ino);
	} else {
		PRdebug("Failed unhiding #%lld: %d\n", ino, err);
		output_error(err);
	}
}

static void handle_clearing(void)
{
	int err;

	PRdebug("Clear all\n");

	if (ibuffer[1] != '\0') {
		PRdebug("Unvalid clearing format\n");
		output_error(-EINVAL);
		return;
	}
	err = humble_hash_clear();
	if (!err) {
		print_out("0\n");
	} else {
		PRdebug("Failed clearing: %d\n", err);
		output_error(err);
	}
}

static ssize_t device_write(struct file *filp, const char __user *buffer,
                            size_t size, loff_t *offset)
{
	int err;
	ssize_t bytes_written = 0;

	if (size >= INPUT_BUFFER_SIZE) return -ENOSPC;

	bytes_written = strncpy_from_user(ibuffer, buffer, size);
	if (bytes_written >= 0) {
		ibuffer[bytes_written] = '\0';
		if (bytes_written > 0 && ibuffer[bytes_written - 1] == '\n') {
			ibuffer[bytes_written - 1] = '\0';
		}
	}

	PRdebug("Write (%d): %s\n", size, ibuffer);

	switch (ibuffer[0]) {
	case 'H':
		handle_hiding();
		break;
	case 'U':
		handle_unhiding();
		break;
	case 'C':
		handle_clearing();
		break;
	default:
		PRdebug("Unknown: %s\n", ibuffer);
		output_error(-EINVAL);
		break;
	}
	return bytes_written;
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
		return -EBUSY;
	}
	++g_open_count;
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *node, struct file *filp)
{
	--g_open_count;
	module_put(THIS_MODULE);
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
