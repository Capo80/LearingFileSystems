#ifndef _ONEFILEFS_H
#define _ONEFILEFS_H

#include <linux/types.h>

#define ONEFILEFS_MAGIC 0x42424242
#define ONEFILEFS_DEFAULT_BLOCK_SIZE 4096
#define ONEFILEFS_FILENAME_MAXLEN 255

#define ONEFILEFS_SB_BLOCK_NUMBER 0

#define ONEFILEFS_ROOT_INODE_NUMBER 1
#define ONEFILEFS_FILE_INODE_NUMBER 2

#define ONEFILEFS_INODES_BLOCK_NUMBER 1
#define ONEFILEFS_ROOT_DATA_BLOCK_NUMBER 2
#define ONEFILEFS_FILE_DATA_BLOCK_NUMBER 3

//inode definition
struct onefilefs_inode {
	mode_t mode;
	uint64_t inode_no;
	uint64_t data_block_number;

	union {
		uint64_t file_size;
		uint64_t dir_children_count;
	};
};

//dir definition (how the dir datablock is organized)
// our dir block is filled with an array of theese (only one actually)
struct onefilefs_dir_record {
	char filename[ONEFILEFS_FILENAME_MAXLEN];
	uint64_t inode_no;
};


//superblock definition
struct onefilefs_sb_info {
	uint64_t version;
	uint64_t magic;
	uint64_t block_size;
	uint64_t inodes_count;
	uint64_t free_blocks;

	//padding to fit into a block
	char padding[ (4 * 1024) - (5 * sizeof(uint64_t))];
};


#endif