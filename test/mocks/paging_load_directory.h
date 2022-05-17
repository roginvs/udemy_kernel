
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

void paging_load_directory(uint32_t *directory)
{
    printf("ERROR: paging_load_directory should never be called in tests\n");
    exit(1);
}