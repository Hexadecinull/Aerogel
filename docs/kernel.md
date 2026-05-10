# Kernel

Populated as each subsystem is implemented.

## Entry Points

- `arch/x86/boot/stage2.asm` jumps to `kmain()` in protected mode
- `arch/x86_64/boot/efi_main.c` jumps to `kmain()` in long mode

## kmain()

1. Initialise serial debug output
2. Print Aerogel banner
3. Initialise GDT, IDT
4. Initialise PMM (parse memory map from bootloader)
5. Initialise VMM
6. Initialise heap
7. Initialise PIT / APIC
8. Enable interrupts
9. Initialise drivers
10. Enter scheduler / idle loop
