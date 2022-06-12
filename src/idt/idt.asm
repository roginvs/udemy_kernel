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


extern no_interrupt_handler
global no_interrupt
no_interrupt:
    pushad ; push all general-purpose registers    
    call no_interrupt_handler
    popad
    iret


extern interrupt_handler
%macro interrupt 1
    ; TODO: Do we need those to be global? No need as for me
    global int%1:function
    int%1:
        nop
        nop
        nop
        ; INTERRUPT FRAME START
        ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
        ; uint32_t ip; This is $esp
        ; uint32_t cs; This is $esp+4  
        ; uint32_t flags; This is $esp+8
        ; uint32_t sp; This is $esp+12
        ; uint32_t ss; This is $esp+16
        ; So CPU does this:
        ;   push ss
        ;   push sp
        ;   push flags
        ;   push cs
        ;   push ip

        ; TODO: Interruptions like GP or PagePault also pushes error code.
        ; This error code must be popped from the stack if we want to continue execution of the user program

        ; Pushes the general purpose registers to the stack
        pushad
        ; Interrupt frame end

        ; Push the stack pointer so that we are pointing to the interrupt frame
        push esp
        push dword %1
        call interrupt_handler
        add esp, 8
        popad
        iret
%endmacro

%assign i 0
; 512=PEACHOS_TOTAL_INTERRUPTS
%rep 512 
    interrupt i
%assign i i+1
%endrep


extern isr80h_handler
global isr80h_wrapper


; No need to have "cli" instruction because of flags on interrupt handlers
; idt_descriptor->type_attr, lower 4 bits = "32 bit interrupt gate"
; and gate clears IF flag (original flags are on the stack and restored via iret)

isr80h_wrapper:
    nop
    nop
    ; We are still on user program memory paging
    ; But CS and SS are changed
    ; Other selectors DS,ES,FS,GS are still the same

    ; Check interrupt_handler comments for registers in stack
    
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

%macro interrupt_array_entry 1
    dd int%1
%endmacro

global interrupt_pointer_table
interrupt_pointer_table:
%assign i 0
; 512=PEACHOS_TOTAL_INTERRUPTS
%rep 512
    interrupt_array_entry i
%assign i i+1
%endrep 