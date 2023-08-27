#ifndef ALLOC
#define ALLOC

#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#define META_SIZE sizeof(struct meta_block)
#define ALLOCATION_FAILED NULL
struct meta_block {
	size_t size;
	int free;
	struct meta_block * next;
	
};

extern struct meta_block * meta_block_head;
extern struct meta_block * meta_block_tail;

void * m_alloc(size_t size);
void m_free(void * ptr);
void * findSuitableMemory();
void * re_alloc(void *, size_t);
void * c_alloc(size_t, size_t);


struct meta_block * create_meta_block(size_t size);

void * requestSpace();
struct meta_block * findSuitableFreeBlock(struct meta_block ** head, size_t size);
void mergeFreeBlocks(struct meta_block *);

void test();

void printMetaBlockInfo(struct meta_block *);
void printMetaBlockList(struct meta_block *, int);
void printIntDataAssociatedWithBlock(struct meta_block *);

int validPartition(size_t, size_t);
void partition(struct meta_block *, size_t);

int numberOfMetaBlocks();
struct meta_block * getNthMetaBlock(int n);
void * getPtrForMB(struct meta_block *);
struct meta_block * getMBForDM(void * ptr);

struct meta_block * getMetaBlock(void * ptr);

struct meta_block * createBlock(struct meta_block ** head, size_t size);

#endif

