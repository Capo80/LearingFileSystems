# One File FS

This FS contains a single file, the block organization is the following.

```
---------------------------------------------------
| Block 0    | Block 1    | Block 2   | Block 3   |
| Superblock | Root inode | Root data | File data |
|            | File inode |           |           |
---------------------------------------------------
```

Current operations:
- iterate, used to read a directory, it reads the information of the root dir, which has only one children, the file
- lookup, connect a dentry to an inode (this is used by ls to read the file information)
- file read, reads our only file

This FS has an actual superblock struct definition, with very little information because we don't do much.

There is also a simple makefs script to to format a device for this filesystem.

Current information created by makefs:
- The super block information
- Root inode
- File inode
- File contents

The FS will not not accept a mount on a disk that is not correctly set up with the superblock information (the rest can be omitted).

Still following older commits of: https://github.com/psankar/simplefs, but i am adapting the code for newer kernel versions

Create a file as a base for the filesystem and a directory for mounting
- dd bs=4096 count=100 if=/dev/zero of=image
- mkdir mount
- ./onefilemakefs image

Build the module and mount it in the directory (check dsemg to see it it works), can now ls and cd into it:
- make
- insmod onefilefs.ko
- mount -o loop -t onefilefs image ./mount/
- cd mount/
- ls
- cd ..
- umount ./mount/
- rmmod onefilefs.ko
