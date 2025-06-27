/*
** ftp_basics3.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Thu Nov 24 16:15:16 2005 steck
** Last update Sun Nov 27 02:03:50 2005 steck
*/
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "ftp.h"

int	ftp_cmd_syst(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!client_check_logged(cl, ftp))
    return 0;
  if (s[1] && send_reply(501, cl, ftp))
    return 0;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : syst\n", cl->i);
  send_reply(215, cl, ftp);
  return 0;
}

static int		get_free_port(struct s_ftp *ftp)
{
  int			sock;
  struct sockaddr_in	test;
  int			len;
  int			port;

  memset(&test, 0, sizeof (struct sockaddr_in));
  len = sizeof (struct sockaddr_in);
  port = ftp->port + 2 + random() % (0xFFFF - ftp->port);
  do
  {
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
      return -1;
    test.sin_family = AF_INET;
    test.sin_port = htons(port);
    test.sin_addr.s_addr = INADDR_ANY;
  }
  while (bind(sock, (struct sockaddr *)&test, len) == -1);
  close(sock);
  return port;
}

int	ftp_cmd_pasv(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  char	*str;
  char	*addr;
  int	a;
  int	b;
  int	c;
  int	d;

  if (!client_check_logged(cl, ftp))
    return 0;
  if (s[1] && send_reply(501, cl, ftp))
    return 0;
  cl->port.port = get_free_port(ftp);
  addr = inet_ntoa(ftp->addr.sin_addr);
  sscanf(addr, "%i.%i.%i.%i", &a, &b, &c, &d);
  str = my_malloc(60);
  sprintf(str, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
	  a, b, c, d, (cl->port.port / 0xFF), (cl->port.port & 0xFF));
  net_send(str, cl, ftp);
  my_free(str);
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : pasv\n", cl->i);
  return 0;
}

int	ftp_cmd_abor(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!client_check_logged(cl, ftp))
    return 0;
  if (s[1] && send_reply(501, cl, ftp))
    return 0;
  if (cl->on_transfert)
  {
    pthread_cancel(cl->thr);
    cl->on_transfert = 0;
    send_reply(426, cl, ftp);
    if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : abor\n", cl->i);
  }
  send_reply(226, cl, ftp);
  return 0;
}
