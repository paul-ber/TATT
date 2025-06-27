/*
** reply.c for ftp in /u/a1/sterck_j/rendu/sterck_j-ftpd/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Tue Nov 22 18:30:07 2005 steck
** Last update Sun Nov 27 05:20:24 2005 steck
*/
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include "ftp.h"

static const struct
{
  int	num;
  char	*s;
} replies[] =
{
  { 150, "150 File status okay; about to open data connection." },
  { 200, "200 Command okay." },
  { 214, "214 Help message." },
  { 215, "215 UNIX Type: ??" },
  { 220, "220 Service ready for new user." },
  { 221, "221 Service closing control connection." },
  { 226, "226 Closing data connection." },
  { 230, "230 User logged in, proceed." },
  { 250, "250 Requested file action okay, completed." },
  { 331, "331 User name okay, need password." },
  { 332, "332 Need account for login." },
  { 425, "425 Can't open data connection." },
  { 426, "426 Connection closed; transfer aborted." },
  { 450, "450 Requested file action not taken." },
  { 500, "500 Syntax error, command unrecognized." },
  { 501, "501 Syntax error in parameters or arguments." },
  { 502, "502 Command not implemented." },
  { 503, "503 Bad sequence of commands." },
  { 504, "504 Command not implemented for that parameter." },
  { 530, "530 Not logged in." },
  { 550, "550 Requested action not taken." },
  { 0, NULL }
};

int	send_reply(int r, struct s_client *cl, struct s_ftp *ftp)
{
  int	i;

  for (i = 0; replies[i].s; i++)
    if (replies[i].num == r)
    {
      net_send(replies[i].s, cl, ftp);
      return 1;
    }
  return 1;
}
