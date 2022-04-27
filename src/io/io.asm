section .asm

global insb
insb:
    push ebp
    mov ebp, esp

    xor eax, eax ; eax = 0
    mov edx, [ebp + 8]
    in al, dx

    pop ebp
    ret


global insw
insw:
    push ebp
    mov ebp, esp

    xor eax, eax ; eax = 0
    mov edx, [ebp + 8]
    in ax, dx

    pop ebp
    ret

global outb
outb:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 12]
    mov edx, [ebp + 8]
    out dx, al

    pop ebp
    ret

global outw
outw:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 12]
    mov edx, [ebp + 8]
    out dx, ax

    pop ebp
    ret