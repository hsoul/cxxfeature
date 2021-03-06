#include "grammar_test.h"
#include <chrono>
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
        Rep* rep = new (cap) Rep; // sizeof(Rep) ?????????????????????cap ??????????????????
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

#pragma pack(push, 1) //???????????????1, 2, 4, 8
struct S1
{
    char m1;
    short m2;
};
#pragma pack(pop)

#pragma pack(push, 4) //???????????????1, 2, 4, 8
struct S2
{
    char m1;
    short m2;
};
#pragma pack(pop)

void Test()
{
    struct S1 s1;
    struct S1 s2;
    s1.m2 = 0x3217;
    printf("s    address:   %p\n", &s1);
    printf("s.m2 address:   %p\n", &(s1.m2));
    printf("s    address:   %p\n", &s2);
    printf("s.m2 address:   %p\n", &(s2.m2));

    short x = 0x3217;
    char* p = (char*)&x;
    int b1 = p[0];
    int b2 = p[1];
    printf("sizeof x     %d: %x %x %p %p\n", (int)sizeof(x), b1, b2, p, p + 1);

    p = (char*)&s1.m2;
    b1 = p[0];
    b2 = p[1];
    printf("sizeof s1.m2 %d: %x %x %p %p\n", (int)sizeof(s1.m2), b1, b2, p, p + 1);

    union
    {
        int a;  // 4 bytes
        char b; // 1 byte
    } data;

    data.a = 1; //???4 bytes??????????????????????????? 0x 00 00 00 01

    // b?????????char?????????1Byte???a?????????int??????4Byte
    //?????????????????????data??????????????????b??????????????????a??????????????????????????????
    if (1 == data.b)
    { //??????case??????a??????????????????????????????b??????a?????????????????????????????????????????????(??????)????????????????????????????????????
        printf("Little_Endian\n");
    }
    else
    {
        printf("Big_Endian\n");
    }
}

} // namespace Std_byte_order

namespace Grammar_out_for_define_vars
{
static std::vector<int> vec = {0, 1};

void func(int& v)
{
    ++v;
    vec[v] = 1;
}

void Test()
{
    int times = 10000000;
    auto start = std::chrono::high_resolution_clock::now();
    int a = 0;
    for (int i = 0; i < times; ++i)
    {
        a = 0;
        func(a);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double count = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "out define [" << times << "] time " << count << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < times; ++i)
    {
        int b = 0;
        func(b);
    }
    end = std::chrono::high_resolution_clock::now();
    count = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "int define [" << times << "] time " << count << std::endl;
}

} // namespace Grammar_out_for_define_vars

namespace Grammar_move_bin
{
#define COLOR_COUNT_MAX 8
#define COLOR_MASK_MAX  ((1 << COLOR_COUNT_MAX) - 1)

void Test()
{
    auto&& swar = [](unsigned char i) -> unsigned char {
        i = (i & 0x55) + ((i >> 1) & 0x55);
        i = (i & 0x33) + ((i >> 2) & 0x33);
        i = (i & 0x0F) + ((i >> 4) & 0x0F);
        return i;
    };
    std::array<int, COLOR_MASK_MAX + 1> color_mask_2_count;
    for (int i = 0; i <= COLOR_MASK_MAX; ++i)
    {
        color_mask_2_count[i] = swar((unsigned char)i);
    }

    for (int i = 0; i < (int)color_mask_2_count.size(); ++i)
    {
        printf("%d -> %d\n", i, color_mask_2_count[i]);
    }
}

} // namespace Grammar_move_bin

namespace Grammar_string
{

#define SUB_STR "hello world"

void Test()
{
    std::string str = "begin'" SUB_STR "'end";
    std::cout << str << std::endl;
    const char* c_str = // "begin'" "sdfasf" "'end" // ???????????????????????????????????????????????????????????????????????? // https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation
        "begin'"
        "sdfasf"
        "'end";
    std::cout << c_str << std::endl;
}

} // namespace Grammar_string

void DoGrammarTest()
{
    // Std_class_new::Test();
    // Std_byte_order::Test();
    // Grammar_out_for_define_vars::Test();
    // Grammar_move_bin::Test();
    Grammar_string::Test();
}

} // namespace Test