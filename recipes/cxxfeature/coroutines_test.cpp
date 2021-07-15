#include "coroutines_test.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <random>
#include <recipes/base/ptype.hpp>
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

namespace TestCoroutines_sender_reciever
{
class Event
{
public:
    Event() = default;

    Event(const Event&) = delete;
    Event(Event&&) = delete;
    Event& operator=(const Event&) = delete;
    Event& operator=(Event&&) = delete;

    class Awaiter;
    Awaiter operator co_await() const noexcept;

    void notify() noexcept;
private:
    friend class Awaiter;

    mutable std::atomic<void*> suspendedWaiter{nullptr};
    mutable std::atomic<bool> notified{false};
};

class Event::Awaiter
{
public:
    Awaiter(const Event& eve)
        : event(eve)
    {
    }

    bool await_ready() const;                                       // co_await suspend前给一次机会，判断是否可以避免挂起协程避免暂停带来的开销
    bool await_suspend(std::coroutine_handle<> corHandle) noexcept; // 暂停前的准备，通常在这个函数中分配任务系统，线程去完成异步操作，然后进入暂停状态。注意这个函数有一个非常重要的参数coroutine_handle(协程句柄)，通过这个句柄我和可以操控协程的状态，通常我们需要将这个句柄保存起来，在适当的时机（异步任务完成时）通过它恢复当前协程
    void await_resume() noexcept {}                                 // 协程恢复后执行的代码，值得注意的是恢复后不是恢复前
private:
    friend class Event;

    const Event& event;
    std::coroutine_handle<> coroutineHandle;
};

bool Event::Awaiter::await_ready() const
{ // (7)

    // allow at most one waiter
    if (event.suspendedWaiter.load() != nullptr)
    {
        throw std::runtime_error("More than one waiter is not valid");
    }

    std::cout << "await_ready() event is notified " << event.notified << std::endl;

    // event.notified == false; suspends the coroutine
    // event.notified == true; the coroutine is executed such as a usual function
    return event.notified;
}
// (8)
bool Event::Awaiter::await_suspend(std::coroutine_handle<> corHandle) noexcept
{
    coroutineHandle = corHandle;

    if (event.notified)
    {
        std::cout << "await_suspend() not need suspend" << std::endl;
        return false; // return false not suspend, continue to call the function body
    }

    std::cout << "await_suspend() need suspend and save context" << std::endl;
    // store the waiter for later notification
    event.suspendedWaiter.store(this); // save context, return true then suspend, wait resume to continue to call the function body

    return true;
}

void Event::notify() noexcept
{ // (6)
    notified = true;

    std::cout << "will notify" << std::endl;

    // try to load the waiter
    auto* waiter = static_cast<Awaiter*>(suspendedWaiter.load());

    std::cout << "get awaiter " << waiter << std::endl;

    // check if a waiter is available
    if (waiter != nullptr)
    {
        // resume the coroutine => await_resume
        waiter->coroutineHandle.resume();
    }
    else
    {
        std::cout << "not notify instantent" << std::endl;
    }
}

Event::Awaiter Event::operator co_await() const noexcept
{
    return Awaiter{*this};
}

struct Task
{
    struct promise_type
    {
        Task get_return_object() { return {}; } // 协程工厂（函数）调用开始用于创建协程对象
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() { return {}; }
        void return_void() {}
        // void return_value(T t) {} // 接收从协程返回的数据
        void unhandled_exception() {}
    };
};

Task receiver(Event& event)
{ // (3)
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "receiver() will await " << &event << std::endl;

    co_await event;

    std::cout << "receiver() await event end " << &event << std::endl;

    std::cout << "Got the notification! " << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Waited " << elapsed.count() << " seconds." << std::endl;
}

using namespace std::chrono_literals;

void Test()
{
    std::cout << std::endl;

    std::cout << "Notification before waiting" << std::endl;
    Event event1{};
    auto senderThread1 = std::thread([&event1] { event1.notify(); }); // (1)
    auto receiverThread1 = std::thread(receiver, std::ref(event1));   // (4)

    receiverThread1.join();
    senderThread1.join();

    std::cout << std::endl;

    std::cout << "Notification after 2 seconds waiting" << std::endl;
    Event event2{};
    auto receiverThread2 = std::thread(receiver, std::ref(event2)); // (5)
    auto senderThread2 = std::thread([&event2] {
        std::this_thread::sleep_for(2s);
        event2.notify(); // (2)
    });

    receiverThread2.join();
    senderThread2.join();

    std::cout << std::endl;
}
} // namespace TestCoroutines_sender_reciever

namespace TestCoroutines_call_sequence
{
template <typename T>
struct MyFuture
{
    std::shared_ptr<T> value;
    MyFuture(std::shared_ptr<T> p)
        : value(p)
    { // (3)
        std::cout << "    MyFuture::MyFuture" << '\n';
    }
    ~MyFuture()
    {
        std::cout << "    MyFuture::~MyFuture" << '\n';
    }
    T get()
    {
        std::cout << "    MyFuture::get" << '\n';
        return *value;
    }

    struct promise_type
    {                                                   // (4)
        std::shared_ptr<T> ptr = std::make_shared<T>(); // (11)
        promise_type()
        {
            std::cout << "        promise_type::promise_type" << '\n';
        }
        ~promise_type()
        {
            std::cout << "        promise_type::~promise_type" << '\n';
        }
        MyFuture<T> get_return_object()
        {
            std::cout << "        promise_type::get_return_object" << '\n';
            return ptr;
        }
        void return_value(T v)
        {
            std::cout << "        promise_type::return_value" << '\n';
            *ptr = v;
        }
        std::suspend_never initial_suspend()
        { // (6)
            std::cout << "        promise_type::initial_suspend" << '\n';
            return {};
        }
        std::suspend_never final_suspend() noexcept
        { // (7)
            std::cout << "        promise_type::final_suspend" << '\n';
            return {};
        }
        void return_void() {}
        void unhandled_exception()
        {
            std::exit(1);
        }
    }; // (5)
};

MyFuture<int> createFuture()
{ // (2)
    std::cout << "createFuture" << '\n';
    co_return 2021;
}

void Test()
{

    std::cout << '\n';

    auto fut = createFuture(); // (1)
    auto res = fut.get();      // (8)
    std::cout << "res: " << res << '\n';

    std::cout << '\n';
}
} // namespace TestCoroutines_call_sequence

namespace TestCoroutines_call_sequence1
{

template <typename T>
struct MyFuture
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    handle_type coro; // (5)

    MyFuture(handle_type h)
        : coro(h)
    {
        std::cout << "    MyFuture::MyFuture" << '\n';
    }
    ~MyFuture()
    {
        std::cout << "    MyFuture::~MyFuture" << '\n';
        if (coro) coro.destroy(); // (8)
    }

    T get()
    {
        std::cout << "    MyFuture::get" << '\n';
        coro.resume(); // (6)
        return coro.promise().result;
    }

    struct promise_type
    {
        T result;
        promise_type()
        {
            std::cout << "        promise_type::promise_type" << '\n';
        }
        ~promise_type()
        {
            std::cout << "        promise_type::~promise_type" << '\n';
        }
        auto get_return_object()
        { // (3)
            std::cout << "        promise_type::get_return_object" << '\n';
            return MyFuture{handle_type::from_promise(*this)};
        }
        void return_value(T v)
        {
            std::cout << "        promise_type::return_value" << '\n';
            result = v;
        }
        std::suspend_always initial_suspend() // 如果在此处挂起的话，执行权会返回到 caller
        {                                     // (1)
            std::cout << "        promise_type::initial_suspend" << '\n';
            return {};
        }
        std::suspend_always final_suspend() noexcept
        { // (2)
            std::cout << "        promise_type::final_suspend" << '\n';
            return {};
        }
        // std::suspend_never final_suspend() noexcept
        // { // (2)
        //     std::cout << "        promise_type::final_suspend" << '\n';
        //     return {};
        // }
        void return_void() {}
        void unhandled_exception()
        {
            std::exit(1);
        }
    };
};

MyFuture<int> createFuture()
{
    std::cout << "createFuture" << '\n';
    co_return 2021;
}

void Test()
{

    std::cout << "begin \n";

    auto fut = createFuture(); // (4) // 函数在此处继续执行不代表协程function body全部执行完，有可能一行代码都没执行，比如 initial_suspend() return std::suspend_always

    {
        auto res = fut.get(); // (7)
        std::cout << "returned res: " << res << '\n';
    }

    // {
    //     std::cout << "not get res: " << '\n';
    // }

    std::cout << "end\n";
}

// begin
//         promise_type::promise_type
//         promise_type::get_return_object
//     MyFuture::MyFuture
//         promise_type::initial_suspend // 此处挂起了协程
// not get res:
// end
//     MyFuture::~MyFuture
//         promise_type::~promise_type

// begin
//         promise_type::promise_type
//         promise_type::get_return_object
//     MyFuture::MyFuture
//         promise_type::initial_suspend
//     MyFuture::get
// createFuture
//         promise_type::return_value
//         promise_type::final_suspend
// res: 2021
// end
//     MyFuture::~MyFuture
//         promise_type::~promise_type

} // namespace TestCoroutines_call_sequence1

namespace TestCoroutines_LazyFutureOnOtherThead
{
template <typename T>
struct MyFuture
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    MyFuture(handle_type h)
        : coro(h)
    {
    }
    ~MyFuture()
    {
        if (coro) coro.destroy();
    }

    T get()
    { // (1)
        std::cout << "    MyFuture::get:  "
                  << "std::this_thread::get_id(): "
                  << std::this_thread::get_id() << '\n';

        std::thread t([this] { coro.resume(); }); // (2)
        t.join();
        return coro.promise().result;
    }

    struct promise_type
    {
        promise_type()
        {
            std::cout << "        promise_type::promise_type:  "
                      << "std::this_thread::get_id(): "
                      << std::this_thread::get_id() << '\n';
        }
        ~promise_type()
        {
            std::cout << "        promise_type::~promise_type:  "
                      << "std::this_thread::get_id(): "
                      << std::this_thread::get_id() << '\n';
        }

        T result;
        auto get_return_object()
        {
            return MyFuture{handle_type::from_promise(*this)};
        }
        void return_value(T v)
        {
            std::cout << "        promise_type::return_value:  "
                      << "std::this_thread::get_id(): "
                      << std::this_thread::get_id() << " value = [";
            std::cout << v << ']' << std::endl;
            result = v;
        }
        std::suspend_always initial_suspend()
        {
            return {};
        }
        std::suspend_always final_suspend() noexcept
        {
            std::cout << "        promise_type::final_suspend:  "
                      << "std::this_thread::get_id(): "
                      << std::this_thread::get_id() << '\n';
            return {};
        }
        void unhandled_exception()
        {
            std::exit(1);
        }
    };
};

MyFuture<int> createFuture()
{
    co_return 2021;
}

void Test()
{
    // std::cout << '\n';

    std::cout << "main:  "
              << "std::this_thread::get_id(): "
              << std::this_thread::get_id() << '\n';

    auto fut = createFuture();
    auto res = fut.get();
    std::cout << "res: " << res << '\n';

    // std::cout << '\n';
}
} // namespace TestCoroutines_LazyFutureOnOtherThead

namespace TestCoroutines_Generator
{
template <typename T>
struct Generator
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    Generator(handle_type h)
        : coro(h)
    {
        std::cout << "        Generator::Generator" << '\n';
    }

    handle_type coro;

    ~Generator()
    {
        std::cout << "        Generator::~Generator" << '\n';
        if (coro) coro.destroy();
    }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& oth)
        : coro(oth.coro)
    {
        oth.coro = nullptr;
    }
    Generator& operator=(Generator&& oth)
    {
        coro = oth.coro;
        oth.coro = nullptr;
        return *this;
    }
    T getNextValue()
    {
        std::cout << "        Generator::getNextValue" << '\n';
        coro.resume(); // (13)
        return coro.promise().current_value;
    }
    struct promise_type
    {
        promise_type()
        { // (2)
            std::cout << "            promise_type::promise_type" << '\n';
        }

        ~promise_type()
        {
            std::cout << "            promise_type::~promise_type" << '\n';
        }

        std::suspend_always initial_suspend()
        { // (5)
            std::cout << "            promise_type::initial_suspend" << '\n';
            return {}; // (6)
        }

        std::suspend_always final_suspend() noexcept
        {
            std::cout << "            promise_type::final_suspend" << '\n';
            return {};
        }

        auto get_return_object()
        { // (3)
            std::cout << "            promise_type::get_return_object" << '\n';
            return Generator{handle_type::from_promise(*this)}; // (4)
        }

        std::suspend_always yield_value(int value)
        { // (8)
            std::cout << "            promise_type::yield_value" << '\n';
            current_value = value; // (9)
            return {};             // (10)
        }

        void return_void() {}

        void unhandled_exception()
        {
            std::exit(1);
        }

        T current_value;
    };
};

Generator<int> getNext(int start = 10, int step = 10)
{
    std::cout << "    getNext: start" << '\n';
    auto value = start;
    while (true)
    { // (11)
        std::cout << "    getNext: before co_yield" << '\n';
        co_yield value; // (7)
        std::cout << "    getNext: after co_yield" << '\n';
        value += step;
    }
}

void Test()
{
    auto gen = getNext(); // (1)
    for (int i = 0; i <= 2; ++i)
    {
        auto val = gen.getNextValue();        // (12)
        std::cout << "main: " << val << '\n'; // (14)
    }
}
} // namespace TestCoroutines_Generator

namespace TestCoroutines_GenericGenerator
{
template <typename T>
struct Generator
{

    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    Generator(handle_type h)
        : coro(h)
    {
    }

    handle_type coro;

    ~Generator()
    {
        if (coro) coro.destroy();
    }
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& oth)
        : coro(oth.coro)
    {
        oth.coro = nullptr;
    }
    Generator& operator=(Generator&& oth)
    {
        coro = oth.coro;
        oth.coro = nullptr;
        return *this;
    }
    T getNextValue()
    {
        coro.resume();
        return coro.promise().current_value;
    }
    struct promise_type
    {
        promise_type() {}

        ~promise_type() {}

        std::suspend_always initial_suspend()
        {
            return {};
        }
        std::suspend_always final_suspend() noexcept
        {
            return {};
        }
        auto get_return_object()
        {
            return Generator{handle_type::from_promise(*this)};
        }

        std::suspend_always yield_value(const T value)
        {
            current_value = value;
            return {};
        }
        void return_void() {}
        void unhandled_exception()
        {
            std::exit(1);
        }

        T current_value;
    };
};

template <typename Cont>
Generator<typename Cont::value_type> getNext(Cont cont)
{
    for (auto c : cont) co_yield c;
}

void Test()
{
    std::cout << '\n';

    std::string helloWorld = "Hello world";
    auto gen = getNext(helloWorld); // (1)
    for (int i = 0; i < (int)helloWorld.size(); ++i)
    {
        std::cout << gen.getNextValue() << " "; // (4)
    }

    std::cout << "\n\n";

    auto gen2 = getNext(helloWorld); // (2)
    for (int i = 0; i < 5; ++i)
    { // (5)
        std::cout << gen2.getNextValue() << " ";
    }

    std::cout << "\n\n";

    std::vector myVec{1, 2, 3, 4, 5};
    auto gen3 = getNext(myVec); // (3)
    for (int i = 0; i < (int)myVec.size(); ++i)
    { // (6)
        std::cout << gen3.getNextValue() << " ";
    }

    std::cout << '\n';
}
} // namespace TestCoroutines_GenericGenerator

namespace TestCoroutines_AWaiterWorkFlow
{
struct MySuspendAlways
{ // (1)
    bool await_ready() const noexcept
    {
        std::cout << "        MySuspendAlways::await_ready" << '\n';
        return false;
    }
    void await_suspend(std::coroutine_handle<>) const noexcept
    {
        std::cout << "        MySuspendAlways::await_suspend" << '\n';
    }
    void await_resume() const noexcept
    {
        std::cout << "        MySuspendAlways::await_resume" << '\n';
    }
};

struct MySuspendNever
{ // (2)
    bool await_ready() const noexcept
    {
        std::cout << "        MySuspendNever::await_ready" << '\n';
        return true;
    }
    void await_suspend(std::coroutine_handle<>) const noexcept
    {
        std::cout << "        MySuspendNever::await_suspend" << '\n';
    }
    void await_resume() const noexcept
    {
        std::cout << "        MySuspendNever::await_resume" << '\n';
    }
};

struct Job
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;
    Job(handle_type h)
        : coro(h)
    {
    }
    ~Job()
    {
        if (coro) coro.destroy();
    }
    void start()
    {
        coro.resume();
    }

    struct promise_type
    {
        auto get_return_object()
        {
            return Job{handle_type::from_promise(*this)};
        }
        MySuspendAlways initial_suspend()
        { // (3)
            std::cout << "    Job prepared" << '\n';
            return {};
        }
        MySuspendAlways final_suspend() noexcept
        { // (4)
            std::cout << "    Job finished" << '\n';
            return {};
        }
        void return_void() {}
        void unhandled_exception() {}
    };
};

Job prepareJob()
{
    co_await MySuspendNever(); // (5)
}

void Test()
{

    std::cout << "Before job" << '\n';

    auto job = prepareJob(); // (6)
    job.start();             // (7)

    std::cout << "After job" << '\n';
}
} // namespace TestCoroutines_AWaiterWorkFlow

namespace TestCoroutines_StartJobWithAutomaticResumption
{
std::random_device seed;
auto gen = std::bind_front(std::uniform_int_distribution<>(0, 1), // (1)
                           std::default_random_engine(seed()));

struct MySuspendAlways
{ // (3)
    bool await_ready() const noexcept
    {
        std::cout << "        MySuspendAlways::await_ready" << '\n';
        return gen();
    }
    bool await_suspend(std::coroutine_handle<> handle) const noexcept
    { // (5)
        std::cout << "        MySuspendAlways::await_suspend" << '\n';
        handle.resume(); // (6)
        return true;
    }
    void await_resume() const noexcept
    { // (4)
        std::cout << "        MySuspendAlways::await_resume" << '\n';
    }
};

struct Job
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;
    Job(handle_type h)
        : coro(h)
    {
    }
    ~Job()
    {
        if (coro) coro.destroy();
    }

    struct promise_type
    {
        auto get_return_object()
        {
            return Job{handle_type::from_promise(*this)};
        }
        MySuspendAlways initial_suspend()
        { // (2)
            std::cout << "    Job prepared" << '\n';
            return {};
        }
        std::suspend_always final_suspend() noexcept
        {
            std::cout << "    Job finished" << '\n';
            return {};
        }
        void return_void() {}
        void unhandled_exception() {}
    };
};

Job performJob()
{
    co_await std::suspend_never();
}

void Test()
{

    std::cout << "Before jobs" << '\n';

    performJob();
    performJob();
    performJob();
    performJob();

    std::cout << "After jobs" << '\n';
}
} // namespace TestCoroutines_StartJobWithAutomaticResumption

namespace TestCoroutines_StartJobWithAutomaticResumptionOnThread
{
std::random_device seed;
auto gen = std::bind_front(std::uniform_int_distribution<>(0, 1),
                           std::default_random_engine(seed()));

struct MyAwaitable
{
    std::jthread& outerThread;
    bool await_ready() const noexcept
    {
        auto res = gen();
        if (res)
            std::cout << " (executed)" << '\n';
        else
            std::cout << " (suspended)" << '\n';
        return res; // (6)
    }
    void await_suspend(std::coroutine_handle<> h)
    {                                                    // (7)
        outerThread = std::jthread([h] { h.resume(); }); // (8)
    }
    void await_resume() {}
};

struct Job
{
    static inline int JobCounter{1};
    Job()
    {
        ++JobCounter;
    }

    struct promise_type
    {
        int JobNumber{JobCounter};
        Job get_return_object() { return {}; }
        std::suspend_never initial_suspend()
        { // (2)
            std::cout << "    Job " << JobNumber << " prepared on thread "
                      << std::this_thread::get_id();
            return {};
        }
        std::suspend_never final_suspend() noexcept
        { // (3)
            std::cout << "    Job " << JobNumber << " finished on thread "
                      << std::this_thread::get_id() << '\n';
            return {};
        }
        void return_void() {}
        void unhandled_exception() {}
    };
};

Job performJob(std::jthread& out)
{
    co_await MyAwaitable{out}; // (1)
}

void Test()
{
    std::vector<std::jthread> threads(8);      // (4)
    for (auto& thr : threads) performJob(thr); // (5)
}
} // namespace TestCoroutines_StartJobWithAutomaticResumptionOnThread

void DoCoroutinesTest()
{
    // TestCoroutinesSimple::Test();
    // TestCoroutinesHelloWorld::Test();
    // TestCoroutinesYield::Test();
    // TestCoroutines_co_return::Test();
    // TestCoroutines_guanwang::Test();
    // TestCoroutines_future::Test();
    // TestCoroutines_sender_reciever::Test();
    // TestCoroutines_call_sequence::Test();
    // TestCoroutines_call_sequence1::Test();
    // TestCoroutines_LazyFutureOnOtherThead::Test();
    // TestCoroutines_Generator::Test();
    // TestCoroutines_GenericGenerator::Test();
    // TestCoroutines_AWaiterWorkFlow::Test();
    // TestCoroutines_StartJobWithAutomaticResumption::Test();
    TestCoroutines_StartJobWithAutomaticResumptionOnThread::Test();
}

} // namespace Test
