#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
void traite(size_t size, char *buf, struct sockaddr_in *sa, int sock)
{
    register unsigned int i;
    char cmd[128];
    for (i = 0; i < size && buf[i] != ’ ’; ++i)
        cmd[i] = buf[i];
    cmd[i] = ’\0’;
    printf(">%s<\n", cmd);
    if (strcmp(cmd, "ping") == 0)
        sendto(sock, "pong\n", 5, 0, (struct sockaddr*)sa,
                sizeof (struct sockaddr_in));
    if (strcmp(cmd, "help") == 0)
    {
        char *s = "commandes : ping help\n";
        sendto(sock, s, strlen(s), 0, (struct sockaddr*)sa,
                sizeof (struct sockaddr_in));
    }
    printf("fin de traite\n");
}
int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in sa;
    char buffer[1024];
    size_t fromlen, recsize;
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(7777);
    if (-1 == bind(sock,(struct sockaddr *)&sa, sizeof(struct sockaddr)))
    {
        perror("error bind failed");
        close(sock);
        exit(1);
    }
    while(1)
    {
        printf("En attente...\n");
        fromlen = sizeof(struct sockaddr);
        recsize = recvfrom(sock, (void *)buffer, 1024, 0, (struct sockaddr *)&sa,
                &fromlen);
        if (recsize != 0)
        {
            printf("recu : %d\n", recsize);
            buffer[recsize - 1] = ’\0’;
            traite(recsize, buffer, &sa, sock);
        }
    }
}
