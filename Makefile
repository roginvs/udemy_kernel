# By default make runs first label it sees

FILES = ./build/kernel.asm.o ./build/kernel.o ./build/terminal.o ./build/disk/disk.o ./build/disk/streamer.o ./build/fs/pparser.o ./build/fs/file.o ./build/fs/fat/fat16.o ./build/fs/fat/fat32.o ./build/string/string.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o  ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin	

	# We need 128 MB of space for the file system
	# Zeros works ok
	dd if=/dev/zero bs=1048576 count=128 >> ./bin/os.bin 
	gcc add-empty-fat.c -o ./bin/add-empty-fat && ./bin/add-empty-fat ./bin/os.bin
	sudo mkdir -p /mnt/d
	sudo mount -t vfat ./bin/os.bin /mnt/d
	# Copy a file over
	df /mnt/d
	# df --block-size=512 --total --all /mnt/d
	# for i in `seq 1 100`; do sudo touch /mnt/d/Q$$i.txt; done # To add multiple files to test
	sudo cp ./hello.txt /mnt/d
	sudo mkdir -p /mnt/d/folderA/folderB
	sudo cp ./art1.txt /mnt/d/folderA/folderB
	sudo cp ./art2.txt /mnt/d/folderA/folderB
	sudo cp ./art3.txt /mnt/d/folderA/folderB
	ls -la /mnt/d
	sudo umount /mnt/d

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc ${FLAGS} -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	mkdir -p bin
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	mkdir -p build
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
	mkdir -p build
	i686-elf-gcc $(INCLUDES) ${FLAGS} -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

./build/terminal.o: ./src/terminal.c
	mkdir -p build
	i686-elf-gcc $(INCLUDES) ${FLAGS} -std=gnu99 -c ./src/terminal.c -o ./build/terminal.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	mkdir -p ./build/idt
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	mkdir -p ./build/idt
	i686-elf-gcc $(INCLUDES) -I./src/idt ${FLAGS} -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/memory/memory.c
	mkdir -p ./build/memory
	i686-elf-gcc $(INCLUDES) -I./src/memory ${FLAGS} -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o

./build/io/io.asm.o: ./src/io/io.asm
	mkdir -p ./build/io
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	mkdir -p ./build/memory/heap
	i686-elf-gcc $(INCLUDES) -I./src/memory/heap $(FLAGS) -std=gnu99 -c ./src/memory/heap/heap.c -o ./build/memory/heap/heap.o

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	mkdir -p ./build/memory/heap
	i686-elf-gcc $(INCLUDES) -I./src/memory/heap $(FLAGS) -std=gnu99 -c ./src/memory/heap/kheap.c -o ./build/memory/heap/kheap.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	mkdir -p ./build/memory/paging
	i686-elf-gcc $(INCLUDES) -I./src/memory/paging $(FLAGS) -std=gnu99 -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	mkdir -p ./build/memory/paging
	nasm -f elf -g ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/disk/disk.o: ./src/disk/disk.c
	mkdir -p ./build/disk
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/disk/streamer.o: ./src/disk/streamer.c
	mkdir -p ./build/disk
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/streamer.c -o ./build/disk/streamer.o

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
	mkdir -p ./build/fs/fat
	i686-elf-gcc $(INCLUDES) -I./src/fs -I./src/fat $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o

./build/fs/fat/fat32.o: ./src/fs/fat/fat32.c
	mkdir -p ./build/fs/fat
	i686-elf-gcc $(INCLUDES) -I./src/fs -I./src/fat $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat32.c -o ./build/fs/fat/fat32.o

./build/fs/pparser.o: ./src/fs/pparser.c
	mkdir -p ./build/fs
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/fs/file.o: ./src/fs/file.c
	mkdir -p ./build/fs
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/file.c -o ./build/fs/file.o


./build/string/string.o: ./src/string/string.c
	mkdir -p ./build/string
	i686-elf-gcc $(INCLUDES) -I./src/string $(FLAGS) -std=gnu99 -c ./src/string/string.c -o ./build/string/string.o


run: all
	qemu-system-x86_64 -hda ./bin/os.bin

debug: all
	gdb -x debug.gdb

clean:
	# rm -rf ./bin/boot.bin
	# rm -rf ./bin/kernel.bin
	# rm -rf ./bin/os.bin
	# rm -rf ${FILES}
	# rm -rf ./build/kernelfull.o
	rm -rf bin build