#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct interrupt_frame;
typedef void *(*ISR80H_COMMAND)(struct interrupt_frame *frame);

struct idt_desc
{
    uint16_t offset_low;  // Offset bits 0-15
    uint16_t selector;    // Selector thats in out GDT
    uint8_t zero;         // Does nothing
    uint8_t type_attr;    // Type and attributes
    uint16_t offset_high; // Offset bits 16-31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit; // Size of descriptor table - 1
    uint32_t base;  // Base address of the interrupt descriptor table
} __attribute__((packed));

struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    /** This is stack pointer at the pushad moment
     * It is not interesting for us because we have stack pointer below
     * which was pushed by CPU on interrupt */
    uint32_t reserved;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

void idt_init();

void enable_interrupts();
void disable_interrupts();
void isr80h_register_command(int command_id, ISR80H_COMMAND command);

#endif