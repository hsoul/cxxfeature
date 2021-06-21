#include "coroutine.h"
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __APPLE__ && __MACH__
    #include <sys/ucontext.h>
#else
    #include <ucontext.h>
#endif

#define STACK_SIZE (1024 * 1204)
#define DEFAULT_COROUTINE_NUM 16

struct coroutine;

struct schedule
{
    char stack_[STACK_SIZE];
    ucontext_t main_ctx_;
    int coroutines_num_;
    int capacity_;
    int running_coroutine_id_;
    struct coroutine** coroutines_map_;
};

struct coroutine
{
    coroutine_func func_;
    void* user_data_;
    ucontext_t ctx_;
    struct schedule* schedule_;
    int status_;
    char* stack_;
    ptrdiff_t stack_capacity_;
    ptrdiff_t stack_size_;
};

struct coroutine* _co_new(struct schedule* S, coroutine_func func, void* user_data)
{
    struct coroutine* co = static_cast<struct coroutine*>(malloc(sizeof(*co)));
    co->func_ = func;
    co->user_data_ = user_data;
    co->schedule_ = S;
    co->stack_capacity_ = 0;
    co->stack_size_ = 0;
    co->status_ = COROUTINE_READY;
    co->stack_ = NULL;

    return co;
}

void _co_delete(struct coroutine* co)
{
    free(co->stack_);
    free(co);
}

struct schedule* schedule_open()
{
    struct schedule* S = static_cast<struct schedule*>(malloc(sizeof(*S)));
    S->coroutines_num_ = 0;
    S->capacity_ = DEFAULT_COROUTINE_NUM;
    S->running_coroutine_id_ = -1;
    S->coroutines_map_ = static_cast<struct coroutine**>(malloc(sizeof(struct coroutine*) * S->capacity_));
    memset(S->coroutines_map_, 0, sizeof(struct coroutine*) * S->capacity_);

    return S;
}

void schedule_close(struct schedule* S)
{
    for (int i = 0; i < S->capacity_; ++i)
    {
        struct coroutine* co = S->coroutines_map_[i];
        if (co)
        {
            _co_delete(co);
        }
    }
    free(S->coroutines_map_);
    S->coroutines_map_ = NULL;
    free(S);
}

int coroutine_new(struct schedule* S, coroutine_func func, void* user_data)
{
    struct coroutine* co = _co_new(S, func, user_data);
    if (S->coroutines_num_ >= S->capacity_)
    {
        int id = S->capacity_;
        S->coroutines_map_ = static_cast<struct coroutine**>(realloc(S->coroutines_map_, S->capacity_ * 2 * sizeof(struct coroutine*)));
        memset(S->coroutines_map_ + S->capacity_, 0, sizeof(struct coroutine*) * S->capacity_);
        S->coroutines_map_[id] = co;
        S->capacity_ *= 2;
        ++S->coroutines_num_;
        return id;
    }
    else
    {
        for (int i = 0; i < S->capacity_; ++i)
        {
            int id = (i + S->coroutines_num_) % S->capacity_;
            if (S->coroutines_map_[id] == NULL)
            {
                S->coroutines_map_[id] = co;
                ++S->coroutines_num_;
                return id;
            }
        }
    }
    assert(0);
    return -1;
}

static void main_func(uint32_t low32, uint32_t high32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
    struct schedule* S = (struct schedule*)ptr;
    int running_coroutine_id = S->running_coroutine_id_;
    struct coroutine* C = S->coroutines_map_[running_coroutine_id];
    C->func_(S, C->user_data_);
    _co_delete(C);
    S->coroutines_map_[running_coroutine_id] = NULL;
    --S->coroutines_num_;
    S->running_coroutine_id_ = -1;
}

