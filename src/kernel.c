#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "disk/disk.h"
#include "keyboard/keyboard.h"
#include "string/string.h"
#include "isr80h/isr80h.h"
#include "task/task.h"
#include "task/process.h"
#include "fs/file.h"
#include "./terminal.h"
#include "task/tss.h"
#include "memory/memory.h"
#include "gdt/gdt.h"
#include "config.h"
#include "status.h"

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

struct tss tss;

// TODO: Do we have real address offsets in GDT or are they virtual?
// Some forums says that it is virtual with no proof
// Need to check cpu documentation to find this out

struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},       // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a}, // Kernel code segment 0x9a = 10011010b
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92}, // Kernel data segment 0x92 = 10010010b
    // TODO: According to my expriments flags here must be the same as in CS/DS registers
    // If there is a mismatch then interrupt happens
    // Search by "Conforming bit"
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},           // User code segment   0xF8 = 11111000b
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},           // User data segment   0xF2 = 11110010b
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} // TSS Segment         0xE9 = 11101001b
};

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_chunk);
}

void print_art()
{

    int artFd = fopen("0:/art.txt", "r");

    if (artFd)
    {
        struct file_stat sb;
        fstat(artFd, &sb);

        char *buf = kzalloc(sb.filesize + 1);

        memset(buf, 0x0, sb.filesize + 1);
        fread(buf, sb.filesize, 1, artFd);

        terminal_initialize();
        print(buf);
        kfree(buf);
    }
}

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

    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // Load the TSS, 0x28 = 40 = 5 (index of tss segment in GDT) * 8 (size of each GDT entry)
    tss_load(0x28);

    // Setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // Switch to kernel paging chunk
    paging_switch(kernel_chunk);

    // Enable paging
    enable_paging();

    // Register the kernel commands
    isr80h_register_commands();

    // TODO: Enable me when first program loads
    // enable_interrupts();

    // Initialize all the system keyboards
    keyboard_init();

    print("Kernel loaded\n");
    print_art();
    terminal_goto(0, 0);

    struct process *process = 0;
    int res = process_load_switch("0:/blank.bin", &process);
    if (res != PEACHOS_ALL_OK)
        panic("Failed to load blank.bin\n");

    task_run_first_ever_task();

    while (1)
    {
    }
}