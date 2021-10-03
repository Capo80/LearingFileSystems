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
