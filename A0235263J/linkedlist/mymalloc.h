// mymalloc.h
// Lab 4 Exercise 1
// Your implementation goes in this file.
// Do not change the function prototypes given.
// You may add helper functions as necessary.

#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <unistd.h>

#define MEMSIZE (64 << 10)    // Size of memory in bytes
#define FREE 0
#define OCCUPIED 1

// Block information structure for linked list
typedef struct BlockInfo {
    unsigned int size;       // Size of this block
    unsigned int status;     // Status of this block (FREE or OCCUPIED)
    struct BlockInfo *nextNode; // Pointer to next block in the list
} BlockInfo;

// Meta-data about the heap
typedef struct HeapInfo {
    BlockInfo *base_address;
    unsigned int total_size;
    unsigned int block_meta_size;
} HeapInfo;

int setupHeapRegion();
unsigned int get_index(void *ptr);
void *mymalloc(size_t);
void myfree(void *);

// Debugging routine
void print_memlist();

#endif // MYMALLOC_H
