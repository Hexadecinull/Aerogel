# Aerogel Architecture

## Overview

Aerogel targets two independent boot paths that converge at the kernel entry point.

```
x86  : BIOS -> MBR (stage1) -> stage2 -> protected mode -> kmain()
x86_64: UEFI -> efi_main() -> long mode -> kmain()
```

## Memory Map (x86 protected mode)

| Region            | Address range         | Contents                  |
|-------------------|-----------------------|---------------------------|
| Real mode IVT     | 0x00000 – 0x003FF     | Overwritten after PM jump |
| BDA               | 0x00400 – 0x004FF     | BIOS data (read only)     |
| Stage2 + loader   | 0x07E00 – 0x0FFFF     | Second-stage bootloader   |
| Kernel            | 0x10000 – ...         | Loaded by stage2          |
| Stack             | 0x0FFFF (grows down)  | Bootloader stack          |
| EBDA              | 0x80000 – 0x9FFFF     | Extended BIOS data        |
| VGA text buffer   | 0xB8000               | 80x25 text mode           |
| BIOS ROM          | 0xE0000 – 0xFFFFF     | Firmware                  |

## Memory Map (x86_64 long mode)

| Region            | Virtual address        | Contents                   |
|-------------------|------------------------|----------------------------|
| Kernel            | 0xFFFFFFFF80000000     | Higher-half kernel         |
| Physical map      | 0xFFFF800000000000     | Full physical memory map   |
| User space        | 0x0000000000000000     | User processes (future)    |

## Kernel Subsystems

- **PMM** — Bitmap-based physical page allocator
- **VMM** — Page table manager, handles mapping/unmapping
- **Heap** — `kmalloc`/`kfree` backed by VMM
- **Sched** — Round-robin preemptive scheduler (PIT/APIC driven)
- **VFS** — Virtual filesystem with FAT32 and tmpfs backends
- **IPC** — Pipes and signals

## Driver Model

Drivers register themselves at boot via a static driver table. No dynamic loading in early phases.
