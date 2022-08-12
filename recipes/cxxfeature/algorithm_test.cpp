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

namespace Algorithm_itos
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

template <typename T>
size_t convert(char buf[], T value)
{
    T i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';

    std::reverse(buf, p);

    return p - buf;
}

void Test()
{
    char buf[32] = {0};
    convert(buf, 3276);
    std::cout << strlen(buf) << " : " << buf << std::endl;
}

} // namespace Algorithm_itos

void DoAlgorithmTest()
{
    // Algorithm_copy_n::Test();
    Algorithm_itos::Test();

    char atleast[1];
}
} // namespace Test