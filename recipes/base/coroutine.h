#pragma once

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct schedule;

typedef void (*coroutine_func)(struct schedule*, void* user_data);

struct schedule* schedule_open();
void schedule_close(struct schedule*);

int coroutine_new(struct schedule*, coroutine_func, void* user_data);
void coroutine_resume(struct schedule*, int id);
int coroutine_status(struct schedule*, int id);
int coroutine_string_status(struct schedule*, int id, char* buffer, int buffer_size);
int coroutine_running(struct schedule*);
void coroutine_yield(struct schedule*);
