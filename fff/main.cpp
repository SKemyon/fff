#include "fff.h"
#include "tests_insert.h"
#include "tests_cnstr.h"
#include "tests_copy_cleat_reserve.h"
#include "tests_operator.h"
#include "tests_etc.h"

int main()
{
    runAllTests();
    AllCopyClearReserv();
    runAllCnstr();
    runAllInsert();
    AllOperators();
}