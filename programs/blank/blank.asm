[BITS 32]

section .asm

global _start

_start:
# Here is an example of privileged instruction
#     mov ecx, 0xB8000
#     INVLPG [ecx]    

    ; call print_message  ; call did not work before because we did not map stack before!

label:
    push message
    mov eax, 1 ; Command print
    int 0x80
    add esp, 4
    
    jmp $


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