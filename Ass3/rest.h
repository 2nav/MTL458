#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int ct = 0;

typedef struct block_header
{
    size_t size;
    int is_available;
    struct block_header *nxt;
    struct block_header *prv;

} block_header;

static block_header *head = NULL;

block_header *find_first_free_block(size_t size);
block_header *get_space(size_t size);
void split(block_header *block, size_t size);
void append(block_header *block);
void coalesce(block_header *block);

void set_block_attributes(block_header *block, size_t size, int is_available, block_header *nxt, block_header *prv)
{
    printf("set_block_attributes\n");
    printf("%d %p\n", block == NULL, block.size);
    block->size = size;
    printf("set_block_attributes\n");
    block->is_available = is_available;
    block->nxt = nxt;
    block->prv = prv;
}

void *my_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    block_header *cur_block = NULL;
    size = (size + 7) & ~7;
    cur_block = find_first_free_block(size);

    if (cur_block)
    {
        cur_block->is_available = 0;
        if (cur_block->size >= size + sizeof(block_header) + 8)
            split(cur_block, size);
        printf("%d\n", cur_block->size);
        return (void *)(cur_block + 1);
    }
    else
    {
        cur_block = get_space(size);
        if (!cur_block)
            return NULL;
        append(cur_block);
        return (void *)(cur_block + 1);
    }
}

void *my_calloc(size_t nelem, size_t elsize)
{
    if (nelem == 0 || elsize == 0)
        return NULL;

    // // Check for multiplication overflow
    // if (nelem != 0 && _UINTPTR_MAX_ / nelem < elsize) {
    //     // Overflow detected
    //     return NULL;
    // }

    size_t size = nelem * elsize;

    // if(ct == 1)
    void *cur_ptr = my_malloc(size);
    if (cur_ptr)
        memset(cur_ptr, 0, size);
    return cur_ptr;
}

void my_free(void *ptr)
{
    if (!ptr)
        return;

    block_header *cur_block = (block_header *)ptr - 1;
    cur_block->is_available = 1;
    coalesce(cur_block);
}

block_header *find_first_free_block(size_t size)
{
    block_header *cur = head;
    while (cur)
    {
        if (cur->is_available && cur->size >= size)
            return cur;
        cur = cur->nxt;
    }
    return NULL;
}

block_header *get_space(size_t size)
{
    block_header *cur_block;
    void *heap_end;

    heap_end = sbrk(0);
    if (sbrk(size + sizeof(block_header)) == (void *)-1)
        return NULL;

    cur_block = (block_header *)heap_end;
    printf("here now\n");
    set_block_attributes(cur_block, size, 0, NULL, NULL);
    return cur_block;
}

void split(block_header *block, size_t size)
{
    block_header *new_block;

    if (ct == 1)
        printf("%d\n", size);

    printf("please\n");

    new_block = (block_header *)((char *)block + sizeof(block_header) + size);
    printf("here %p\n", new_block.size);
    printf("gonna sed\n");
    set_block_attributes(new_block, (block->size) - size - sizeof(block_header), 1, block->nxt, block);
    printf("%d %d \n", (block->size) - size - sizeof(block_header), block->size);
    if (new_block->nxt)
        new_block->nxt->prv = new_block;

    block->size = size;
    block->nxt = new_block;
}

void append(block_header *block)
{
    if (!head)
        head = block;
    else
    {
        block->nxt = head;
        head->prv = block;
        head = block;
        // block_header *current = head;
        // while (current -> nxt) current = current -> nxt;
        // current -> nxt = block;
        // block -> prv = current;
    }
}

void coalesce(block_header *block)
{
    if (block->nxt && block->nxt->is_available)
    {
        block->size += sizeof(block_header) + block->nxt->size;
        block->nxt = block->nxt->nxt;
        if (block->nxt)
            block->nxt->prv = block;
    }

    if (block->prv && block->prv->is_available)
    {
        block->prv->size += sizeof(block_header) + block->size;
        block->prv->nxt = block->nxt;
        if (block->nxt)
            block->nxt->prv = block->prv;
    }
}