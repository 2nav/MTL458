#include <stdio.h>
#include "mmu.h"

int main()
{
    // Test your implementation here
    int *ptr = (int *)my_malloc(10 * sizeof(int));
    printf("Pointer 1 %lld\n", ptr);
    for (int i = 0; i < 10; i++)
    {
        ptr[i] = i;
    }
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", ptr[i]);
    }
    printf("%lld\n", ptr);
    // get size from header of malloc block
    malloc_header_t *header = (malloc_header_t *)((void *)ptr - sizeof(malloc_header_t));
    printf("Size of block: %d\n", header->size);
    printf("Magic number: %x\n", header->magic);
    my_free(ptr);
    printf("Magic number: %x\n", header->magic);

    int *ptr2 = (int *)my_malloc(30 * sizeof(int));
    printf("Pointer 2 %lld\n", ptr2);
    for (int i = 0; i < 40; i++)
    {
        ptr2[i] = i;
    }
    malloc_header_t *header2 = (malloc_header_t *)((void *)ptr2 - sizeof(malloc_header_t));
    printf("Size of block: %d\n", header2->size);
    printf("Magic number: %x\n", header2->magic);
    my_free(ptr2);
    printf("Magic number: %x\n", header2->magic);
    printf("Head size %d\n", head->size);

    int *ptr3 = (int *)my_malloc(10 * sizeof(int));
    printf("Pointer 3 %lld\n", ptr3);
    for (int i = 0; i < 10; i++)
    {
        ptr3[i] = i;
    }

    int *ptr4 = (int *)my_malloc(100 * sizeof(int));
    printf("Pointer 4 %lld\n", ptr4);
    my_free(ptr3);
    printf("head size %d\n", head->size);
    my_free(ptr4);
    printf("head size %d\n", head->size);
    return 0;
}