#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "disk/disk.h"
#include "string/string.h"
#include "fs/file.h"
#include "./terminal.h"
#include "memory/memory.h"
#include "gdt/gdt.h"
#include "config.h"

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

void panic(const char *msg)
{
    print(msg);
    while (1)
    {
    }
}

struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00}, // NULL Segment
    {.base = 0x00, .limit = 0xffffffff,
     // Kernel code segment
     // 0x9a = 10011010b
     .type = 0x9a},
    {.base = 0x00, .limit = 0xffffffff,
     // Kernel data segment
     // 10010010b
     .type = 0x92}};

void kernel_main()
{
    terminal_initialize();
    print("Hello world!\ntest");

    // Transform our defined gdt_structured into gdt_real structure
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS);

    // Load the gdt
    gdt_load(gdt_real, sizeof(gdt_real));

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