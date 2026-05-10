[bits 64]
[section .text]

; void switch_context(u64 *old_rsp, u64 new_rsp)
; rdi = &old_rsp, rsi = new_rsp
global switch_context
switch_context:
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov  [rdi], rsp
    mov  rsp, rsi

    pop  r15
    pop  r14
    pop  r13
    pop  r12
    pop  rbx
    pop  rbp
    ret

global thread_trampoline
thread_trampoline:
    pop  rax          ; fn
    pop  rdi          ; arg
    call rax

    extern thread_exit
    call thread_exit
.hang:
    cli
    hlt
    jmp  .hang

section .note.GNU-stack noalloc noexec nowrite progbits
