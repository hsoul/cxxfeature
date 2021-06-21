#include "coroutine_unitls_test.h"
#include "coroutines_test.h"
#include <bitset>
#include <chrono>
#include <iostream>
#include <thread>

int main(int argn, char** argv)
{
    // Test::DoTest();
    Test::DoCoroutineUnitlsTest();
    // Test::DoPthreadTest();
    // Test::DoStreamTest();
    // Test::DoNetTest(argn, argv);
    return 0;
}
