/**


gcc -Wall -m32 \
  -s -Wl,--build-id=none \
  sysout.c -o sysout.elf && ./sysout.elf

dumpelf sysout.elf | less
readelf -a sysout.elf
objdump -x sysout.elf
objdump --disassemble-all -M intel sysout.elf | less

*/

#include <unistd.h>

int main()
{
    char str[] = "Hello world!\n";

    write(1, str, sizeof(str));

    return 0;
};


