#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mymalloc.h"


#define HDRSIZE 4
#define MEMSIZE 5000

#define malloc(x) mymymalloc(x,__LINE__,__FILE__)
#define free(x) myfree(x,__LINE__,__FILE__)



static void * memory;
static char myBlock[5000];
void *  pages[5];
void * currentPage;
int isInitialized = 0;
int memoryInitialized;
int pageSize;


/* Initializes the 8MB memory as well as all the pages in memory
	INPUT: None
	OUTPUT: None
*/
void initializeMemory(){
	int error, i;
	void * page;
	//If the memory has not been initialized, enter
	
		memoryInitialized = 1;
		pageSize = sysconf(_SC_PAGESIZE);	//get the page size of this system
		error = posix_memalign(&memory, pageSize, 10 * pageSize); // Create memory that is size of 10 pages
		if (error != 0){
			printf("Error Allocating 8MB memory\n");
			return;
		}

		for (i = 0; i < 10; i++){
			page = memory + (i * pageSize);
			initializePage(page);

		}
	currentPage = memory;
	

}


/* Initializes the page passed in as an argument. Puts in a prologue, epilogue
	header, and footer block
	INPUT: void * to beginning of page
	OUTPUT: None
*/
void initializePage(void * page){
	u_int * ptr;

	ptr = (u_int *) page;
	*ptr = pageSize; 	//Sets the page size
	*ptr = *ptr << 12;	//bit shifts to left by 12
	*ptr = *ptr | 0xFFF;	//sets thread_id to be initially 0xFFF


	ptr = ptr + 1; // Move 4 bytes over to the header
	*ptr = (pageSize - (2 * HDRSIZE)) | 0 ; // Set the header to remaining size and unallocated
	u_int value = pageSize - 2* HDRSIZE;
	ptr = ptr + (pageSize - 2* HDRSIZE)/4; //At the beginnning of the epilogue block
	*ptr = 0 | 1;	// Setting the epilogue block to the siz
	ptr = ptr - 1; // Move back 4 bytes to footer
	*ptr = (pageSize - 2 * HDRSIZE) | 0; //footer

}

/* Gets the page's thread id stored in the prologue block
	INPUT: void * to beginning of page
	OUTPUT: the page's thread id returned as a u_int 
*/
u_int getPageID(void * page){
	u_int * ptr;
	
	ptr = (u_int *) page;


	return (*ptr & 0xFFF);	//ID is in last twelve bits
}

/* Gets the page size stored in the prologue block
	INPUT: void * to beginning of page
	OUTPUT: the pageSize returned as a u_int 
*/
u_int getPageSize(void * page){
	u_int * ptr;
	u_int size;

	ptr = (u_int *) page;
	size = *ptr & 0xFFFF000; //size bits are in bits 12-27
	size = size >> 12;	//Shift these bits into the right place
	return size;
}


/* Sets the page's thread_id in the prologue block
	INPUT: void * to beginning of page
	OUTPUT: none 
*/
void setPageID(void * page, int thread_id){
	u_int * ptr;
	
	ptr = (u_int *) page;

	*ptr = *ptr & ~0xFFF;	//clears the lower twelve bits
	*ptr = *ptr | thread_id;

	return;
}


/* Sets the page's size in the prologue block
	INPUT: void * to beginning of page
	OUTPUT: none 
*/
void setPageSize(void * page, int new_size){
	u_int * ptr;
	
	int page_id = getPageID(page);
	
	ptr = (u_int *) page;

	*ptr = *ptr & 0x0;	//clears all bits
	*ptr = new_size;	//sets the new size
	*ptr = *ptr << 12;	//shifts bits over by 12
	*ptr = *ptr | page_id;


	return;
}

/* Creates a space in memory based on size, if available. Returns NULL if not
	INPUT: size_t of the block to created
	OUTPUT: The char pointer of the available space in memory; NULL if no place
	This also initializes the header and footer.
*/
void * mymalloc(size_t size, char * b, int a){
	size_t extendedSize;
	char * ptr;
	char *headerPointer;
	char * footerPointer;
	int oldSize, difference, adjustedSize;
	
	if (memoryInitialized == 0){
		initializeMemory();
		memoryInitialized = 1;
	}

	//Spurrious case. size = 0
	if (size == 0){
		// printf("WARNING. Zero size. Returning NULL Pointer at %s and line %d\n", b, a );
		return NULL;
	}
	else if(size > MEMSIZE){
		// printf("Warning: size exceeds memory size. Return null at %s and line %d\n", b,a);
		return NULL;
	}

	//Adjustment for overhead and alignment
	adjustedSize = (HDRSIZE - (size % HDRSIZE)) % HDRSIZE;
	extendedSize = size + adjustedSize + (2 * HDRSIZE);

	//Get pointer to block (beginnig of usuable memory) and set value if usuable
	if ((ptr = findFit(extendedSize)) != NULL){
		 oldSize = getSize(ptr);		//Get the old size of the block
		 difference =  oldSize - extendedSize;
		//Case 1: If there is enough space for the memory + header/footer
		if ((oldSize > extendedSize) && (difference > (2 * HDRSIZE))){	
			setValue(getFooter(ptr), difference,0);
			setValue(getHeader(ptr), extendedSize,1);
			footerPointer = ptr;
			footerPointer = ptr + extendedSize - (2 * HDRSIZE);
			//footerPointer = (char *) (ptr + extendedSize - (2 * HDRSIZE));
			setValue(footerPointer,extendedSize,1);
			setValue(footerPointer + 4, difference, 0);

		}
		else{
			setValue(getHeader(ptr), oldSize, 1);
			setValue(getFooter(ptr), oldSize, 1);
		}


		return ptr;
	}
	// Pointer way null -- no place available
	//printf("Unable to allocated space. Returning NULL in %s at line %d \n", __FILE__, __LINE__);
	return NULL;

}





