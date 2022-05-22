[BITS 32]
section .asm

global restore_general_purpose_registers
global task_return
global user_registers

; void task_return(struct registers* regs);
task_return:
    mov ebp, esp
    ; PUSH THE DATA SEGMENT (SS WILL BE FINE)
    ; PUSH THE STACK ADDRESS
    ; PUSH THE FLAGS
    ; PUSH THE CODE SEGMENT
    ; PUSH IP

    ; Let's access the structure passed to us
    mov ebx, [ebp+4]
    ; push the data/stack selector
    push dword [ebx+44]
    ; Push the stack pointer
    push dword [ebx+40]

    ; TODO: Why we push current flags instead of "push dword [ebx+40-4]" ?
    ; Push the flags
    pushf
    pop eax
    or eax, 0x200
    push eax

    ; Push the code segment
    push dword [ebx+32]

    ; Push the IP to execute
    push dword [ebx+28]

    ; Setup some segment registers
    ; Note: no need to change stack segment register because iret instruction do this for us
    ;  and we already pushed stack segment above
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    

    ; TODO: As I understand we should push address of this structure, so we have 
    ; to "push ebx"? Or "push dword [ebp+4]"
    push dword [ebx+4]
    call restore_general_purpose_registers
    add esp, 4

    ; Let's leave kernel land and execute in user land!
    iretd
    
; void restore_general_purpose_registers(struct registers* regs);
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]
    pop ebp
    ret

; void user_registers()
user_registers:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret 