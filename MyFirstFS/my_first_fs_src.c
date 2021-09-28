#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>

// this is the fuction to creates a new inode
// @sb: superblock of the filesystem
// @dir: directory inode in which to create the new inode
// @dev: device of the filesystem
struct inode *firstfs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev)
{
    struct timespec64 curr_time;

    //allocates the inode using superblock operations
    struct inode *inode = new_inode(sb);

    if (inode) {
        inode->i_ino = get_next_ino();

        //inits inode users
        inode_init_owner(inode, dir, mode);
        
        ktime_get_real_ts64(&curr_time);
        inode->i_atime = inode->i_mtime = inode->i_ctime = curr_time;

        switch (mode & S_IFMT) {
        //new inode is a dir
        case S_IFDIR:
            /* i_nlink will be initialized to 1 in the inode_init_always function
             * (that gets called inside the new_inode function),
             * We change it to 2 for directories, for covering the "." entry */
            //number of links that exists for this inode (its new so only one)
            inc_nlink(inode);
            break;
        //new
        case S_IFREG:
        case S_IFLNK:
        default:
            printk(KERN_ERR "firstfs can create meaningful inode for only root directory at the moment\n");
            return NULL;
            break;
        }
    }
    return inode;
}

//function that fill the super block with information
//not much inside for now
int firstfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *inode;

    //Unique identifier of the filesystem
    sb->s_magic = 0x42424242;

    inode = firstfs_get_inode(sb, NULL, S_IFDIR, 0);
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
}

static void firstfs_kill_superblock(struct super_block *s)
{
    printk(KERN_INFO "firstfs superblock is destroyed. Unmount succesful.\n");
    //we have no information in our superblock currently
    return;
}

//ramsfs_mount, called on file system mounting (duh)
struct dentry *firstfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    //mount_nodev is for a virtual file system, or one that doesnt need a real device
    //return mount_nodev(fs_type, flags, data, ramfs_fill_super);

    //mount_bdev is for a file system with an actual block device
    struct dentry *ret;

    ret = mount_bdev(fs_type, flags, dev_name, data, firstfs_fill_super);

    if (unlikely(IS_ERR(ret)))
        printk(KERN_ERR "Error mounting firstfs");
    else
        printk(KERN_INFO "firstfs is succesfully mounted on [%s]\n",dev_name);

    return ret;
}

//file system structure
static struct file_system_type firstfs_type = {
        .name           = "firstfs",
        .mount          = firstfs_mount,
        .kill_sb        = firstfs_kill_superblock,
        .fs_flags       = FS_USERNS_MOUNT,
};

static int firstfs_init(void)
{
    int ret;

    //register filesystem
    ret = register_filesystem(&firstfs_type);
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully registered firstfs\n");
    else
        printk(KERN_ERR "Failed to register firstfs. Error:[%d]", ret);

    return ret;
}

static void firstfs_exit(void)
{
    int ret;

    //unregister filesystem
    ret = unregister_filesystem(&firstfs_type);

    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully unregistered firstfs\n");
    else
        printk(KERN_ERR "Failed to unregister firstfs. Error:[%d]", ret);
}

module_init(firstfs_init);
module_exit(firstfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pasquale Caporaso <caporasopasquale97@gmail.com>");
MODULE_DESCRIPTION("FIRST_FS");