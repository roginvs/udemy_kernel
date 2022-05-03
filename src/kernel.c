#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/file.h"
#include "./terminal.h"
#include "memory/memory.h"

void echo_keyboard()
{
    while (1)
    {
        while (1)
        {
            char c = insb(0x64);
            if (c & 0b1)
            {
                break;
            }
        }
        // terminal_writechar('A',3);
        char c = insb(0x60);
        // terminal_writechar('=',3);
        terminal_writehex(c, 3);
    }
}

static struct paging_4gb_chunk *kernel_chunk = 0;

void kernel_main()
{
    terminal_initialize();
    print("Hello world!\ntest");

    // Initialize the heap
    kheap_init();

    // Initialize filesystems
    fs_init();

    // Search and initialize the disks
    disk_search_and_init();

    // Initialize the Interrupt Descriptor Table
    idt_init();

    // Setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Switch to kernel paging chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    // Enable paging
    enable_paging();

    enable_interrupts();

    print("Kernel loaded\n");

    int fd = fopen("0:/folderA/folderB/art2.txt", "r");
    // int fd = fopen("0:/hello.txt", "r");

    if (fd)
    {
        char buf[100000];
        fread(buf, 68 * 15, 1, fd);
        // terminal_initialize();
        // memset(buf, 'A', 1000);
        // fread(buf, 68 * 100, 1, fd);
        // print(buf);
        // print("\n");
        // fread(buf, 68 * 20, 1, fd);
        // fread(buf, 68 * 20, 1, fd);
        // fread(buf, 68 * 20, 1, fd);

        memset(buf, 0x0, 100000);
        fread(buf, 68 * 10000000, 1, fd);
        //  fread(buf, 1000, 1, fd);
        print(buf);

        // struct file_stat s;
        // fstat(fd, &s);
        // fclose(fd);

        // print("testing\n");
    }
    print("Going idle");

    while (1)
    {
    }
}