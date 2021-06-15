#include "coroutines_test.h"
#include "./base/ptype.hpp"
#include <algorithm>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
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
  generator(generator const&) = delete;
  generator(generator&& rhs)
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
    std::suspend_always yield_value(std::string_view value)
    {
      value_ = value;
      std::cout << value_ << std::endl;
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
  std::string_view s("hello");
  // std::cout << "Hello" << std::endl;
  co_yield s;
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

namespace TestCoroutines_co_return
{
struct HelloCoroutine // Coroutine
{
public:
  struct HelloPromise // return value
  {
  public:
    HelloCoroutine get_return_object()
    {
      return std::coroutine_handle<HelloPromise>::from_promise(*this); // 协程句柄用于控制协程
    }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() { return {}; }
    void unhandled_exception() {}
    void return_value(int value)
    {
      std::cout << "got co_return value" << value << std::endl;
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
  co_await std::suspend_always{};
  std::cout << "world!" << std::endl;
  co_return 42;
}
void Test()
{
  HelloCoroutine co = hello();

  std::cout << "calling resume" << std::endl;
  co.handle_.resume();

  std::cout << "destory" << std::endl;
  co.handle_.destroy();
}
} // namespace TestCoroutines_co_return

namespace TestCoroutines_guanwang
{
auto switch_to_new_thread(std::jthread& out)
{
  struct awaitable
  {
    std::jthread* p_out;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h)
    {
      std::jthread& out = *p_out;
      if (out.joinable())
        throw std::runtime_error("Output jthread parameter not empty");
      out = std::jthread([h] { h.resume(); });
      // Potential undefined behavior: accessing potentially destroyed *this
      // std::cout << "New thread ID: " << p_out->get_id() << '\n';
      std::cout << "New thread ID: " << out.get_id() << '\n'; // this is OK
    }
    void await_resume() {}
  };
  return awaitable{&out};
}

struct task
{
  struct promise_type
  {
    task get_return_object() { return {}; }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };
};

task resuming_on_new_thread(std::jthread& out)
{
  std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
  co_await switch_to_new_thread(out);
  // awaiter destroyed here
  std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}

void Test()
{
  std::jthread out;
  resuming_on_new_thread(out);
}
} // namespace TestCoroutines_guanwang

namespace TestCoroutines_future
{
using namespace std::chrono_literals;

// std::future<std::string> remote_query(uint32_t query_index)
// {
//   return std::async([query_index]() {
//     std::this_thread::sleep_for(4s);
//     return std::to_string(query_index) + " times query: Hello, world!";
//   });
// }

// std::future<void> remote_query_all()
// {
//   uint32_t query_index = 0;
//   for (;;)
//   {
//     std::string ret = co_await remote_query(++query_index);
//     std::cout << ret << std::endl;
//   }
// }

// void Test()
// {
//   remote_query_all();

//   while (true)
//   {
//     std::this_thread::sleep_for(1s);
//     std::cout << "main thread doing other things..." << std::endl;
//   }
// }
} // namespace TestCoroutines_future

void DoTest()
{
  // TestCoroutinesSimple::Test();
  // TestCoroutinesHelloWorld::Test();
  // TestCoroutinesYield::Test();
  // TestCoroutines_co_return::Test();
  // TestCoroutines_guanwang::Test();
  // TestCoroutines_future::Test();
}

} // namespace Test
