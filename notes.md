# TO check

- make and Makefile
- ld file and linker
- try to load kernel from disk using real mode
- try page fault

# TODO

- How CPU determinate what privilege level it runs? Is it CS + GDT? Yes.
- Why interrupt handlers can access user stack memory? User stack is different per task, is it?
- Track down in the debugger what happens when interrupt is called, what values registers have and so on

# TODO

- Proper ELF loader, with setting up executable/readonly bits

# TODO

Cleanup task switching, for example:

- add two kernel heaps, one is below user memory mapped addresses and another is probably above
  (this is to be able to access kernel data from user memory paging)
  (second heap can be for user program data and etc, should be accesses via manual calculation
  from virtual address into physical)
- explicit understanding what paging we have now, no need to double switch pages

# TODO

- Why there is separate handler for interrupt 80

# TODO

Write document with memory ranges, including hardware reserved addresses, kernel base, kernel heap,
kernel stack, and etc

# User memory mapping

No need, we still need kernel memory to be mapped, just because when we change memory table we
still executing in the kernel space.

- Currently user memory lower part is mapped 1-to-1 to real memory, and this is used in
  the interrupt handlers and so on. Can we do kernel without mixing user memory and kernel memory ranges? Having an address from user land we can calculate what address it is in the kernel land
- Memory we need for user program:
  - code segment
  - data segment
  - stack segment
  - GDT page (i suppose we need this because segment selectors points into GDT)
  - maybe interrupts table too?
- Files can be changed:
  - idt.asm
  - idt.c
  - idt.h
  - task.c
  - task.h
  - task.asm
  - paging.c
  - paging.h
  - process.c
- Initial steps to do:
  - Check x86 docs for protected mode (why GDT entry have bits and selector registers have latest 2 bits?)
  - Write kernel memory documentation (what and where is mapped, things like kernel stack and so on)
  - Do a simple user program with memory starting from 0x1000 (just as example)

# fat notes

https://github.com/nibblebits/PeachOS/commit/9d38819b6985ea5c154382dbc34d2635925c1983

# Install

apt-get install nasm qemu-system-x86 make bless gdb

# Build

nasm -f bin ./boot.asm -o boot.bin

# Disasm

ndisasm ./bin/boot.bin

ndisasm -u blank.bin

# Run

qemu-system-x86_64 -hda boot.bin

# Debug run

gdb
target remote | qemu-system-x86_64 -hda ./boot.bin -S -gdb stdio
layout asm
info registers

info registers cs eip ss esp ds es

# Debug run (new)

```
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
print $eax # print register

stepi

layout prev # show code
finish # To exit from current function
next # Step without going into function

x/10 0x400018 # Show memory at this address
x/32b 0x400018 # Show bytes memory at this address
x /20xb $eax # Show memory at eax

info registers ss esp
print *(uint32_t*)($esp + 4)
set $edi = 0xAABBCCDD
x /20xb $esp # This is stack immediately after int instruction, 5 dwords
x /32xb $esp # Same to show addresses where TSS kernel stack begins, 0x600000
x /32xb $esp # pushad decreased stack pointer by 32 bytes = 8 registers
x /52xb $esp
```

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
apt-get install libcunit1-dev libcunit1-dev:i386 valgrind libc6-dbg # For unit tests
apt-get install pax-utils # fordumpelf

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

Intel references
https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

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
git cherry-pick 02d289125b45d59ff00fa12dd3a932fc0786241c # Lecture 51, merged, watched
git cherry-pick 2f76c41c697825ae089836c77cf03d218ba6db65 # Lecture 52, merged, watched
git cherry-pick 3ee00c3b3bd46d22b58398d67cc184843e5927e0 # Lecture 53, merged, half-watched
git cherry-pick bd01ba97e9f635a2adb4cf332234920b65e71b02 # Lecture 54, merged, watched
git cherry-pick 1a889ea20762409f35f493e6e8d01b416d4d2796 # Lecture 55, merged, skipped
git cherry-pick 36d1e75ad918643040839688798bbc99e0812a05 # Lecture 56, merged
git cherry-pick 9084038f0284090d7e4ec88cd1fcdbe29017fceb # Lecture 57, merged
git cherry-pick e722c4468c5a4b699b2be7940d02590a28d76a6f # Lecture 58, merged
git cherry-pick 80ac7483493b0b9862c1ef083d811944370ab1af # Lecture 59, merged
git cherry-pick 75f7e7fcefeb12a47ea4734154e42e4942ecfaff # Lecture 60, merged
git cherry-pick 20c2325acf1e243bc11ec4163e06d0e6444fdc6d # Lecture 61, merged
```

===

Make demo fat32

```
# 263636 is 0x405d4 = total sectors
dd if=/dev/zero of=/tmp/myfat bs=512 count=263636
mkfs.vfat -F 32 -f 2 -R 512 -s 4 -h 0 /tmp/myfat

# This works?
mkfs.vfat -F 32 -f 2 -R 496 -s 4 -h 0 /tmp/myfat
```

==
Fat notes:

512(reserved) * 512 = start of the fat
Fat is 4 bytes each, upper 4 bits are not used. Each value points to index of next cluster or
hold special values (end-of-chain or bad cluster);
For cluster N data is = (N-2)*cluster_size + reserved_sectors*bytes_per_sector + fat_size_sectors * bytes_per_sector \* fat_count;
