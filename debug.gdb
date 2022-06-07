
add-symbol-file ./build/kernelfull.o 0x100000
add-symbol-file ./programs/blank/blank.elf
add-symbol-file ./programs/stdlib/build/peachos.o
add-symbol-file ./programs/stdlib/build/start.o
add-symbol-file ./programs/stdlib/build/stdlib.o

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