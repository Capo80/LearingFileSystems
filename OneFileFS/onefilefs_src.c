#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "onefilefs.h"

//function that fill the super block with information
//not much inside for now
int onefilefs_fill_super(struct super_block *sb, void *data, int silent)
{   
    struct inode *root_inode;
    struct buffer_head *bh;
    struct onefilefs_sb_info *sb_disk;
    struct timespec64 curr_time;

    //we now look if the block device has a superblock with the correct information
    bh = (struct buffer_head *)sb_bread(sb, ONEFILEFS_SB_BLOCK_NUMBER);

    sb_disk = (struct onefilefs_sb_info *)bh->b_data;

    printk(KERN_INFO "The magic number obtained in disk is: [%lld]\n", sb_disk->magic);

    if (unlikely(sb_disk->magic != ONEFILEFS_MAGIC)) {
        printk(KERN_ERR "The filesystem that you try to mount is not of type onefilefs. Magicnumber mismatch.");
        return -EPERM;
    }

    if (unlikely(sb_disk->block_size != ONEFILEFS_DEFAULT_BLOCK_SIZE)) {
        printk(KERN_ERR "onefilefs seem to be formatted using a non-standard block size.");
        return -EPERM;
    }

    printk(KERN_INFO "onefilefs filesystem of version [%lld] formatted with a block size of [%lld] detected in the device.\n", sb_disk->version, sb_disk->block_size);

    //Unique identifier of the filesystem
    sb->s_magic = ONEFILEFS_MAGIC;

    sb->s_fs_info = sb_disk; // <--- ??

    //set up our root inode
    root_inode = new_inode(sb);
    root_inode->i_ino = ONEFILEFS_ROOT_INODE_NUMBER;
    inode_init_owner(root_inode, NULL, S_IFDIR);
    root_inode->i_sb = sb;
    root_inode->i_op = &onefilefs_inode_ops;
    root_inode->i_fop = &onefilefs_dir_operations;

    ktime_get_real_ts64(&curr_time);
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = curr_time;

    //get our root inode from the disk insted of the superblock
    root_inode->i_private = onefilefs_get_inode(sb, ONEFILEFS_ROOT_INODE_NUMBER);

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
}

static void onefilefs_kill_superblock(struct super_block *s)
{
    printk(KERN_INFO "onefilefs superblock is destroyed. Unmount succesful.\n");
    //we have no information in our superblock currently
    return;
}

//ramsfs_mount, called on file system mounting (duh)
struct dentry *onefilefs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    //mount_nodev is for a virtual file system, or one that doesnt need a real device
    //return mount_nodev(fs_type, flags, data, ramfs_fill_super);

    //mount_bdev is for a file system with an actual block device
    struct dentry *ret;

    ret = mount_bdev(fs_type, flags, dev_name, data, onefilefs_fill_super);

    if (unlikely(IS_ERR(ret)))
        printk(KERN_ERR "Error mounting onefilefs");
    else
        printk(KERN_INFO "onefilefs is succesfully mounted on [%s]\n",dev_name);

    return ret;
}

//file system structure
static struct file_system_type onefilefs_type = {
        .name           = "onefilefs",
        .mount          = onefilefs_mount,
        .kill_sb        = onefilefs_kill_superblock,
        .fs_flags       = FS_USERNS_MOUNT,
};

static int onefilefs_init(void)
{
    int ret;

    //register filesystem
    ret = register_filesystem(&onefilefs_type);
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully registered onefilefs\n");
    else
        printk(KERN_ERR "Failed to register onefilefs. Error:[%d]", ret);

    return ret;
}

static void onefilefs_exit(void)
{
    int ret;

    //unregister filesystem
    ret = unregister_filesystem(&onefilefs_type);

    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully unregistered onefilefs\n");
    else
        printk(KERN_ERR "Failed to unregister onefilefs. Error:[%d]", ret);
}

module_init(onefilefs_init);
module_exit(onefilefs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pasquale Caporaso <caporasopasquale97@gmail.com>");
MODULE_DESCRIPTION("ONEFILEFS");