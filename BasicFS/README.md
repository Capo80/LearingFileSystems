# Basic FS

This FS adds some operations to the basic FS so that it actually does something

Current operations:
- iterate, used to read a directory, can be called but it does nothing

Still following older commits of: https://github.com/psankar/simplefs, but i am adapting the code for newer kernel versions

Create a file as a base for the filesystem and a directory for mounting
- dd bs=1M count=100 if=/dev/zero of=image
- mkdir mount

Build the module and mount it in the directory (check dsemg to see it it works), can now ls and cd into it:
- make
- insmod basic_fs_src.ko
- mount -o loop -t basicfs image ./mount/
- cd mount/
- ls
- cd ..
- umount ./mount/
- rmmod basic_fs_src.ko
