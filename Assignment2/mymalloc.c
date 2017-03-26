#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include "mymalloc.h"
#include "my_pthread_t.h"


#define HDRSIZE 4


#define malloc(x) mymymalloc(x,__LINE__,__FILE__)
#define free(x) myfree(x,__LINE__,__FILE__)



static void * memory;
static char myBlock[5000];
void * whichMemory;
int OS_SIZE = 5;
int USR_SIZE = 5;
int memoryInitialized = 0;
int pageSize = 0;
int fileDescriptor;
int buffer[1024];


/* Initializes the 8MB memory as well as all the pages in memory
	INPUT: None
	OUTPUT: None
*/
void initializeMemory(){
	int error, i;
	int * ptr;
	//If the memory has not been initialized, enter
		OS_SIZE = pow(2,21);
		USR_SIZE = pow(2,20) * 6;
		memoryInitialized = 1;
		pageSize = sysconf(_SC_PAGESIZE);	//get the page size of this system
		error = posix_memalign(&memory, pageSize, pow(2,24)); // Create memory that is size of 10 pages
		if (error != 0){
			printf("Error Allocating 8MB memory\n");
			return;
		}

		/*
		**** SETTING THE OS REGION OF MEMORY ****
		*/
		//Keeping last 4 bytes of OS REGION as footer
		ptr = (int *) memory;
		//This sets the header to 2^21 -4 | 0;
		int adjustedSize = OS_SIZE - 4;
		*ptr = adjustedSize; // writes the adjusted size to the first 4 bits
		*ptr = *ptr << 1; // move size over 1 bit so we put in a zero

		ptr = ptr + OS_SIZE/4;	//Go to the end of OS REGION
		ptr = ptr - HDRSIZE/4;	//At the beginning of the footer

		//setting the epilogue to 0 | 1;
		//*ptr = *ptr & 0x0; //clears area
		*ptr = 1;	///puts a 1 in the last place


		//setting the footer
		ptr = ptr - HDRSIZE/4; //move back four bytes for the footer

		*ptr = adjustedSize; // writes the adjusted size to the first 4 bits
		*ptr = *ptr << 1; // move size over 1 bit so we put in a zero


		/*
		**** SETTING THE USER REGION OF MEMORY ****
		*/
		ptr = (int *) memory;
		ptr = ptr + OS_SIZE/4; // Gets us to the beginning of user space
		*ptr = USR_SIZE; //Puts the user memory size as a header
		*ptr = *ptr << 1; //Moves over by one so that zero in the last place

		ptr = ptr + USR_SIZE/4; // Moves to end of user space
		//*ptr = *ptr & 0x0; //clears area
		*ptr = 1;	///puts a 1 in the last place 

		ptr = ptr - HDRSIZE/4; //move back four bytes for the footer
		*ptr = USR_SIZE; //Puts the user memory size as a header
		*ptr = *ptr << 1; //Moves over by one so that zero in the last place

	// create swap file
	fileDescriptor = open("swapfile.txt", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	for (i = 0; i < 16*1024; i++) {
		write(fileDescriptor, buffer, 1024);
	}
}






/* Creates a space in memory based on size, if available. Returns NULL if not
	INPUT: size_t of the block to created
	OUTPUT: The char pointer of the available space in memory; NULL if no place
	This also initializes the header and footer.
*/
void * myallocate(size_t size, char * b, int a, int id){
	size_t extendedSize;
	char * ptr;
	char *headerPointer;
	char * footerPointer;
	int oldSize, difference, adjustedSize;
	
	if (memoryInitialized == 0){
		initializeMemory();
		initializeScheduler();
		memoryInitialized = 1;
	}

	//Spurrious case. size = 0
	if (size == 0){
		// printf("WARNING. Zero size. Returning NULL Pointer at %s and line %d\n", b, a );
		return NULL;
	}
	else if(size > USR_SIZE){
		// printf("Warning: size exceeds memory size. Return null at %s and line %d\n", b,a);
		return NULL;
	}
	
	if (id == 999)
		whichMemory = memory + OS_SIZE;
	else
		whichMemory = memory;
	

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

void * findFit(int extendedSize){
	void * ptr = (void *)whichMemory;	//beginning of memory
	
	ptr = ptr + (HDRSIZE); 	//Move past prologue block and header
	//TODO: ^^^^^^ELIMINATE^^^^^
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

	memBlock = (char *)memory;
	setValue(memBlock,0,1); 	//Setting the prologue block
	char * epilogue = memBlock + USR_SIZE - HDRSIZE;	//Get address of epilogue
	setValue(epilogue, 0, 1);	//Setting the epilogue block

	int memorySize = USR_SIZE - (2 * HDRSIZE);	//Remaining memory size after prologue/epilogue
	//Setting value for one contiguous memory block
	setValue((memBlock + HDRSIZE), memorySize, 0);	//Set Header
	setValue((epilogue - HDRSIZE), memorySize, 0);	//Set footer
	
}



/* Frees the memory to which the pointer references
	INPUT: The void pointer
	OUTPUT: None
*/

void mydeallocate(void * ptr, char * b, int a, int id){
	char *  next;
	char * previous;
	int size;
	
	if (ptr == NULL){
		printf("Unable to free a NULL pointer in %s line  %d \n",b,a);
		return;
	}

	if (id == 999)
		whichMemory = memory + OS_SIZE;
	else
		whichMemory = memory;

	int relativeAddress = (ptr) - whichMemory;
	
	if (relativeAddress > (OS_SIZE + USR_SIZE) - 2*HDRSIZE || relativeAddress < HDRSIZE){
		printf("Not a freeable memory address in %s line  %d \n",__FILE__,__LINE__);
		return;
	}
	

	if (getAllocation(ptr) == 0){
		printf("Can not free an already free block in %s line %d\n", __FILE__, __LINE__);
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
void coalesce(void * ptr){
	void * previous;
	void * next;
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
void * getPrevious(void * ptr){
	//RYAN CHECK THIS BECAUSE IT DIFFERS FROM THE TEXTBOOK
	void * footer = ptr - 8;
	int size = (*(int*) footer) & ~1;
	// This is the first block. The previous block is the prologue block
	if ((size == 0) )
		return NULL;
	void * previous = ptr - size;
	return previous;
}


/* Returns the pointer to the next block. NOTE: this pointer points to the beginning
	of usable memory. You must call getHeader() to get the header
	INPUT: The char pointer pointing to the beginning of usuable memory
	OUTPUT: The address of the next block stored in a pointer
*/
void * getNext(void * ptr){
	//RYAN CHECK THIS BECAUSE IT DIFFERS FROM THE TEXTBOOK
	int size = getSize(ptr);
	void * next = ptr + size;
	//The current block is the last block. We have reached the epilogue block
	if ( (getSize(next) == 0) && (getAllocation(next) == 1) )
		return NULL;
	return next;
}

/* Returns the address to the HEADER
	INPUT: The char pointer 
	OUTPUT: The address of the header stored in a pointer
*/
void * getHeader(void * p){
	p = p - HDRSIZE;
	return p;
}


/* Returns the address to the FOOTER
	INPUT: The char pointer
	OUTPUT: The address of the header stored in a pointer
*/
void * getFooter(void * p){
	p = p + getSize(p) - (2 * HDRSIZE);
	return p;
}


/*Gets the size from the 4 byte header
	INPUT: char pointer to header
	OUPUT: return the size stored as an int
*/
int getSize(void * ptr){
	int size = (*(int *)getHeader(ptr)) & ~1;
	return size;
}


/*Gets the allocated bit from the 4 byte header
	INPUT: char pointer to header
	OUPUT: return the last bit as an int
*/
int getAllocation(void * ptr){
	int allocated = (*(int *)getHeader(ptr)) & 1;
	return allocated;
}


/*  Sets the value of a four byte word based on size and allocation bit
	INPUT: char pointer, int size, int allocated flag
	OUTPUT: None
*/
void setValue(void * p, int size, int allocation){
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

void swap_in(int16_t newPage, int des) {
	void *pagePtr, *desPtr;
	desPtr = memory + OS_SIZE + des*pageSize;
	if (newPage < MEMORYPAGES) {
		pagePtr = memory + OS_SIZE + newPage*pageSize;
		memcpy(desPtr, pagePtr, pageSize);
		memset(pagePtr, 0, pageSize);
		mprotect(pagePtr, pageSize, PROT_NONE);
	}
	else if (newPage < TOTALPAGES) {
		newPage -= MEMORYPAGES;
		lseek(fileDescriptor, newPage*pageSize, SEEK_SET);
		read(fileDescriptor, desPtr, pageSize);
	}
	mprotect(desPtr, pageSize, PROT_NONE);
}

int16_t swap_out(int16_t page) {
	void *pagePtr, *desPtr;
	int16_t freePage = nextFreePage();
	pagePtr = memory + OS_SIZE + page*pageSize;
	if (freePage < MEMORYPAGES) {
		desPtr = memory + OS_SIZE + freePage*pageSize;
		memcpy(desPtr, pagePtr, pageSize);
		mprotect(desPtr, pageSize, PROT_READ|PROT_WRITE);
	}
	else if (freePage < TOTALPAGES) {
		freePage -= MEMORYPAGES;
		lseek(fileDescriptor, freePage*pageSize, SEEK_SET);
		write(fileDescriptor, pagePtr, pageSize);
	}
	else {
		return 6969;
	}
	memset(pagePtr, 0, pageSize);
	mprotect(pagePtr, pageSize, PROT_NONE);
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

 
  
