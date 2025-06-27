/*
** my_malloc.c for minishell in /u/a1/sterck_j/rendu/sterck_j-minishell/src
** 
** Made by steck
** Login   <sterck_j@epita.fr>
** 
** Started on  Mon Nov 14 18:16:01 2005 steck
** Last update Sun Nov 27 02:02:13 2005 steck
*/
#include <pthread.h>
#include <stdlib.h>

void	*my_malloc(size_t n)
{
  void	*m;

  if (!(m = malloc(n)))
    exit(138);
  return m;
}

int	my_free(void *p)
{
  free(p);
  return 1;
}

void	*my_realloc(void *p, unsigned int n)
{
  void	*r;

  if (!(r = realloc(p, n)))
    exit(138);
  return r;
}
