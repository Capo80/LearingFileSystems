#ifndef _BASICFS_H
#define _BASICFS_H

#include <linux/types.h>

#define BASICFS_MAGIC 0x42424242
#define BASICFS_DEFAULT_BLOCK_SIZE 4096
#define BASICFS_FILENAME_MAXLEN 255

#define BASICFS_ROOT_INODE_NUMBER 1
#define BASICFS_ROOTDIR_DATABLOCK_NUMBER 2

//inode definition
struct basicfs_inode {
	mode_t mode;
	uint32_t inode_no;
	uint32_t data_block_number;

	union {
		uint32_t file_size;
		uint32_t dir_children_count;
	};
};

//dir definition
struct basicfs_dir_record {
	char filename[BASICFS_FILENAME_MAXLEN];
	uint32_t inode_no;
};


//superblock definition
struct basicfs_sb_info {
	uint32_t version;
	uint32_t magic;
	uint32_t block_size;
	uint32_t free_blocks;

	struct basicfs_inode root_inode;

	//padding to fit into a block
	char padding[ (4 * 1024) - (4 * sizeof(uint32_t)) - sizeof(struct basicfs_inode)];
};


#endif