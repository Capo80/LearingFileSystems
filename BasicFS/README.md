# Basic FS

This FS adds some operations to  MyFirstFS so that it actually does something.

Current operations:
- iterate, used to read a directory, can be called but it does nothing

We now have an actual superblock struct definition for this FS, with very little information because we don't do much.

Lastly i have added a simple makefs script to actually write some information permanently on the disk.

Current information created by makefs:
- The super block information
- Root inode

The FS will not not accept a mount on a disk that is not correctly set up with this information.

Still following older commits of: https://github.com/psankar/simplefs, but i am adapting the code for newer kernel versions

Create a file as a base for the filesystem and a directory for mounting
- dd bs=4096 count=100 if=/dev/zero of=image
- mkdir mount
- ./mkfs-simplefs image

Build the module and mount it in the directory (check dsemg to see it it works), can now ls and cd into it:
- make
- insmod basic_fs_src.ko
- mount -o loop -t basicfs image ./mount/
- cd mount/
- ls
- cd ..
- umount ./mount/
- rmmod basic_fs_src.ko
