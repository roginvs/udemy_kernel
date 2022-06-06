[BITS 32]

; TODO What this "function" do?
; Improtant for dynamic linking? What is the difference if no ":function"?
global print:function

; void print(const char* filename)
print:
    push ebp
    mov ebp, esp
    push dword[ebp+8]
    mov eax, 1 ; Command print
    int 0x80
    add esp, 4

    pop ebp
    ret 