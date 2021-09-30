#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>

//the iterate is used by the new readdir operation, we only an empry root inode as of now so we do nothing
//but we can see the FS accept an "ls" call
//if we had a real system here we would search for the inode of the directory in filp structure and fill the struct dirent with the information
//there is also a shared version, currently not needed
static int basicfs_iterate(struct file *filp, struct dir_context* ctx)
{
    printk(KERN_INFO "basicfs iterate has been called.\n");
    return 0;
}

//add the iterate in the dir operations
const struct file_operations basicfs_dir_operations = {
    .owner = THIS_MODULE,
    .iterate = basicfs_iterate,
};

//not really sure what this is supposed to do for now
//probrably recovers a dentry from an inode?
struct dentry *basicfs_lookup(struct inode *parent_inode,
                   struct dentry *child_dentry, unsigned int flags)
{
    printk(KERN_INFO "basicfs lookup has been called.\n");
    return NULL;
}

//look up goes in the inode operations
static struct inode_operations basicfs_inode_ops = {
    .lookup = basicfs_lookup,
};


// this is the fuction to creates a new inode
// @sb: superblock of the filesystem
// @dir: directory inode in which to create the new inode
// @dev: device of the filesystem
struct inode *basicfs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev)
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
            printk(KERN_ERR "basicfs can create meaningful inode for only root directory at the moment\n");
            return NULL;
            break;
        }
    }
    return inode;
}

//function that fill the super block with information
//not much inside for now
int basicfs_fill_super(struct super_block *sb, void *data, int silent)
{
    struct inode *inode;

    //Unique identifier of the filesystem
    sb->s_magic = 0x42424242;

    //get a root inode for the system (its a directory)
    inode = basicfs_get_inode(sb, NULL, S_IFDIR, 0);    
    //add the operations to the root inode
    inode->i_op = &basicfs_inode_ops;
    inode->i_fop = &basicfs_dir_operations;
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
}

static void basicfs_kill_superblock(struct super_block *s)
{
    printk(KERN_INFO "basicfs superblock is destroyed. Unmount succesful.\n");
    //we have no information in our superblock currently
    return;
}

//ramsfs_mount, called on file system mounting (duh)
struct dentry *basicfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
    //mount_nodev is for a virtual file system, or one that doesnt need a real device
    //return mount_nodev(fs_type, flags, data, ramfs_fill_super);

    //mount_bdev is for a file system with an actual block device
    struct dentry *ret;

    ret = mount_bdev(fs_type, flags, dev_name, data, basicfs_fill_super);

    if (unlikely(IS_ERR(ret)))
        printk(KERN_ERR "Error mounting basicfs");
    else
        printk(KERN_INFO "basicfs is succesfully mounted on [%s]\n",dev_name);

    return ret;
}

//file system structure
static struct file_system_type basicfs_type = {
        .name           = "basicfs",
        .mount          = basicfs_mount,
        .kill_sb        = basicfs_kill_superblock,
        .fs_flags       = FS_USERNS_MOUNT,
};

static int basicfs_init(void)
{
    int ret;

    //register filesystem
    ret = register_filesystem(&basicfs_type);
    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully registered basicfs\n");
    else
        printk(KERN_ERR "Failed to register basicfs. Error:[%d]", ret);

    return ret;
}

static void basicfs_exit(void)
{
    int ret;

    //unregister filesystem
    ret = unregister_filesystem(&basicfs_type);

    if (likely(ret == 0))
        printk(KERN_INFO "Sucessfully unregistered basicfs\n");
    else
        printk(KERN_ERR "Failed to unregister basicfs. Error:[%d]", ret);
}

module_init(basicfs_init);
module_exit(basicfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pasquale Caporaso <caporasopasquale97@gmail.com>");
MODULE_DESCRIPTION("BASIC_FS");