#include "fff.h"
#include "tests_insert.h"
#include "tests_cnstr.h"
#include "tests_copy_cleat_reserve.h"
#include "tests_operator.h"
#include "tests_etc.h"
#include <gtest/gtest.h>

int mymain()
{
    runAllTests();
    AllCopyClearReserv();
    runAllCnstr();
    runAllInsert();
    AllOperators();
    return 0;
}