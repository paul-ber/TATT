/*
** serv.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Tue Nov 22 15:00:42 2005 steck
** Last update Sun Nov 27 05:03:37 2005 steck
*/
#include <pthread.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "ftp.h"

static int		serv_set_sock(struct s_ftp *ftp, int *sock,
				      struct sockaddr_in *addr)
{
  int			len = sizeof (struct sockaddr_in);

  memset(addr, 0, sizeof (struct sockaddr_in));
  *sock = socket(AF_INET, SOCK_STREAM, 0);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(ftp->port);
  addr->sin_addr.s_addr = INADDR_ANY;
  if (bind(*sock, (struct sockaddr *)addr, len))
  {
    fprintf(ftp->log_f, "Failed to bind on port %d...\n", ftp->port);
    return 1;
  }
  if (listen (*sock, 5))
  {
    fprintf(ftp->log_f, "Failed to listen on port %d...\n", ftp->port);
    return 1;
  }
  fprintf(ftp->log_f, "Waiting for connexion on port %d...\n", ftp->port);
  return 0;
}

static void	serv_check_resident_evil(struct s_ftp *ftp)
{
  int		pid;
  int		r;
  int		i;

  while ((pid = waitpid(-1, &r, WNOHANG)) != -1)
    for (i = 0; i < MAX_CLIENTS; i++)
      if (ftp->clients[i].pid == pid)
      {
	ftp->clients[i].free = 1;
	ftp->clients[i].pid = -1;
      }
}

static void	serv_serv(struct s_ftp *ftp, int sock,
			  struct sockaddr_in *addr, int len)
{
  int		i;

  while (ftp->run)
  {
    serv_check_resident_evil(ftp);
    while ((i = client_new(ftp)) == -1)
      serv_check_resident_evil(ftp);
    ftp->clients[i].socket = accept(sock, (struct sockaddr *)addr, &len);
    serv_check_resident_evil(ftp);
    getsockname(ftp->clients[i].socket, (struct sockaddr *)&ftp->addr, &len);
    if (ftp->v > 0)
      fprintf(ftp->log_f, "Client %s connected as %i\n",
	      inet_ntoa(addr->sin_addr), i);
    if ((ftp->clients[i].pid = fork()))
      close(ftp->clients[i].socket);
    else
    {
      close(sock);
      serv_do_it(&(ftp->clients[i]), ftp);
      exit(0);
    }
  }
}

int			serv_loop(struct s_ftp *ftp)
{
  int			sock;
  struct sockaddr_in	addr;
  int			i;

  if (serv_set_sock(ftp, &sock, &addr))
    return 1;
  if (ftp->log_f != stdout)
  {
    if (fork())
      exit(0);
    setsid();
    chdir("/");
    for (i = 0; i < 3; i++)
      close(i);
  }
  serv_serv(ftp, sock, &addr, sizeof (struct sockaddr_in));
  if (ftp->v > 0)
    fprintf(ftp->log_f, "Server exiting...\n");
  return 1;
}
