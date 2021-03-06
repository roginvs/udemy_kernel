#include "./terminal.h"
#include <stddef.h>
#include "string/string.h"

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

void terminal_goto(int x, int y)
{
    terminal_row = x;
    terminal_col = y;
}

void terminal_backspace()
{
    if (terminal_row == 0 && terminal_col == 0)
    {
        return;
    }

    if (terminal_col == 0)
    {
        terminal_row -= 1;
        terminal_col = VGA_WIDTH;
    }

    terminal_col -= 1;
    terminal_writechar(' ', 15);
    terminal_col -= 1;
}

void terminal_writechar(char c, char colour)
{
    if (c == '\n')
    {
        terminal_row += 1;
        terminal_col = 0;
        return;
    }

    if (c == 0x08)
    {
        terminal_backspace();
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

void print(const char *str)
{
    size_t len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        terminal_writechar(str[i], 15);
    }
}
