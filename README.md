# LearingFileSystems

A repositories containing the code i used to learn about FileSystem programming in the linux kernel

Every folder is a different filesystem, i am doing an incremental approach to make things simpler. Every folder contains a README with information on the code and how to run it.

Current FileSystems:
- MyFirstFS, the simplest, mount and umount, nothing else
- OneFileFS, a filesystem with a single file

All the file system are tested on a kernel linux 5.8.

## Resources

An implementation of every operation of a FS (file present in the linux kernel):

https://elixir.bootlin.com/linux/latest/source/fs/libfs.c

A simple FS implementation, can follow commits for incremental development (outdated): 

https://github.com/psankar/simplefs

Another simple FS implementation, a bit more modern:

https://github.com/jserv/simplefs

Quick, simple, hands-on explanation of VFS end FS

https://devarea.com/wp-content/uploads/2017/10/Linux-VFS-and-Block.pdf

Linux kernel documentation:

https://linux-kernel-labs.github.io/refs/heads/master/labs/filesystems_part1.html

https://linux-kernel-labs.github.io/refs/heads/master/labs/filesystems_part2.html
