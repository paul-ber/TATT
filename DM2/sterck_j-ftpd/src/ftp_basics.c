/*
** ftp_basics.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Tue Nov 22 18:16:07 2005 steck
** Last update Sun Nov 27 04:51:56 2005 steck
*/
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "ftp.h"

int	ftp_cmd_user(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!s[1] && send_reply(332, cl, ftp))
    return 0;
  if (s[2] && send_reply(501, cl, ftp))
    return 0;
  if (cl->user)
    my_free(cl->user);
  cl->logged = 0;
  cl->user = strdup(s[1]);
  send_reply(331, cl, ftp);
  return 0;
}

static int	ftp_cmd_getchroot(struct s_client *cl, struct s_ftp *ftp)
{
  cl->chroot = ufile_get_chroot(ftp, cl->user);
  if (!cl->wd)
    cl->wd = strdup("/");
  if (chdir(cl->chroot) == -1)
    return 0;
  my_free(cl->chroot);
  cl->chroot = strdup(getcwd(NULL, 42));
  return 1;
}

int	ftp_cmd_pass(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!s[1] && send_reply(501, cl, ftp))
    return 0;
  if ((!cl->user || cl->logged) && send_reply(503, cl, ftp))
    return 0;
  if (!ufile_is_user(ftp, cl->user) || !ufile_is_pass(ftp, cl->user, s[1]) ||
      !ftp_cmd_getchroot(cl, ftp))
  {
    my_free(cl->user);
    cl->user = NULL;
    send_reply(530, cl, ftp);
    return 0;
  }
  send_reply(230, cl, ftp);
  cl->logged = 1;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i has logged in as %s in %s\n", cl->i,
	    cl->user, cl->chroot);
  return 0;
}

int	ftp_cmd_quit(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (s[1] && send_reply(500, cl, ftp))
    return 0;
  if (ftp->v > 0)
    fprintf(ftp->log_f, "Client %i has logged out\n", cl->i);
  send_reply(221, cl, ftp);
  return 1;
}

int	ftp_cmd_noop(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (s[1])
    send_reply(500, cl, ftp);
  else
  {
    if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : noop\n", cl->i);
    send_reply(200, cl, ftp);
  }
  return 0;
}

int	ftp_cmd_help(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  char	*msg = "214-The following commands are recognized.\r\n"
    "ABOR APPE CWD  DELE HELP LIST MKD\r\n"
    "MODE NOOP PASS PASV PORT PWD  QUIT\r\n"
    "RMD  STOR RMD  SYST TYPE USER RETR\r\n"
    "214 Help OK.";

  if (!s[1])
  {
    if (ftp->v > 1)
      fprintf(ftp->log_f, "Client %i : help\n", cl->i);
    net_send(msg, cl, ftp);
  }
  else
    send_reply(501, cl, ftp);
  return 0;
}
