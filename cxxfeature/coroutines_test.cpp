#include "coroutines_test.h"
#include "./base/ptype.hpp"
#include <algorithm>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <string_view>
#include <vector>

namespace Test
{

namespace TestCoroutinesSimple
{

struct generator
{
  struct promise_type;
  using handle = std::coroutine_handle<promise_type>;

  struct promise_type
  {
    int current_value;
    static auto get_return_object_on_allocation_failure()
    {
      return generator{nullptr};
    }
    auto get_return_object() { return generator{handle::from_promise(*this)}; }
    auto initial_suspend() { return std::suspend_always{}; }
    auto final_suspend() { return std::suspend_always{}; }
    void unhandled_exception() { std::terminate(); }
    void return_void() {}
    auto yield_value(int value)
    {
      current_value = value;
      return std::suspend_always{}; // 这是一个 awaiter 结构, 见第二篇文章
    }
  };
  bool move_next() { return coro ? (coro.resume(), !coro.done()) : false; }
  int current_value() { return coro.promise().current_value; }
  generator(generator const &) = delete;
  generator(generator &&rhs)
      : coro(rhs.coro)
  {
    rhs.coro = nullptr;
  }
  ~generator()
  {
    if (coro) coro.destroy();
  }
private:
  generator(handle h)
      : coro(h)
  {
  }
  handle coro;
};

generator f()
{
  co_yield 1;
  co_yield 2;
}

void Test()
{
  auto g = f();         // 停在 initial_suspend 那里
  while (g.move_next()) // 每次调用就停在下一个 co_await 那里
  {
    std::cout << g.current_value() << std::endl;
  }
}

} // namespace TestCoroutinesSimple

namespace TestCoroutinesHelloWorld
{
struct HelloCoroutine // Coroutine
{
public:
  struct HelloPromise
  {
  public:
    HelloCoroutine get_return_object()
    {
      return std::coroutine_handle<HelloPromise>::from_promise(*this); // 协程句柄用于控制协程
    }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() { return {}; }
    void unhandled_exception() {}
  };
public:
  using promise_type = HelloPromise;
  using PromiseHandle = std::coroutine_handle<HelloPromise>;
  HelloCoroutine(PromiseHandle h)
      : handle_(h)
  {
  }
  PromiseHandle handle_;
};
HelloCoroutine hello()
{
  std::cout << "Hello" << std::endl;
  co_await std::suspend_always{};
  std::cout << "world!" << std::endl;
}
void Test()
{
  HelloCoroutine co = hello();
  co.handle_.resume();

  std::cout << "destory" << std::endl;
  co.handle_.destroy();
}
} // namespace TestCoroutinesHelloWorld

namespace TestCoroutinesYield
{

struct HelloCoroutine // Coroutine
{
public:
  struct HelloPromise // return value
  {
  public:
    std::string_view value_;
  public:
    HelloCoroutine get_return_object()
    {
      return std::coroutine_handle<HelloPromise>::from_promise(*this); // 协程句柄用于控制协程
    }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() { return {}; }
    void unhandled_exception() {}
    std::suspend_always yeild_value(std::string_view value)
    {
      value_ = value;
      std::count << value_ << std::endl;
      return {};
    }
  };
public:
  using promise_type = HelloPromise;
  using PromiseHandle = std::coroutine_handle<HelloPromise>;
  HelloCoroutine(PromiseHandle h)
      : handle_(h)
  {
  }
  PromiseHandle handle_;
};
HelloCoroutine hello()
{
  std::cout << "Hello" << std::endl;
  co_yeild;
  std::cout << "world!" << std::endl;
}
void Test()
{
  HelloCoroutine co = hello();

  std::cout << "calling resume" << std::endl;
  co.handle_.resume();

  std::cout << "destory" << std::endl;
  co.handle_.destroy();
}
} // namespace TestCoroutinesYield

void DoTest()
{
  TestCoroutinesSimple::Test();
  TestCoroutinesHelloWorld::Test();
}

} // namespace Test
