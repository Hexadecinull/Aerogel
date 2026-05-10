#include <stdio.h>
#include <string.h>
#include <stdint.h>

static void emit(char **buf, char *end, char c) {
    if (*buf < end) **buf = c;
    (*buf)++;
}

static void emit_str(char **buf, char *end, const char *s) {
    while (*s) emit(buf, end, *s++);
}

static void emit_uint(char **buf, char *end, uintptr_t val, int base, int upper) {
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    int  len = 0;
    if (val == 0) { emit(buf, end, '0'); return; }
    while (val) { tmp[len++] = digits[val % (uintptr_t)base]; val /= (uintptr_t)base; }
    while (len--) emit(buf, end, tmp[len]);
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
    char *cur = buf;
    char *end = buf + size - 1;

    while (*fmt) {
        if (*fmt != '%') { emit(&cur, end, *fmt++); continue; }
        fmt++;
        switch (*fmt++) {
        case 'c': emit(&cur, end, (char)va_arg(ap, int)); break;
        case 's': emit_str(&cur, end, va_arg(ap, const char *)); break;
        case 'd': {
            long v = va_arg(ap, long);
            if (v < 0) { emit(&cur, end, '-'); v = -v; }
            emit_uint(&cur, end, (uintptr_t)v, 10, 0);
            break;
        }
        case 'u': emit_uint(&cur, end, (uintptr_t)va_arg(ap, unsigned long), 10, 0); break;
        case 'x': emit_uint(&cur, end, (uintptr_t)va_arg(ap, unsigned long), 16, 0); break;
        case 'X': emit_uint(&cur, end, (uintptr_t)va_arg(ap, unsigned long), 16, 1); break;
        case 'p': {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
            uintptr_t ptr = (uintptr_t)va_arg(ap, void*);
#pragma GCC diagnostic pop
            emit_str(&cur, end, "0x");
            emit_uint(&cur, end, ptr, 16, 0);
            break;
        }
        case '%': emit(&cur, end, '%'); break;
        default:  emit(&cur, end, '?'); break;
        }
    }

    if (size > 0) *cur = '\0';
    return (int)(cur - buf);
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return r;
}
