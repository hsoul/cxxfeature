#include "grammar_test.h"
#include <iostream>
#include <map>
#include <stdio.h>
#include <vector>

namespace Test
{
namespace Grammar_emplace
{
class T
{
public:
    T() { printf("T() common construct\n"); }
    T(int i) { printf("T(int) common construct\n"); }
    T(const T& t) { printf("T(T&) copy construct\n"); }
    T(T&& t) { printf("T(T&&) move construct\n"); }
    T& operator=(T& arg)
    {
        printf("T assignmet\n");
        return *this;
    }
};

static T g_t;

void func(T a)
{
    g_t = a;
}

void Test()
{
    std::vector<T> vec(32);
    vec.clear();
    std::map<int, T> map;
    T temp;

    printf("\n---- std::move(temp)  ----\n");
    std::move(temp);

    printf("\n---- push_back(T())  ----\n");
    vec.push_back(T());

    printf("\n---- push_back(T(1)) ----\n");
    vec.push_back(T(1));

    printf("\n---- emplace_back()  ----\n");
    vec.emplace_back();

    printf("\n---- emplace_back(1) ----\n");
    vec.emplace_back(1);

    printf("\n---- emplace_back(std::move(temp)) ----\n");
    vec.emplace_back(std::move(temp));

    printf("\n---- emplace_back(temp) ----\n");
    vec.emplace_back(temp);

    printf("\n---- func(temp) ----\n");
    func(temp);

    printf("\n---- map[1] = temp ----\n");
    map[1] = temp;

    printf("\n---- map.insert({2, temp}) ----\n");
    map.insert({2, temp});

    printf("\n---- map.insert(std::make_pair(3, temp)) ----\n");
    map.insert(std::make_pair(3, temp));

    printf("\n---- map.emplace(4, temp); ----\n");
    map.emplace(4, temp);

    printf("\n---- map.emplace(5, temp); ----\n");
    map.emplace(5, 3);

    printf("\n---- results ----\n");
    printf("vec.size() = %d, map.size() %d\n", (int)vec.size(), (int)map.size());
}

} // namespace Grammar_emplace

void DoGrammarTest()
{
    Grammar_emplace::Test();
}

} // namespace Test