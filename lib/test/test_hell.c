#include "hell.c"
#include "ctest.h"


TEST_DEFINE(testAdd, res)
{
        TEST_AUTONAME(res);
        test_check(res, "Correctly add 5 and 3", add(5, 3) == 8);
}


TEST_START(
        testAdd,
)
