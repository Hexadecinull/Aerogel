# Roadmap

## Phase 0 — Infrastructure ✓
Repository skeleton, build system, CI pipelines, toolchain definitions.

## Phase 1 — x86 Boot
MBR stage1 → stage2 → A20 → protected mode → kernel stub jump.

## Phase 2 — x86 Kernel Foundation
GDT, IDT, ISR/IRQ handlers, PIT timer, serial debug (COM1).

## Phase 3 — Memory Management
Physical memory manager (bitmap allocator), virtual memory manager, kernel heap.

## Phase 4 — Basic I/O Drivers
PS/2 keyboard and mouse, VGA text mode.

## Phase 5 — x86_64 Boot
UEFI bootloader, long mode, parity with Phase 2–4.

## Phase 6 — Storage & Filesystem
ATA PIO driver, AHCI driver, VFS layer, FAT32, tmpfs.

## Phase 7 — Userspace Foundation
Process scheduler, basic shell, setup wizard.

## Phase 8 — GUI Foundation
Framebuffer driver, window manager primitives, USB HID.

## Phase 9 — Rust Components & Networking
Network stack in Rust, Bluetooth support.
