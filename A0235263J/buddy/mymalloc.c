#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mymalloc.h"
#include "llist.h"

char _heap[MEMSIZE] = {0};
TNode *buddyList[MAX_ORDER] = {NULL};

// Do not change this. Used by the test harness.
// You may however use this function in your code if necessary.
long get_index(void *ptr) {
    if(ptr == NULL)
        return -1;
    else
        return (long) ((char *) ptr - &_heap[0]);
}

long get_size(void *ptr) {
    if(ptr == NULL)
        return 0;

    long index = get_index(ptr);
    for (int i = 0; i < MAX_ORDER; i++) {
        TNode *node = find_node(buddyList[i], index);
        if (node != NULL) {
            return (long) node->pdata->size;
        }
    }
    return 0;
}

void print_memlist() {
    for (int order = MAX_ORDER - 1; order >= 0; order--) {
        TNode *current = buddyList[order];
        int block_size_kb = (1 << order);  // 2^order KB
        printf("Block size %d KB: ", block_size_kb);

        while(current != NULL) {
            if(current->pdata->status == FREE)
                printf("FREE, ");
            else
                printf("ALLOCATED, ");

            printf("%u, ", current->pdata->start_addr / 1024);
            printf("%u -> ", current->pdata->size / 1024);
            current = current->next;
        }
        printf("\n");
    }
}

void add_mem_node(TNode **llist, unsigned int start_addr, unsigned int size, int status) {
    TData *data = (TData *) malloc(sizeof(TData));
    data->status = status;
    data->start_addr = start_addr;
    data->size = size;
    TNode *node = make_node(start_addr, data);
    insert_node(llist, node, ASCENDING);
}

void free_mem_node(TNode **llist, TNode *node) {
    if(node->pdata) {
        free(node->pdata);
    }
    delete_node(llist, node);
}

TNode *find_best_free_node(TNode **llist) {
    reset_traverser(*llist, FRONT);
    TNode *node;
    do{
        node = succ(*llist);
        if(node != NULL)
            if(node->pdata->status == FREE) {
                return node;
            }
    } while(node != NULL);

    return NULL;
}

void *allocate_memory(TNode **llist, TNode* node) {
    unsigned int best_node_start_addr = node->pdata->start_addr;
    unsigned int blockSize = node->pdata->size;
    free_mem_node(llist, node);

    add_mem_node(llist, best_node_start_addr, blockSize, ALLOCATED);
    return &_heap[best_node_start_addr];
}

// Allocates size bytes of memory and returns a pointer
// to the first byte.
void *mymalloc(size_t size) {
    if (size > MEMSIZE || size <= 0) return NULL;

    if (buddyList[MAX_ORDER - 1] == NULL) {
        add_mem_node(&buddyList[MAX_ORDER - 1], 0, MEMSIZE, FREE);
    }

    unsigned int required_size = MINIMUM_BLOCK_SIZE;
    int target_order = 0;

    while (required_size < size) {
        required_size <<= 1;
        target_order++;
    }

    TNode *free_block = find_best_free_node(&buddyList[target_order]);
    if (free_block != NULL) {
        return allocate_memory(&buddyList[target_order], free_block);
    }

    for (int order = target_order + 1; order < MAX_ORDER; order++) {
        if (buddyList[order] != NULL) {
            TNode *larger_block = find_best_free_node(&buddyList[order]);
            if (larger_block != NULL) {
                int current_split_order = order;
                while (current_split_order > target_order) {
                    larger_block->pdata->status = ALLOCATED;

                    unsigned int half_size = larger_block->pdata->size / 2;

                    add_mem_node(&buddyList[current_split_order - 1],
                               larger_block->pdata->start_addr, half_size, FREE);
                    add_mem_node(&buddyList[current_split_order - 1],
                               larger_block->pdata->start_addr + half_size, half_size, FREE);

                    current_split_order--;
                    larger_block = find_best_free_node(&buddyList[current_split_order]);
                }

                TNode *target_free_block = find_best_free_node(&buddyList[target_order]);
                return allocate_memory(&buddyList[target_order], target_free_block);
            }
        }
    }

    return NULL;
}

// Frees memory pointer to by ptr.
void myfree(void *ptr) {
    if(ptr == NULL) return;

    unsigned int start_addr = (char *)ptr - _heap;

    TNode *block_to_free = NULL;
    int block_order = -1;

    for (int order = 0; order < MAX_ORDER && block_to_free == NULL; order++) {
        block_to_free = find_node(buddyList[order], start_addr);
        if (block_to_free != NULL) {
            block_order = order;
        }
    }

    if (block_to_free == NULL) {
        dbprintf("Unable to find pointer for freeing\n");
        return;
    }

    if (block_to_free->pdata->status == FREE) {
        dbprintf("Block at address %u is already free\n", start_addr);
        return;
    }

    dbprintf("Freeing block: Start=%u KB, Size=%u KB\n",
             start_addr >> 10, block_to_free->pdata->size >> 10);

    block_to_free->pdata->status = FREE;

    int current_order = block_order;
    TNode *current_block = block_to_free;

    while (current_order < MAX_ORDER - 1) {
        unsigned int buddy_addr = current_block->pdata->start_addr ^
                                 (current_block->pdata->size);

        TNode *buddy_block = find_node(buddyList[current_order], buddy_addr);

        if (buddy_block != NULL && buddy_block->pdata->status == FREE) {
            unsigned int merged_start = (current_block->pdata->start_addr < buddy_addr) ?
                                      current_block->pdata->start_addr : buddy_addr;
            unsigned int merged_size = current_block->pdata->size * 2;

            free_mem_node(&buddyList[current_order], current_block);
            free_mem_node(&buddyList[current_order], buddy_block);

            add_mem_node(&buddyList[current_order + 1], merged_start, merged_size, FREE);

            current_order++;
            current_block = find_node(buddyList[current_order], merged_start);
        } else {
            break;
        }
    }
}

