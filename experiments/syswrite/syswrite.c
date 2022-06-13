/**




gcc -ffreestanding -c -fno-pie -m32 syswrite.c -o syswrite.o  && \  
ld syswrite.o -o syswrite.bin -m elf_i386 -Ttext 0x10000 --oformat binary

ldd syswrite.elf

dumpelf syswrite.elf | less
readelf -a syswrite.elf
objdump -x syswrite.elf
objdump --disassemble-all -M intel syswrite.elf | less

*/

#include <unistd.h>
//#include <sys/syscall.h>
#include <stddef.h>

int syscall_3(int func_id, size_t arg1, size_t arg2, size_t arg3); /* Prototype */

__asm__( /* Assembly function body */
"syscall_3:\n"
"  mov $100,%eax\n"
"  ret\n"
);


int main()
{
    __asm__(
        "nop\n"
        "nop\n"
    );

    volatile char str[] = "Hello world!\n";

    syscall_3(1 /* sys_write */, 1 /* stdout */, (size_t)str, sizeof(str));
    
    return 0;
};

void _start(){
    main();
}