#include "algorithm_test.h"
#include "coroutine_unitls_test.h"
#include "coroutines_test.h"
#include "grammar_test.h"
#include "std_test.h"
#include <bitset>
#include <chrono>
#include <iostream>
#include <thread>

int main(int argn, char** argv)
{
    // Test::DoCoroutinesTest();
    // Test::DoCoroutineUnitlsTest();
    // Test::DoAlgorithmTest();
    Test::DoGrammarTest();
    // Test::DoStdTest();

    return 0;
}
