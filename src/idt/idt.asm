section .asm

global enable_interrupts
enable_interrupts:
    sti
    ret
global disable_interrupts
disable_interrupts:
    cli
    ret

extern int21h_handler

global idt_load
idt_load:
    push ebp
    mov ebp, esp

    ; + 0 points to pushed ebp
    ; + 4 points to return address
    mov ebx, [ebp + 8]
    lidt [ebx]


    pop ebp
    ret


global int21h
int21h:
    cli
    pushad ; push all general-purpose registers    
    call int21h_handler
    popad
    sti
    iret

extern no_interrupt_handler
global no_interrupt
no_interrupt:
    cli
    pushad ; push all general-purpose registers    
    call no_interrupt_handler
    popad
    sti
    iret