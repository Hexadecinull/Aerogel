#pragma once
#include <stdarg.h>

typedef void (*kprint_fn_t)(const char *s);

void kprintf_set_backend(kprint_fn_t fn);
void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list ap);
