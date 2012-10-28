#include "humble.h"

#define HASH_BITS 8
#define HASH_BUCKET_COUNT (1 << HASH_BITS)

struct hash_entry_parent {
	struct hlist_node      link;

	struct inode           *inode;
	struct file_operations *old_fops;

	int                    hidden_cnt;
};

struct hash_entry_file {
	struct hlist_node        link;

	struct inode             *inode;
	struct inode_operations  *old_iops;
	struct file_operations   *old_fops;

	struct hash_entry_parent *parent;
};

#define entry_file(lp)   (hlist_entry((lp), struct hash_entry_file,   link))
#define entry_parent(lp) (hlist_entry((lp), struct hash_entry_parent, link))
#define get_bucket(hash, ino) ((hash) + hash_64((ino), HASH_BITS))


/*
 *  g_hash_lock allows concurrent read access for `is hidden' checks,
 *  but blocks on hiding/unhiding.
 *
 *  It is supposed to be held in every private function.
 */

static struct hlist_head g_humble_file_hash[HASH_BUCKET_COUNT];
static struct hlist_head g_humble_parent_hash[HASH_BUCKET_COUNT];
static DECLARE_RWSEM(g_hash_lock);

static struct hash_entry_file* humble_get_file(u64 ino)
{
	struct hlist_node *node;
	struct hlist_head *bucket = get_bucket(g_humble_file_hash, ino);
	hlist_for_each(node, bucket) {
		if (entry_file(node)->inode->i_ino == ino) {
			return entry_file(node);
		}
	}
	return NULL;
}

static struct hash_entry_parent* humble_get_parent(u64 ino)
{
	struct hlist_node *node;
	struct hlist_head *bucket = get_bucket(g_humble_parent_hash, ino);
	hlist_for_each(node, bucket) {
		if (entry_parent(node)->inode->i_ino == ino) {
			return entry_parent(node);
		}
	}
	return NULL;
}


int humble_hash_contains(u64 ino)
{
	int res;
	down_read(&g_hash_lock);
	res = (humble_get_file(ino) != NULL);
	up_read(&g_hash_lock);
	return res;
}

/*  Errors:
 *    -EEXIST  the inode is already hidden
 *    -ENOMEM  could not allocate enough memory
 */
int humble_hash_add(struct inode *f_inode, struct inode *p_inode)
{
	int err = 0;
	struct hlist_head *bucket = NULL;
	struct hash_entry_parent *pentry = NULL;
	struct hash_entry_file *fentry = NULL;
	int release_p = 0;

	down_write(&g_hash_lock);
	if (humble_get_file(f_inode->i_ino)) {
		err = -EEXIST;
		goto out;
	}

	pentry = humble_get_parent(p_inode->i_ino);
	if (pentry) {
		pentry->hidden_cnt += 1;
	} else {
		pentry = kmalloc(sizeof(*pentry), GFP_KERNEL);
		if (!pentry) {
			err = -ENOMEM;
			goto nomem;
		}

		INIT_HLIST_NODE(&pentry->link);
		pentry->inode = p_inode;
		ihold(p_inode);
		pentry->old_fops = p_inode->i_fop;
		pentry->hidden_cnt = 1;
		release_p = 1;

		bucket = get_bucket(g_humble_parent_hash, p_inode->i_ino);
		hlist_add_head(&pentry->link, bucket);
	}

	fentry = kmalloc(sizeof(*fentry), GFP_KERNEL);
	if (!fentry) {
		err = -ENOMEM;
		goto nomem;
	}

	INIT_HLIST_NODE(&fentry->link);
	fentry->inode = f_inode;
	ihold(f_inode);
	fentry->old_iops = f_inode->i_op;
	fentry->old_fops = f_inode->i_fop;
	fentry->parent = pentry;

	bucket = get_bucket(g_humble_file_hash, f_inode->i_ino);
	hlist_add_head(&fentry->link, bucket);

	goto out;
nomem:
	if (release_p) {
		iput(p_inode);
	}
	kfree(fentry);
	kfree(pentry);
out:
	up_write(&g_hash_lock);
	return err;
}

/*  Errors:
 *    -ENOENT     no such file in hash
 *    -EBADF      the file has lost its parent, cannot restore
 *    -ENOTEMPTY  trying to remove the file when its parent
 *                directory still exists in the hash
 */
int humble_hash_remove(u64 ino)
{
	int err = 0;
	struct hash_entry_file *fentry = NULL;
	struct hash_entry_parent *pentry = NULL;

	down_write(&g_hash_lock);
	fentry = humble_get_file(ino);
	if (!fentry) {
		err = -ENOENT;
		goto out;
	}
	pentry = fentry->parent;
	if (!pentry) {
		PRcritical("File #%lld has lost its parent\n", ino);
		err = -EBADF;
		goto out;
	}
	if (humble_get_file(pentry->inode->i_ino)) {
		err = -ENOTEMPTY;
		goto out;
	}

	fentry->inode->i_op = fentry->old_iops;
	fentry->inode->i_fop = fentry->old_fops;
	hlist_del(&fentry->link);
	iput(fentry->inode);
	kfree(fentry);

	if (--pentry->hidden_cnt == 0) {
		pentry->inode->i_fop = pentry->old_fops;
		hlist_del(&pentry->link);
		iput(pentry->inode);
		kfree(pentry);
	}
out:
	up_write(&g_hash_lock);
	return err;
}

int humble_hash_clear(void)
{
	int err = 0;
	struct hlist_node *node = NULL, *next = NULL;
	struct hlist_head *bucket = NULL;
	struct hash_entry_file *fentry = NULL;
	struct hash_entry_parent *pentry = NULL;

	down_write(&g_hash_lock);
	for (bucket = g_humble_file_hash;
	     bucket < g_humble_file_hash + HASH_BUCKET_COUNT;
	     ++bucket)
	{
		hlist_for_each_safe(node, next, bucket) {
			fentry = entry_file(node);
			fentry->inode->i_op = fentry->old_iops;
			fentry->inode->i_fop = fentry->old_fops;
			hlist_del(node);
			iput(fentry->inode);
			kfree(fentry);
		}
	}
	for (bucket = g_humble_parent_hash;
	     bucket < g_humble_parent_hash + HASH_BUCKET_COUNT;
	     ++bucket)
	{
		hlist_for_each_safe(node, next, bucket) {
			pentry = entry_parent(node);
			pentry->inode->i_fop = pentry->old_fops;
			hlist_del(node);
			iput(pentry->inode);
			kfree(pentry);
		}
	}
	up_write(&g_hash_lock);
	return err;
}
