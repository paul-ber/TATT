/*
** ftp.h for my_ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Mon Nov 21 21:40:11 2005 steck
** Last update Sun Nov 27 04:59:11 2005 steck
*/
#ifndef FTP_H_
# define FTP_H_

# include <stdio.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <time.h>

# define MAX_CLIENTS	64
# define TYPE_ASCII_N	1
# define TYPE_IMAGE	4
# define MODE_STREAM	1
# define T_MODE_ACTV	1
# define T_MODE_PASV	2

typedef void	(*t_type_f)(int sock, char *s, int n);

struct			s_users
{
  char			*name;
  char			*passwd;
  char			*wd;
  struct s_users	*next;
};

struct			s_port
{
  char			*addr;
  unsigned short	port;
};

struct		s_client
{
  int		logged;
  int		end;
  int		free;
  int		pid;
  int		i;
  int		socket;
  char		*user;
  char		*wd;
  char		*chroot;
  time_t	begin;
  int		type;
  int		mode;
  int		t_mode;
  pthread_t	thr;
  int		on_transfert;
  t_type_f	send_convert;
  struct s_port	port;
  unsigned long	tr;
};

struct			s_ftp
{
  int			port;
  struct sockaddr_in	addr;
  int			v;
  int			run;
  FILE			*log_f;
  FILE			*ufile_f;
  struct s_client	clients[MAX_CLIENTS];
  struct s_users	*users;
};

struct			s_par
{
  FILE			*f;
  struct s_client	*cl;
  struct s_ftp		*ftp;
};

/*
** ftp.c
*/
struct s_ftp	*ftp_new(void);
int		ftp_free(struct s_ftp *ftp);

/*
** my_malloc.c
*/
void		*my_malloc(size_t t);
int		my_free(void *f);

/*
** ufile.c
*/
int		ufile_read(struct s_ftp *ftp);
int		ufile_is_user(struct s_ftp *ftp, char *name);
int		ufile_is_pass(struct s_ftp *ftp, char *name, char *pass);
char		*ufile_get_chroot(struct s_ftp *ftp, char *name);

/*
** client.c
*/
void		client_init(struct s_ftp *ftp);
int		client_new(struct s_ftp *ftp);
int		client_check_logged(struct s_client *cl, struct s_ftp *ftp);

/*
** serv.c
*/
int		serv_loop(struct s_ftp *ftp);

/*
** serv_client.c
*/
void		serv_do_it(struct s_client *cl, struct s_ftp *ftp);

/*
** reply.c
*/
int		send_reply(int r, struct s_client *cl, struct s_ftp *ftp);

/*
** net.c
*/
int		net_send(char *s, struct s_client *cl, struct s_ftp *ftp);
char		*net_recv(struct s_client *cl, struct s_ftp *ftp);

/*
** ftp_basics.c
*/
int		ftp_cmd_user(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_pass(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_quit(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_help(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_noop(struct s_client *cl, struct s_ftp *ftp, char **s);

/*
** ftp_basics2.c
*/
int		ftp_cmd_pwd(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_dele(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_mkd(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_rmd(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_type(struct s_client *cl, struct s_ftp *ftp, char **s);

/*
** ftp_basics3.c
*/
int		ftp_cmd_syst(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_pasv(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_abor(struct s_client *cl, struct s_ftp *ftp, char **s);

/*
** ftp_cwd.c
*/
int		ftp_cmd_cwd(struct s_client *cl, struct s_ftp *ftp, char **s);
char		*get_absolute_dir(char *s, struct s_client *cl);
int		good_dir(char *s, struct s_client *cl);


/*
** ftp_retr.c
*/
int		ftp_cmd_retr(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_port(struct s_client *cl, struct s_ftp *ftp, char **s);
int		connect_data(struct s_client *cl);

/*
** ftp_stor.c
*/
int		ftp_cmd_stor(struct s_client *cl, struct s_ftp *ftp, char **s);
int		ftp_cmd_appe(struct s_client *cl, struct s_ftp *ftp, char **s);

/*
** ftp_list.c
*/
int		ftp_cmd_list(struct s_client *cl, struct s_ftp *ftp, char **s);

/*
** type.c
*/
void		send_str_image(int sock, char *b, int n);
void		send_str_ascii(int sock, char *b, int n);


#endif /* !FTP_H_ */
