/*
** 
** infected.c in 
** 
** Author  : <sk at devhell dot org>
** Started : Tue Oct  7 02:05:18 2003
** Updated : Tue Oct  7 02:35:00 2003
**
** $Id$
**
*/
#include <stdio.h>

#if 0
void	hijacked(int arg)
{
  printf("arg %i hijacked\n", arg);
  old_subfunc(arg);
}
#endif

extern char *old_get_macaddr(void);
char *new_get_macaddr(void)
{
  char *s;
  printf("get_macaddr hijacked\n");
  s = old_get_macaddr();
  printf("%s\n", s);
  return s;
}
