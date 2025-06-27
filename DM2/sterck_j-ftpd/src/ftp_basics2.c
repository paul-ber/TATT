/*
** ftp_basics2.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Wed Nov 23 14:50:31 2005 steck
** Last update Sun Nov 27 02:03:26 2005 steck
*/
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include "ftp.h"

int	ftp_cmd_pwd(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  char	*str;

  if (!client_check_logged(cl, ftp))
    return 0;
  if (s[1] && send_reply(501, cl, ftp))
    return 0;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : pwd\n", cl->i);
  str = my_malloc(1024 * sizeof (char));
  snprintf(str, 1024, "257 \"%s\"", cl->wd);
  net_send(str, cl, ftp);
  my_free(str);
  return 0;
}

int	ftp_cmd_dele(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!good_dir(s[1], cl) && send_reply(450, cl, ftp))
    return 0;
  if (unlink(s[1]) == -1 && send_reply(450, cl, ftp))
    return 0;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : dele %s\n", cl->i, s[1]);
  send_reply(250, cl, ftp);
  return 0;
}

int	ftp_cmd_mkd(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  char	str[1024];

  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!good_dir(s[1], cl) && send_reply(550, cl, ftp))
    return 0;
  if (mkdir(s[1], 0777) == -1 && send_reply(550, cl, ftp))
    return 0;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : mkd %s\n", cl->i, s[1]);
  sprintf(str, "257 \"%s\" created.", s[1]);
  net_send(str, cl, ftp);
  return 0;
}

int	ftp_cmd_rmd(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!good_dir(s[1], cl) && send_reply(550, cl, ftp))
    return 0;
  if (rmdir(s[1]) == -1 && send_reply(550, cl, ftp))
    return 0;
  send_reply(250, cl, ftp);
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : rmd %s\n", cl->i, s[1]);
  return 0;
}

static int	cmd_type_i(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (s[2] && send_reply(501, cl, ftp))
    return 0;
  cl->type = TYPE_IMAGE;
  cl->send_convert = send_str_image;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : type %s\n", cl->i, s[1]);
  send_reply(200, cl, ftp);
  return 0;
}

static int	cmd_type_a(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (s[2] && strcmp(s[2], "N") && send_reply(504, cl, ftp))
    return 0;
  if (!s[2] || strcmp(s[2], "N") == 0)
  {
    cl->type = TYPE_ASCII_N;
    cl->send_convert = send_str_ascii;
  }
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : type %s\n", cl->i, s[1]);
  send_reply(200, cl, ftp);
  return 0;
}

int	ftp_cmd_type(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  if (!client_check_logged(cl, ftp))
    return 0;
  if (!s[1] && send_reply(501, cl, ftp))
    return 0;
  if (strcasecmp(s[1], "I") == 0)
    return cmd_type_i(cl, ftp, s);
  if (strcasecmp(s[1], "A") == 0)
    return cmd_type_a(cl, ftp, s);
  send_reply(504, cl, ftp);
  return 0;
}
