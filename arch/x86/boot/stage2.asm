[bits 16]
[org 0x7E00]

KERNEL_LBA     equ 17
KERNEL_SECTORS equ 64
KERNEL_SEG     equ 0x1000
KERNEL_OFF     equ 0x0000
KERNEL_PHYS    equ (KERNEL_SEG * 16 + KERNEL_OFF)

VBE_FLAG_ADDR  equ 0x5FFC
VBE_INFO_ADDR  equ 0x6000

    cli
    xor  ax, ax
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0x7C00
    sti

    mov  [drive_id], dl

    call enable_a20
    call setup_vbe
    call get_memory_map
    call load_kernel

    lgdt [gdt_ptr]

    cli
    mov  eax, cr0
    or   eax, 0x01
    mov  cr0, eax

    jmp  0x08:pm_entry

; ── VBE framebuffer setup ────────────────────────────────────────────────────
setup_vbe:
    push es
    xor  ax, ax
    mov  es, ax

    mov  dword [VBE_FLAG_ADDR], 0

    ; Get mode info for mode 0x0115 (800x600x24bpp)
    mov  ax, 0x4F01
    mov  cx, 0x0115
    mov  di, VBE_INFO_ADDR
    int  0x10
    cmp  ax, 0x004F
    jne  .try_linear

    ; Check mode is valid (XRes at offset 0x12 should be 800)
    cmp  word [es:VBE_INFO_ADDR + 0x12], 800
    jne  .try_linear

    ; Set linear mode 0x0115 | 0x4000
    mov  ax, 0x4F02
    mov  bx, 0x4115
    xor  di, di
    int  0x10
    cmp  ax, 0x004F
    jne  .done

    mov  dword [VBE_FLAG_ADDR], 1
    jmp  .done

.try_linear:
    ; Try mode 0x0118 (1024x768x24)
    mov  ax, 0x4F01
    mov  cx, 0x0118
    mov  di, VBE_INFO_ADDR
    int  0x10
    cmp  ax, 0x004F
    jne  .done

    mov  ax, 0x4F02
    mov  bx, 0x4118
    xor  di, di
    int  0x10
    cmp  ax, 0x004F
    jne  .done

    mov  dword [VBE_FLAG_ADDR], 1

.done:
    pop  es
    ret

; ── E820 memory enumeration ──────────────────────────────────────────────────
get_memory_map:
    push es
    xor  ax, ax
    mov  es, ax
    mov  di, 0x0500
    xor  ebx, ebx
    xor  bp, bp

.loop:
    mov  edx, 0x534D4150
    mov  eax, 0xE820
    mov  ecx, 24
    int  0x15
    jc   .done
    cmp  eax, 0x534D4150
    jne  .done
    test ecx, ecx
    jz   .next
    mov  eax, [es:di+8]
    or   eax, [es:di+12]
    jz   .next
    inc  bp
    add  di, 24
.next:
    test ebx, ebx
    jz   .done
    cmp  bp, 128
    jb   .loop
.done:
    mov  [0x4FC], bp
    pop  es
    ret

; ── Kernel disk load ─────────────────────────────────────────────────────────
load_kernel:
    mov  ax, KERNEL_SEG
    mov  es, ax
    mov  si, kdap
    mov  ah, 0x42
    mov  dl, [drive_id]
    int  0x13
    jc   .err
    xor  ax, ax
    mov  es, ax
    ret
.err:
    mov  si, msg_err
.print:
    lodsb
    test al, al
    jz   .halt
    mov  ah, 0x0E
    xor  bh, bh
    int  0x10
    jmp  .print
.halt:
    cli
    hlt

drive_id db 0
msg_err  db 'Stage2: disk error', 0

%include "a20.asm"

align 4
kdap:
    db 0x10
    db 0x00
    dw KERNEL_SECTORS
    dw KERNEL_OFF
    dw KERNEL_SEG
    dq KERNEL_LBA

align 8
gdt_table:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_table - 1
    dd gdt_table

[bits 32]
pm_entry:
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    mov  esp, 0x90000
    jmp  KERNEL_PHYS
