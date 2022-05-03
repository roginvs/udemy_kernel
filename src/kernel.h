#ifndef KERNEL_H
#define KERNEL_H

#define PEACHOS_MAX_PATH 108

#include "./terminal.h"

void kernel_main(void);

#define ERROR(value) (void *)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
#endif