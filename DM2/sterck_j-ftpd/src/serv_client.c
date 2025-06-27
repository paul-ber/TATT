/*
** serv_client.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Sat Nov 26 23:26:24 2005 steck
** Last update Sun Nov 27 04:47:49 2005 steck
*/
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "ftp.h"

typedef int	(*t_func)(struct s_client *cl, struct s_ftp *ftp,
			  char **s);

static const struct
{
  char		*cmd;
  t_func	f;
} cmds[] =
{
  { "noop", ftp_cmd_noop },
  { "user", ftp_cmd_user },
  { "pass", ftp_cmd_pass },
  { "quit", ftp_cmd_quit },
  { "help", ftp_cmd_help },
  { "cwd", ftp_cmd_cwd },
  { "dele", ftp_cmd_dele },
  { "rmd", ftp_cmd_rmd },
  { "mkd", ftp_cmd_mkd },
  { "pwd", ftp_cmd_pwd },
  { "type", ftp_cmd_type },
  { "retr", ftp_cmd_retr },
  { "port", ftp_cmd_port },
  { "syst", ftp_cmd_syst },
  { "pasv", ftp_cmd_pasv },
  { "list", ftp_cmd_list },
  { "stor", ftp_cmd_stor },
  { "appe", ftp_cmd_appe },
  { "abor", ftp_cmd_abor },
  { NULL, NULL }
};

static char	**mk_argv_list(char *s)
{
  int		i = 0;
  char		**str = NULL;
  char		*p;

  do
  {
    p = strtok(i == 0 ? s : NULL, " ");
    ++i;
    str = realloc(str, i * sizeof (char *));
    str[i - 1] = p ? strdup(p) : NULL;
  } while (p);
  return str;
}

static void	free_p(char **p, char *s)
{
  int		i;

  for (i = 0; p[i]; i++)
    my_free(p[i]);
  my_free(p);
  my_free(s);
}

static void	init_serv_client(struct s_client *cl, struct s_ftp *ftp)
{
  char		motd[255];
  FILE		*f;

  if (!(f = fopen("banner.txt", "r")))
  {
    net_send("220 Welcome to ExploitMeSteckFTP v1.0", cl, ftp);
  }
  else
  {
    fgets(motd, 254, f);
    net_send(motd, cl, ftp);
    fclose(f);
  }
  cl->end = 0;  
}

void		serv_do_it(struct s_client *cl, struct s_ftp *ftp)
{
  char		*s;
  char		**p;
  int		i;

  init_serv_client(cl, ftp);
  while (!cl->end && (s = net_recv(cl, ftp)))
  {
    if ((p = mk_argv_list(s)) && !p[0])
      send_reply(500, cl, ftp);
    else
    {
      while (cl->on_transfert)
      	if (strcasecmp("abor", p[0]) == 0)
	  ftp_cmd_abor(cl, ftp, p);
      for (i = 0; cmds[i].cmd; i++)
	if (strcasecmp(cmds[i].cmd, p[0]) == 0 &&
	    ((cl->end = cmds[i].f(cl, ftp, p)) + 1))
	  break;
      if (!cmds[i].cmd)
	send_reply(500, cl, ftp);
    }
    free_p(p, s);
    fflush(ftp->log_f);
  }
  close(cl->socket);
}
