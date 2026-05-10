#include <canary.h>
#include <panic.h>

uptr __stack_chk_guard = (uptr)0xDEADC0DEUL;

void __stack_chk_fail(void) {
    kpanic("Stack smashing detected — aborting");
}
