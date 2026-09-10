.intel_syntax noprefix
.globl enter_usermode
enter_usermode:
    mov edx, [esp+4]      # entry point -- use edx, never touched below
    mov ecx, [esp+8]      # user stack
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push 0x23              # SS
    push ecx               # ESP
   	mov eax, 0x202
    push eax                 # EFLAGS
    push 0x1B                # CS
    push edx                 # EIP -- edx still holds the original entry point
    iret
