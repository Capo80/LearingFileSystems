#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "onefilefs.h"

//this script can be used to write a block directly into a device
//will use this to format the device for mounting the FileSystem
int main(int argc, char *argv[])
{
	int fd;
	ssize_t ret;
	struct onefilefs_sb_info sb;
	struct onefilefs_inode root_inode;
	
	if (argc != 2) {
		printf("Usage: mkfs-onefilefs <device>\n");
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("Error opening the device");
		return -1;
	}

	//the superblock data of out filesystem
	sb.version = 1;
	sb.magic = ONEFILEFS_MAGIC;
	sb.block_size = ONEFILEFS_DEFAULT_BLOCK_SIZE;
	sb.inodes_count = 1; //only one inode currently
	sb.free_blocks = ~0;

	ret = write(fd, (char *)&sb, sizeof(sb));

	/* Just a redundant check. Not required ideally. */
	if (ret != ONEFILEFS_DEFAULT_BLOCK_SIZE) {
		printf("bytes written [%d] are not equal to the default block size\n", (int)ret);
		close(fd);
		return ret;
	}

	printf("Super block written succesfully\n");

	//Create our root inode
	root_inode.mode = S_IFDIR;
	root_inode.inode_no = ONEFILEFS_ROOT_INODE_NUMBER;
	root_inode.data_block_number = ONEFILEFS_ROOTDIR_DATABLOCK_NUMBER;
	root_inode.dir_children_count = 0;

	ret = write(fd, (char *)&root_inode, sizeof(root_inode));

	if (ret != sizeof(root_inode)) {
		printf("The inode store was not written properly. Retry your mkfs\n");
		close(fd);
		return ret;
	}

	printf("inode store written succesfully\n");
	close(fd);

	return 0;
}