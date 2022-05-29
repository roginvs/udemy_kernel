#include "task.h"
#include "kernel.h"
#include "status.h"
#include "process.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"

// The current task that is running
struct task *current_task = 0;

// Task linked list
struct task *task_tail = 0;
struct task *task_head = 0;

int task_init(struct task *task, struct process *process);

struct task *task_current()
{
    return current_task;
}

struct task *task_new(struct process *process)
{
    int res = 0;
    struct task *task = kzalloc(sizeof(struct task));
    if (!task)
    {
        res = -ENOMEM;
        goto out;
    }

    res = task_init(task, process);
    if (res != PEACHOS_ALL_OK)
    {
        goto out;
    }

    if (task_head == 0)
    {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (ISERR(res))
    {
        task_free(task);
        return ERROR(res);
    }

    return task;
}

struct task *task_get_next()
{
    // TODO: What if current_task is NULL ?
    // At this point it will never happen because it is only called from task_list_remove

    if (!current_task->next)
    {
        return task_head;
    }

    return current_task->next;
}

static void task_list_remove(struct task *task)
{
    if (task->prev)
    {
        task->prev->next = task->next;
    }

    // TODO: Update task->next->prev = task->prev ?

    if (task == task_head)
    {
        task_head = task->next;
    }

    if (task == task_tail)
    {
        task_tail = task->prev;
    }

    if (task == current_task)
    {
        current_task = task_get_next();
    }
}

int task_free(struct task *task)
{
    paging_free_4gb(task->page_directory);
    task_list_remove(task);

    // Finally free the task data
    kfree(task);
    return 0;
}

int task_switch(struct task *task)
{
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

void task_save_state(struct task *task, struct interrupt_frame *frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}
int copy_string_from_task(struct task *task, void *virtual, void *phys, int max)
{
    if (max >= PAGING_PAGE_SIZE)
    {
        return -EINVARG;
    }

    int res = 0;
    char *tmp = kzalloc(max);
    if (!tmp)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t *task_directory = task->page_directory->directory_entry;
    uint32_t old_entry = paging_get(task_directory, tmp);
    paging_map(task->page_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(task->page_directory);
    strncpy(tmp, virtual, max);
    kernel_page();

    res = paging_set(task_directory, tmp, old_entry);
    if (res < 0)
    {
        res = -EIO;
        goto out_free;
    }

    strncpy(phys, tmp, max);

out_free:
    kfree(tmp);
out:
    return res;
}
void task_current_save_state(struct interrupt_frame *frame)
{
    if (!task_current())
    {
        panic("No current task to save\n");
    }

    struct task *task = task_current();
    task_save_state(task, frame);
}

/**
 * Looks like this function is not used
 * */
int task_page()
{
    user_registers();
    // TODO: How can we change "current_task" in the next function if we
    // already changed DS register in previous function?

    task_switch(current_task);
    return 0;
}

int task_page_task(struct task *task)
{
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

void task_run_first_ever_task()
{
    if (!current_task)
    {
        panic("task_run_first_ever_task(): No current task exists!\n");
    }
    task_switch(task_head);
    // TODO: Why this function can access kernel memory
    // if we already switched paging?
    // Maybe we do not have paging enabled in kernel flags?
    // Or maybe "translation lookaside buffer" still holds old value
    task_return(&task_head->registers);
}

int task_init(struct task *task, struct process *process)
{
    memset(task, 0, sizeof(struct task));

    // TODO: Maybe remove PAGING_IS_PRESENT flag?
    // We do not need all entire address space, we will map virtual addresses

    // Map the entire 4GB address space to its self
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT);
    if (!task->page_directory)
    {
        return -EIO;
    }
    // PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL
    // print("MAP STARTED ");

    /*
    for (uint32_t addr = 0xFF000; addr < 0x1900000; addr += 4096)
    {
        paging_map(task->page_directory,
                   (char *)addr,
                   (char *)addr,
                   PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    }
    // */
    /*
    for (uint32_t addr = 0xF0000000; addr < 0xFFFFF000; addr += 4096)
    {
        paging_map(task->page_directory,
                   (char *)addr,
                   (char *)addr,
                   0);
    }
    */

    // print("DONE ");

    // Cannot access memory at address 0x1050fd
    /*
        char *addr;
        addr = get_gdt_address();
        addr = (char *)((uint32_t)addr & 0xFFFFF000); // Align to page
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
        addr += 4096;
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

        addr = get_tss_address();
        addr = (char *)((uint32_t)addr & 0xFFFFF000); // Align to page
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
        addr += 4096;
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

        addr = get_idt_descriptors_addr();
        addr = (char *)((uint32_t)addr & 0xFFFFF000); // Align to page
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
        addr += 4096;
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

        addr = get_idtr_descriptor_addr();
        addr = (char *)((uint32_t)addr & 0xFFFFF000); // Align to page
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
        addr += 4096;
        paging_map(task->page_directory, addr, addr, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    */
    /*
        addr = (char *)&idt_descriptors;
        addr = (char *)((uint32_t)addr & 0xFFFFF000); // Align to page
        paging_map_to(task->page_directory, addr, addr, addr + 4096, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

        addr = (char *)&idtr_descriptor;
        addr = (char *)((uint32_t)addr & 0xFFFFF000); // Align to page
        paging_map_to(task->page_directory, addr, addr, addr + 4096, PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    */
    // Crashes at paging_switch, 0x1050fd
    /*

    eax = 0x1413000

    */

    // As we load binary (not elf) we have mixed code+data

    task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    task->registers.flags = 0x202;

    task->process = process;

    return 0;
}

void *task_get_stack_item(struct task *task, int index)
{
    // This variable is created on kernel stack
    // Which start from 0x600000 as defined in TSS (my guess)
    // So we can write to this variable when we switch pages
    void *result = 0;

    uint32_t *sp_ptr = (uint32_t *)task->registers.esp;

    // Switch to the given tasks page
    task_page_task(task);

    result = (void *)sp_ptr[index];

    // Switch back to the kernel page
    kernel_page();

    return result;
}