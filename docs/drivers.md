# Drivers

Populated as each driver is implemented.

## Input
- `ps2_keyboard` — IRQ1, scancode set 2
- `ps2_mouse`    — IRQ12, PS/2 mouse protocol

## Display
- `vga_text`    — 0xB8000 text mode, 80x25
- `framebuffer` — Linear framebuffer (from UEFI GOP or VESA)

## Storage
- `ata`  — PIO mode ATA
- `ahci` — SATA AHCI (DMA)

## Bus
- `pci` — Configuration space access via I/O ports 0xCF8/0xCFC

## Timer
- `pit`  — 8253/8254 Programmable Interval Timer
- `apic` — Local APIC timer (replaces PIT on SMP)

## Serial
- `serial` — 16550 UART, COM1 at 0x3F8 (debug output)
