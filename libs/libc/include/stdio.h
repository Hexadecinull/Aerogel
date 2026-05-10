#pragma once

#include <stdarg.h>
#include <stddef.h>

int  snprintf (char *buf, size_t size, const char *fmt, ...);
int  vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
