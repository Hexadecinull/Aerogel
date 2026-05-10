#include "sched.h"
#include "thread.h"
#include <pit.h>
#include <string.h>

extern void switch_context(uptr *old_esp, uptr new_esp);
extern void thread_set_current(thread_t *t);
extern void thread_init_idle(void);
extern thread_t *thread_idle(void);

static thread_t *run_queue_head = NULL;
static bool      need_resched   = false;

static void enqueue(thread_t *t) {
    t->state = THREAD_READY;
    if (!run_queue_head) {
        run_queue_head = t;
        t->next = t->prev = t;
    } else {
        t->prev = run_queue_head->prev;
        t->next = run_queue_head;
        run_queue_head->prev->next = t;
        run_queue_head->prev = t;
    }
}

static thread_t *dequeue_next(void) {
    if (!run_queue_head) return NULL;
    thread_t *t = run_queue_head;
    if (t->next == t) {
        run_queue_head = NULL;
    } else {
        run_queue_head = t->next;
        t->prev->next  = t->next;
        t->next->prev  = t->prev;
    }
    t->next = t->prev = NULL;
    return t;
}

void sched_init(void) {
    run_queue_head = NULL;
    thread_init_idle();
}

void sched_add(thread_t *t) { enqueue(t); }

void sched_remove(thread_t *t) {
    if (!t->next) return;
    if (t->next == t) { run_queue_head = NULL; }
    else {
        if (run_queue_head == t) run_queue_head = t->next;
        t->prev->next = t->next;
        t->next->prev = t->prev;
    }
    t->next = t->prev = NULL;
}

void sched_tick(void) {
    thread_t *cur = thread_current();
    if (!cur) return;
    if (cur->quantum_remaining > 0) cur->quantum_remaining--;
    if (cur->quantum_remaining == 0) need_resched = true;

    u64 now = pit_ticks();
    thread_t *t = run_queue_head;
    if (!t) return;
    thread_t *start = t;
    do {
        if (t->state == THREAD_SLEEPING && t->wake_tick <= now) {
            t->state = THREAD_READY;
        }
        t = t->next;
    } while (t && t != start);
}

void sched_yield(void) {
    __asm__ volatile("cli");

    thread_t *cur  = thread_current();
    thread_t *next = dequeue_next();

    if (!next) {
        need_resched = false;
        __asm__ volatile("sti");
        return;
    }

    if (cur->state == THREAD_RUNNING) {
        cur->state = THREAD_READY;
        enqueue(cur);
    }

    cur->quantum_remaining  = SCHED_QUANTUM_TICKS;
    next->state             = THREAD_RUNNING;
    next->quantum_remaining = SCHED_QUANTUM_TICKS;
    need_resched            = false;

    thread_set_current(next);
    switch_context(&cur->esp, next->esp);

    __asm__ volatile("sti");
}

void sched_sleep(u32 ms) {
    thread_t *cur = thread_current();
    cur->state     = THREAD_SLEEPING;
    cur->wake_tick = pit_ticks() + (u64)ms / 10;
    sched_remove(cur);
    sched_yield();
}

void sched_wake(thread_t *t) {
    if (t->state == THREAD_SLEEPING || t->state == THREAD_BLOCKED) {
        t->state = THREAD_READY;
        enqueue(t);
    }
}
