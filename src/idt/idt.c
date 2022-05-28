#include "idt.h"
#include "../config.h"
#include "../memory/memory.h"
#include "../kernel.h"
#include "task/task.h"
#include "io/io.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

static ISR80H_COMMAND isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];

extern void idt_load(struct idtr_desc *ptr);

extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

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
    idt_set(0x80, isr80h_wrapper);

    idt_load(&idtr_descriptor);
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= PEACHOS_MAX_ISR80H_COMMANDS)
    {
        panic("The command is out of bounds\n");
    }

    if (isr80h_commands[command_id])
    {
        panic("Your attempting to overwrite an existing command\n");
    }

    isr80h_commands[command_id] = command;
}

void *isr80h_handle_command(int command, struct interrupt_frame *frame)
{
    void *result = 0;

    if (command < 0 || command >= PEACHOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        return 0;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func)
    {
        return 0;
    }

    result = command_func(frame);
    return result;
}

void *isr80h_handler(int command, struct interrupt_frame *frame)
{
    void *res = 0;
    kernel_page();
    // TODO: Is frame pointer on task stack?
    // Probably no, check TSS
    // https://stackoverflow.com/questions/70662563/when-kernel-stacks-esp-is-stored-to-tss-for-interrupt-return-iret
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}