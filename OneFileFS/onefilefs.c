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

//the iterate is used by the new readdir operation, we only an empry root inode as of now so we do nothing
//but we can see the FS accept an "ls" call
//if we had a real system here we would search for the inode of the directory in file structure and fill the struct dirent with the information
//there is also a shared version, currently not needed
//the ctx gives us information on where the VFS wants to start reading
static int onefilefs_iterate(struct file *file, struct dir_context* ctx)
{
    struct inode *inode = file_inode(file); //inode of the directory to read
    struct super_block *sb = inode->i_sb; //superblock of the FS
    struct buffer_head *bh;
    struct onefilefs_inode *sfs_inode = inode->i_private;
    struct onefilefs_dir_record *record;
    int parent = inode->i_ino;

    printk(KERN_INFO "We are inside readdir. The pos[%lld], inode number[%lu], superblock magic [%lu], datablock number [%llu]\n", ctx->pos, inode->i_ino, sb->s_magic,  sfs_inode->data_block_number);

    //check that this inode is a directory
    if (unlikely(!S_ISDIR(sfs_inode->mode))) {
        printk(KERN_ERR "inode %llu not a directory", sfs_inode->inode_no);
        return -ENOTDIR;
    }

    //read the information from the device
    bh = (struct buffer_head *)sb_bread(sb, sfs_inode->data_block_number);

    printk(KERN_INFO "This dir has [%lld] childen\n", sfs_inode->dir_children_count);

    //we have a total of 2 + children files we can return, if we get asked more return nothing
    if (ctx->pos == 2 + sfs_inode->dir_children_count) {
        brelse(bh);
        return 0;
    }

    //now pass the . and .. entries
    if (ctx->pos < 2) {
        if (!dir_emit_dots(file, ctx)) {
            printk(KERN_ERR "Could not emit dots\n");
            brelse(bh);
            return 0;
        }
        ctx->pos = 2;
    }

    //finally iterate through the actual children
    record = (struct onefilefs_dir_record *) bh->b_data;
    for (; ctx->pos < sfs_inode->dir_children_count+2; ctx->pos++) {
        printk(KERN_INFO "Got filename: %s\n", record->filename);
        dir_emit(ctx, record->filename, ONEFILEFS_FILENAME_MAXLEN, parent, S_IFDIR);
        record++;   // move onto next children
    }
    /*
    old way
    for (i=0; i < sfs_inode->dir_children_count; i++) {
        printk(KERN_INFO "Got filename: %s\n", record->filename);
        filldir(dirent, record->filename, ONEFILEFS_FILENAME_MAXLEN, pos, record->inode_no, DT_UNKNOWN);
        pos += sizeof(struct onefilefs_dir_record);
        record ++;
    }
    */
    //brelse(bh);
    return 1;
}

//add the iterate in the dir operations
const struct file_operations onefilefs_dir_operations = {
    .owner = THIS_MODULE,
    .iterate = onefilefs_iterate,
};

//not really sure what this is supposed to do for now
//probrably recovers a dentry from an inode?
struct dentry *onefilefs_lookup(struct inode *parent_inode,
                   struct dentry *child_dentry, unsigned int flags)
{
    printk(KERN_INFO "onefilefs lookup has been called.\n");
    return NULL;
}

//look up goes in the inode operations
static struct inode_operations onefilefs_inode_ops = {
    .lookup = onefilefs_lookup,
};


// get an inode from its inode number
// currently we have only one inode, the root inode, which is in block 1, so we simply return that
struct onefilefs_inode *onefilefs_get_inode(struct super_block *sb, uint64_t inode_no)
{
    struct onefilefs_sb_info *sfs_sb = sb->s_fs_info;
    struct onefilefs_inode *sfs_inode = NULL;

    int i;
    struct buffer_head *bh;

    // who needs to release this??
    // do i malloc and copy the memory??
    bh = (struct buffer_head *)sb_bread(sb, ONEFILEFS_INODES_BLOCK_NUMBER); // all of our 2 inodes are in here
    sfs_inode = (struct onefilefs_inode *) bh->b_data;

    //currently we have only 2 inodes in the block this is not that useful
    for (i=0; i < sfs_sb->inodes_count; i++) {
        if (sfs_inode->inode_no == inode_no) {
            return sfs_inode;
        }
        sfs_inode++;
    }

    return NULL;
}

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