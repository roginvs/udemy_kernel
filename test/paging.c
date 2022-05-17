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
    CU_ASSERT(0 == mock_allocs_count);
}
void add_paging_suite()
{
    CU_pSuite pSuite = CU_add_suite("Suite_1", init_paging_suite, clean_paging_suite);
    if (NULL == pSuite)
    {
        exit(1);
    }

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