/* Uses the first fit algorithm to find the next block of adequate size
	INPUT: The size of the block to be allocated
	OUTPUT: The char pointer of the available space in memory; NULL if no place
*/

char * findFit(int extendedSize){
	char * ptr = (char *)currentPage;	//beginning of memory
	ptr = ptr + (2 * HDRSIZE); 	//Move past prologue block and header
	
	//blockSize and allocated bit of the first block in memory
	int blockSize  = getSize(ptr);
	int allocBit = getAllocation(ptr);
	while ( blockSize != 0 )	{	//Conditions for epilogue block
		//if it is unallocated and the current size can accommodate the new block
		if ((allocBit != 1) && (blockSize >= extendedSize))
			return ptr;
		
		//go to next block and get blockSize and allocBit
		ptr = ptr + blockSize;
		blockSize = getSize(ptr);
		allocBit = getAllocation(ptr);
	}
	//at epilogue block without finding space
	return NULL;

}




/* Initializes the memory block with prologue, epilogue, header and footer
	INPUT: The char pointer to the beginning of the memory block
	OUTPUT: None
*/

void initialize(){
	char * memBlock;

	memBlock = (char *)currentPage;
	setValue(memBlock,0,1); 	//Setting the prologue block
	char * epilogue = memBlock + MEMSIZE - HDRSIZE;	//Get address of epilogue
	setValue(epilogue, 0, 1);	//Setting the epilogue block

	int memorySize = MEMSIZE - (2 * HDRSIZE);	//Remaining memory size after prologue/epilogue
	//Setting value for one contiguous memory block
	setValue((memBlock + HDRSIZE), memorySize, 0);	//Set Header
	setValue((epilogue - HDRSIZE), memorySize, 0);	//Set footer
	
}



/* Frees the memory to which the pointer references
	INPUT: The void pointer
	OUTPUT: None
*/

void myfree(void * ptr, char * b, int a){
	char *  next;
	char * previous;
	int size;
	
	if (ptr == NULL){
		// printf("Unable to free a NULL pointer in %s line  %d \n",b,a);
		return;
	}

	int relativeAddress = (char*)(ptr) - (char*)currentPage;
	/*
	if (relativeAddress > sizeof(myBlock) - 2*HDRSIZE || relativeAddress < HDRSIZE){
		// printf("Not a freeable memory address in %s line  %d \n",__FILE__,__LINE__);
		return;
	}
	*/

	if (getAllocation(ptr) == 0){
		//printf("Can not free an already free block in %s line %d\n", __FILE__, __LINE__);
	}

	//Gets the size and allocated bit
	size = getSize(ptr);
	
	//sets the value for header and footer
	setValue(getHeader(ptr), size, 0);
	setValue(getFooter(ptr), size, 0);


	coalesce(ptr);

}

/* Coalesces memory blcoks based on the current, previous, and next blocks
	INPUT: The char pointer pointing to the block we want to free
	OUTPUT: None
*/
void coalesce(char * ptr){
	char * previous;
	char * next;
	int allocPrevious, allocNext, size;

	previous  = getPrevious(ptr);
	next = getNext(ptr);
	size = getSize(ptr);

	//This is to check if the next or previos blocks are null
	if (previous != NULL)
		allocPrevious = getAllocation(previous);
	else
		allocPrevious = 1;
	
	if (next != NULL)
		allocNext = getAllocation(next);
	else
		allocNext = 1;


	//Case 1: Neither the previous or next blocks are free
	if ((allocPrevious == 1) && (allocNext == 1)){
		return;
	}
	//Case 2: Previous block is free, but next block is allocated
	else if ((allocPrevious == 0) && (allocNext == 1)){
		size = size + getSize(previous); 
		setValue(getFooter(previous), 0 ,0);
		setValue(getHeader(previous), size, 0);
		setValue(getFooter(ptr), size, 0);
		setValue(getHeader(ptr), 0,0);

	}

	//Case 3: Next block is free, but previous block is allocated
	else if ((allocPrevious == 1) && (allocNext == 0)){
		size = size + getSize(next); 
		setValue(getFooter(ptr), 0 ,0);
		setValue(getHeader(ptr), size, 0);
		setValue(getFooter(next), size, 0);
		setValue(getHeader(next), 0,0);

	}

	//Case 4: Both previous and next blocks are free
	else if ((allocPrevious == 0) && (allocNext == 0)){
		size = size + getSize(previous) + getSize(next); 
		setValue(getFooter(previous),0 ,0);
		setValue(getHeader(previous), size, 0);
		setValue(getFooter(ptr), 0,0);
		setValue(getHeader(ptr), 0, 0);
		setValue(getFooter(next), size, 0);
		setValue(getHeader(next), 0 , 0);

	}


	
}




