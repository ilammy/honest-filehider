#include "humble.h"

static int __init humble_init(void)
{
	int err = 0;

	PRinfo("Loading");
	err = humble_devfile_startup_once();
	if (err) {
		PRcritical("Could not create a control device file\n");
	}
	return err;
}

static void __exit humble_exit(void)
{
	if (humble_hash_clear()) {
		PRcritical("Could not unhide remaining files\n");
	}
	humble_devfile_cleanup_once();
	PRinfo("Unloaded");
}

MODULE_AUTHOR("Alex Lozovsky");
MODULE_DESCRIPTION("Humble VFS-level filehiding module");
MODULE_LICENSE("GPL");

module_init(humble_init);
module_exit(humble_exit);
