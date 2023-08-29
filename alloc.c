#include "alloc.h"
#include <stdio.h>

/* Approach:
 * 1. use sbrk to increase the size of the heap
 * 2. we reserve segements of data on the heap for dynamically allocated memory
 * and a segment containing meta information on the dynamic memory
 * VISUAL: 
 * heap -> meta information | memory | meta information | memory
 * 3. the blocks of meta information are connected via a linkedlist
 * 
 */

// head and tail of the linkedlist : meta_block contains size of dynamically allocated memory and whether the memory it points to is free
struct meta_block * meta_block_head = NULL;
struct meta_block * meta_block_tail = NULL;

// main function to allocate memory
void * m_alloc(size_t size){		
	// check if we can access the heap
	void * end_of_heap = sbrk(0);
	if (end_of_heap == (void *) -1){
		printf("ERROR IN REQUESTING END OF DATA SEGMENT FROM SBRK");
		return NULL;
	}
	
	// allocating memory for first time
	if (meta_block_head == NULL){
		struct meta_block * block = createBlock(&meta_block_tail, size);
		if (block == NULL){
			return NULL; // block could not be created
		}
		meta_block_head = block;
		// return the ptr right after the meta_block end
		/* this is the pointer pointing to the dynamically allocated 
		   memory */
		return block + 1;
	}
	
	// look for free block
	struct meta_block * free_block = findSuitableFreeBlock(&meta_block_head, size);
	if(free_block == NULL){
		// no suitable block found, hence we create a new one at the end of the list
		struct meta_block * new_block = createBlock(&meta_block_tail, size);
		return new_block + 1;
		
	} else {
		// partition the existing free block into a subblock that to use and subblock that remains free
		partition(free_block, size);
		return free_block + 1;		
	}
}

// helper function to see if a valid parititon can be made
int validPartition(size_t fullsize, size_t bytes_to_take){
	size_t remaining_size = fullsize - bytes_to_take;
	if (remaining_size > META_SIZE) return 1;
	return -1;
 
}

void * re_alloc(void * ptr, size_t size){
	// get the meta_block for this pointer
	struct meta_block * block = getMBForDM(ptr);
	int current_size = block -> size;
	// check if the current block can be readjusted for reallocation
	if(current_size > size && validPartition(current_size, size) == 1){
		// free then partition the block
		m_free(ptr);
		partition(block, size);
		return block + 1;
		
	}
	
	// check if sbrk is able to get the end of the data segment
	void * newptr = sbrk(0);
	if(newptr == (void *) -1){
		return NULL;
	}

	// above conditions failed, so we create a new meta_block and a new segment of allocated memory
	// at the end of the heap
	struct meta_block * new_block = createBlock(&meta_block_tail, size);
	if(new_block == NULL){
		return NULL;
	}

	// copy the elements in the case of a new block, then free the old segment
	unsigned char * old = block + 1;
	unsigned char * new = new_block + 1;
	for(size_t i = 0; i < current_size; i++){
		new[i] = old[i];
	}
	m_free(ptr);
	return new_block + 1;
}
void * c_alloc(size_t size, size_t size_of_elements){
	// initialize memory then set every byte to 0
	void * ptr = m_alloc(size * size_of_elements);
	if(ptr == NULL){
		return NULL;
	}
	unsigned char * byte_ptr = ptr;
	for(size_t i; i < size_of_elements; i++){
		byte_ptr[i] = 0;
	}
	
	return ptr;
	
	
} 

void partition(struct meta_block * block, size_t newsize){
	// store original starting position of the meta_block
	struct meta_block * original_pos_of_block = block;
			
	// update the information of the meta_block
	size_t original_size = block -> size;
	size_t remaining_bytes = original_size - newsize;
	block -> size = newsize;
	block -> free = 0;

	block = block + 1;

	// go to the location in memory where the split begins and create a new meta_block
	// and init the block with the right information.
	void * data = (void *) block;
	data = data + newsize;

	// we can insert the meta_block here
	struct meta_block * new_meta_block = (struct meta_block *) data;
	new_meta_block -> size = remaining_bytes - META_SIZE;
	new_meta_block -> free = 1;
	new_meta_block -> next = original_pos_of_block -> next;
	
	// update the linked list of meta blocks
	original_pos_of_block -> next = new_meta_block;
	if (new_meta_block -> next == NULL) {
		meta_block_tail = new_meta_block;
	}
	// any bytes of memory we do not change, keep it's original value
		
}

