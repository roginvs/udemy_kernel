#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"

int main(int argc, char **argv)
{
    printf("My age is %i\n", 98);

    char words[] = "hello how are you";

    const char *token = strtok(words, " ");
    while (token)
    {
        printf("%s\n", token);
        token = strtok(NULL, " ");
    }
    return 0;
}