#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

void terminal_writedword(uint32_t d, uint8_t color);
void terminal_writechar(char c, char colour);
void terminal_writehex(uint8_t c, uint8_t color);
void terminal_initialize();
void terminal_writechar(char c, char colour);

void print(const char *str);

#endif