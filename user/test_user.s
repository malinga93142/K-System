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
    int 65                          # print the message first

    mov eax, 0x09000000
    mov dword ptr [eax], 0xCAFEBABE  # deliberate page fault -- auto-map test

    mov ebx, [eax]                    # read back, should be 0xCAFEBABE now

    int 64                              # halt, LAST
1:
    jmp 1b
test_code_end:
