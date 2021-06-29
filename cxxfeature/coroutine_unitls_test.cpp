#include "coroutine_unitls_test.h"
#include "./base/coroutine.h"
#include <cstddef>
#include <iostream>
#include <stdio.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <unistd.h>

/*
// 用户上下文的获取和设置
int getcontext(ucontext_t *ucp);
int setcontext(const ucontext_t *ucp);

// 操纵用户上下文
void makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...);
int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);

typedef struct {
    ucontext_t *uc_link;
    sigset_t    uc_sigmask;
    stack_t     uc_stack;
    mcontext_t  uc_mcontext;
    ...
} ucontext_t;

*/

namespace Test
{

namespace CoroutineUnitl_ucontext
{

static ucontext_t ctx[3];

static void f1(void)
{
    puts("start f1");
    swapcontext(&ctx[1], &ctx[2]);
    puts("finish f1");
}

static void f2(void)
{
    puts("start f2");
    swapcontext(&ctx[2], &ctx[1]);
    puts("finish f2");
}

void Test(void)
{
    char st1[8192];
    char st2[8192];

    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = st1;
    ctx[1].uc_stack.ss_size = sizeof st1;
    ctx[1].uc_link = &ctx[0];
    makecontext(&ctx[1], f1, 0);

    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = st2;
    ctx[2].uc_stack.ss_size = sizeof st2;
    ctx[2].uc_link = &ctx[1];
    makecontext(&ctx[2], f2, 0);

    swapcontext(&ctx[0], &ctx[2]);
}

} // namespace CoroutineUnitl_ucontext

namespace CoroutineUnitl_ucontextprint
{
void Test()
{
    ucontext_t context;

    int i = 0;
    getcontext(&context);
    // int i = 0;
    printf("hello world %d\n", i);
    ++i;
    sleep(1);
    setcontext(&context);
}
} // namespace CoroutineUnitl_ucontextprint

namespace CoroutineUnitl_makecontext
{

void func()
{
    std::cout << "func call" << std::endl;
}

void Test()
{
    ucontext_t context;
    char stack[1024];

    getcontext(&context);
    context.uc_stack.ss_sp = stack;
    context.uc_stack.ss_size = sizeof(stack);
    context.uc_link = NULL;
    makecontext(&context, func, 0);
    printf("hello world\n");
    sleep(1);
    setcontext(&context); // 执行完context中函数直接结束程序，不会输出program end
    printf("program end\n");
}
} // namespace CoroutineUnitl_makecontext

namespace CoroutineUnitl_ucontextcallfunc
{
void func1()
{
    std::cout << "func1 call" << std::endl;
}

void func2()
{
    std::cout << "func2 call" << std::endl;
}

void Test()
{
    ucontext_t context1;
    ucontext_t context2;

    char stack1[1024];
    char stack2[1024];

    getcontext(&context1);
    context1.uc_stack.ss_sp = stack1;
    context1.uc_stack.ss_size = sizeof(stack1);
    context1.uc_link = NULL;
    makecontext(&context1, func1, 0);

    getcontext(&context2);
    context2.uc_stack.ss_sp = stack2;
    context2.uc_stack.ss_size = sizeof(stack2);
    context2.uc_link = &context1;
    makecontext(&context2, func2, 0);

    std::cout << "hello world" << std::endl;
    sleep(1);
    setcontext(&context2);

    printf("program end\n");
}

} // namespace CoroutineUnitl_ucontextcallfunc

namespace CoroutineUnitl_muticontext // 最具代表性
{
static ucontext_t ctx[3];

static void f1()
{
    std::cout << "start f1" << std::endl;
    swapcontext(&ctx[1], &ctx[2]); // 将当前context 保存到ctx[1],切换到上下文ctx[2], 然后执行
    printf("finish f1\n");
}

static void f2()
{
    std::cout << "start f2" << std::endl;
    swapcontext(&ctx[2], &ctx[1]);
    printf("finish f2\n");
}

void Test()
{
    char stack1[1024];
    char stack2[1024];

    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = stack1;
    ctx[1].uc_stack.ss_size = sizeof(stack1);
    ctx[1].uc_link = &ctx[0]; // 将执行return 0
    makecontext(&ctx[1], f1, 0);

    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = stack2;
    ctx[2].uc_stack.ss_size = sizeof(stack2);
    ctx[2].uc_link = &ctx[1];
    makecontext(&ctx[2], f2, 0);

    swapcontext(&ctx[0], &ctx[2]);

    std::cout << "program end" << std::endl;
}

} // namespace CoroutineUnitl_muticontext

namespace CoroutineUnitl_usercoroutine
{
struct args
{
    int start_num_;
    int total_num_;
};

static void foo(struct schedule* S, void* user_data)
{
    printf("foo start\n");
    struct args* arg = static_cast<struct args*>(user_data);
    int start = arg->start_num_;
    for (int i = 0; i < arg->total_num_; ++i)
    {
        printf("coroutine %d : %d\n", coroutine_running(S), start + i);
        coroutine_yield(S);
    }
    printf("foo end\n");
}

// static int jtset()
// {
//     return 0;
// }

// static void foo2(struct schedule* S, void* user_data)
// {
//     printf("foo start\n");
//     int an = 1;
//     printf("hello world %d\n", an);
//     coroutine_yield(S);
//     int ret = jtset();
//     printf("dosomething else %d\n", ret);
//     coroutine_yield(S);
//     printf("foo end\n");
// }

static void test(struct schedule* S)
{
    // struct args arg1 = {10, 1};
    struct args arg2 = {100, 2};

    // int co1 = coroutine_new(S, foo, &arg1);
    int co2 = coroutine_new(S, foo, &arg2);
    // int co3 = coroutine_new(S, foo2, NULL);

    printf("main start\n");

    while (coroutine_status(S, co2)) // || coroutine_status(S, co3))
    {
        char buffer[64]{0};
        printf("before status %d %s\n", coroutine_string_status(S, co2, buffer, sizeof(buffer)), buffer); // , coroutine_status(S, co2)); //, coroutine_status(S, co3));
        // if (coroutine_status(S, co1))
        //     coroutine_resume(S, co1);
        if (coroutine_status(S, co2))
            coroutine_resume(S, co2);
        // if (coroutine_status(S, co3))
        //     coroutine_resume(S, co3);
        // printf("after status %d %d\n\n", coroutine_status(S, co1), coroutine_status(S, co2)); // , coroutine_status(S, co3));
        printf("\n\n");
    }

    printf("main end\n");
}

void Test()
{
    struct schedule* S = schedule_open();
    test(S);
    schedule_close(S);
}

} // namespace CoroutineUnitl_usercoroutine

void DoCoroutineUnitlsTest()
{
    // CoroutineUnitl_ucontext::Test();
    // CoroutineUnitl_ucontextsimple::Test();
    // CoroutineUnitl_makecontext::Test();
    // CoroutineUnitl_ucontextcallfunc::Test();
    // CoroutineUnitl_muticontext::Test();
    CoroutineUnitl_usercoroutine::Test();
} // namespace )

} // namespace Test