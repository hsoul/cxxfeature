#include "grammar_test.h"
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <netinet/in.h>
#include <ostream>
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

void func(T a)
{
    static T g_t;
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

namespace Std_class_new
{
class Rep
{
public:
    static void* operator new(size_t size, size_t extra_size)
    {
        std::cout << "new size: " << size << " " << extra_size << std::endl;
        return malloc(size + extra_size);
    }
    static void operator delete(void* p)
    {
        std::cout << "delete" << std::endl;
        free(p);
    }
    static Rep* Create(size_t cap)
    {
        Rep* rep = new (cap) Rep; // sizeof(Rep) 为第一个参数，cap 是第二个参数
        return rep;
    }
private:
    int value_;
};

void Test()
{
    std::cout << "sizeof Rpe = " << sizeof(Rep) << std::endl;
    Rep* rep = Rep::Create(8);
    delete rep;
}

} // namespace Std_class_new

namespace Std_byte_order
{

#pragma pack(push, 1) //后面可改为1, 2, 4, 8
struct S1
{
    char m1;
    long m2;
};
#pragma pack(pop)

#pragma pack(push, 4) //后面可改为1, 2, 4, 8
struct S2
{
    char m1;
    long m2;
};
#pragma pack(pop)

void Test()
{
    // struct S1 s1;
    // struct S1 s2;
    // printf("s    address:   %p\n", &s1);
    // printf("s.m2 address:   %p\n", &(s1.m2));
    // printf("s    address:   %p\n", &s2);
    // printf("s.m2 address:   %p\n", &(s2.m2));

    short x = 0x3217;
    char* p = (char*)&x;
    int b1 = p[0];
    int b2 = p[1];
    printf("sizeof x %d: %x %x %p %p\n", (int)sizeof(x), b1, b2, p, p + 1);

    union
    {
        int a;  // 4 bytes
        char b; // 1 byte
    } data;

    data.a = 1; //占4 bytes，十六进制可表示为 0x 00 00 00 01

    // b因为是char型只占1Byte，a因为是int型占4Byte
    //所以，在联合体data所占内存中，b所占内存等于a所占内存的低地址部分
    if (1 == data.b)
    { //走该case说明a的低字节，被取给到了b，即a的低字节存在了联合体所占内存的(起始)低地址，符合小端模式特征
        printf("Little_Endian\n");
    }
    else
    {
        printf("Big_Endian\n");
    }
}

} // namespace Std_byte_order

void DoGrammarTest()
{
    // Std_class_new::Test();
    Std_byte_order::Test();
}

} // namespace Test