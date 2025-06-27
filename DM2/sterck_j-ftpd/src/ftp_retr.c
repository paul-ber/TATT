/*
** ftp_retr.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Thu Nov 24 14:49:47 2005 steck
** Last update Sun Nov 27 05:14:32 2005 steck
*/
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "ftp.h"

static int		connect_data_act(struct s_client *cl)
{
  int			sock;
  struct sockaddr_in	serv;

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    return -1;
  serv.sin_family = AF_INET;
  serv.sin_port = htons(cl->port.port);
  serv.sin_addr.s_addr = inet_addr(cl->port.addr);
  if (connect(sock, (struct sockaddr *)&serv, sizeof (struct sockaddr_in)) < 0)
    return -1;
  return sock;
}

static int		connect_data_pasv(struct s_client *cl)
{
  int			sock;
  struct sockaddr_in	serv;
  int			len;
  int			sock_serv;

  memset(&serv, 0, sizeof (struct sockaddr_in));
  len = sizeof (struct sockaddr_in);
  if ((sock_serv = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    return -1;
  serv.sin_family = AF_INET;
  serv.sin_port = htons(cl->port.port);
  serv.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr *)&serv, len))
    return -1;
  if (listen (sock, 5))
    return -1;
  sock_serv = accept(sock, (struct sockaddr *)&serv, &len);
  close(sock);
  return sock;
}

int			connect_data(struct s_client *cl)
{
  return cl->t_mode == T_MODE_PASV ?
    connect_data_pasv(cl) : connect_data_act(cl);
}

static int	retr_send(struct s_client *cl, struct s_ftp *ftp,
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
    close(sock);
    send_reply(226, cl, ftp);
    return 0;
  }
  return 0;
}

static void	*retr_thread(void *param)
{
  struct s_par	*p = param;

  p->cl->on_transfert = 1;
  retr_send(p->cl, p->ftp, p->f);
  fclose(p->f);
  p->cl->on_transfert = 0;
  my_free(param);
  pthread_exit(NULL);
  return NULL;
}

int		ftp_cmd_retr(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  FILE		*f;
  struct s_par	*p;

  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!good_dir(s[1], cl) && send_reply(550, cl, ftp))
    return 0;
  if (!(f = fopen(s[1], "rb")) && send_reply(450, cl, ftp))
    return 0;
  p = my_malloc(sizeof (struct s_par));
  p->cl = cl;
  p->ftp = ftp;
  p->f = f;
  pthread_create(&(cl->thr), NULL, retr_thread, p);
  if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : retr %s\n", cl->i, s[1]);
  return 0;
}

static int	count(char *s, char sym)
{
  int		i;
  int		nb = 0;

  if (!s)
    return 0;
  for (i = 0; s[i]; i++)
    if (s[i] == sym)
      nb++;
  return nb;
}

static void	ftp_cmd_getaddr(struct s_client *cl, struct s_ftp *ftp, char *s)
{
  char		*str;
  unsigned int	h1 = 0;
  unsigned int	h2 = 0;
  unsigned int	h3 = 0;
  unsigned int	h4 = 0;
  unsigned int	p1 = 0;
  unsigned int	p2 = 0;

  if (sscanf(s, "%u,%u,%u,%u,%u,%u", &h1, &h2, &h3, &h4, &p1, &p2)
      < 6 && send_reply(501, cl, ftp))
    return;
  if (cl->port.addr)
    my_free(cl->port.addr);
  str = my_malloc(16 * sizeof (char));
  cl->port.port = (p1 % 256) * 256 + (p2 % 256);
  sprintf(str, "%u.%u.%u.%u", h1 % 256, h2 % 256, h3 % 256, h4 % 256);
  cl->port.addr = str;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : port=%d addr=%s\n", cl->i, cl->port.port,
	    cl->port.addr);
  send_reply(200, cl, ftp);
}

int		ftp_cmd_port(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (count(s[1], ',') != 5 && send_reply(501, cl, ftp))
    return 0;
  ftp_cmd_getaddr(cl, ftp, s[1]);
  return 0;
}
