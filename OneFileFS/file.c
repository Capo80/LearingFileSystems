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
    struct onefilefs_inode *inode = filp->f_inode->i_private;
    struct buffer_head *bh;
    char *buffer;

    //check that off is whithin boundaries of the block (so offset can go from 0 to BLOCK_SIZE)
    if (*off >= ONEFILEFS_DEFAULT_BLOCK_SIZE)
        return 0;
    else if (*off + len > ONEFILEFS_DEFAULT_BLOCK_SIZE)
        len = ONEFILEFS_DEFAULT_BLOCK_SIZE - *off;

    //get write mutex
    if (mutex_lock_interruptible(&onefilefs_write_lock)) {
        printk(KERN_ERR "Failed to acquire mutex lock %s +%d\n", __FILE__, __LINE__);
        return 0;
    }
    //read the block, memcpy in change, mark block as dirty
    bh = (struct buffer_head *)sb_bread(filp->f_path.dentry->d_inode->i_sb, inode->data_block_number);
    buffer = (char *)bh->b_data;
    memcpy(buffer + *off, buf, len);

    *off += len;
    
    //mark buffer as dirty - system will update it when ready
    mark_buffer_dirty(bh);

    //release mutex and memory
    brelse(bh);
    mutex_unlock(&onefilefs_write_lock);
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
            struct onefilefs_inode *sfs_inode;

            sfs_inode = onefilefs_get_inode(sb, record->inode_no);

            inode = new_inode(sb);
            inode->i_ino = record->inode_no;
            inode_init_owner(inode, parent_inode, sfs_inode->mode);
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

            inode->i_private = sfs_inode;

            d_add(child_dentry, inode);
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
