#include <iostream>
#include <chrono>
#include <thread>
#include <bitset>
#include "coroutines_test.h"

int main(int argn, char** argv)
{
    Test::DoTest();
    // Test::DoPthreadTest();
    // Test::DoStreamTest();
    // Test::DoNetTest(argn, argv);
    return 0;
}
