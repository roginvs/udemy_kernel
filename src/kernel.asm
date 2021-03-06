[BITS 32]

global _start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov ebp, 0x00200000 
  mov esp, ebp ; Set stack pointer?

  ; Enable the A20 line
  in al, 0x92
  or al, 2
  out 0x92, al

  ; Remap the master PIC Programmable Interrupt Controller
  mov al, 00010001b
  out 0x20, al ; Tell master PIC to initialize

  mov al, 0x20 ; Interrupt where master ISR should start
  out 0x21, al

  mov al, 00000001b
  out 0x21, al
  ; End of remap the master PIC

  call kernel_main
  
  
  jmp $

global kernel_registers
kernel_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret

times 512-($ - $$) db 0

