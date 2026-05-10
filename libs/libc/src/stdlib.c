#include <stdlib.h>

int atoi(const char *s) { return (int)atol(s); }

long atol(const char *s) {
    long result = 0;
    int  sign   = 1;
    while (*s == ' ' || *s == '\t') s++;
    if      (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }
    while (*s >= '0' && *s <= '9') result = result * 10 + (*s++ - '0');
    return sign * result;
}
