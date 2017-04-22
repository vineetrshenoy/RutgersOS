// Naorin Hossain, Vasishta Kalinadhabhotta, Vineet Shenoy
// Tested on: adapter.cs.rutgers.edu

#ifndef TEST_T
#define TEST_T

#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


#define BLOCK_SIZE 512
#define BITS_PER_BLOCK 4096
#define BITS_PER_BYTE 8
#define INODES_PER_BLOCK 8
#define ZERO_INDEX_BITS 7
#define VALUE 513

typedef struct{

	int size;
	int direct_ptrs[12];
	int indirect_ptr;
	int test;
	short flags;
	

}inode;


typedef struct{

	inode list[8];
	

}inode_entry;


typedef struct{
	
	int dataregion_bitmap_blocks;
	int dataregion_bitmap_start;
	int inode_bitmap_blocks;
	int inode_bitmap_start;
	int inode_blocks;
	int inode_blocks_start;
	int dataregion_blocks;
	int dataregion_blocks_start;

}metadata_info;

int get_bitmap_info(int total_size, metadata_info * info);
int check_inode_status(int inode_number);
int set_inode_status(int inode_number, int status);
int check_dataregion_status(int datablock_number);
int set_dataregion_status(int datablock_number, int status);



#endif