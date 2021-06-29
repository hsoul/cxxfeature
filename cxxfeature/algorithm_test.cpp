#include "algorithm_test.h"
#include <algorithm>
#include <iostream>
#include <vector>

namespace Test
{

namespace Algorithm_copy_n
{

void Test()
{
    std::vector<char> v1(10, 0);
    std::vector<char> v2(10, 1);

    std::copy_n(v2.begin() + 5, 4, &*v1.begin() + 3);

    for (const auto& item : v1)
    {
        std::cout << (int)item << " ";
    }

    std::cout << std::endl;
}

} // namespace Algorithm_copy_n

void DoAlgorithmTest()
{
    Algorithm_copy_n::Test();
}
} // namespace Test