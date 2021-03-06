#include "idt.h"
#include "../config.h"
#include "../memory/memory.h"
#include "../kernel.h"
#include "task/task.h"
#include "io/io.h"
#include "status.h"
#include "task/process.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void *interrupt_pointer_table[PEACHOS_TOTAL_INTERRUPTS];

static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[PEACHOS_TOTAL_INTERRUPTS];

// TODO: Should we initialize this to zero?
static ISR80H_COMMAND isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];

extern void idt_load(struct idtr_desc *ptr);

extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

void no_interrupt_handler()
{
    outb(0x20, 0x20); // Send an acknowledgement to the PIC
}

void idt_zero()
{
    print("Device by zero error");
}

void interrupt_handler(int interrupt, struct interrupt_frame *frame)
{
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0)
    {
        task_current_save_state(frame);
        interrupt_callbacks[interrupt](frame);
        // TODO: Restore task state maybe?
    }

    task_page();

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

void idt_handle_exception()
{
    process_terminate(task_current()->process);
    task_next();
}

void idt_clock()
{
    // We have to do this manually because task_next never returns.
    //  So, "interrupt_handler" can not call its own outb
    outb(0x20, 0x20);

    // Switch to the next task
    task_next();
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }
    // We are not using interrupt_pointer_table for those two interrupts? Why?
    idt_set(0, idt_zero);
    idt_set(0x80, isr80h_wrapper);

    for (int i = 0; i < 0x20; i++)
    {
        idt_register_interrupt_callback(i, idt_handle_exception);
    }

    idt_register_interrupt_callback(0x20, idt_clock);

    idt_load(&idtr_descriptor);
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    if (interrupt < 0 || interrupt >= PEACHOS_TOTAL_INTERRUPTS)
    {
        return -EINVARG;
    }

    interrupt_callbacks[interrupt] = interrupt_callback;
    return 0;
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
    // Reminder: we are on kernel stack selector
    // https://stackoverflow.com/questions/70662563/when-kernel-stacks-esp-is-stored-to-tss-for-interrupt-return-iret
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}