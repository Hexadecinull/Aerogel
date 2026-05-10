# Bootloader

## x86 — BIOS/MBR

### Stage 1 (mbr.asm)
- Loaded by BIOS at 0x7C00, exactly 512 bytes
- Relocates itself to 0x0600 to free space at 0x7C00 for stage2
- Reads stage2 sectors from disk using INT 13h extensions (LBA)
- Jumps to stage2 at 0x7E00

### Stage 2 (stage2.asm)
- Enables A20 line
- Loads GDT, switches to 32-bit protected mode
- Loads kernel ELF from disk
- Jumps to kernel entry point

### A20 (a20.asm)
- Tries fast A20 (port 0x92) first
- Falls back to keyboard controller method
- Validates A20 enabled by reading wrap-around address

## x86_64 — UEFI

### efi_main.c
- UEFI application entry point with standard EFI_SYSTEM_TABLE
- Uses EFI boot services to allocate memory, load kernel from ESP
- Sets up page tables for long mode
- Exits boot services and jumps to kernel
