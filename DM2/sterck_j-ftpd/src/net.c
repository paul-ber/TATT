/*
** net.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Wed Nov 23 00:31:59 2005 steck
** Last update Sun Nov 27 02:03:10 2005 steck
*/
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include "ftp.h"

int	net_send(char *s, struct s_client *cl, struct s_ftp *ftp)
{
  char	*s2;

  if (ftp->v == 3)
    fprintf(ftp->log_f, "To client %i : %s\n", cl->i, s);
  s2 = my_malloc((strlen(s) + 3) * sizeof (char));
  sprintf(s2, "%s\r\n", s);
  send(cl->socket, s2, strlen(s2), 0);
  my_free(s2);
  return 1;
}

char	*net_recv(struct s_client *cl, struct s_ftp *ftp)
{
  char	*s;
  int	i;

  s = my_malloc(1024 * sizeof (char));
  i = recv(cl->socket, s, 1024, 0);
  if (i == -1)
  {
    my_free(s);
    return NULL;
  }
  if (i > 1)
    s[i - 2] = '\0';
  else
    s[0] = '\0';
  if (ftp->v == 3)
    fprintf(ftp->log_f, "From client %i : %s\n", cl->i, s);
  return s;
}
