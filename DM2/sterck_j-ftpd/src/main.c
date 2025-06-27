/*
** main.c for my_ftpd in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Mon Nov 21 12:13:14 2005 steck
** Last update Sun Nov 27 03:42:04 2005 steck
*/
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "ftp.h"

extern char	*optarg;
typedef int	(*t_parse)(struct s_ftp *ftp);

static int	parse_v(struct s_ftp *ftp)
{
  char		*inv = NULL;

  ftp->v = strtoul(optarg, &inv, 10);
  if ((!inv || *inv == '\0') && ftp->v <= 4)
    printf("Log level: %d\n", ftp->v);
  else
    fprintf(stderr, "Error: %s: bad verbose number, using mode 1\n",
	    optarg);
  return 1;
}

static int	parse_p(struct s_ftp *ftp)
{
  char		*inv = NULL;

  ftp->port = strtoul(optarg, &inv, 10);
  if (!inv || *inv == '\0')
    return 1;
  fprintf(stderr, "Error: %s: bad port number\n", optarg);
  return 0;
}

static int	parse_u(struct s_ftp *ftp)
{
  if ((ftp->ufile_f = fopen(optarg, "r")))
  {
    printf("Reading configuration file: '%s'\n", optarg);
    return ufile_read(ftp);
  }
  fprintf(stderr, "Error: %s: %s\n", optarg, strerror(errno));
  return 1;
}

static int	parse_l(struct s_ftp *ftp)
{
  if ((ftp->log_f = fopen(optarg, "a")))
    printf("Log events in: '%s'\n", optarg);
  else
    fprintf(stderr, "Error: %s: %s, output to stdout\n",
	    optarg, strerror(errno));
  return 1;
}

static const struct
{
  char		p;
  t_parse	f;
} parse[] =
{
  { 'p', parse_p },
  { 'u', parse_u },
  { 'v', parse_v },
  { 'l', parse_l },
  { '\0', NULL }
};

static int	usage(void)
{
  char		*msg = "usage: my_ftp -u user_file [-v num] [-l log_file] [-p port]\n user_file must contain lines like 'user:password:home'\n";

  printf(msg);
  printf(" v represents the output messages (default is 0):\n");
  printf("  if num equals 0, no output\n");
  printf("  if num equals 1, ftp error messages are printed\n");
  printf("  if num equals 2, ftp users actions are printed\n");
  printf("  if num equals 3, all ftp dialog is printed\n");
  printf(" log_file is the log file (default is stdout)\n");
  printf(" port is the server port (default is 21)\n");
  return 1;
}

static int	get_opts(int argc, char **argv, struct s_ftp *ftp)
{
  int		i;
  int		j;
  FILE		*f;
  char		banner[255];

  while ((i = getopt(argc, argv, "p:u:l:v:")) != -1)
  {
    for (j = 0; parse[j].f; j++)
      if (parse[j].p == i)
	if (!parse[j].f(ftp))
	  return 0;
  }
  if (!ftp->ufile_f && usage())
    return 0;
  if (!ftp->log_f)
    ftp->log_f = stdout;
  if (ftp->v != 0 && (f = fopen("banner.txt", "r")))
  {
    fprintf(ftp->log_f, "banner :\n");
    fgets(banner, 254, f);
    fprintf(ftp->log_f, banner);
    fprintf(ftp->log_f, "\n");
    fclose(f);
  }
  return 1;
}

int		main(int argc, char *argv[])
{
  struct s_ftp	*ftp;

  srandom(time(NULL));
  ftp = ftp_new();
  if (!get_opts(argc, argv, ftp) && ftp_free(ftp))
    return 1;
  fclose(ftp->ufile_f);
  client_init(ftp);
  serv_loop(ftp);
  ftp_free(ftp);
  return 0;
}
