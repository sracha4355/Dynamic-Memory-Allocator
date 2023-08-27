#include "alloc.h"
#include <stdio.h>

struct meta_block * meta_block_head = NULL;
struct meta_block * meta_block_tail = NULL;

void * m_alloc(size_t size){		
	void * end_of_heap = sbrk(0);
	if (end_of_heap == (void *) -1){
		printf("ERROR IN REQUESTING END OF DATA SEGMENT FROM SBRK");
		return NULL;
	}
	
	// allocate memory for first time
	if (meta_block_head == NULL){
		struct meta_block * block = createBlock(&meta_block_tail, size);
		if (block == NULL){
			return NULL; // block could not be created
		}
		meta_block_head = block;
		return block + 1;
	}
	
	// look for free block
	struct meta_block * free_block = findSuitableFreeBlock(&meta_block_head, size);
	if(free_block == NULL){
		// no suitable block found, hence we create a new one at the end of the list
		struct meta_block * new_block = createBlock(&meta_block_tail, size);
		//puts("could not find free block, creating one");	
		return new_block + 1;
		
	} else {
		puts("partitioning freed block for usage");
		partition(free_block, size);
		return free_block + 1;		
	}
}

int validPartition(size_t fullsize, size_t bytes_to_take){
	size_t remaining_size = fullsize - bytes_to_take;
	if (remaining_size > META_SIZE) return 1;
	return -1;
 
}

// test these 2 functions, then start the github process
void * re_alloc(void * ptr, size_t size){
	// get the meta block for the pointer and check a valid partition can be made
	// valid partition defintion: we split the current block into 2 block
	// but the new block needs to be able to accomodate a meta_block
	struct meta_block * block = getMBForDM(ptr);
	int current_size = block -> size;
	if(current_size > size && validPartition(current_size, size) == 1){
		// we can use this block
		puts("reusing this block");
		m_free(ptr);
		partition(block, size);
		return block + 1;
		
	}
	
	// check if sbrk is able to get the end of the data segment
	void * newptr = sbrk(0);
	if(newptr == (void *) -1){
		printf("ERROR IN REQUESTING END OF DATA SEGMENT FROM SBRK");
		return NULL;
	}
	struct meta_block * new_block = createBlock(&meta_block_tail, size);
	if(new_block == NULL){
		return NULL;
	}
	// copy the elements
	unsigned char * old = block + 1;
	unsigned char * new = new_block + 1;
	for(size_t i = 0; i < current_size; i++){
		new[i] = old[i];
	}
	m_free(ptr);
	return new_block + 1;
}
void * c_alloc(size_t size, size_t size_of_elements){
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
	struct meta_block * original_pos_of_block = block;
		
	size_t original_size = block -> size;
	//printf("original_size %d\n", original_size);
	size_t remaining_bytes = original_size - newsize;
	//printf("remaining_bytes %d\n", remaining_bytes);
	block -> size = newsize;
	block -> free = 0;
	
	
	
	block = block + 1;
	
	void * data = (void *) block;
	data = data + newsize;
	// we can insert the meta_block here
	struct meta_block * new_meta_block = (struct meta_block *) data;
	new_meta_block -> size = remaining_bytes - META_SIZE;
	new_meta_block -> free = 1;
	new_meta_block -> next = original_pos_of_block -> next;
	
	
	
	original_pos_of_block -> next = new_meta_block;
	if (new_meta_block -> next == NULL) {
		meta_block_tail = new_meta_block;
	}
	
	//puts("the free block we are using");
	//printMetaBlockInfo(original_pos_of_block);
	//puts("the newly created free block from split");
	//printMetaBlockInfo(new_meta_block);
	
	
}

struct meta_block * createBlock(struct meta_block ** tail, size_t size){
	void * start_of_segment = requestSpace(size);
	if(start_of_segment == ALLOCATION_FAILED){
		return ALLOCATION_FAILED;
	}
	
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

struct meta_block * findSuitableFreeBlock(struct meta_block ** ptr_to_head, size_t size){
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
	ptr = ptr - META_SIZE;
	struct meta_block * block = ptr;
	block -> free = 1;
}

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
		// current = current -> next
		size_t tmpSize = current -> size;
		void * p = (void *) current + META_SIZE + tmpSize;
		current = (struct meta_block *) p;
		block_num++;
		i++;
	}

}

int numberOfMetaBlocks(){
	struct meta_block * current = meta_block_head;
	int len = 0;
	while (current != NULL){
		len++;
		current = current -> next;
	}
	return len;
}

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
void * getPtrForMB(struct meta_block * block){
	struct meta_block * tmp = block + 1;
	return (void *) tmp;
}
struct meta_block * getMBForDM(void * ptr){
	void * p = ptr - META_SIZE;
	return (struct meta_block *) p;
}



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

struct meta_block * getMetaBlock(void * ptr){
	return (struct meta_block *) ptr - 1;
	
}


void test(){
	printf("test function for linking\n");
}

