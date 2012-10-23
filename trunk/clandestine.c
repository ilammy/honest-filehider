#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/namei.h>
#include <linux/path.h>
#include <linux/rwsem.h>

#include "humble.h"


static filldir_t g_original_filldir;

static int filtering_filldir(void *buffer, const char *name, int namelen,
                             loff_t offset, u64 ino, unsigned d_type)
{
	if (humble_hash_contains(ino)) {
		return 0;
	}

	return g_original_filldir(buffer, name, namelen, offset, ino, d_type);
}

/*
 *  We need to pass the original filldir() to filtering_filldir(),
 *  this is done via g_original_filldir pointer.
 *
 *  The pointer is protected by two locks:
 *
 *    * g_readdir_lock ensures that no call to filterind_readdir() will
 *      overwrite current g_original_filldir while the filtering_filldir()
 *      may need it;
 *
 *    * g_filldir_lock permits any number of recursive calls to readdir()
 *      from the same filtering_readdir() (e.g., when VFS reads the `.'
 *      directory entry), but blocks anything else.
 */

static DEFINE_MUTEX(g_readdir_lock);
static DECLARE_RWSEM(g_filldir_lock);

static int filtering_readdir(struct file *dir, void *data, filldir_t filldir)
{
	int err;

	down_read(&g_filldir_lock);
	if (filldir == g_original_filldir) {
		err = dir->f_dentry->d_sb->s_root->d_inode->i_fop
			->readdir(dir, data, filtering_filldir);
		up_read(&g_filldir_lock);
		return err;
	}
	up_read(&g_filldir_lock);

	mutex_lock(&g_readdir_lock);
	g_original_filldir = filldir;
	err = dir->f_dentry->d_sb->s_root->d_inode->i_fop
		->readdir(dir, data, filtering_filldir);
	down_write(&g_filldir_lock);
	g_original_filldir = NULL;
	up_write(&g_filldir_lock);
	mutex_unlock(&g_readdir_lock);

	return err;
}

/*
 * Returning -ENOENT from hidden files' ops as if the files really do not exist.
 */

static ssize_t notfound_read(struct file *file, char __user *buf,
                             size_t count, loff_t *offset)
{
	return -ENOENT;
}

static ssize_t notfound_write(struct file *file, const char __user *buf,
                              size_t count, loff_t *offset)
{
	return -ENOENT;
}

static int notfound_readdir(struct file *dir, void *data, filldir_t filldir)
{
	return -ENOENT;
}

static int notfound_mmap(struct file *file, struct vm_area_struct *dest)
{
	return -ENOENT;
}

static int notfound_open(struct inode *inode, struct file *file)
{
	return -ENOENT;
}

static int notfound_release(struct inode *inode, struct file *file)
{
	return -ENOENT;
}

static int notfound_rmdir(struct inode *parent, struct dentry *dir)
{
	return -ENOENT;
}

static int notfound_rename(struct inode *inode_old, struct dentry *dentry_old,
                           struct inode *inode_new, struct dentry *dentry_new)
{
	return -ENOENT;
}

static int notfound_setattr(struct dentry *dentry, struct iattr *attrs)
{
	return -ENOENT;
}

static int notfound_getattr(struct vfsmount *mnt, struct dentry *dentry,
                            struct kstat *stat)
{
	return -ENOENT;
}

/*
 * Method tables shared between all hidden files and their parents.
 */

static struct file_operations filtering_fops = {
	.owner   = THIS_MODULE,
	.readdir = filtering_readdir
};

static struct file_operations notfound_fops = {
	.owner   = THIS_MODULE,
	.read    = notfound_read,
	.write   = notfound_write,
	.readdir = notfound_readdir,
	.mmap    = notfound_mmap,
	.open    = notfound_open,
	.release = notfound_release
};

static struct inode_operations notfound_iops = {
	.rmdir   = notfound_rmdir,
	.rename  = notfound_rename,
	.setattr = notfound_setattr,
	.getattr = notfound_getattr
};


/*
 *  Hides the file located at @path, writes its inode number to @ino.
 */
int humble_hide_file(const char *path, u64 *ino)
{
	int err;
	struct path dest;
	struct inode *fnode;
	struct inode *pnode;

	err = kern_path(path, 0, &dest);
	if (err) {
		printk(KERN_NOTICE "Humble: could not find file %s\n", path);
		goto out;
	}

	dget(dest.dentry);
	dget(dest.dentry->d_parent);
	fnode = dest.dentry->d_inode;
	pnode = dest.dentry->d_parent->d_inode;
	ihold(fnode);
	ihold(pnode);

	if ((fnode->i_sb->s_root->d_inode == fnode) ||
	    (fnode->i_sb->s_root->d_inode == pnode))
	{
		printk(KERN_NOTICE
			"Humble: hiding of mount points or their "
			"direct descendants is prohibited\n");
		err = -EPERM;
		goto free_nodes;
	}

	err = humble_hash_add(fnode, pnode);
	if (err) {
		printk(KERN_NOTICE "Humble: could not add file to hash\n");
		goto free_nodes;
	}
	fnode->i_op = &notfound_iops;
	fnode->i_fop = &notfound_fops;
	pnode->i_fop = &filtering_fops;

	if (ino != NULL) {
		*ino = fnode->i_ino;
	}
	goto out;
free_nodes:
	iput(pnode);
	iput(fnode);
out:
	dput(dest.dentry->d_parent);
	dput(dest.dentry);
	return err;
}

int humble_unhide_file(u64 ino)
{
	return humble_hash_remove(ino);;
}
