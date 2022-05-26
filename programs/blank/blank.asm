[BITS 32]

section .asm

global _start

_start:


print_message:
    mov esi, msg1
    mov ecx, [0xB8000]
    .loop:
      lodsb ; Loads the byte from the SI address into the AL register and inrement SI
      cmp al, 0 ; Compare the byte to 0
      je .done
      mov [ecx], al
      add ecx, 2
    jmp .loop
   .done:
      

label:
    jmp label

msg1 db "User program started",0