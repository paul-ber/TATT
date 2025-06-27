/*
** ftp_list.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Thu Nov 24 23:37:19 2005 steck
** Last update Sun Nov 27 04:44:26 2005 steck
*/
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "ftp.h"

static int	list_send(struct s_client *cl, struct s_ftp *ftp,
			  FILE *f)
{
  int		sock;
  char		tmp[1024];
  int		n;

  if ((sock = connect_data(cl)) == -1 && send_reply(425, cl, ftp))
    return 0;
  send_reply(150, cl, ftp);
  if (cl->mode == MODE_STREAM)
  {
    while (!feof(f) && (n = fread(tmp, 1, 1024, f)) > 0)
      cl->send_convert(sock, tmp, n);
    send_reply(226, cl, ftp);
    close(sock);
    return 0;
  }
  return 0;
}

int	ftp_cmd_list(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  FILE	*f;
  char	*str;

  if (!client_check_logged(cl, ftp))
    return 0;
  if (s[1] && send_reply(501, cl, ftp))
    return 0;
  str = my_malloc(strlen("/bin/ls -l 2>/dev/null ") +
		  strlen(getcwd(NULL, 42)) + 1);
  sprintf(str, "/bin/ls -l 2>&1 %s", getcwd(NULL, 42));
  if (!(f = popen(str, "r")) && send_reply(451, cl, ftp) && my_free(str))
    return 0;
  my_free(str);
  list_send(cl, ftp, f);
  pclose(f);
  if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : list\n", cl->i);
  return 0;
}