/* Returns the pointer to the previous block. NOTE: this pointer points to the beginning
	of usable memory. You must call getHeader() to get the header
	INPUT: The char pointer pointing to the beginning of usuable memory
	OUTPUT: The address of the previous block stored in a pointer
*/
char * getPrevious(char * ptr){
	//RYAN CHECK THIS BECAUSE IT DIFFERS FROM THE TEXTBOOK
	char * footer = ptr - 8;
	int size = (*(int*) footer) & ~1;
	// This is the first block. The previous block is the prologue block
	if ((size == 0) )
		return NULL;
	char * previous = ptr - size;
	return previous;
}


/* Returns the pointer to the next block. NOTE: this pointer points to the beginning
	of usable memory. You must call getHeader() to get the header
	INPUT: The char pointer pointing to the beginning of usuable memory
	OUTPUT: The address of the next block stored in a pointer
*/
char * getNext(char * ptr){
	//RYAN CHECK THIS BECAUSE IT DIFFERS FROM THE TEXTBOOK
	int size = getSize(ptr);
	char * next = ptr + size;
	//The current block is the last block. We have reached the epilogue block
	if ( (getSize(next) == 0) && (getAllocation(next) == 1) )
		return NULL;
	return next;
}

/* Returns the address to the HEADER
	INPUT: The char pointer 
	OUTPUT: The address of the header stored in a pointer
*/
char * getHeader(char * p){
	p = p - HDRSIZE;
	return p;
}


/* Returns the address to the FOOTER
	INPUT: The char pointer
	OUTPUT: The address of the header stored in a pointer
*/
char * getFooter(char * p){
	p = p + getSize(p) - (2 * HDRSIZE);
	return p;
}


/*Gets the size from the 4 byte header
	INPUT: char pointer to header
	OUPUT: return the size stored as an int
*/
int getSize(char * ptr){
	int size = (*(int *)getHeader(ptr)) & ~1;
	return size;
}


/*Gets the allocated bit from the 4 byte header
	INPUT: char pointer to header
	OUPUT: return the last bit as an int
*/
int getAllocation(char * ptr){
	int allocated = (*(int *)getHeader(ptr)) & 1;
	return allocated;
}


/*  Sets the value of a four byte word based on size and allocation bit
	INPUT: char pointer, int size, int allocated flag
	OUTPUT: None
*/
void setValue(char * p, int size, int allocation){
	int * ptr = (int *) p; 	// Casts to int pointer. Good practice since we are writing ints
	*ptr = size | allocation;
}



/*  Creates the header and footer given a pointer, size, and allocated flag
	INPUT: char pointer, int size, int allocated flag
	OUTPUT: None
*/

char * createExtremities(char * p, int size, int allocated){
	// CREATE HEADER: ORs the size and allocated bit. This writes size to the upper
	// 31 bits and the allocated flag to the LSB
	*p = (int) (size | allocated);


	p = p  + size - HDRSIZE; 		// moves to the end of the block
	
	//CREATE FOOTER: Same as CREATE HEADER
	*p = (int) (size | allocated);

	p = (p - size) + (2* HDRSIZE); // returns the beginning of usuable memory
	return p;
}


void loadPages(){
	int i;
	//Load first 5 pages
	for(i = 0; i < 5; i++){
		pages[i] = memory + (i * pageSize);
	}
	//Protect last 5 pages
	while (i < 10){
		mprotect(memory + (i * pageSize), pageSize, PROT_NONE);
		i++;
	}

	i = 5;
	while (i < 10){
		mprotect(memory + (i * pageSize), pageSize, PROT_WRITE);
		i++;
	}
	//Try to load something into page 7. Should succeed
	char * newptr = (char *)(memory + (7*pageSize));
	*newptr = 'a';
	printf("This line should run\n");

}

/*

int main(){

	
	initializeMemory();
	setPageID(memory, 42);
	int x = getPageSize(memory);
	int y = getPageID(memory);
	/*
	int i;	
	char * array = (char *) mymalloc(sizeof(char) * 10);

	for(i = 0; i < 5; i++){
		pages[i] = memory + (i * pageSize);
	}
	
	return 0;
	
}
*/

 
  

