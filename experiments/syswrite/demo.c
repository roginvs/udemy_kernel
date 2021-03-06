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

int syscall_3(int func_id, size_t arg1, size_t arg2, size_t arg3); /* Prototype */
int syscall_1(int func_id, size_t arg1); /* Prototype */

volatile char str[] = "Hello world from data!\n";


void _start(){
    __asm__(
        "nop\n"
        "nop\n"
    );

    

    syscall_3(4 /* sys_write */, 1 /* stdout */, (size_t)&str, sizeof(str));
    
    volatile char str2[] = "Hello world from stack!\n";
    syscall_3(4 /* sys_write */, 1 /* stdout */, (size_t)&str2, sizeof(str2));
    
    syscall_1(1 /* exit */ , 0 /* exit code */);
    return; // Never returns
}



__asm__( /* Assembly function body */
"syscall_3:\n"
"  push ebx\n" // We have to save EBX accorting to x86 calling convention. EAX, ECX and EDX can be modified
"  mov eax, [esp+8]\n"
"  mov ebx, [esp+12]\n"
"  mov ecx, [esp+16]\n"
"  mov edx, [esp+20]\n"
"  int 0x80\n"
"  pop ebx\n"
"  ret\n"
);

__asm__( /* Assembly function body */
"syscall_1:\n"
"  push ebx\n"
"  mov eax, [esp+8]\n"
"  mov ebx, [esp+12]\n"
"  int 0x80\n"
"  pop ebx\n"
"  ret\n"
);


