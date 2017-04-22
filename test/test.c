#include "test.h"


char buffer[BLOCK_SIZE];
struct stat s;
metadata_info info; 
char * filepath = "/home/vshenoy/Rutgers/RutgersOS/test/fsfile";

extern int diskfile;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

// CURRENTLY WORKS ONLY FOR TOTAL SIZE, MULTIPLES OF 4 MB (8MB, 16MB,32MB, ETC)
/*
	This function initializes all the structure: 25% is for metadata, 75% for data
	

	INPUT: The total size of the file, metadata_info pointer to store metadata value
	OUTPUT: 0 on success

*/
int get_bitmap_info(int total_size, metadata_info * info){

	// ----------------------------------------------
	//This gets just the information for the data regions 

	int data_size = 0.75 * total_size; //75% of the total space will be used for the data region
	int data_blocks = data_size / BLOCK_SIZE;  //Gets the actual number of data blocks
	// See how many bitmap blocks are needed to address all the data blocks.
	//Each bitmap block can address BITS_PER_BLOCK blcoks
	int data_bitmap_blocks = data_blocks / BITS_PER_BLOCK; 
	info->dataregion_blocks = data_blocks;
	info->dataregion_bitmap_blocks = data_bitmap_blocks;

	//----------------------------------------------------

	int metadata_size = total_size - data_size; //25% of th total space for metadata
	int num_metadata_blocks = metadata_size / (BLOCK_SIZE); //Number of blocks based on size
	num_metadata_blocks = num_metadata_blocks - data_bitmap_blocks - 1; // Total blocks minus data_bitmap_blocks minus superblock

	int inode_bitmap = ceil((double) num_metadata_blocks / VALUE); //gets the number of inodes. See documentation
	num_metadata_blocks = num_metadata_blocks - inode_bitmap;

	info->inode_blocks = num_metadata_blocks;
	info->inode_bitmap_blocks = inode_bitmap;
	

	info->dataregion_bitmap_start = 1;
	info->inode_bitmap_start = 1 + info->dataregion_bitmap_blocks;

	info->inode_blocks_start = 1 + info->dataregion_bitmap_blocks + info->inode_bitmap_blocks;
	info->dataregion_blocks_start = 1 + info->dataregion_bitmap_blocks + info->inode_bitmap_blocks + info->inode_blocks;


	return 0;

}




/*
	Checks the status of a specific inode. Returns this value
	

	INPUT: The inode number to check
	OUTPUT: The status (1 allocated, 0 unallocated)

*/
int check_inode_status(int inode_number){

	int blk_number = inode_number/(BITS_PER_BLOCK);	//Finds out in which block bit is
	block_read(info.inode_bitmap_start + blk_number, buffer);	//reads the block

	int byte_offset = (inode_number - (BITS_PER_BLOCK * blk_number)) / BITS_PER_BYTE; //Finds how many bytes from begining that specific bit is
	char * ptr = buffer + byte_offset; 
	char data_bits = *ptr; //Obtains 8 bits in which desired bit is contained

	int bit_offset = inode_number - (BITS_PER_BLOCK * blk_number) - (byte_offset * BITS_PER_BYTE); //where in the 8 bits the desired is lcoated
	int bit = ZERO_INDEX_BITS - bit_offset;	//counting offset from the MSB
	

	bit = (data_bits & ( 1 << bit )) >> bit; //Gets the required bit
		
	return bit;
}


/*
	Sets the status of a specific inode. Returns this value
	

	INPUT: The inode number to set, the value to set it to
	OUTPUT: The status (1 allocated, 0 unallocated)

*/
int set_inode_status(int inode_number, int status){

	int blk_number = inode_number/(BITS_PER_BLOCK);	//Finds out in which block bit is
	block_read(info.inode_bitmap_start + blk_number, buffer);	//reads the block

	int byte_offset = (inode_number - (BITS_PER_BLOCK * blk_number)) / BITS_PER_BYTE;//Finds how many bytes from begining that specific bit is
	char * ptr = buffer + byte_offset;
	char data_bits = *ptr; //Obtains 8 bits in which desired bit is contained

	int bit_offset = inode_number - (BITS_PER_BLOCK * blk_number) - (byte_offset * BITS_PER_BYTE); //where in the 8 bits the desired is lcoated
	int bit = ZERO_INDEX_BITS - bit_offset;

	data_bits ^= (-status ^ data_bits) & (1 << bit); //sets the required bit
	*ptr = data_bits;	//puts set of 8 bits back into buffer
	block_write(info.inode_bitmap_start + blk_number, buffer); //writes block back to file
	return 0;

}