void coroutine_resume(struct schedule* S, int id) // 此函数执行权从主逻辑流向协程切换，即在swapcontext()函数之前和之后的所有代码都是在主逻辑流中执行的，执行swapcontext()函数，执行权切到协程
{
    assert(S->running_coroutine_id_ == -1);
    assert(id >= 0 && id < S->capacity_);

    struct coroutine* C = S->coroutines_map_[id];
    if (C == NULL)
        return;

    int status = C->status_;
    switch (status)
    {
        case COROUTINE_READY: {
            getcontext(&C->ctx_);
            C->ctx_.uc_stack.ss_sp = S->stack_;
            C->ctx_.uc_stack.ss_size = STACK_SIZE;
            C->ctx_.uc_link = &S->main_ctx_; // 协程执行完毕切回主逻辑流
            S->running_coroutine_id_ = id;
            C->status_ = COROUTINE_RUNNING;
            uintptr_t ptr = (uintptr_t)S;
            makecontext(&C->ctx_, (void(*)(void))main_func, 2, (uint32_t)ptr, uint32_t(ptr >> 32));
            printf("%d resume ready begin\n", id);
            swapcontext(&S->main_ctx_, &C->ctx_);
            printf("%d resume ready end\n", id);
            break;
        }
        case COROUTINE_SUSPEND: {
            memcpy(S->stack_ + STACK_SIZE - C->stack_size_, C->stack_, C->stack_size_);
            S->running_coroutine_id_ = id;
            C->status_ = COROUTINE_RUNNING;
            printf("%d resume suspend begin\n", id);
            swapcontext(&S->main_ctx_, &C->ctx_);
            printf("%d resume suspend end\n", id);
            break;
        }
        default:
            assert(0);
    }
}

int coroutine_status(struct schedule* S, int id)
{
    assert(id >= 0 && id < S->capacity_);
    if (S->coroutines_map_[id] == NULL)
    {
        return COROUTINE_DEAD;
    }
    return S->coroutines_map_[id]->status_;
}

int coroutine_string_status(struct schedule* S, int id, char* buffer, int buffer_size)
{
    assert(id >= 0 && id < S->capacity_);
    if (S->coroutines_map_[id] == NULL)
    {
        snprintf(buffer, buffer_size, "COROUTINE_DEAD");
        return COROUTINE_DEAD;
    }
    switch (S->coroutines_map_[id]->status_)
    {
        case COROUTINE_DEAD:
            snprintf(buffer, buffer_size, "COROUTINE_DEAD");
            break;
        case COROUTINE_READY:
            snprintf(buffer, buffer_size, "COROUTINE_READY");
            break;
        case COROUTINE_RUNNING:
            snprintf(buffer, buffer_size, "COROUTINE_RUNNING");
            break;
        case COROUTINE_SUSPEND:
            snprintf(buffer, buffer_size, "COROUTINE_SUSPEND");
            break;
        default:
            assert(0);
    }
    return S->coroutines_map_[id]->status_;
}

int coroutine_running(struct schedule* S)
{
    return S->running_coroutine_id_;
}

static void _save_stack(struct coroutine* C, char* stack_top)
{
    char dummy = 0;
    int cur_stack_size = stack_top - &dummy;
    assert(cur_stack_size <= STACK_SIZE);
    printf("%d cur stack size %d\n", C->schedule_->running_coroutine_id_, cur_stack_size);
    if (C->stack_capacity_ < cur_stack_size)
    {
        free(C->stack_);
        C->stack_capacity_ = cur_stack_size;
        C->stack_ = static_cast<char*>(malloc(C->stack_capacity_));
    }
    C->stack_size_ = cur_stack_size;
    memcpy(C->stack_, &dummy, C->stack_size_);
}

void coroutine_yield(struct schedule* S) // 此函数，执行权从协程到主逻辑流切换，即在swapcontext()函数之前和之后的所有代码都是在协程中执行的，执行swapcontext()函数执行权切到主逻辑流
{
    int running_coroutine_id = S->running_coroutine_id_;
    assert(running_coroutine_id >= 0);
    struct coroutine* C = S->coroutines_map_[running_coroutine_id];
    assert((char*)&C > S->stack_);
    _save_stack(C, S->stack_ + STACK_SIZE);
    C->status_ = COROUTINE_SUSPEND;
    S->running_coroutine_id_ = -1;
    printf("%d yield begin\n", running_coroutine_id);
    swapcontext(&C->ctx_, &S->main_ctx_);
    printf("%d yield end\n", running_coroutine_id);
}