/*
** ufile.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Mon Nov 21 22:35:51 2005 steck
** Last update Sun Nov 27 02:02:18 2005 steck
*/
#include <pthread.h>
#include <string.h>
#include "ftp.h"

static int	ufile_error(void)
{
  char		*s = "Error: Bad users file format\n";

  fprintf(stderr, s);
  return 0;
}

static void	stripbn(char *s)
{
  int		i;

  if (!s)
    return;
  for (i = 0; s[i] != '\0' && s[i] != '\n'; i++)
    ;
  s[i] = '\0';
}

static void		ufile_printaccess(struct s_ftp *ftp)
{
  struct s_users	*u;
  char			*s = "User ";

  printf(s);
  for (u = ftp->users; u; u = u->next)
  {
    printf("'%s'", u->name);
    if (u->next)
    {
      if(u->next->next)
	printf(", ");
      else
	printf(" and ");
    }
  }
  printf(" have fun on this FTP\n");
}

int			ufile_read(struct s_ftp *ftp)
{
  struct s_users	*u;
  struct s_users	*o = NULL;
  char			s[512];

  while ((fgets(s, 512, ftp->ufile_f)))
  {
    u = my_malloc(sizeof (struct s_users));
    if (o)
      o->next = u;
    u->name = strdup(strtok(s, ":"));
    u->passwd = strdup(strtok(NULL, ":"));
    u->wd = strdup(strtok(NULL, ":"));
    stripbn(u->wd);
    u->next = NULL;
    if (!u->name || !u->passwd || !u->wd)
      return ufile_error();
    if (!ftp->users)
      ftp->users = u;
    o = u;
  }
  if (!ftp->users)
    return ufile_error();
  ufile_printaccess(ftp);
  return 1;
}

int			ufile_is_user(struct s_ftp *ftp, char *name)
{
  struct s_users	*u;

  for (u = ftp->users; u; u = u->next)
    if (strcmp(name, u->name) == 0)
      return 1;
  return 0;
}

int			ufile_is_pass(struct s_ftp *ftp,
				      char *name, char *pass)
{
  struct s_users	*u;

  for (u = ftp->users; u; u = u->next)
    if (strcmp(name, u->name) == 0)
    {
      if (strcmp(pass, u->passwd) == 0)
	return 1;
      return 0;
    }
  return 0;
}

char			*ufile_get_chroot(struct s_ftp *ftp, char *name)
{
  struct s_users	*u;

  for (u = ftp->users; u; u = u->next)
    if (strcmp(name, u->name) == 0)
    {
      return u->wd;
    }
  return NULL;
}
