/*
** ftp.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Mon Nov 21 21:43:48 2005 steck
** Last update Sun Nov 27 05:16:05 2005 steck
*/
#include <pthread.h>
#include <stdio.h>
#include "ftp.h"

struct s_ftp	*ftp_new(void)
{
  struct s_ftp	*ftp;

  ftp = my_malloc(sizeof (struct s_ftp));
  ftp->port = 21;
  ftp->v = 0;
  ftp->log_f = NULL;
  ftp->ufile_f = NULL;
  ftp->users = NULL;
  ftp->run = 1;
  return ftp;
}

int	ftp_free(struct s_ftp *ftp)
{
  if (ftp->log_f)
    fclose(ftp->log_f);
  return 1;
}
