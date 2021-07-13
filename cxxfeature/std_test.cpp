#include "std_test.h"
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <stdio.h>
#include <vector>

namespace Test
{
namespace Std_trace_container
{

template <typename T1, typename T2>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& x)
{
    return os << x.first << '=' << x.second;
}

template <typename ContainerType>
inline std::ostream& TraceContainer(std::ostream& os, const ContainerType& x)
{
    os << '[';
    typename ContainerType::const_iterator i = x.begin();
    typename ContainerType::const_iterator e = x.end();
    if (i != e)
    {
        os << *i;
        for (++i; i != e; ++i)
        {
            os << ',' << *i;
        }
    }
    os << ']';
    return os;
}

template <typename T1, typename T2>
inline std::ostream& operator<<(std::ostream& os, const std::map<T1, T2>& x)
{
    return TraceContainer(os, x);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& x)
{
    return TraceContainer(os, x);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const std::list<T>& x)
{
    return TraceContainer(os, x);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const std::deque<T>& x)
{
    return TraceContainer(os, x);
}

void Test()
{
    std::map<int, int> map = {
        {1, 1},
        {2, 2},
        {3, 3}
    };
    std::list<int> list = {1, 2, 3};
    std::cout << map;
    std::cout << list << "\n";
    std::cout << std::endl;
}

} // namespace Std_trace_container

void DoStdTest()
{
    Std_trace_container::Test();
}
} // namespace Test