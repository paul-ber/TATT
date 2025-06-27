/*
** ftp_stor.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Thu Nov 24 14:49:47 2005 steck
** Last update Sun Nov 27 04:45:45 2005 steck
*/
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "ftp.h"

static void	write_ascii_conv(char *b, int t, FILE *f)
{
  int		i;
  int		j;

  for (i = 0; i < t; ++i)
    if (b[i] == '\r')
      break;
  if (i == t)
  {
    fwrite(b, 1, t, f);
    return;
  }
  b[i++] = '\n';
  for (j = i; i < t; ++i, ++j)
  {
    if (b[i] != '\r')
      b[j] = b[i];
    else
    {
      if (b[++i] == '\n')
	b[j] = '\n';
      else if ((b[j] = '\r'))
	b[++j] = b[i];
    }
  }
  fwrite(b, 1, j, f);
}

static int	stor(struct s_client *cl, struct s_ftp *ftp,
			   FILE *f)
{
  int		sock;
  char		tmp[1024];
  int		n;

  f = f;
  if ((sock = connect_data(cl)) == -1 && send_reply(425, cl, ftp))
    return 0;
  send_reply(150, cl, ftp);
  if (cl->mode == MODE_STREAM)
  {
    while ((n = recv(sock, tmp, 1, 0)) > 0)
    {
      if (cl->type == TYPE_IMAGE)
	fwrite(tmp, 1, n, f);
      else
	write_ascii_conv(tmp, n, f);
    }
    close(sock);
    send_reply(226, cl, ftp);
    return 0;
  }
  return 0;
}

int	ftp_cmd_stor(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  FILE	*f;

  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!good_dir(s[1], cl) && send_reply(450, cl, ftp))
    return 0;
  if (!(f = fopen(s[1], "wb")) && send_reply(450, cl, ftp))
    return 0;
  stor(cl, ftp, f);
  fclose(f);
  if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : stor %s\n", cl->i, s[1]);
  return 0;
}

int	ftp_cmd_appe(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  FILE	*f;

  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!good_dir(s[1], cl) && send_reply(450, cl, ftp))
    return 0;
  if (!(f = fopen(s[1], "ab")) && send_reply(450, cl, ftp))
    return 0;
  stor(cl, ftp, f);
  fclose(f);
  if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : appe %s\n", cl->i, s[1]);
  return 0;
}
