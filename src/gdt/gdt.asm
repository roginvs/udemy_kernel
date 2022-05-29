section .asm
global gdt_load

gdt_load:
    mov eax, [esp+4] ; GDT start address
    mov [eax + 2], eax ; Set GDT address into first zero descriptor
    push ebx
    mov bx, [esp+8]
    mov [eax], bx
    pop ebx
    lgdt [eax]
    ret

gdt_load_old:
    mov eax, [esp+4]
    mov [gdt_descriptor + 2], eax
    mov ax, [esp+8]
    mov [gdt_descriptor], ax
    lgdt [gdt_descriptor]
    ret


section .data
gdt_descriptor: ; Not used anymore
    dw 0x00 ; Size
    dd 0x00 ; GDT Start Address 