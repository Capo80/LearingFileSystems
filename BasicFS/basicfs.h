#ifndef _BASICFS_H
#define _BASICFS_H

#include <linux/types.h>

#define BASICFS_MAGIC 0x42424242
#define BASICFS_DEFAULT_BLOCK_SIZE 4096
#define BASICFS_FILENAME_MAXLEN 255

#define BASICFS_SB_BLOCK_NUMBER 0
#define BASICFS_FILE_BLOCK_NUMBER 1
#define BASICFS_ROOT_INODE_NUMBER 1
#define BASICFS_ROOTDIR_DATABLOCK_NUMBER 2

//inode definition
struct basicfs_inode {
	mode_t mode;
	uint64_t inode_no;
	uint64_t data_block_number;

	union {
		uint64_t file_size;
		uint64_t dir_children_count;
	};
};

//dir definition
struct basicfs_dir_record {
	char filename[BASICFS_FILENAME_MAXLEN];
	uint64_t inode_no;
};


//superblock definition
struct basicfs_sb_info {
	uint64_t version;
	uint64_t magic;
	uint64_t block_size;
	uint64_t inodes_count;
	uint64_t free_blocks;

	//padding to fit into a block
	char padding[ (4 * 1024) - (5 * sizeof(uint64_t))];
};


#endif