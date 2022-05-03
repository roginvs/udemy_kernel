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

    print("Kernel loaded");

    int fd = fopen("0:/folderA/folderB/art1.txt", "r");
    // int fd = fopen("0:/hello.txt", "r");

    if (fd)
    {
        // struct file_stat s;
        // fstat(fd, &s);
        // fclose(fd);

        print("testing\n");
    }
    print("Going idle");

    while (1)
    {
    }
}