#pragma once
#include "thread.h"

#define SCHED_QUANTUM_TICKS 10

void sched_init(void);
void sched_add(thread_t *t);
void sched_yield(void);
void sched_sleep(u32 ms);
void sched_wake(thread_t *t);
void sched_tick(void);
void sched_remove(thread_t *t);
