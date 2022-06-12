#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"

void do_int13()
{
  char *ptr = (char *)0x00;
  *ptr = 0x50;
}
int main(int argc, char **argv)
{
  for (int i = 0; i < argc; i++)
  {
    printf("%s\n", argv[i]);
  }

  do_int13();

  while (1)
  {
  }
  return 0;
}