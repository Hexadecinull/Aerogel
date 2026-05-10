[bits 16]
[org 0x7C00]

STAGE2_LBA     equ 1
STAGE2_SECTORS equ 16
STAGE2_SEG     equ 0x0000
STAGE2_OFF     equ 0x7E00

    cli
    xor  ax, ax
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0x7C00
    sti

    mov  [drive_id], dl

    mov  ah, 0x41
    mov  bx, 0x55AA
    int  0x13
    jc   .error
    cmp  bx, 0xAA55
    jne  .error

    mov  si, dap
    mov  ah, 0x42
    mov  dl, [drive_id]
    int  0x13
    jc   .error

    mov  dl, [drive_id]
    jmp  STAGE2_SEG:STAGE2_OFF

.error:
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
msg_err  db 'Boot error', 0

align 4
dap:
    db 0x10
    db 0x00
    dw STAGE2_SECTORS
    dw STAGE2_OFF
    dw STAGE2_SEG
    dq STAGE2_LBA

times 446-($-$$) db 0
times 64          db 0
dw 0xAA55
