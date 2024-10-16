#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#include "mmu.h"

int main()
{
    // 1. Simple allocation and deallocation
    printf("Test 1: Simple allocation and deallocation\n");
    int *num = (int *)my_malloc(sizeof(int));
    assert(num != NULL); // Ensure memory was allocated
    *num = 42;           // Store a value in the allocated memory
    printf("Value stored: %d\n", *num);
    my_free(num); // Deallocate memory
    printf("Simple allocation and deallocation passed\n\n");

    // 2. Allocate and zero-initialize memory (emulate calloc)
    printf("Test 2: Allocate and initialize memory to zero\n");
    size_t arr_size = 10;
    int *arr = (int *)my_malloc(arr_size * sizeof(int));
    assert(arr != NULL);
    memset(arr, 0, arr_size * sizeof(int)); // Zero out the memory
    for (size_t i = 0; i < arr_size; i++)
    {
        assert(arr[i] == 0); // Ensure memory is zero-initialized
    }
    printf("Array initialized with zeros and verified.\n");
    my_free(arr);
    printf("Array deallocation passed\n\n");

    // 3. Allocate large blocks of memory
    printf("Test 3: Allocating a large block of memory\n");
    size_t large_size = 1000000; // 1 million ints
    int *large_block = (int *)my_malloc(large_size * sizeof(int));
    assert(large_block != NULL); // Ensure large memory allocation succeeded
    for (size_t i = 0; i < large_size; i++)
    {
        large_block[i] = i; // Store values
    }
    printf("Large block allocation and data storage passed\n");
    my_free(large_block);
    printf("Large block deallocation passed\n\n");

    // 4. Test for NULL pointer
    printf("Test 4: Allocation of zero bytes\n");
    void *zero_alloc = my_malloc(0);
    assert(zero_alloc == NULL); // Should return NULL for zero-size allocation
    printf("Zero-size allocation returned NULL as expected.\n\n");

    // 5. Stress test with multiple allocations and deallocations
    printf("Test 5: Stress test with multiple allocations and deallocations\n");
    const int NUM_BLOCKS = 1000;
    void *blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        blocks[i] = my_malloc((i + 1) * 10); // Allocate increasing sizes
        assert(blocks[i] != NULL);
    }
    printf("Multiple allocations succeeded.\n");
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        my_free(blocks[i]);
    }
    printf("Multiple deallocations passed.\n\n");

    // 6. Test freeing a NULL pointer (should do nothing)
    printf("Test 6: Freeing a NULL pointer\n");
    my_free(NULL); // Should not cause any errors
    printf("Freeing NULL passed.\n\n");

    // 7. Test for realloc-like behavior (allocate new block, move data)
    printf("Test 7: Allocate, deallocate, and reallocate\n");
    char *str = (char *)my_malloc(50 * sizeof(char)); // Allocate 50 bytes
    assert(str != NULL);
    strcpy(str, "Testing custom my_malloc and my_free");
    printf("Stored string: %s\n", str);
    my_free(str);                                // Deallocate
    str = (char *)my_malloc(100 * sizeof(char)); // Reallocate with larger size
    assert(str != NULL);
    strcpy(str, "Reallocated memory is working fine");
    printf("New string after reallocation: %s\n", str);
    my_free(str);
    printf("Reallocation test passed.\n\n");

    // 8. Boundary conditions (allocate the exact same memory block after free)
    printf("Test 8: Allocate same size after free\n");
    int *a = (int *)my_malloc(32 * sizeof(int));
    int *b = (int *)my_malloc(32 * sizeof(int));
    printf("head size %d\n", head->size);
    my_free(a);
    printf("head size %d\n", head->size);
    int *c = (int *)my_malloc(32 * sizeof(int)); // Check if same space is reused
    printf("Two allocations %p %p %p \n", a, b, c);
    assert(c == a); // Expecting the same address if memory reuse works properly
    printf("Same size memory reuse passed.\n");
    my_free(b);
    my_free(c);

    printf("All tests passed successfully!\n");
    return 0;
}
