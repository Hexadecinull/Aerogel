#include "panic.h"
#include <kprintf.h>

NORETURN void kpanic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    kprintf("\n[KERNEL PANIC] ");
    kvprintf(fmt, ap);
    va_end(ap);
    for (;;) __asm__ volatile("cli; hlt");
}
