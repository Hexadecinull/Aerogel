[bits 32]
[section .text]

; void switch_context(u32 *old_esp, u32 new_esp)
global switch_context
switch_context:
    push ebp
    push ebx
    push esi
    push edi

    mov  eax, [esp + 20]
    mov  [eax], esp

    mov  esp, [esp + 24]

    pop  edi
    pop  esi
    pop  ebx
    pop  ebp
    ret

; void thread_trampoline(void)
; Stack on entry: [fn_ptr, arg_ptr] below return address slot
global thread_trampoline
thread_trampoline:
    pop  eax          ; fn
    pop  ecx          ; arg
    push ecx
    call eax
    add  esp, 4

    extern thread_exit
    call thread_exit
.hang:
    cli
    hlt
    jmp  .hang

section .note.GNU-stack noalloc noexec nowrite progbits
