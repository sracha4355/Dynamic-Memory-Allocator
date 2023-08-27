#include <stdio.h>
#include <assert.h>
#include "alloc.h"
 
void printPointer(char * prompt, void * ptr){
	printf("%s: %p\n", prompt, ptr);
} 
 
void first_block() {
	size_t size = sizeof(int) * 3;
	printf("SIZE OF META BLOCK %zu\n", META_SIZE);
	void * p = sbrk(0);	
	void * end = m_alloc(size);
	printf("Address of start of heap %p\n", (void*) p);
	printf("Address of start of dm %p\n", (void*) end);
	printf("Address of start of heap + bytes allocated %p\n", (void*) p + META_SIZE);
	assert(p + META_SIZE == end);
	puts("Assertion passed");
	
	printPointer("head of meta block list", meta_block_head);
	printPointer("tail of meta block list", meta_block_tail);
	assert(meta_block_head == meta_block_tail);
	puts("Assertion passed");
	
	
	int * arr = (void *) end;
	for(int i = 0; i < size/sizeof(int); i++){
		arr[i] = i;		
	}
	
	for(int i = 0; i < size/sizeof(int); i++){
		printf("%d\n", arr[i]);			
	}
	
	
	
}

void second_block(){
	size_t size = sizeof(int) * 2;
	void * p = sbrk(0);
	void * end = m_alloc(size);
	void * ptr_later_for_freeing = end;
	printf("Address of start of heap %p\n", (void*) p);
	printf("Address of start of dm %p\n", (void*) end);
	printf("Address of start of heap + bytes allocated %p\n", (void*) p + META_SIZE);
	assert(p + META_SIZE == end);
	puts("Assertion passed");
	printPointer("head of meta block list", meta_block_head);
	printPointer("tail of meta block list", meta_block_tail);
	assert(meta_block_head != meta_block_tail);
	puts("Assertion passed");
	
	int * newArr = end;
	for(int i = 0; i < size/sizeof(int); i++){
		newArr[i] = 4;		
	}
	
	
	end = end - META_SIZE - (sizeof(int) * 3)  - META_SIZE;
	printf("%p\n", end);
	int * arr = end + META_SIZE;
	for(int i = 0; i < 3; i++){
		printf("%d\n", arr[i]);
	}
	
	printMetaBlockList(meta_block_head, 1);
	void * start = meta_block_head;
	start += META_SIZE;
	int * arr3 = start;
	for(int i = 0; i < 3; i++){
		printf("%d\n", arr3[i]);
	}
	arr += 3;
	start = (void *) arr;
	start += META_SIZE;
	int * arr2 = start;
	for(int i = 0; i < size/sizeof(int); i++){
		printf("%d\n", arr2[i]);
	}
	
	puts("freeing newly allocated block"); 
	m_free(ptr_later_for_freeing);
	printMetaBlockList(meta_block_head, 0);
}


void partitionTests() {
	int result = validPartition(10, 5);
	printf("result: %d\n", result);
	assert(result == -1);
	result = validPartition(35, 10);
	printf("result: %d\n", result);
	assert(result == 1);
}

void createFirstBlock(size_t sizesArr[], size_t SIZE){
	/*
	allocate the memory and check the pointers
	check then check the data
	and the size of the meta block linkedlist
	*/
	
	size_t size_for_block = sizesArr[0];
	void * p = sbrk(0);
	void * dm = m_alloc(size_for_block * sizeof(int));
	puts("Asserting: start of the heap + size of a meta block = pointer to our dynamically allocated memory");
	assert(p + META_SIZE == dm);
	puts("Assertion passed");
	
	int length = 0;
	struct meta_block * curr = meta_block_head;
	while(curr != NULL){
		length ++;
		curr = curr -> next;
	}
	puts("Asserting: length of meta block list = 1");
	assert(length == 1);
	puts("Asserting: head and tail of meta block are the same");
	assert(meta_block_head == meta_block_tail);
	
	int * arr = (int * ) dm;
	int arr_size = size_for_block;
	puts("data in the allocated memory");
	for(size_t i = 0; i < size_for_block; i++){
		arr[i] = size_for_block + i;
		printf("%d: %d\n", i, arr[i]);
	}
}

