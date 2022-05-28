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
    pushad ; push all general-purpose registers    
    call int21h_handler
    popad
    iret

extern no_interrupt_handler
global no_interrupt
no_interrupt:
    pushad ; push all general-purpose registers    
    call no_interrupt_handler
    popad
    iret


extern isr80h_handler
global isr80h_wrapper

; No need to have "cli" instruction because of flags on interrupt handlers
; idt_descriptor->type_attr, lower 4 bits = "32 bit interrupt gate"
; and gate clears IF flag (original flags are on the stack and restored via iret)

isr80h_wrapper:
    ; TODO: As far as I understand we are still on user program memory,
    ; i.e. paging and stack segment

    ; INTERRUPT FRAME START
    ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
    ; uint32_t ip
    ; uint32_t cs;
    ; uint32_t flags
    ; uint32_t sp;
    ; uint32_t ss;
    ; Pushes the general purpose registers to the stack
    pushad
    
    ; INTERRUPT FRAME END

    ; Push the stack pointer so that we are pointing to the interrupt frame
    push esp

    ; EAX holds our command lets push it to the stack for isr80h_handler
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    add esp, 8

    ; Restore general purpose registers for user land
    popad
    mov eax, [tmp_res]
    iretd

section .data
; Inside here is stored the return result from isr80h_handler
tmp_res: dd 0 