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
    struct paging_4gb_chunk *kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_free_4gb(kernel_chunk);
}
void add_paging_suite()
{
    CU_pSuite pSuite = CU_add_suite("Suite_1", init_paging_suite, clean_paging_suite);
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