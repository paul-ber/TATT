/*
** client.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Tue Nov 22 14:26:32 2005 steck
** Last update Sun Nov 27 02:02:26 2005 steck
*/
#include <pthread.h>
#include "ftp.h"

int	client_new(struct s_ftp *ftp)
{
  int	i;

  for (i = 0; i < MAX_CLIENTS; i++)
  {
    if (ftp->clients[i].free)
      break;
  }
  if (i == MAX_CLIENTS)
    return -1;
  ftp->clients[i].free = 0;
  ftp->clients[i].logged = 0;
  ftp->clients[i].pid = 0;
  ftp->clients[i].socket = 0;
  time(&ftp->clients[i].begin);
  ftp->clients[i].tr = 0;
  ftp->clients[i].wd = NULL;
  ftp->clients[i].user = NULL;
  ftp->clients[i].type = TYPE_ASCII_N;
  ftp->clients[i].send_convert = send_str_ascii;
  ftp->clients[i].mode = MODE_STREAM;
  ftp->clients[i].port.port = 20;
  ftp->clients[i].port.addr = NULL;
  ftp->clients[i].on_transfert = 0;
  ftp->clients[i].t_mode = T_MODE_ACTV;
  return i;
}

void	client_init(struct s_ftp *ftp)
{
  int	i;

  for (i = 0; i < MAX_CLIENTS; i++)
  {
    ftp->clients[i].free = 1;
    ftp->clients[i].logged = 0;
    ftp->clients[i].pid = 0;
    ftp->clients[i].socket = 0;
    ftp->clients[i].tr = 0;
    ftp->clients[i].i = i;
    ftp->clients[i].wd = NULL;
    ftp->clients[i].user = NULL;
  }
}

int	client_check_logged(struct s_client *cl, struct s_ftp *ftp)
{
  if (!cl->logged)
  {
    send_reply(530, cl, ftp);
    return 0;
  }
  return 1;
}