/*
	Checks the status of a specific inode. Returns this value
	

	INPUT: The inode number to check
	OUTPUT: The status (1 allocated, 0 unallocated)

*/
int check_dataregion_status(int datablock_number){

	int blk_number = datablock_number/(BITS_PER_BLOCK);	//Finds out in which block bit is
	block_read(info.dataregion_bitmap_start + blk_number, buffer);	//reads the block

	int byte_offset = (datablock_number - (BITS_PER_BLOCK * blk_number)) / BITS_PER_BYTE; //Finds how many bytes from begining that specific bit is
	char * ptr = buffer + byte_offset; 
	char data_bits = *ptr; //Obtains 8 bits in which desired bit is contained

	int bit_offset = datablock_number - (BITS_PER_BLOCK * blk_number) - (byte_offset * BITS_PER_BYTE); //where in the 8 bits the desired is lcoated
	int bit = ZERO_INDEX_BITS - bit_offset;	//counting offset from the MSB
	

	bit = (data_bits & ( 1 << bit )) >> bit; //Gets the required bit
		
	return bit;
}


/*
	Sets the status of a specific inode. Returns this value
	

	INPUT: The inode number to set, the value to set it to
	OUTPUT: The status (1 allocated, 0 unallocated)

*/
int set_dataregion_status(int datablock_number, int status){

	int blk_number = datablock_number/(BITS_PER_BLOCK);	//Finds out in which block bit is
	block_read(info.dataregion_bitmap_start + blk_number, buffer);	//reads the block

	int byte_offset = (datablock_number - (BITS_PER_BLOCK * blk_number)) / BITS_PER_BYTE;//Finds how many bytes from begining that specific bit is
	char * ptr = buffer + byte_offset;
	char data_bits = *ptr; //Obtains 8 bits in which desired bit is contained

	int bit_offset = datablock_number - (BITS_PER_BLOCK * blk_number) - (byte_offset * BITS_PER_BYTE); //where in the 8 bits the desired is lcoated
	int bit = ZERO_INDEX_BITS - bit_offset;

	data_bits ^= (-status ^ data_bits) & (1 << bit); //sets the required bit
	*ptr = data_bits;	//puts set of 8 bits back into buffer
	block_write(info.dataregion_bitmap_start + blk_number, buffer); //writes block back to file
	return 0;

}







int main(){	
	
	
	inode_entry entry;
	int i, count;
	count = 0;
	entry.list[0].size = 9999;
	int entry_size = sizeof(entry);
	printf("The inode entry size is %d \n", entry_size);
	

	//printf("About to open disk\n");
	disk_open(filepath);
	
	//printf("The disk file is %d\n", diskfile);

	fstat(diskfile, &s);
	int size = s.st_size;
	

	printf("The disk size is %d\n", size);

	get_bitmap_info(size, &info);

	char * ptr = buffer;
	for (i = 0; i < 512; i++){
		*ptr = 0xAA;
		ptr = ptr + 1;
	}

	printf("The number of inode bitmap blocks is %d\n", info.inode_bitmap_blocks);
	printf("The number of inode blocks is %d\n", info.inode_blocks);
	printf("The number of data region bitmap blocks is %d\n", info.dataregion_bitmap_blocks);
	printf("The number of data region blocks is %d\n", info.dataregion_blocks);
	printf("\n");
	printf("\n");
	printf("\n");

	printf("Writing the superblock\n");
	block_write(0, buffer);
	count++;

	printf("Writing the data bitmap\n");
	for (i = 0; i < info.dataregion_bitmap_blocks; i++){
		block_write(count, buffer);
		count++;
	}

	printf("Writing the inode bitmap\n");
	for (i = 0; i < info.inode_bitmap_blocks; i++){
		block_write(count, buffer);
		count++;
	}

	printf("Writing the inode blocks\n");
	for (i = 0; i < info.inode_blocks; i++){
		block_write(count, entry);
		count++;
	}


	block_read(8, buffer);
	//printf("The size is %d\n", entry.list[0].size);

	printf("------------------------------\n");

	int x = check_dataregion_status(3001);
	int y = check_dataregion_status(3002);
	int z = check_dataregion_status(3003);
	
	printf("The data region status before is: %d and %d and %d\n", x, y, z);

	
	set_dataregion_status(3001, 1);
	set_dataregion_status(3002, 0);
	set_dataregion_status(3003, 1);


	x = check_dataregion_status(3001);
	y = check_dataregion_status(3002);
	z = check_dataregion_status(3003);
	
	printf("The dataregion status after is: %d and %d and %d\n", x, y, z);
	

	printf("About to close disk\n");
	disk_close(filepath);
	

	//char * ptr = buffer;
	//printf("The character is %c\n", *ptr);
	/*
	char byte;
	byte = 0xAA;
	printf("\n"BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
	printf("\n");
	int k = 7;


	int a = (byte & ( 1 << k )) >> k;
	printf("The bit %dth bit is %d\n", k, a);
	/*
	int x = 1;
	int n = 4;
	byte ^= (-x ^ byte) & (1 << n);
	printf("\n"BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(byte));
	//number ^= (-x ^ number) & (1 << n);
	*/
	
}