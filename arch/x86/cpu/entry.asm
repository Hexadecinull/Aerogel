[bits 32]
[section .text.entry]

extern kmain
extern _bss_start
extern _bss_end

global _start
_start:
    mov  esp, 0x90000

    mov  edi, _bss_start
    mov  ecx, _bss_end
    sub  ecx, edi
    xor  eax, eax
    rep  stosb

    call kmain

.hang:
    cli
    hlt
    jmp  .hang

section .note.GNU-stack noalloc noexec nowrite progbits
