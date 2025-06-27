/*
** ftp_cwd.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Wed Nov 23 17:19:30 2005 steck
** Last update Sun Nov 27 02:04:17 2005 steck
*/
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include "ftp.h"

int	good_dir(char *s, struct s_client *cl)
{
  char	*dir;

  dir = get_absolute_dir(dirname(s), cl);
  if (strncmp(dir, cl->chroot, strlen(cl->chroot)))
  {
    my_free(dir);
    return 0;
  }
  my_free(dir);
  return 1;
}

char		*get_absolute_dir(char *s, struct s_client *cl)
{
  char		*old;
  char		*new;
  char		*tmp;

  old = getcwd(NULL, 42);
  if (s[0] == '/')
  {
    tmp = my_malloc((strlen(s) + strlen(cl->chroot) + 1) * sizeof (char));
    sprintf(tmp, "%s%s", cl->chroot, s);
  }
  else
    tmp = strdup(s);
  if (chdir(tmp) == -1 && my_free(old) && my_free(tmp))
    return NULL;
  new = getcwd(NULL, 42);
  chdir(old);
  my_free(old);
  my_free(tmp);
  if (new[strlen(new)] == '/')
    return new;
  tmp = my_malloc((strlen(new) + 1) * sizeof (char));
  sprintf(tmp, "%s/", new);
  my_free(new);
  return tmp;
}

static int	cwd_get_dirs(char *s, struct s_client *cl, char **adir,
			      char **ldir)
{
  char		*a_dir;

  if (!(a_dir = get_absolute_dir(s, cl)) && my_free(a_dir))
    return 0;
  if (strncmp(a_dir, cl->chroot, strlen(cl->chroot)))
  {
    *adir = strdup(cl->chroot);
    *ldir = strdup("/");
    return 1;
  }
  *adir = a_dir;
  *ldir = strdup(a_dir + strlen(cl->chroot));
  return 1;
}

int	ftp_cmd_cwd(struct s_client *cl, struct s_ftp *ftp, char **s)
{
  char	*adir;
  char	*ldir;

  if (!client_check_logged(cl, ftp))
    return 0;
  if ((!s[1] || s[2]) && send_reply(501, cl, ftp))
    return 0;
  if (!cwd_get_dirs(s[1], cl, &adir, &ldir) && send_reply(550, cl, ftp))
    return 0;
  chdir(adir);
  if (cl->wd)
    my_free(cl->wd);
  cl->wd = ldir;
  if (ftp->v > 1)
    fprintf(ftp->log_f, "Client %i : cwd %s\n", cl->i, s[1]);
  send_reply(250, cl, ftp);
  return 0;
}
