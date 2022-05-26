
add-symbol-file ./build/kernelfull.o 0x100000

set disassembly-flavor intel

# break _start
# break kernel_main
break *0x7C00

# target remote | qemu-system-x86_64 -hda ./bin/os.bin -S -gdb stdio
target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio

c

# layout asm
layout split

# Breakpoint on user program
break *0x400000

stepi