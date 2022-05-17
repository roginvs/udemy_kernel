#include <stdio.h>
// #include <string.h>
#include "CUnit/Basic.h"
#include <CUnit/CUnit.h>
#include <CUnit/CUError.h>
#include <stdlib.h>

int init_paging_suite()
{
}
int clean_paging_suite()
{
}
void add_paging_suite()
{
    CU_pSuite pSuite = CU_add_suite("Suite_1", init_paging_suite, clean_paging_suite);
    if (NULL == pSuite)
    {
        exit(1);
    }

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