FILES=./build/start.asm.o ./build/start.o ./build/peachos.o ./build/peachos.asm.o ./build/stdlib.o ./build/stdio.o  ./build/string.o ./build/memory.o
INCLUDES=-I./src
FLAGS= -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ${FILES}
	i686-elf-ld -m elf_i386 -relocatable ${FILES} -o ./stdlib.elf

./build/start.o: ./src/start.c
	mkdir -p build
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/start.c -o ./build/start.o

./build/start.asm.o: ./src/start.asm
	mkdir -p build
	nasm -f elf ./src/start.asm -o ./build/start.asm.o

./build/peachos.asm.o: ./src/peachos.asm
	mkdir -p build
	nasm -f elf ./src/peachos.asm -o ./build/peachos.asm.o

./build/peachos.o: ./src/peachos.c
	mkdir -p build
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/peachos.c -o ./build/peachos.o

./build/stdlib.o: ./src/stdlib.c
	mkdir -p build
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdlib.c -o ./build/stdlib.o

./build/stdio.o: ./src/stdio.c
	mkdir -p build
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/stdio.c -o ./build/stdio.o

./build/string.o: ./src/string.c
	mkdir -p build
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/string.c -o ./build/string.o

./build/memory.o: ./src/memory.c
	mkdir -p build
	i686-elf-gcc ${INCLUDES} $(FLAGS) -std=gnu99 -c ./src/memory.c -o ./build/memory.o



clean:
	rm -rf ${FILES}
	rm ./stdlib.elf 