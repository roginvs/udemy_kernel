#include "idt.h"
#include "../config.h"
#include "../memory/memory.h"
#include "../kernel.h"
#include "io/io.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc *ptr);

extern void int21h();
extern void no_interrupt();

void int21h_handler()
{
    print("Keyboard pressed");
    outb(0x20, 0x20); // Send an acknowledgement to the PIC
}
void idt_zero()
{
    print("Device by zero error");
}

void no_interrupt_handler()
{
    outb(0x20, 0x20); // Send an acknowledgement to the PIC
}

void idt_set(int interrupt_no, void *address)
{
    struct idt_desc *idt_descriptor = &idt_descriptors[interrupt_no];

    idt_descriptor->offset_low = (uint16_t)((uint32_t)address & 0x0000FFFF);
    idt_descriptor->selector = KERNEL_CODE_SELECTOR;
    idt_descriptor->zero = 0;
    // lower 1110 is 32 bit interrupt gate
    // higher 1110 is Present - Descriptor Privilege Level - Storage Segment
    idt_descriptor->type_attr = 0b11101110;
    idt_descriptor->offset_high = (uint16_t)(((uint32_t)address & 0xFFFF0000) >> 16);
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, no_interrupt);
    }
    idt_set(0, idt_zero);
    idt_set(0x21, int21h);

    idt_load(&idtr_descriptor);
}

void isr80h_handle_command(int command, struct interrupt_frame *frame)
{
}

void *isr80h_handler(int command, struct interrupt_frame *frame)
{
    void *res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}