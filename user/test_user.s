# test_user.S
.intel_syntax noprefix
.global test_code_start
.global test_code_end

test_code_start:
    call get_msg
hello_start:
    .ascii "hello from ring3"
hello_end:
get_msg:
    pop edx
    mov ecx, hello_end - hello_start
    int 65
    ; int 64
1:
    jmp 1b
test_code_end:
