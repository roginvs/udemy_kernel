#include "peachos.h"
#include "stdlib.h"

int main(int argc, char **argv)
{
    print("Hello how are you!\n");

    void *ptr = malloc(512);
    strcpy(ptr, "hello world");
    print(ptr);

    free(ptr);

    while (1)
    {
        if (getkey() != 0)
        {
            print("key was pressed\n");
        }
    }
    return 0;
}