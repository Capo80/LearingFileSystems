# First FS

This FS is the basics of the basics, can only be mounted and unmounted, no operations implemented

There is no makefs, can mount this on any device.

Pretty much copied from an older commit of: https://github.com/psankar/simplefs

Create a file as a base for the filesystem and a directory for mounting
- dd bs=1M count=100 if=/dev/zero of=image
- mkdir mount

Build the module and mount it in the directory (check dsemg to see it it works):
- make
- insmod my_first_fs.ko
- mount -o loop -t firstfs image ./mount/
- umount ./mount/
- rmmod my_first_fs.ko
