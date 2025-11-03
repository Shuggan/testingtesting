// mymalloc.c
// Lab 4 Exercise 1
// Your implementation goes in this file.
// You may add helper functions as necessary.


#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

// Meta-data about the heap
// This structure holds information about the entire heap region, including
// the base address, total size, and size of each block's meta-data.
static HeapInfo _heap;

// Do not change this. Used by the test harness.
// You may however use this function in your code if necessary.
unsigned int get_index(void *ptr) {
    if(ptr == NULL)
        return -1;
    else
        return (unsigned int) (ptr - (void *)_heap.base_address);
}


// Helper function to initialize a new block
void setupNewBlockInfo(BlockInfo *binfo, unsigned int size) {
    binfo->nextNode = NULL;
    binfo->size = size;
    binfo->status = FREE;
}

// Setup the heap region. Returns 1 on success, 0 on failure.
int setupHeapRegion() {
    void* base_address;
	base_address = sbrk(0);
	if(sbrk(MEMSIZE) == (void*) - 1 ) {
		printf("Cannot set break! Behavior undefined!\n");
		return 0;
	}
    _heap.base_address = (BlockInfo *) base_address;
    _heap.total_size = MEMSIZE;
    _heap.block_meta_size = sizeof(BlockInfo);

    // Initialize the first block to cover the entire heap minus metadata
    setupNewBlockInfo(_heap.base_address, _heap.total_size - _heap.block_meta_size);

    return 1;
}

void print_memlist() {
    printf("Total Size = %d bytes\n", _heap.total_size);
    printf("Start Address = %p\n", _heap.base_address);
    printf("Partition Meta Size = %d bytes\n", _heap.block_meta_size);

    BlockInfo* current = _heap.base_address;
    for (current = _heap.base_address; current != NULL;
          current = current->nextNode) {
        printf("Status: ");
        if(current->status == FREE)
            printf("FREE ");
        else
            printf("ALLOCATED ");

        printf("Start index: + %u ", get_index(current));
        printf("Length: %u\n", current->size);
    }
}



// Allocates size bytes of memory and returns a pointer
// to the first byte.
// Helper function to split a block into allocated and free parts
void splitNode(BlockInfo *block, unsigned int size) {
    BlockInfo *newBlock;
    newBlock = (BlockInfo *) ((void *)block + _heap.block_meta_size + size);
    setupNewBlockInfo(newBlock, block->size - size - _heap.block_meta_size);
    newBlock->nextNode = block->nextNode;
    block->nextNode = newBlock;
    block->size = size;
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    // Memory is not large enough
    if(size > MEMSIZE)
        return NULL;

    // Best-fit allocation strategy
    BlockInfo *current;
    unsigned int best_size = MEMSIZE + 1;
    BlockInfo *best_block = NULL;

    for (current = _heap.base_address; current != NULL;
          current = current->nextNode) {
        if (current->status == OCCUPIED) continue;
        if (current->size < size) continue;

        // Update best block if this one is better (smaller but still fits)
        if (current->size < best_size) {
            best_size = current->size;
            best_block = current;
        }
    }

    if (best_block == NULL) { // heap full
        return NULL;
    }

    // Split the block if there's enough space left for another block + metadata + minimum data
    if (best_block->size >= size + _heap.block_meta_size + 4) {
        splitNode(best_block, size);
    }

    best_block->status = OCCUPIED;
    return (void*)best_block + _heap.block_meta_size;
}

// Helper function to merge two adjacent free blocks
void mergeNode(BlockInfo *current, BlockInfo *next) {
    current->size = current->size + next->size + _heap.block_meta_size;
    current->nextNode = next->nextNode;
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
    // Check for invalid pointer
    if(ptr == NULL || ptr < (void*)_heap.base_address ||
        ptr >= (void*)_heap.base_address + _heap.total_size) {
        printf("Unable to find pointer\n");
        return;
    }

    BlockInfo *toBeFreed;
    toBeFreed = (BlockInfo *) (ptr - _heap.block_meta_size);
    toBeFreed->status = FREE;

    // Merge adjacent free blocks
    BlockInfo *currentNode = _heap.base_address;
    BlockInfo *nextNode = currentNode->nextNode;

    while(nextNode != NULL) {
        if(currentNode->status == FREE && nextNode->status == FREE) {
            mergeNode(currentNode, nextNode);
            nextNode = currentNode->nextNode;
        } else {
            currentNode = currentNode->nextNode;
            nextNode = currentNode->nextNode;
        }
    }
}