void createSecondBlock(size_t sizesArr[], size_t SIZE){
	/*
	allocate the memory and check the pointers
	check then check the data
	and the size of the meta block linkedlist
	and check the data of the first blocks and if it was overwritten
	*/
	
	size_t size_for_block = sizesArr[1];
	void * p = sbrk(0);
	void * dm = m_alloc(size_for_block * sizeof(int));
	puts("Asserting: start of the heap + size of a meta block = pointer to our dynamically allocated memory");
	assert(p + META_SIZE == dm);
	puts("Assertion passed");
	
	int length = 0;
	struct meta_block * curr = meta_block_head;
	while(curr != NULL){
		length ++;
		curr = curr -> next;
	}
	puts("Asserting: length of meta block list = 2");
	assert(length == 2);
	puts("Asserting: head and tail of meta block are the same");
	assert(meta_block_head != meta_block_tail);
	
	// the array we just allocated for
	int * arr = (int * ) dm;
		
	int arr_size = size_for_block;
	puts("data in the allocated memory");
	for(size_t i = 0; i < size_for_block; i++){
		arr[i] = size_for_block + i;
		printf("%d: %d\n", i, arr[i]);
	}
	
	size_t head_block_size = meta_block_head -> size;
	curr = meta_block_head;
	curr = curr + 1;
	arr = (int *) curr;
	int * lastNum = arr[head_block_size/sizeof(int) - 1];
	size_t * oneAfterLastNum = arr[head_block_size/sizeof(int)];
	
	puts("Asserting: traveled back to the last data in the memory pointed to by first meta block, then traveled one head to get into latest meta block");
	puts("check if the sizes match up");
	assert(oneAfterLastNum == (meta_block_tail -> size));
	assert((meta_block_tail -> size) == sizesArr[1] * sizeof(int));
	puts("assertion complete");
	
	/*
	void * prev_block = (void *) arr - META_SIZE - (sizeof(int));
	int * num = (int *) prev_block;
	
	assert(*num == 4);
	printf("%d\n", *num);
	
	struct meta_block * block = meta_block_head + 1;
	test = (int * ) block;
	for(size_t i = 0; i < sizesArr[0]; i++){
		printf("%d: %d\n", i, test[i]);
	} */
	
	printf("meta block head: %p\n", (void * )meta_block_head);
	puts("\nprinting the blocks with their data");
	printMetaBlockList(meta_block_head,1);
	
}

void createThirdBlock(size_t sizesArr[], size_t SIZE){
	/*
	allocate the memory and check the pointers
	check then check the data
	and the size of the meta block linkedlist
	and check the data of the second blocks and if it was overwritten
	*/
	size_t size_for_block = sizesArr[2];
	void * p = sbrk(0);
	void * dm = m_alloc(size_for_block * sizeof(int));
	puts("Asserting: start of the heap + size of a meta block = pointer to our dynamically allocated memory");
	assert(p + META_SIZE == dm);
	puts("Assertion passed");
	
	int length = 0;
	struct meta_block * curr = meta_block_head;
	while(curr != NULL){
		length ++;
		curr = curr -> next;
	}
	puts("Asserting: length of meta block list = 2");
	assert(length == 3);
	puts("Asserting: head and tail of meta block are the same");
	assert(meta_block_head != meta_block_tail);
	

	puts("\nprinting the blocks with their data");
	int * arr = (int * ) dm;		
	puts("data in the allocated memory");
	for(size_t i = 0; i < size_for_block; i++){
		arr[i] = size_for_block + i;
		printf("%d: %d\n", i, arr[i]);
	}
	
	printMetaBlockList(meta_block_head, 1);
	

}

void freeSecondBlock(size_t sizesArr[], size_t SIZE){
	/*
		free the second block and make sure the first and third block's data is in tact
	*/
	struct meta_block * block = getNthMetaBlock(2);
	int * arr = (int *) getPtrForMB(block);
	size_t size_for_block = sizesArr[1];
	for(size_t i = 0; i < size_for_block; i++){
		printf("%d: %d\n", i, arr[i]);
	}
	
	m_free(arr);
	printMetaBlockList(meta_block_head, 1);
	puts("asserting that the second block has been marked as free");
	assert(block -> free == 1);
	puts("assertion passed");
}


void partitionFreedSecondBlockForNewBlock(size_t sizesArr[], size_t SIZE){
	size_t size_for_block = sizesArr[3];
	struct meta_block * second_block = getNthMetaBlock(2);
	size_t secondblock_original_size = second_block -> size;
	
	struct meta_block * original_third_block = getNthMetaBlock(3);
	void * ptr = m_alloc(size_for_block * sizeof(int));
	printMetaBlockList(meta_block_head, 1);
	
	size_t new_size_for_second_block = size_for_block * sizeof(int);
	
	struct meta_block * new_block_from_partition = second_block -> next;
	puts("Asserting if the free block we are using's size has changed to the new size");	
	assert(second_block -> size == size_for_block * sizeof(int));
	puts("Assertions passed");
	puts("asserting if the new block from the partition points to the original third block");
	puts("because new block is the new third block");
	assert(second_block -> next -> next == original_third_block);
	puts("Assertions passed");
	puts("asserting if the size of the block from the partition is correct, so it does not mess with subsequent memory");
	assert(second_block -> next -> size == secondblock_original_size - new_size_for_second_block - META_SIZE);
	puts("Assertions passed");
	
	printMetaBlockList(meta_block_head, 1);
}

