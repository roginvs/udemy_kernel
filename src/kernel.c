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

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

uint16_t terminal_make_char(char c, char colour)
{
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour)
{
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_writechar(char c, char colour)
{
    if (c == '\n')
    {
        terminal_row += 1;
        terminal_col = 0;
        return;
    }

    terminal_putchar(terminal_col, terminal_row, c, colour);
    terminal_col += 1;
    if (terminal_col >= VGA_WIDTH)
    {
        terminal_col = 0;
        terminal_row += 1;
    }
}
void terminal_initialize()
{
    video_mem = (uint16_t *)(0xB8000);
    terminal_row = 0;
    terminal_col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            terminal_putchar(x, y, ' ', 0);
        }
    }
}

void print(const char *str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
}

uint8_t hex_code(uint8_t c)
{
    c = c + 0x30;
    if (c > 0x39)
        c = c + 0x07;
    return c;
}
void terminal_writehex(uint8_t c, uint8_t color)
{
    terminal_writechar(hex_code((c >> 4) & 0x0F), color);
    terminal_writechar(hex_code(c & 0x0F), color);
}

void terminal_writedword(uint32_t d, uint8_t color)
{
    terminal_writehex((d >> 24) & 0xFF, color);
    terminal_writehex((d >> 16) & 0xFF, color);
    terminal_writehex((d >> 8) & 0xFF, color);
    terminal_writehex((d >> 0) & 0xFF, color);
}

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

    int fd = fopen("0:/hello.txt", "r");
    if (fd)
    {
        print("\nWe opened hello.txt\n");
        char buf[14];
        fseek(fd, 2, SEEK_SET);
        fread(buf, 11, 1, fd);
        buf[13] = 0x00;
        print(buf);
    }
    while (1)
    {
    }
}