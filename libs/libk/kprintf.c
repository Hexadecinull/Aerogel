#include <kprintf.h>
#include <stdio.h>
#include <string.h>

static kprint_fn_t kprint_backend = NULL;

void kprintf_set_backend(kprint_fn_t fn) {
    kprint_backend = fn;
}

void kprintf(const char *fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (kprint_backend) kprint_backend(buf);
}

void kvprintf(const char *fmt, va_list ap) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    if (kprint_backend) kprint_backend(buf);
}
