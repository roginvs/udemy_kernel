#ifndef CONFIG_H
#define CONFIG_H

#define PEACHOS_TOTAL_INTERRUPTS 512

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// 100MB heap size
#define PEACH_OS_HEAP_SIZE_BYTES (1024 * 1024 * 100)
#define PEACH_OS_HEAP_BLOCK_SIZE 4096
#define PEACH_OS_HEAP_ADDRESS 0x01000000
#define PEACH_OS_HEAP_TABLE_ADDRESS 0x00007E00

#define PEACHOS_SECTOR_SIZE 512

#define PEACHOS_MAX_FILESYSTEMS 12
#define PEACHOS_MAX_FILE_DESCRIPTORS 512

#define PEACHOS_MAX_PATH 108

#define PEACHOS_TOTAL_GDT_SEGMENTS 6

#define PEACHOS_PROGRAM_VIRTUAL_ADDRESS 0x400000
#define PEACHOS_USER_PROGRAM_STACK_SIZE 1024 * 16
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000
#define PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START - PEACHOS_USER_PROGRAM_STACK_SIZE

#define PEACHOS_MAX_PROGRAM_ALLOCATIONS 1024
#define PEACHOS_MAX_PROCESSES 12

// This value is "segment selector" which is
//    15    3	2	1   0
//    Index--   TI	RPL--
// Basically "gdt offset" + (use_local_descriptor_table_or_global ? 0b100 : 0) +
//   + privilege_level (0-3)
// So, for user segment it is 4th record in GDT, so offset is 4*8 = 32
// Thus we want to run in ring 3 we add 0b11, so we have 32+3 = 35 = 0x23
#define USER_CODE_SEGMENT (3 * 8 + 3)
#define USER_DATA_SEGMENT (4 * 8 + 3)

#define PEACHOS_MAX_ISR80H_COMMANDS 1024

#endif