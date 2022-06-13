/**



rm *.o *.bin *.elf ;
gcc -ffreestanding -c -fno-pie -m32 -fno-if-conversion -fno-if-conversion2 -fno-tree-loop-if-convert syswrite.c -o syswrite.o  && \
ld syswrite.o -o syswrite.elf -m elf_i386 -Ttext=0x10000 -Tbss=0x30000 -Tdata=0x40000 --oformat elf32-i386 && \
./syswrite.elf

ldd syswrite.elf

dumpelf syswrite.elf | less
readelf -a syswrite.elf
objdump -x syswrite.elf
objdump --disassemble-all -M intel syswrite.elf | less

*/

#include <unistd.h>
//#include <sys/syscall.h>
#include <stddef.h>

int main();
void _start(){
       __asm__(
        "nop\n"
        "nop\n"
    );
       __asm__(
        "nop\n"
        "nop\n"
    );
    main();
}

int syscall_3(int func_id, size_t arg1, size_t arg2, size_t arg3); /* Prototype */

__asm__( /* Assembly function body */
"syscall_3:\n"
//"  mov $100,%eax\n"

"  mov %eax, 1\n"
"  mov %ebx, 33\n"
"  int $0x80\n"
"  ret\n"
);



int main()
{
    __asm__(
        "nop\n"
        "nop\n"
    );

    //volatile char str[] = "Hello world!\n";

    //syscall_3(1 /* sys_write */, 1 /* stdout */, (size_t)str, sizeof(str));
    syscall_3(1 /* sys_write */, 1 /* stdout */, (size_t)0, sizeof(0));
    
    return 0;
};

