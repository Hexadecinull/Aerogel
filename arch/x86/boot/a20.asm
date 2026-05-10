; Included by stage2.asm. Provides enable_a20 and a20_check.

enable_a20:
    call a20_check
    jnz  .done

    call a20_bios
    call a20_check
    jnz  .done

    call a20_fast
    call a20_check
    jnz  .done

    call a20_kbd
    call a20_check
.done:
    ret

; Returns ZF=0 if A20 already active, ZF=1 if not.
a20_check:
    pushf
    push ds
    push es
    push di
    push si
    cli

    xor  ax, ax
    mov  es, ax
    mov  di, 0x0500

    mov  ax, 0xFFFF
    mov  ds, ax
    mov  si, 0x0510

    mov  al, [es:di]
    push ax
    mov  al, [ds:si]
    push ax

    mov  byte [es:di], 0x00
    mov  byte [ds:si], 0xFF

    cmp  byte [es:di], 0xFF

    pop  ax
    mov  [ds:si], al
    pop  ax
    mov  [es:di], al

    mov  ax, 0
    je   .exit
    mov  ax, 1
.exit:
    pop  si
    pop  di
    pop  es
    pop  ds
    popf
    ret

a20_bios:
    push ax
    mov  ax, 0x2401
    int  0x15
    pop  ax
    ret

a20_fast:
    push ax
    in   al, 0x92
    test al, 0x02
    jnz  .done
    or   al, 0x02
    and  al, 0xFE
    out  0x92, al
.done:
    pop  ax
    ret

a20_kbd:
    call .wait_in
    mov  al, 0xAD
    out  0x64, al

    call .wait_in
    mov  al, 0xD0
    out  0x64, al

    call .wait_out
    in   al, 0x60
    push ax

    call .wait_in
    mov  al, 0xD1
    out  0x64, al

    call .wait_in
    pop  ax
    or   al, 0x02
    out  0x60, al

    call .wait_in
    mov  al, 0xAE
    out  0x64, al
    call .wait_in
    ret

.wait_in:
    in   al, 0x64
    test al, 0x02
    jnz  .wait_in
    ret

.wait_out:
    in   al, 0x64
    test al, 0x01
    jz   .wait_out
    ret
