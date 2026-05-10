[bits 64]
[section .text]

global gdt_flush
gdt_flush:
    lgdt [rdi]
    mov  ax, 0x10
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    lea  rax, [rel .flush]
    push qword 0x08
    push rax
    retfq
.flush:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
