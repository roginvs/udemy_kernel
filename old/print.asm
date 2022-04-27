
; This function prints hex value of AL register
; AL register must be in the range 0x00-0x0F
_print_one_char_hex:
  add al, 0x30
  cmp al, 0x3a
  jl .print_char_hex_is_digit
  add al, 7
  .print_char_hex_is_digit:
  mov ah, 0x0E
  int 0x10
  ret

; This function prints AL as hex value
_print_al_hex:
  push bx
  mov bl, al
  shr al, 4
  call _print_one_char_hex
  mov al, bl
  and al, 0x0F
  call _print_one_char_hex
  pop bx
  ret

_print_new_line:
  mov al, 10
  call print_char
  mov al, 13
  call print_char
  ret

print_memory:
  ; Use zero data segment
  push ds
  ;mov ax, 0x7c0
  ;mov ax, 0x0
  mov ax, 0x7c0
  mov ds, ax

  mov si, 0x00
  .loop:
    lodsb
    call _print_al_hex

    mov ax, si
    and ax, 0x07
    cmp ax, 0
    jne .no_newline
      call _print_new_line
    .no_newline:

    cmp si, 0x60
    jl .loop
  
  call _print_new_line

  ; Restore data segment
  pop ds
  ret


