# TO check

- make and Makefile
- ld file and linker
- try to load kernel from disk using real mode
- try page fault

# fat notes

https://github.com/nibblebits/PeachOS/commit/9d38819b6985ea5c154382dbc34d2635925c1983

# Install

apt-get install nasm qemu-system-x86 make bless gdb

# Build

nasm -f bin ./boot.asm -o boot.bin

# Disasm

ndisasm ./bin/boot.bin

# Run

qemu-system-x86_64 -hda boot.bin

# Debug run

gdb
target remote | qemu-system-x86_64 -hda ./boot.bin -S -gdb stdio
layout asm
info registers

# Debug run (new)

cd bin
gdb
add-symbol-file ../build/kernelfull.o 0x100000
break \_start
target remote | qemu-system-x86_64 -hda ./os.bin -S -gdb stdio
c
layout asm
stepi # one step
bt # backtrace
<keep press enter>

break kernel.c:80 # break on this line

break \*0x7C00 # Break on this address

print ptr # print variable name

info register eax # print register

# Debug run

## This is what happens on startup

ss = 0, esp = 0x6f00

## Then we update esp in boot.asm

esp = 0x7c00

## And then in kernel.asm

ss = 0x10, esp = 0x00200000

# Flash drive

## List

fdisk -l

## Backup

dd bs=512 count=1 if="/dev/disk/by-id/usb-Kingston_DataTraveler_3.0_60A44C3FACC9B131799701A0-0:0" of=flash-backup.bin

## Write

dd if=boot.bin of="/dev/disk/by-id/usb-Kingston_DataTraveler_3.0_60A44C3FACC9B131799701A0-0:0"

# Install cross compiler

```sh
apt-get install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev
apt-get install libcloog-isl-dev # Not works?

wget https://www.nic.funet.fi/pub/gnu/ftp.gnu.org/pub/gnu/binutils/binutils-2.35.tar.gz
tar -xvf binutils-2.35.tar.gz

wget https://ftp.mpi-inf.mpg.de/mirrors/gnu/mirror/gcc.gnu.org/pub/gcc/releases/gcc-10.2.0/gcc-10.2.0.tar.gz
tar -xvf gcc-10.2.0.tar.gz


export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir build-binutils
cd build-binutils
../binutils-2.35/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

which -- $TARGET-as || echo $TARGET-as is not in the PATH

mkdir build-gcc
cd build-gcc
../gcc-10.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

$HOME/opt/cross/bin/$TARGET-gcc --version
```

# Links

Registers
https://www.eecg.utoronto.ca/~amza/www.mindsec.com/files/x86regs.html

BIOS Parameter Block
https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29

Exceptions
https://wiki.osdev.org/Exceptions

Interrupt list
http://www.ctyme.com/rbrown.htm

Interrupts in protected mode
https://wiki.osdev.org/Interrupts
https://wiki.osdev.org/IDT

Cross compiler
https://wiki.osdev.org/GCC_Cross-Compiler

Intel manual
https://web.archive.org/web/20090219101735if_/http://download.intel.com:80/design/PentiumII/manuals/24319102.PDF

One Intel instruction
https://c9x.me/x86/html/file_module_x86_id_139.html

ATA
https://wiki.osdev.org/ATA_read/write_sectors
https://wiki.osdev.org/ATA_PIO_Mode

Memory map
https://wiki.osdev.org/Memory_Map_(x86)

=====

## Cherry-pick commits

git remote add nibblebits https://github.com/nibblebits/PeachOS.git
git pull nibblebits

https://github.com/nibblebits/PeachOS/commit/9d38819b6985ea5c154382dbc34d2635925c1983
https://github.com/nibblebits/PeachOS/commit/02d289125b45d59ff00fa12dd3a932fc0786241c

git log --pretty="format:%H %s" nibblebits/master

```
# Already cherry-picked two commits
git cherry-pick 9d38819b6985ea5c154382dbc34d2635925c1983 # Lecture 50, merged, watched
git cherry-pick 02d289125b45d59ff00fa12dd3a932fc0786241c # Lecture 51
# 52
# 53
# 54
# 55
# 56
# 57
# 58
# 59
# 60
```
