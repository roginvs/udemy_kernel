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
  while (1)
  {
    for (int i = 0; i < argc; i++)
    {
      print(argv[i]);
    }
    print("Loop started.");
    infinite_loop();
    print("Oh-la-la!");
    for (int i = 0; i < 10000000; i++)
    {
      //
    }
  }

  return 0;
}