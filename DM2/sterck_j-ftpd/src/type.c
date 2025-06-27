/*
** type.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Thu Nov 24 23:47:18 2005 steck
** Last update Sun Nov 27 02:04:51 2005 steck
*/
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include "ftp.h"

void	send_str_image(int sock, char *b, int l)
{
  send(sock, b, l, 0);
}

void	send_str_ascii(int sock, char *b, int l)
{
  int	n = 0;
  int	i;
  int	j;
  char	*s;

  for (i = 0; i < l; i++)
    if (b[i] == '\n')
      n++;
  if (n == 0)
  {
    send(sock, s, j, 0);
    return;
  }
  s = my_malloc(l + n * 2 + 1);
  for (i = 0, j = 0; i < l; i++, j++)
  {
    if (b[i] != '\n')
      s[j] = b[i];
    else if ((s[j] = '\r'))
      s[++j] = '\n';
  }
  send(sock, s, j, 0);
  my_free(s);
}
