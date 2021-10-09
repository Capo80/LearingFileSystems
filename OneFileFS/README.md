# One File FS

## Disclaimer

This FS has been created for learning purposes and is objectively terrible, most of the stuff in here is outdated and should not be used.

Problems:
- using of normal write and read and not address space operations
- memory managment is a mess, no use of caches and probrably a lot of leaks
- file permissions do not work
- there is too little information in the inode and some of it is not updated properly
 
The objective of this FS was to understand how to link the operations and how to interact with the underlying block device, so i feel like the current state is a good stopping point.

## Description


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
- file write, writes in our only file

This FS has an actual superblock struct definition, with very little information because we don't do much.

Max size of the file is one block, it should be pretty simple to make the size expandible but i dont think i'll do it in this file system.

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
- cat Hitchhikers\ guide\ to\ the\ galaxy 
- echo "For instance, on the planet Earth, man had always assumed that he was more intelligent than dolphins because he had achieved so much—the wheel, New York, wars and so on—whilst all the dolphins had ever done was muck about in the water having a good time. But conversely, the dolphins had always believed that they were far more intelligent than man—for precisely the same reasons." >> Hitchhikers\ guide\ to\ the\ galaxy 
- cd ..
- umount ./mount/
- rmmod onefilefs.ko
