FILES=./build/demo.o
INCLUDES=
FLAGS= -m32 -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ${FILES}
	mkdir -p build
	gcc -g -m32 -T ./linker.ld -o ./demo.elf -ffreestanding -O0 -nostdlib -fpic -g ${FILES}

./build/demo.o: ./demo.c
	mkdir -p build
	gcc ${INCLUDES} -I./ $(FLAGS) -std=gnu99 -c ./demo.c -o ./build/demo.o

clean:	
	rm -rf ${FILES}
	rm ./demo.elf 