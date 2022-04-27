; ORG 0x7C00
ORG 0x0
BITS 16

; Skip BIOS Parameter Block
JMP SHORT start
NOP

; Fill BIOS Parameter Block with zeros
;times 33 db 0
times 0x3c-3 db 0 ; This is much more than needed


; What is no start label?   
start:
  jmp 0x7c0:step2 ; Set code segment


handle_zero:
  mov ah, 0Eh
  mov al, 'A'
  mov bx, 0x00
  int 0x10
  iret

handle_one:
  mov ah, 0Eh
  mov al, 'V'
  mov bx, 0x00
  int 0x10
  iret

step2:
  cli ; Clear interrups
  mov ax, 0x7c0
  mov ds, ax ; Data segment
  mov es, ax ; Extra segment, not nessesary
  mov ax, 0x0
  mov ss, ax
  mov sp, 0x7c00 ; Top of stack, just below our code?
  sti ; Enables interrups

  
  call print_memory

  ; Move address of interruption into int table
  mov word[ss:0x00], handle_zero
  mov word[ss:0x02], 0x7c0
  mov word[ss:0x04], handle_one
  mov word[ss:0x06], 0x7c0

  ;int 1
  ; Cause divide by zero exception which is int 0
  ;mov ax, 0x00
  ;div ax


  ;mov si, message
  ;call print

  ; Print how many disks we have
  ; https://stanislavs.org/helppc/bios_data_area.html
  mov al, [ss:0x475]
  call _print_al_hex
  call _print_new_line

      
  ; Reading from the disk
  mov ah, 0x2 ; READ SECTOR Command
  mov al, 0x1 ; How many sectors to read
  mov ch, 0 ; Cylinder number is zero
  mov cl, 2 ; Read sector two (lol, starts from 1)
  mov dh, 0 ; Head is zero
  ; DL is already set by BIOS
  mov bx, 1024 ; Address to place
  int 0x13

  jc error
  
  mov si, 1024
  call print
  jmp $ ; $ is the same line

error:
  mov al, ah ; Status
  call _print_al_hex
  call _print_new_line  

  mov si, error_message
  call print
  jmp $ ; $ is the same line

  
  


print: 
  mov bx, 0 ; Page number, what it is?
.loop:
  lodsb ; Loads the byte from the SI address into the AL register and inrement SI
  cmp al, 0 ; Compare the byte to 0
  je .done
  call print_char
  jmp .loop
.done:
  ret

; This function expect character to be in AL register
print_char:
  mov ah, 0eh ; or 0x0E
  int 0x10
  ret

message: db 'Hello, world!', 10, 13, 0 ; 10 is the new line

error_message: db 'Something failed', 10, 13, 0

%include "print.asm"


; Set last two bytes to signature
times 510-($ - $$) db 0

dw 0xAA55

; Just add more data to the disk image
; times 512*3 db 0