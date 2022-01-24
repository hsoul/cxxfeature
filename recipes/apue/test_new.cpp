#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <unordered_map>
#include <vector>

#define COMMON_NEW

#ifdef COMMON_NEW

std::unordered_map<void*, std::string> g_using_memory;

void* operator new(size_t size, const char* file, int line)
{
    std::cout << "new memory size : " << size << " file : " << file << " line : " << line << std::endl;
    std::string memory_pos = file;
    file += ':';
    file += line;
    void* ptr = ::operator new(size);
    g_using_memory.insert({ptr, std::move(memory_pos)});
    return ptr;
}

void operator delete(void* ptr)
{
    auto iter = g_using_memory.find(ptr);
    if (iter != g_using_memory.end())
    {
        std::cout << "delete memory ptr : " << ptr << " info : " << iter->second << std::endl;
        g_using_memory.erase(iter);
    }

    ::free(ptr);
}

    #define new new (__FILE__, __LINE__)

#endif

class Obj
{
public:
    Obj(int a)
        : a_(a)
    {
    }
public:
    int a_;
    double b_;
    char* c_;
};

int main(void)
{
    Obj* obj = new Obj(5);
    std::cout << obj->a_ << std::endl;
    delete obj;
    std::vector<Obj> vec = {1, 3, 6};
    for (const auto& elem : vec)
    {
        std::cout << elem.a_ << std::endl;
    }
    std::cout << "finish" << std::endl;
    return 0;
}