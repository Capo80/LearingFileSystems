#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "onefilefs.h"

/*
	This makefs will write the following information onto the disk
	- BLOCK 0, superblock;
	- BLOCK 1, inodes of root dir and the only file;
	- BLOCK 2, datablock of root dir
	- BLOCK 3, datablock of the only file
*/
int main(int argc, char *argv[])
{
	int fd, nbytes;
	ssize_t ret;
	struct onefilefs_sb_info sb;
	struct onefilefs_inode root_inode;
	struct onefilefs_inode file_inode;
	struct onefilefs_dir_record record;
	char *block_padding;
	char file_name[] = "Hitchhikers guide to the galaxy";
	char file_body[] = "In the beginning the Universe was created. This has made a lot of people very angry and been widely regarded as a bad move.\n";

	if (argc != 2) {
		printf("Usage: mkfs-onefilefs <device>\n");
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("Error opening the device");
		return -1;
	}

	//write superblock
	sb.version = 1;
	sb.magic = ONEFILEFS_MAGIC;
	sb.block_size = ONEFILEFS_DEFAULT_BLOCK_SIZE;
	sb.inodes_count = 2; //the root and the file
	sb.free_blocks = ~0;

	ret = write(fd, (char *)&sb, sizeof(sb));

	/* Just a redundant check. Not required ideally. */
	if (ret != ONEFILEFS_DEFAULT_BLOCK_SIZE) {
		printf("bytes written [%d] are not equal to the default block size\n", (int)ret);
		close(fd);
		return ret;
	}

	printf("Super block written succesfully\n");

	//Write root inode
	root_inode.mode = S_IFDIR;
	root_inode.inode_no = ONEFILEFS_ROOT_INODE_NUMBER;
	root_inode.data_block_number = ONEFILEFS_ROOT_DATA_BLOCK_NUMBER;
	root_inode.dir_children_count = 1; //our only file

	ret = write(fd, (char *)&root_inode, sizeof(root_inode));

	if (ret != sizeof(root_inode)) {
		printf("The inode store was not written properly. Retry your mkfs\n");
		close(fd);
		return ret;
	}
	printf("root inode written succesfully\n");

	// write file inode
	file_inode.mode = S_IFREG;
	file_inode.inode_no = ONEFILEFS_FILE_INODE_NUMBER;
	file_inode.data_block_number = ONEFILEFS_FILE_DATA_BLOCK_NUMBER;
	file_inode.file_size = sizeof(file_body);
	file_inode.mode = 0777;
	ret = write(fd, (char *)&file_inode, sizeof(file_inode));

	if (ret != sizeof(root_inode)) {
		printf("The file inode was not written properly. Retry your mkfs\n");
		close(fd);
		return -1;
	}

	printf("file inode written succesfully\n");
	
	//padding for block 1
	nbytes = ONEFILEFS_DEFAULT_BLOCK_SIZE - sizeof(root_inode) - sizeof(file_inode);
	block_padding = malloc(nbytes);

	ret = write(fd, block_padding, nbytes);

	if (ret != nbytes) {
		printf("The padding bytes are not written properly. Retry your mkfs\n");
		close(fd);
		return -1;
	}
	printf("padding in the inode block written sucessfully\n");

	//write dir datablock
	strcpy(record.filename, file_name);
	record.inode_no = ONEFILEFS_FILE_INODE_NUMBER;
	nbytes = sizeof(record);

	ret = write(fd, (char *)&record, nbytes);
	if (ret != nbytes) {
		printf("Writing the rootdirectory datablock (name+inode_no pair for welcomefile) has failed\n");
		close(fd);
		return -1;
	}

	printf("root directory datablock written succesfully\n");

	//padding for block 2
	nbytes = ONEFILEFS_DEFAULT_BLOCK_SIZE - sizeof(record);
	block_padding = realloc(block_padding, nbytes);

	ret = write(fd, block_padding, nbytes);
	free(block_padding);

	if (ret != nbytes) {
		printf("Writing the padding for rootdirectory children datablock has failed\n");
		close(fd);
		return -1;
	}
	printf("padding after the rootdirectory datablock written succesfully\n");

	//write file datablock
	nbytes = sizeof(file_body);
	ret = write(fd, file_body, nbytes);
	if (ret != nbytes) {
		printf("Writing file datablock has failed\n");
		close(fd);
		return -1;
	}
	printf("file datablock has been written succesfully\n");

	close(fd);

	return 0;
}