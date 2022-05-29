#ifndef KERNEL_H
#define KERNEL_H

#define PEACHOS_MAX_PATH 108

#include "./terminal.h"

void kernel_main();
void print(const char *str);
void panic(const char *msg);

void kernel_page();
void kernel_registers();
char *get_gdt_address();
char *get_tss_address();

#define ERROR(value) (void *)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
#endif