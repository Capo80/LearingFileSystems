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

//no concurrent writes for now - only one file and one block anyway
static DEFINE_MUTEX(onefilefs_write_lock);
static DEFINE_MUTEX(onefilefs_inodes_lock);

// get an inode from its inode number
// currently we have only one inode, the root inode, which is in block 1, so we simply return that
// internal function
struct onefilefs_inode *onefilefs_get_inode(struct super_block *sb, uint64_t inode_no)
{
    struct onefilefs_sb_info *sfs_sb = sb->s_fs_info;
    struct onefilefs_inode *ofs_inode = NULL;
    struct onefilefs_inode *to_return = NULL;

    int i;
    struct buffer_head *bh;

    bh = (struct buffer_head *)sb_bread(sb, ONEFILEFS_INODES_BLOCK_NUMBER); // all of our 2 inodes are in here
    ofs_inode = (struct onefilefs_inode *) bh->b_data;

    //currently we have only 2 inodes in the block this is not that useful
    for (i=0; i < sfs_sb->inodes_count; i++) {
        if (ofs_inode->inode_no == inode_no) {
            to_return = kmalloc(sizeof(struct onefilefs_inode), GFP_KERNEL);
            memcpy(to_return, ofs_inode, sizeof(struct onefilefs_inode));
            brelse(bh);
            return to_return;
        }
        ofs_inode++;
    }

    return NULL;
}

ssize_t onefilefs_read(struct file * filp, char __user * buf, size_t len, loff_t * off)
{
    struct onefilefs_inode *inode = filp->f_inode->i_private;
    struct buffer_head *bh;
    uint64_t file_size = inode->file_size;
    char *buffer;
    int nbytes;

    //check that off is whithin boundaries
    if (*off >= file_size)
        return 0;
    else if (*off + len > file_size)
        len = file_size - *off;

    //read the block and copy to user (all our in inside here anyway)
    bh = (struct buffer_head *)sb_bread(filp->f_path.dentry->d_inode->i_sb, inode->data_block_number);
    buffer = (char *)bh->b_data;
    nbytes = min(strlen(buffer), len);

    if (copy_to_user(buf, buffer, nbytes)) {
        brelse(bh);
        printk(KERN_ERR "Error copying file contents to the userspace buffer\n");
        return -EFAULT;
    }

    brelse(bh);

    *off += nbytes;
    return nbytes;
}

//Currently we support only a write on the block already allocated
ssize_t onefilefs_write(struct file * filp, const char __user * buf, size_t len, loff_t * off)
{
    struct onefilefs_inode *ofs_inode = filp->f_inode->i_private;
    struct onefilefs_inode *device_inode;
    struct buffer_head *bh;
    struct super_block* sb = filp->f_inode->i_sb;
    char *buffer;

    //check that off is whithin boundaries of the block (so offset can go from 0 to BLOCK_SIZE)
    if (*off >= ONEFILEFS_DEFAULT_BLOCK_SIZE)
        return 0;
    else if (*off + len > ONEFILEFS_DEFAULT_BLOCK_SIZE)
        len = ONEFILEFS_DEFAULT_BLOCK_SIZE - *off;

    //check that starting offset is <= file size
    //not sure if this is the correct way to do this ??
    //should i even do it? or should i allow write in random places?
    if (*off > ofs_inode->file_size) {
        printk(KERN_ERR "Starting offset is outside file boundaries, pos [%lld], file size [%lld]\n", *off, ofs_inode->file_size);
        return 0;
    }

    //get write mutex
    if (mutex_lock_interruptible(&onefilefs_write_lock)) {
        printk(KERN_ERR "Failed to acquire mutex lock %s +%d\n", __FILE__, __LINE__);
        return 0;
    }

    printk(KERN_INFO "Starting write. pos[%lld], inode number[%llu], superblock magic [%lu], datablock number [%llu]\n", *off, ofs_inode->inode_no, sb->s_magic,  ofs_inode->data_block_number);


    //read the block, memcpy in change, mark block as dirty
    bh = (struct buffer_head *)sb_bread(sb, ofs_inode->data_block_number);
    buffer = (char *)bh->b_data;
    if (copy_from_user(buffer + *off, buf, len)) {
        brelse(bh);
        printk(KERN_ERR "Error copying file contents from the userspace buffer to the kernel space\n");
        return -EFAULT;
    }

    *off += len;
    
    //mark buffer as dirty - system will update it when ready
    mark_buffer_dirty(bh);

    //release mutex and memory
    mutex_unlock(&onefilefs_write_lock);
    brelse(bh);

    //not update inode file size if necessary
    if (*off > ofs_inode->file_size) {

        if (mutex_lock_interruptible(&onefilefs_inodes_lock)) {
            printk(KERN_ERR "Failed to acquire mutex lock %s +%d\n", __FILE__, __LINE__);
            return 0;
        }

        //load the block and save the new inode
        bh = (struct buffer_head *)sb_bread(sb, ONEFILEFS_INODES_BLOCK_NUMBER);
    
        device_inode = (struct onefilefs_inode*) bh->b_data;

        //we only have one file inode and its always in the same place so we don't need to iterate
        device_inode++;

        //size update here
        device_inode->file_size = *off;

        mark_buffer_dirty(bh);
        brelse(bh);
        mutex_unlock(&onefilefs_inodes_lock);

        printk(KERN_INFO "File inode size correctly updated\n");

    }
    
    return len;
}



//this function is called when the VFS wants to connect the child dentry to an inode
//currently we look in the parent inode for the file name (can maybe be changed to an id)
struct dentry *onefilefs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags)
{
    struct onefilefs_inode *parent = parent_inode->i_private;
    struct super_block *sb = parent_inode->i_sb;
    struct buffer_head *bh;
    struct onefilefs_dir_record *record;
    struct timespec64 curr_time;
    int i;

    //we never return a dentry currently, we should check if the dentry is already connected, if it is, we return it
    bh = (struct buffer_head *)sb_bread(sb, parent->data_block_number);
    record = (struct onefilefs_dir_record *) bh->b_data;
    for (i = 0; i < parent->dir_children_count; i++) {
        if (!strcmp(record->filename, child_dentry->d_name.name)) {

            //if its the same we connect the inode
            struct inode *inode;
            struct onefilefs_inode *ofs_inode;

            ofs_inode = onefilefs_get_inode(sb, record->inode_no);

            inode = new_inode(sb);
            inode->i_ino = record->inode_no;
            //inode_init_owner(inode, parent_inode, ofs_inode->mode);
            inode->i_mode = ofs_inode->mode;
            inode->i_sb = sb;
            inode->i_op = &onefilefs_inode_ops;
            
            //check inode type (we now have two, a file and a dir, very fancy)    
            if (S_ISDIR(inode->i_mode))
                inode->i_fop = &onefilefs_dir_operations;
            else if (S_ISREG(inode->i_mode))
                inode->i_fop = &onefilefs_file_operations;
            else
                printk(KERN_ERR "Unknown inode type. Neither a directory nor a file");

            /* FIXME: We should store these times to disk and retrieve them */
            ktime_get_real_ts64(&curr_time);
            inode->i_atime = inode->i_mtime = inode->i_ctime = curr_time;

            inode->i_private = ofs_inode;

            d_add(child_dentry, inode);
            kfree(ofs_inode);
            return NULL;
        }
    }

    return NULL;

}

//look up goes in the inode operations
const struct inode_operations onefilefs_inode_ops = {
    .lookup = onefilefs_lookup,
};

const struct file_operations onefilefs_file_operations = {
    .read = onefilefs_read,
    .write = onefilefs_write
};
