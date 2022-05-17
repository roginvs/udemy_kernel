#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

int mock_allocs_count = 0;

void *kzalloc(size_t size)
{
    mock_allocs_count++;
    // return calloc(1, size);
    return aligned_alloc(4096, size);
}

void *kmalloc(size_t size)
{
    return kzalloc(size);
}

void kfree(void *ptr)
{
    mock_allocs_count--;
    return free(ptr);
}
