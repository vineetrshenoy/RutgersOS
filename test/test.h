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
	

}inode_block;


typedef struct{
	
	int dataregion_bitmap_blocks; //How many block needed for dataregion bitmap
	int dataregion_bitmap_start; // Which block does the dataregion bitmap start
	int inode_bitmap_blocks;	//How many blocks needed for inode bitmap
	int inode_bitmap_start;	//which block does the inode bitmap start
	int inode_blocks;	//How many blocks needed for inodes
	int inode_blocks_start;	//which block does the inodes start
	int total_inodes;
	int dataregion_blocks;	//How many data region blocks are needed
	int dataregion_blocks_start; //what block does the data region start
	int disksize;
	int random[6];

}metadata_info;



typedef struct{
	
	metadata_info list[8];

}super_block;



int get_metadata_info(int total_size, metadata_info * info);
int check_inode_status(int inode_number);
int set_inode_status(int inode_number, int status);
int check_dataregion_status(int datablock_number);
int set_dataregion_status(int datablock_number, int status);
inode get_inode(int inode_number);
void set_inode(int inode_number, inode node);


#endif