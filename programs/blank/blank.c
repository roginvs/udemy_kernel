#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"

int main(int argc, char **argv)
{
  struct process_arguments arguments;
  peachos_process_get_arguments(&arguments);

  printf("%i %s\n", arguments.argc, arguments.argv[0]);
  print(argv[0]);
  print("did this work?\n");

  while (1)
  {
  }
  return 0;
}