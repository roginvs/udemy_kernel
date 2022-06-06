[BITS 32]

section .asm

global _start

# Here is an example of privileged instruction
#     mov ecx, 0xB8000
#     INVLPG [ecx]    

    ; call print_message  ; call did not work before because we did not map stack before!

_start:
    nop
    nop
_loop:
    call getkey

    push eax
    mov eax, 3 ; Command putchar
    int 0x80
    add esp, 4
    
    jmp _loop

getkey:
    mov eax, 2 ; Command getkey
    int 0x80
    cmp eax, 0x00
    je getkey
    ret



print_message_directly_to_video_memory:
    mov esi, msg1
    mov ecx, 0xB8000
   ; mov ecx, 0x400020
    .loop:
      lodsb ; Loads the byte from the SI address into the AL register and inrement SI
      cmp al, 0 ; Compare the byte to 0
      je .done
      mov [ecx], al
      add ecx, 2
    jmp .loop
    .done:
    ret
      
section .data
    message: db 'I can talk with the kernel!', 0 
    msg1: db "User program started",0