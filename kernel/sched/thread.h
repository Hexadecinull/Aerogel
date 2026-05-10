#pragma once
#include <types.h>

#define THREAD_STACK_SIZE (16 * 1024)
#define THREAD_NAME_LEN   32

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_SLEEPING,
    THREAD_ZOMBIE,
} thread_state_t;

typedef void (*thread_fn_t)(void *arg);

typedef struct thread {
    u32            id;
    char           name[THREAD_NAME_LEN];
    thread_state_t state;
    uptr           esp;
    u8            *stack;
    u64            wake_tick;
    u32            quantum_remaining;
    struct thread *next;
    struct thread *prev;
} thread_t;

thread_t *thread_create(const char *name, thread_fn_t fn, void *arg);
void      thread_exit(void);
thread_t *thread_current(void);
u32       thread_count(void);
thread_t *thread_get_by_id(u32 id);
