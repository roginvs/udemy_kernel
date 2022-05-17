#include <stdio.h>
// #include <string.h>
#include "CUnit/Basic.h"
#include <CUnit/CUnit.h>
#include <CUnit/CUError.h>
#include <stdlib.h>

#include "mocks/paging_load_directory.h"

#include "../src/memory/paging/paging.h"
#include "../src/memory/paging/paging.c"

int init_paging_suite()
{
    return 0;
}
int clean_paging_suite()
{
    return 0;
}

int test_zero_allocations()
{
    CU_ASSERT_EQUAL(mock_allocs_count, 0);
}

int test_paging_new_4gb()
{
    // We test that paging_new_4gb returns plain paging
    uint8_t test_flags_array[] = {0, PAGING_IS_PRESENT, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL};
    for (int i = 0; i < sizeof(test_flags_array) / sizeof(uint8_t); i++)
    {
        uint8_t test_flags = test_flags_array[i];

        struct paging_4gb_chunk *chunk_4gb = paging_new_4gb(test_flags);
        uint32_t *directory_table = chunk_4gb->directory_entry;

        // First table, 1024 items
        for (int table_index = 0; table_index < 1024; table_index++)
        {
            uint32_t *table = (uint32_t *)(directory_table[table_index] & 0xFFFFF000);

            for (int table_table_index = 0; table_table_index < 1024; table_table_index++)
            {
                uint32_t table_address = 1024 * 4096 * table_index + table_table_index * 4096;
                CU_ASSERT_EQUAL(table[table_table_index] & 0xFFFFF000, table_address); // Points to zero address in memory
                CU_ASSERT_EQUAL(table[table_table_index] & 0x00000FFF, test_flags);
            }
        }

        paging_free_4gb(chunk_4gb);

        CU_ASSERT_EQUAL(mock_allocs_count, 0);
    }
}
void add_paging_suite()
{
    CU_pSuite pSuite = CU_add_suite("Paging", init_paging_suite, clean_paging_suite);
    if (NULL == pSuite)
    {
        exit(1);
    }

    CU_ADD_TEST(pSuite, test_paging_new_4gb);
    CU_ADD_TEST(pSuite, test_zero_allocations);

    /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
    /*
    if ((NULL == CU_add_test(pSuite, "test of fprintf()", testFPRINTF)) ||
        (NULL == CU_add_test(pSuite, "test of fread()", testFREAD)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }
    */
}