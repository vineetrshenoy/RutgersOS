#include "test.h"


char buffer[BLOCK_SIZE];
inode_block entry_buffer;
struct stat s;
metadata_info info; 
char * path = "/home/vshenoy/Rutgers/RutgersOS/test/fsfile";

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
int get_metadata_info(int total_size, metadata_info * info){

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

	int inode_bitmap =  num_metadata_blocks / VALUE; //gets the number of inodes. See documentation
	  if (inode_bitmap % VALUE != 0){
	    inode_bitmap++;
	  }
	num_metadata_blocks = num_metadata_blocks - inode_bitmap;



	info->disksize = total_size;
	info->inode_blocks = num_metadata_blocks;
	info->inode_bitmap_blocks = inode_bitmap;
	
	info->total_inodes = info->inode_blocks * INODES_PER_BLOCK;

	info->dataregion_bitmap_start = 1;
	info->inode_bitmap_start = 1 + info->dataregion_bitmap_blocks;

	info->inode_blocks_start = 1 + info->dataregion_bitmap_blocks + info->inode_bitmap_blocks;
	info->dataregion_blocks_start = 1 + info->dataregion_bitmap_blocks + info->inode_bitmap_blocks + info->inode_blocks;
	info->rootdirectory = 0;

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

/*
	Gets a copy of the specified inode at returns it to the user
	INPUT: The inode number that is requested
	OUTPUT: A struct containing the inode
*/

inode get_inode(int inode_number){
		
	inode node;
	int blk_number = inode_number / INODES_PER_BLOCK; // Finds which block to read
	block_read(info.inode_blocks_start + blk_number, &entry_buffer); // Reads the block
	
	
	int offset = inode_number - (INODES_PER_BLOCK * blk_number);
	node = (entry_buffer.list[offset]);
	return node;
	
}

/*
	Sets a certain inode in the metadata region
	INPUT: The inode number to write to, the inode itself
	OUTPUT: none
*/
void set_inode(int inode_number, inode node){

	int blk_number = inode_number / INODES_PER_BLOCK; // Finds which block to read
	block_read(info.inode_blocks_start + blk_number, &entry_buffer); // Reads the block
	
	
	int offset = inode_number - (INODES_PER_BLOCK * blk_number);
	entry_buffer.list[offset] = node;
	block_write(info.inode_blocks_start + blk_number, &entry_buffer);
	
}


int sfs_init(){

	inode node;
	inode_block block;
	super_block sblock;
	int count;

	disk_open(path);
	count = 0;

	//clearing all fields for the node
	int i;	
	node.size = 0;
	node.indirect_ptr = 0;
	node.test = 0;
	node.flags = 0;
	

	for(i = 0; i < 12; i++){
		node.direct_ptrs[i] = 0;
	}

	//clearing all fields for the inode_entry
	for(i = 0; i < 8; i++){
		block.list[i] = node;
	}


	fstat(diskfile, &s); //get file information
	get_metadata_info(s.st_size, &info);

	sblock.list[0] = info;

	//printf("Writing the superblock\n");
	block_write(count, &sblock);
	count++;

	//printf("Writing the data bitmap\n");
	for (i = 0; i < info.dataregion_bitmap_blocks; i++){
		block_write(count, buffer);
		count++;
	}

	//printf("Writing the inode bitmap\n");
	for (i = 0; i < info.inode_bitmap_blocks; i++){
		block_write(count, buffer);
		count++;
	}

	//printf("Writing the inode blocks\n");
	for (i = 0; i < info.inode_blocks; i++){
		block_write(count, &block);
		count++;
	}

	//setting root
	filepath_block fblock;

	strcpy(fblock.filepath, "/");	//set the name to "/"
	fblock.inode = 0;
	inode firstNode = get_inode(0); //gets the inode
	firstNode.size = 12 * BLOCK_SIZE; //sets the size
	firstNode.flags = 1;	//sets the flags
	set_inode(0,firstNode); //puts inode back in
	set_inode_status(0,1); //sets inode status to allocated
	set_dataregion_status(0, 1); //sets dataregion status to allocated
	block_write(info.dataregion_blocks_start, &fblock); //writes the block

}

// Get number of directories in a filepath

int get_num_dirs(const char * path){
  int length = strlen(path);
  char slash = 47;
  int i, count;
  count = 0;
  for (i = 0; i < length; i++){
    if (path[i] == slash)
      count++;
  }
  return count;
}

// Parse a path

char ** parsePath(const char * path){

  int length = strlen(path);
  int i, count;
  char slash = 47;
  count = 0;

  //Figures out how many "/" are present -- stores in count
  for (i = 0; i < length; i++){
    if (path[i] == slash)
      count++;
  }
  

  int * indices = (int * ) calloc ((count + 1), sizeof(int)); //stores the index of each slash
  int j = 0;
  for (i = 0; i < length; i++){

    if (path[i] == slash){
      indices[j] = i;
      j++;
    }

    
  }
  indices[count] = length; 

  char ** strings = (char **) calloc(count , sizeof(char *)); //MUST FREE THIS LATER

  for (i = 0; i < count; i++){
    int size = (indices[i + 1] - indices[i]);
    strings[i] = (char *) calloc(size, sizeof(char));
    strncpy(strings[i], (path + indices[i] + 1), (size - 1));
    
  }
  free(indices);
  
  return strings;

}
/*	
// Get inode number given a path.
int findInode(const char *path) {

  super_block sblock;
  block_read(0, &sblock);
  int dataRegionOffset = sblock.list[0].dataregion_blocks_start;
  filepath_block rblock;
  block_read(dataRegionOffset, &rblock);
  int inodeNum = rblock.inode;
  int inodeBlock = ceil(inodeNum/8);
  int inodeOffset = inodeNum - (inodeBlock - 1)*8;
  int numOfDirs = get_num_dirs(path);
  char ** fldrs = parsePath(path);
  inode_block iblock;
  filepath_block fblock;
  int i, j, gotem;
  inode node;
  // go through each inode of each folder in the path to find the inode for the path
  for (i = 0; i < numOfDirs; i++) {
    block_read(inodeBlock, &iblock);
    node = iblock.list[inodeOffset];
    gotem = 0;
    // check each direct_ptr in inode until the correct folder is found
    for (j = 0; j < 12; j++) {
      block_read(node.direct_ptrs[j], &fblock);
      if (strcmp(fblock.filepath, fldrs[i]) == 0) {
        gotem = 1;
        break;
      }
    }
    if (gotem == 1) {
      inodeNum = fblock.inode;
      if (i == numOfDirs - 1) {
        break;
      }
      inodeBlock = ceil(inodeNum/8);
      inodeOffset = inodeNum - (inodeBlock - 1)*8;
    }
    else {
      // do indirect ptr stuff
    }
  }
  for (i = 0; i < numOfDirs; i++) {
    free(fldrs[i]);
  }
  return inodeNum;
}
*/
/*
int sfs_getattr(struct stat *statbuf) {
	int retstat = 0;
    char fpath[PATH_MAX];
    
   log_msg("\nsfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
	  path, statbuf);

    memset(statbuf, 0, sizeof(struct stat)); // initialize buffer
    statbuf->st_dev = 0;
    statbuf->st_blksize = 0;
    statbuf->st_ino = 0;
    memset(&fpath, '?', PATH_MAX);  // initialize fpath before copying path into it so the final character can be found
    int i = 0;
    while (path[i]) {
    	fpath[i] = path[i];
    	i++;
    }
    char finalChar;
    for (i = 0; i < PATH_MAX; i++) {
      if (fpath[i] == '?') {
        finalChar = fpath[i - 1];
        break;
      }
    }

    if (strcmp(path, "/") == 0 || strcmp(finalChar, "/") == 0) {
      statbuf->st_mode = S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
      statbuf->st_nlink = 2;
    }
    else {
      statbuf->st_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
      statbuf->st_nlink = 1;
    }

    statbuf->st_uid = getuid();
    statbuf->st_gid = getgid();
    statbuf->st_rdev = 0;
    super_block sblock;
    block_read(0, &sblock);
    int inodeNum = findInode(path);
    int inodeBlockOffset = sblock.list[0].inode_blocks_start;
    int inodeBlock = inodeNum/8;
    int inodeOffset = inodeNum - inodeBlock*8;
    inode_block iblock;
    block_read(inodeBlockOffset + inodeBlock, &iblock);
    statbuf->st_size = iblock.list[inodeOffset].size;
    statbuf->st_blocks = ceil(statbuf->st_size/BLOCK_SIZE);
    statbuf->st_atime = time(NULL);
    statbuf->st_mtime = time(NULL);
    statbuf->st_ctime = time(NULL);
    
    return retstat;
}
*/
/*
int sfs_readdir(void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    
    int pathInodeNum = findInode(path);
    inode pathInode = get_inode(pathInodeNum);
    int i = 0;
    directory_block dblock;
    int fillerReturn;
    while (pathInode.direct_ptrs[i]) {
      block_read(pathInode.direct_ptrs[i], &dblock);
      fillerReturn = filler(buf, dblock.list[0].d_name, NULL, sizeof(struct dirent));
      if (fillerReturn != 0) {
        return retstat;
      }
    }
    
    return retstat;
}
*/
int find_free_datablock(){
	char data_buffer[BLOCK_SIZE];
	int i, j;
	int totalDatablocks = info.dataregion_blocks;
	for (i = 0; i < totalDatablocks; i++){
		if (check_dataregion_status(i) == 0)
			return i;
		
	}

	return -1;
}

int find_free_inode(){
	int i;
	int totalInodes = info.total_inodes;
	for (i = 0; i < totalInodes; i++){
		if (check_inode_status(i) == 0)
			return i;
		
	}

	return -1;
}

int testmain(){

	/*
	int i;
	int x = get_num_dirs(path);
	char ** fldrs = parsePath(path);
	int y = strlen(fldrs[1]);
	printf("length is %d\n", y);
	for (i = 0; i < x; i++)
		printf("The path is %s\n", fldrs[i]);
	
	*/
	

	/*
	char * string = "hello";
	char test[500] = "hello";
	int x = strcmp(string, test);
	printf("The string is %s\n", test);
	printf("The string is %s\n", string);
	printf("strcmp is %d\n", x);
	*/



	/*
	inode nodeOne, nodeTwo, nodeThree;
	inode_block entry;
	super_block sblock;
	super_block sblock2;
	int i, count;
	count = 0;
	int metasize = sizeof(info);
	sfs_init();
	for (i = 0; i < 8; i++){
		entry.list[i].size = 1010;
		entry.list[i].indirect_ptr = 999;
	}


	int entry_size = sizeof(info);
	printf("The inode entry size is %d \n", entry_size);
	

	//printf("About to open disk\n");
	disk_open(path);
	
	//printf("The disk file is %d\n", diskfile);

	fstat(diskfile, &s);
	int size = s.st_size;
	

	printf("The disk size is %d\n", size);

	get_metadata_info(size, &info);

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

	sblock.list[0] = info;

	printf("Writing the superblock\n");
	block_write(0, &sblock);
	count++;

	block_read(0, &sblock2);


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
		block_write(count, &entry);
		count++;
	}


	//block_read(8, buffer);
	//printf("The size is %d\n", entry.list[0].size);

	printf("------------------------------\n");

	int a = check_inode_status(2500);
	int b = check_inode_status(2501);
	int c = check_inode_status(2502);
	
	printf("The inode region status before is: %d and %d and %d\n", a, b, c);

	
	set_inode_status(2500, 0);
	set_inode_status(2501, 1);
	set_inode_status(2502, 0);


	a = check_inode_status(2500);
	b = check_inode_status(2501);
	c = check_inode_status(2502);
	
	printf("The inode region status before is: %d and %d and %d\n", a, b, c);


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
	
	printf("\n");
	printf("\n");
	printf("\n");

	nodeOne = get_inode(0);
	nodeTwo = get_inode(5000);
	nodeThree = get_inode(10000);

	printf("The old size of inodes 0, 5000, and 10000 are %d, %d, %d\n", nodeOne.size, nodeTwo.size, nodeThree.size);

	nodeOne.size = 9999;
	nodeTwo.size = 8888;
	nodeThree.size = 7777;

	set_inode(0, nodeOne);
	set_inode(5000, nodeTwo);
	set_inode(10000,nodeThree);

	nodeOne = get_inode(0);
	nodeTwo = get_inode(5000);
	nodeThree = get_inode(10000);
	
	printf("The new size of inodes 0, 5000, and 10000 are %d, %d, %d\n", nodeOne.size, nodeTwo.size, nodeThree.size);

	//printf("size of dirent: %d\n", sizeof(struct dirent));
	int bit = find_free_inode();

	printf("About to close disk\n");
	disk_close(path);
	

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



filepath_block find_path_block(char * path){

	inode node;
	filepath_block path_block, block_return;

	int i,j, notfound;

	int numOfDirs = get_num_dirs(path); //Gets the number of directories
	char ** fldrs = parsePath(path);  //Gets the strings for all paths
	block_read(info.dataregion_blocks_start, &block_return);
	i = 0;
	//while we still have directories to search
	while (i < numOfDirs){

		node = get_inode(block_return.inode); //get the inode of the directory
		

		char * searchFolder = fldrs[i]; //The current folder we are looking for
		notfound = 0;
		//search all direct pointers in our inode
		for (j = 0; j < 12; j++){

			//we should always skip 0 (this is root)
			if (node.direct_ptrs[j] != 0){
				block_read(node.direct_ptrs[j], &path_block); //read the block
				//compare the this folder with searchFolder; if match, get next node
				if (strcmp (searchFolder, path_block.filepath) == 0){
					block_return = path_block;
					i++;
					break;
				}
			}
			notfound++;
		}

		if (notfound == 12){
			//Free all ptrs
			for(i = 0; i < numOfDirs; i++)
				free(fldrs[i]);
			free(fldrs);

			return block_return;
		}

	}

	//Free all ptrs
	for(i = 0; i < numOfDirs; i++)
		free(fldrs[i]);
	free(fldrs);
	return block_return;
}


int create(char * path){

	filepath_block block, fblock;

	int numOfDirs = get_num_dirs(path); //
	char ** fldrs = parsePath(path); //

	block = find_path_block(path); //try to find the path
	if (strcmp(fldrs[numOfDirs -1], block.filepath) == 0){
		//the file exists
		printf("already exists\n");
		return -1;
	}

	//find a free inode
	int newInodeNum, newDataBlock;
	
	newInodeNum = find_free_inode();
	set_inode_status(newInodeNum, 1);
	

	newDataBlock = find_free_datablock();
	set_dataregion_status(newDataBlock, 1);	//set dataregion to allocated
	strcpy(fblock.filepath, fldrs[numOfDirs - 1]); //copy the new path name to block
	fblock.inode = newInodeNum; //associate data entry with inode field


	//finding a directptr for new block
	int i, ptr;

	inode node = get_inode(block.inode);
	for (i = 0; i < 12; i++){
		if (node.direct_ptrs[i] == 0){
			ptr = i;
			break;
		}
	}

	node.direct_ptrs[ptr] = info.dataregion_blocks_start + newDataBlock;
	set_inode(block.inode, node);

	//freeing all the ptrs from before
	for(i = 0; i < numOfDirs; i++)
		free(fldrs[i]);
	free(fldrs);

	block_write(info.dataregion_blocks_start + newDataBlock, &fblock);	//write the block to disk

	
	return 0;
}

int main(){	


	
	
	
	
	filepath_block test_filepath;
	filepath_block other;
	char * string = "home";
	//printf("String is %s\n", value);
	//printf("String is %s\n", test);

	sfs_init();
	
	
	strcpy(test_filepath.filepath, "home");
	test_filepath.inode = 1;
	set_inode_status(1,1);
	block_write(info.dataregion_blocks_start + 1, &test_filepath);
	set_dataregion_status(1, 1);

	strcpy(test_filepath.filepath, "vshenoy");
	test_filepath.inode = 2;
	set_inode_status(2,1);
	block_write(info.dataregion_blocks_start + 2, &test_filepath);
	set_dataregion_status(2, 1);

	

	inode root = get_inode(0);
	root.direct_ptrs[0] = info.dataregion_blocks_start + 1;
	set_inode(0, root);

	inode one = get_inode(1);
	one.direct_ptrs[0] = info.dataregion_blocks_start + 2;
	set_inode(1, one);

	
	
	inode a = get_inode(0);
	block_read(a.direct_ptrs[0], &test_filepath);
	printf("Directory is %s\n", test_filepath.filepath);

	inode b = get_inode(1);
	block_read(b.direct_ptrs[0], &test_filepath);
	printf("Directory is %s\n", test_filepath.filepath);

	
	
	
	char * newPath = "/home/vshenoy";
	//char * newPath = "/other/nothing";
	filepath_block block = find_path_block(newPath);
	int numOfDirs = get_num_dirs(newPath);
	char ** fldrs = parsePath(newPath);

	if (strcmp(fldrs[numOfDirs -1], block.filepath) == 0)
		printf("Found the correct directory\n");
	printf("The filepath is %s\n", block.filepath);

	int i;
	for(i = 0; i < numOfDirs; i++)
		free(fldrs[i]);
	free(fldrs);

	char * createPath = "/home/vshenoy/testfile";
	create(createPath);
	
	block = find_path_block(createPath);
	//int numDirs = get_num_dirs(createPath);
	//char ** folders = parsePath(createPath);
	//block_read(info.dataregion_blocks_start + 3, &block);
	printf("The filepath is %s\n", block.filepath);
}	