void useNewThirdBlockAndParition(size_t sizesArr[], size_t SIZE){
	struct meta_block * third_block = getNthMetaBlock(3);
	size_t third_block_orig_size = third_block -> size;
	struct meta_block * original_fourth_block = getNthMetaBlock(4);
	void * ptr = m_alloc(2 * sizeof(int));
	puts("Asserting if the free block we are using's size has changed to the new size");	
	assert(third_block -> size == 2 * sizeof(int));
	puts("Assertions passed");
	puts("asserting if the new block from the partition points to the original fourth block");
	assert(third_block -> next -> next = original_fourth_block);
	puts("Assertions passed");
	puts("asserting if the size of the block from the partition is correct, so it does not mess with subsequent memory");
	assert(third_block -> next -> size == third_block_orig_size - (2 * sizeof(int)) - META_SIZE);
	puts("Assertions passed");
	printMetaBlockList(meta_block_head, 1);

}


void reallocateSecondBlock(size_t sizesArr[], size_t SIZE){
	// block 2 : size 40
	struct meta_block * second_block = getNthMetaBlock(2);
	struct meta_block * orig_third_block = getNthMetaBlock(3);
	
	size_t sec_block_orig_size = second_block -> size;
	void * dm = getPtrForMB(second_block);
	
	size_t second_block_orig_size = second_block -> size;
	size_t newsize = 2 * sizeof(8);
	void * ptrAfterRealloc = re_alloc(dm, newsize);
	
	
	assert(dm == ptrAfterRealloc);
	assert(second_block -> next -> next == orig_third_block);
	assert(second_block -> size == newsize);
	assert(second_block -> next -> size == sec_block_orig_size - META_SIZE - newsize);
	
	puts("assertions passed");
	printMetaBlockList(meta_block_head, 1);
}

void reallocateFirstResultingInNewBlockAtEnd(size_t sizesArr[], size_t SIZE){
	
	struct meta_block * first_block = getNthMetaBlock(1);
	size_t first_block_orig_size = first_block -> size;
	struct meta_block * orig_tail = meta_block_tail;
	void * dm = getPtrForMB(first_block);
	
	size_t newsize = 35 * sizeof(int);
	
	void * ptrAfterRealloc = re_alloc(dm, newsize);
	
	assert(first_block -> free == 1);
	assert(orig_tail -> next == meta_block_tail);
	assert(meta_block_tail -> size == newsize);
	assert(ptrAfterRealloc != dm);
	
	puts("assertions passed");
	printMetaBlockList(meta_block_head, 1);
}

void testcalloc(size_t sizesArr[], size_t SIZE){
	struct meta_block * orig_tail = meta_block_tail;
	void * ptr = c_alloc(15, sizeof(int));
	assert(orig_tail -> next == meta_block_tail);
	struct meta_block * block = getMBForDM(ptr);
	assert(block -> size == 15 * sizeof(int));
	unsigned char * current = ptr;
	for(size_t i; i < 15 * sizeof(int); i++){
		assert(current[i] == 0);
	}
	puts("assertions passed");
	printMetaBlockList(meta_block_head, 1);
	
}

int main(){
	printf("DYNAMIC MEMORY ALLOCATION TESTS\n");
	size_t sizes[4] = {4,35, 10, 10};
	puts("ALL THE TESTS DEPEND ON EACH OTHER");
	puts("TEST 1 - CREATING FIRST BLOCK -----------");
	createFirstBlock(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 1 COMPLETED");
	puts("TEST 2 - CREATING SECOND BLOCK ---------");
	createSecondBlock(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 2 COMPLETED");
	puts("TEST 3 - CREATING THIRD BLOCK ---------");
	createThirdBlock(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 3 COMPLETED");
	puts("TEST 4 FREEING SECOND BLOCK -----------");
	freeSecondBlock(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 4 COMPLETED");
	puts("TEST 5 - reusing the existing free block and checking for a partition");
	partitionFreedSecondBlockForNewBlock(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 5 COMPLETED");
	puts("TEST 6 - reusing the new free block from the partition and checking for a partition");
	useNewThirdBlockAndParition(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 6 COMPLETED");
	puts("TEST 7 - reallocate the second block--------");
	reallocateSecondBlock(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 7 COMPLETED");
	puts("TEST 8 - reallocate the first block result in creating a new block");
	reallocateFirstResultingInNewBlockAtEnd(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 8 COMPLETED");
	puts("TEST 9 - calloc a new block-----------");
	testcalloc(sizes, sizeof(sizes)/sizeof(sizes[0]));
	puts("TEST 9 COMPLETED");
	
	
	
	return 0;
}
