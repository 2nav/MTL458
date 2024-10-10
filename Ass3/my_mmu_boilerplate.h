#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
// Include your Headers below

// You are not allowed to use the function malloc and calloc directly .

const size_t MEM_SIZE = 4096;

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
    head->size = 4096 - sizeof(node_t);
    head->next = NULL;
}

// Function to allocate memory using mmap
void *my_malloc(size_t size)
{
    // Your implementation of my_malloc goes here
    if (head == NULL)
    {
        init();
    }

    // First fit strategy
    node_t *prev = NULL;
    node_t *curr = head;
    size_t total_size = size + sizeof(malloc_header_t);
    while (curr != NULL)
    {
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
        return NULL;
    }
    // allocate memory
    if (curr->size >= total_size + sizeof(node_t))
    {
        node_t *new_node = (node_t *)((char *)curr + total_size);
        new_node->size = curr->size - total_size;
        new_node->next = curr->next;
        curr->size = total_size;
        curr->next = new_node;
    }
}

// Function to allocate and initialize memory to zero using mmap
void *my_calloc(size_t nelem, size_t size)
{
    // Your implementation of my_calloc goes here
}

// Function to release memory allocated using my_malloc and my_calloc
void my_free(void *ptr)
{
    // Your implementation of my_free goes here
}
