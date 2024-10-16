#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
// Include your Headers below

// You are not allowed to use the function malloc and calloc directly .

const size_t MEM_SIZE = 4096;
void *last;
int chunks = 0;

void *my_malloc(size_t size);
// OSTEP chapter 17
typedef struct __node_t
{
    size_t size;
    struct __node_t *next;
} node_t;

typedef struct
{
    size_t size;
    uint32_t magic;
} malloc_header_t;

node_t *head = NULL;

void init()
{
    head = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    head->size = MEM_SIZE - sizeof(node_t);
    head->next = NULL;
    last = (void *)(head + MEM_SIZE);
    // printf("last: %x \n", last);
    chunks++;
}

void *allocate_mmap(size_t size)
{
    size_t allocate_size = (size + 2 * (sizeof(malloc_header_t) + sizeof(node_t)));
    node_t *new_block = (node_t *)mmap(last, allocate_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    last = (void *)new_block + allocate_size;
    if (new_block == MAP_FAILED)
    {
        perror("mmap failed");
        return NULL;
    }
    new_block->size = allocate_size - sizeof(node_t);
    new_block->next = NULL;
    chunks++;
    return new_block;
}

// Function to allocate memory using mmap
void *my_malloc(size_t size)
{
    // Your implementation of my_malloc goes here
    if (head == NULL)
    {
        init();
    }
    if (size == 0)
    {
        return NULL;
    }

    // First fit strategy
    node_t *prev = NULL;
    node_t *curr = head;
    size_t total_size = size + sizeof(malloc_header_t);
    while (curr != NULL)
    {
        // printf("Curr %p Size: %d\n", curr, curr->size);
        if (curr->size >= total_size)
        {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // allocate memory or return if not possible
    if (curr == NULL)
    {
        // No suitable block found, so extend
        node_t *new_chunk = (node_t *)allocate_mmap(size);
        if (new_chunk == NULL)
        {
            return NULL;
        }
        prev->next = new_chunk;
        // return my_malloc(size);
        curr = new_chunk;
        if (curr == NULL)
            return NULL; // sbrk failure
    }
    // printf("Chunks size: %d\n", chunks);
    // allocate memory
    size_t remaining_size = curr->size - total_size;
    if (remaining_size >= 0)
    {
        // split the block, allocate the memory
        node_t *next_block = curr->next;
        malloc_header_t *header = (malloc_header_t *)(curr);
        header->size = size;
        header->magic = 0x12345678;
        // free part of the block
        node_t *new_block = (node_t *)((void *)curr + total_size);
        new_block->size = remaining_size;
        new_block->next = next_block;
        if (prev == NULL)
        {
            head = new_block;
        }
        else
        {
            prev->next = new_block;
        }
        return (void *)header + sizeof(malloc_header_t);
    }
    else
    {
        // allocate the whole block
        if (prev == NULL)
        {
            head = curr->next;
        }
        else
        {
            prev->next = curr->next;
        }
        // header for malloc
        malloc_header_t *header = (malloc_header_t *)(curr);
        header->size = size;
        header->magic = 0x12345678;
        return (void *)(header + 1);
    }
}

// Function to allocate and initialize memory to zero using mmap
void *my_calloc(size_t nelem, size_t size)
{
    // Your implementation of my_calloc goes here
    size_t total_size = nelem * size;
    void *ptr = my_malloc(total_size);
    if (ptr == NULL)
    {
        return NULL;
    }
    memset(ptr, 0, total_size);
    return ptr;
}

// Function to release memory allocated using my_malloc and my_calloc
void my_free(void *ptr)
{
    // Your implementation of my_free goes here
    if (ptr == NULL)
    {
        return;
    }
    malloc_header_t *header = (malloc_header_t *)((void *)ptr - sizeof(malloc_header_t));
    if (header->magic != 0x12345678)
    {
        perror("Invalid pointer(Magic BT)\n");
        return;
    }
    // printf("Freeing block of size: %d\n", header->size);
    // printf("Header: %d\n", header->magic);
    header->magic = 0; // invalidate the header
    // printf("Header: %d\n", header->magic);

    node_t *curr = (node_t *)header;
    curr->size = header->size + sizeof(malloc_header_t) - sizeof(node_t);

    // insert the block in the free list, ordered by address
    node_t *prev = NULL;
    node_t *next = head;
    while (next != NULL)
    {
        if (next > curr)
        {
            break;
        }
        prev = next;
        next = next->next;
    }
    if (prev == NULL)
    {
        head = curr;
        curr->next = next;
    }
    else
    {
        prev->next = curr;
        curr->next = next;
    }

    // Coalesce the list
    if (curr->next != NULL && (void *)curr + curr->size + sizeof(node_t) == (void *)curr->next)
    {
        curr->size += curr->next->size + sizeof(node_t);
        curr->next = curr->next->next;
        // printf("Coalesced");
    }
    if (prev != NULL && (void *)prev + prev->size + sizeof(node_t) == (void *)curr)
    {
        prev->size += curr->size + sizeof(node_t);
        prev->next = curr->next;
        // printf("Coalesced");
    }
}
