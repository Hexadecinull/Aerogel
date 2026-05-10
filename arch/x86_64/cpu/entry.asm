[bits 64]
[section .text.entry]

extern kmain
extern _bss_start
extern _bss_end

global _start
_start:
    mov  rsp, 0x90000

    lea  rdi, [rel _bss_start]
    lea  rcx, [rel _bss_end]
    sub  rcx, rdi
    xor  eax, eax
    rep  stosb

    call kmain

.hang:
    cli
    hlt
    jmp  .hang

section .note.GNU-stack noalloc noexec nowrite progbits