struct meta_block * createBlock(struct meta_block ** tail, size_t size){
	// request space from the OS for the process's data segment, our heap
	void * start_of_segment = requestSpace(size);
	if(start_of_segment == ALLOCATION_FAILED){
		return ALLOCATION_FAILED;
	}
	
	// init the meta_block's information, update the linkedlist, and return the pointer to our dynamically allocated memory	
	struct meta_block * block = (struct meta_block *) start_of_segment;
	block -> size = size;
	block -> free = 0;
	block -> next = NULL;
	
	if (*tail == NULL){
		*tail = block;
	} else {
		(*tail) -> next = block;
		*tail = block;
	}
		
	return block;
}

// helper function to increase the heap's size
void * requestSpace(size_t size){
	void * end_of_heap = sbrk(0);
	if(end_of_heap == (void *) -1){
		return ALLOCATION_FAILED; //macro that evaluates to NULL
	}
	void * start_of_expanded_heap = sbrk(META_SIZE + size);
	if (start_of_expanded_heap == (void *) -1){
		return ALLOCATION_FAILED;
	}
	return start_of_expanded_heap;
}

// helper function to find a free block
struct meta_block * findSuitableFreeBlock(struct meta_block ** ptr_to_head, size_t size){
	// find a free block that will have enought size to accomodate another block
	// after a possible partition is done
	// valid partition can only occur if the remaining space can accomodate a meta_block and there are bytes left for memory allocation
	struct meta_block * current = *ptr_to_head;
	while (current != NULL){
		if(current -> size > size && current -> free == 1 && validPartition(current -> size, size)){
			return current;
		}
		current = current -> next; 
	}	
	return NULL;
}


void m_free (void * ptr) {
	if(ptr == NULL){
		return;
	}
	// get to the meta_block from the pointer
	// in the memory the meta_block and memory for dynamic allocation will be next to each other, hence it can accessed like this
	ptr = ptr - META_SIZE;
	struct meta_block * block = ptr;
	block -> free = 1;
}

// for debugging
void printMetaBlockInfo(struct meta_block * node){
	printf("size: %zu\n", node -> size);
	printf("free: %d\n", node -> free);
	printf("next: %p\n", node -> next);
}

void printMetaBlockList(struct meta_block * head, int printIntData){
	struct meta_block * current = head;
	int listLength = numberOfMetaBlocks();
	int i = 0;
	int block_num = 1;
	while(i < listLength){
		printf("block number %d:\n", block_num);
		printMetaBlockInfo(current);
		if (printIntData == 1)
			printIntDataAssociatedWithBlock(current);
		size_t tmpSize = current -> size;
		void * p = (void *) current + META_SIZE + tmpSize;
		current = (struct meta_block *) p;
		block_num++;
		i++;
	}

}

// get length of the meta_block linked list
int numberOfMetaBlocks(){
	struct meta_block * current = meta_block_head;
	int len = 0;
	while (current != NULL){
		len++;
		current = current -> next;
	}
	return len;
}

// get a certain block, useful for debugging and unit testing
struct meta_block * getNthMetaBlock(int n){
	if(meta_block_head == NULL){
		return NULL;
	}
	struct meta_block * current = meta_block_head;
	for(int i = 0; i < n - 1; i++){
		current = current -> next;
	}
	return current;
}

// these next 2 functions will help get the meta_block for a valid pointer and vice-versa
void * getPtrForMB(struct meta_block * block){
	struct meta_block * tmp = block + 1;
	return (void *) tmp;
}
struct meta_block * getMBForDM(void * ptr){
	void * p = ptr - META_SIZE;
	return (struct meta_block *) p;
}


// for debugging
void printIntDataAssociatedWithBlock(struct meta_block * block){
	puts("data for this block");
	printf("%zu\n", block -> size);
	printf("size: %zu\n", (block -> size)/sizeof(int));
	int SIZE = (size_t)(block -> size)/sizeof(int);
	block = block + 1;

	int * iblock = (int *) block; 
	

	for(int i = 0; i < SIZE; i++){
		
		printf("%d\n", iblock[i]);
	}
	puts("DONE");
}

// get the associated meta_block with a pointer 
struct meta_block * getMetaBlock(void * ptr){
	return (struct meta_block *) ptr - 1;	
}


void test(){
	printf("test function for linking\n");
}

