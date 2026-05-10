#include "thread.h"
#include "sched.h"
#include <stdlib.h>
#include <string.h>
#include <panic.h>

extern void thread_trampoline(void);
extern void switch_context(uptr *old_esp, uptr new_esp);

static thread_t *current_thread = NULL;
static u32       next_id        = 0;
static u32       nthreads       = 0;

static u8 idle_stack[THREAD_STACK_SIZE] ALIGN(16);
static thread_t idle_tcb;

thread_t *thread_create(const char *name, thread_fn_t fn, void *arg) {
    thread_t *t = (thread_t *)kmalloc(sizeof(thread_t));
    memset(t, 0, sizeof(*t));
    t->stack = (u8 *)kmalloc(THREAD_STACK_SIZE);
    if (!t->stack) kpanic("thread_create: out of memory");

    strncpy(t->name, name, THREAD_NAME_LEN - 1);
    t->id    = next_id++;
    t->state = THREAD_READY;
    t->quantum_remaining = SCHED_QUANTUM_TICKS;

    uptr *sp = (uptr *)(t->stack + THREAD_STACK_SIZE);
    *--sp = (uptr)arg;
    *--sp = (uptr)fn;
    *--sp = (uptr)thread_trampoline;
    /* Callee-saved regs (ebp/rbp, ebx/rbx, esi/r12, edi/r13, [r14,r15]) */
    *--sp = 0; /* ebp/rbp */
    *--sp = 0; /* ebx/rbx */
    *--sp = 0; /* esi/r12 */
    *--sp = 0; /* edi/r13 */
#ifdef __x86_64__
    *--sp = 0; /* r14 */
    *--sp = 0; /* r15 */
#endif
    t->esp = (uptr)sp;

    nthreads++;
    return t;
}

void thread_exit(void) {
    thread_t *t = current_thread;
    t->state    = THREAD_ZOMBIE;
    nthreads--;
    sched_yield();
    for (;;) __asm__ volatile("hlt");
}

thread_t *thread_current(void) { return current_thread; }
u32       thread_count(void)   { return nthreads; }

void thread_set_current(thread_t *t) { current_thread = t; }
thread_t *thread_idle(void) { return &idle_tcb; }

void thread_init_idle(void) {
    memset(&idle_tcb, 0, sizeof(idle_tcb));
    strncpy(idle_tcb.name, "idle", THREAD_NAME_LEN - 1);
    idle_tcb.id    = next_id++;
    idle_tcb.state = THREAD_RUNNING;
    idle_tcb.stack = idle_stack;
    idle_tcb.quantum_remaining = SCHED_QUANTUM_TICKS;
    current_thread = &idle_tcb;
    nthreads++;
}

thread_t *thread_get_by_id(u32 id) {
    (void)id;
    return NULL;
}
