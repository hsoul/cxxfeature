#include "type_traits_test.h"

#include <iostream>
#include <type_traits>

namespace Test
{

namespace TT_remove_cv
{
void Test()
{
    typedef std::remove_cv<int>::type A;
    typedef std::remove_cv<const int>::type B;
    typedef std::remove_cv<volatile int>::type C;
    typedef std::remove_cv<const volatile int &>::type D;

    std::cout << std::boolalpha;

    std::cout << "A: "
              << std::is_same<const volatile int, A>::value
              << std::endl;

    std::cout << "B: "
              << std::is_same<const volatile int, B>::value
              << std::endl;

    std::cout << "C: "
              << std::is_same<int, C>::value
              << std::endl;

    std::cout << "D: "
              << std::is_same<int, D>::value
              << std::endl;
}

} // namespace TT_remove_cv

void DoTypeTraitsTest()
{
    TT_remove_cv::Test();
}

} // namespace Test