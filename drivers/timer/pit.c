#include "pit.h"
#include <io.h>
#include <isr.h>

#define PIT_BASE_HZ 1193182UL
#define PIT_CMD     0x43
#define PIT_CH0     0x40

static volatile u64 tick_count = 0;
static u32 ticks_per_ms = 0;

static void pit_handler(isr_frame_t *frame) {
    (void)frame;
    tick_count++;
}

void pit_init(u32 hz) {
    u32 divisor = (u32)(PIT_BASE_HZ / hz);
    ticks_per_ms = hz / 1000;
    if (!ticks_per_ms) ticks_per_ms = 1;

    isr_register(32, pit_handler);
    pic_unmask_irq(0);

    outb(PIT_CMD, 0x36);
    outb(PIT_CH0, (u8)(divisor & 0xFF));
    outb(PIT_CH0, (u8)(divisor >> 8));
}

u64 pit_ticks(void) {
    return tick_count;
}

void pit_sleep_ms(u32 ms) {
    u64 target = tick_count + (u64)ms * ticks_per_ms;
    while (tick_count < target)
        __asm__ volatile("hlt");
}
