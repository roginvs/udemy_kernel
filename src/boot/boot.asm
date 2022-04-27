ORG 0x7C00

BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

JMP SHORT start
NOP

; https://wiki.osdev.org/FAT#FAT_16
; http://elm-chan.org/docs/fat_e.html
; FAT16 Header
OEMIdentifier           db 'PEACHOS '
BytesPerSector          dw 0x200 ; Mostly ignored
SectorsPerCluster       db 0x80 ; Well, lets have this value
ReservedSectors         dw 200 ; We are storing kernel in this reserved sectors
FATCopies               db 0x02 ; Two FAT tables total
RootDirEntries          dw 0x40 ; =64 entries in root dir, hope this is enough. 
                                ; Each entry is 32 bytes, so 4 sectors total
NumSectors              dw 0x00 ; BPB_TotSec16 = 0x0 so SectorsBig is used
MediaType               db 0xF8 ; not using this
SectorsPerFat           dw 0x100 ; Number of sectors occupied by a FAT
SectorsPerTrack         dw 0x20  ; ? why this
NumberOfHeads           dw 0x40 ; ? why this
HiddenSectors           dd 0x00 ; ? why this
SectorsBig              dd 0x773594  ; This is a total amount of sectors in the disk!
                                     ; 844 is minumum working value because:
                                     ; ReservedSectors + SectorsPerFat*FATCopies + RootDirEntries*32/512

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'PEACHOS BOO'
SystemIDString          db 'FAT16   '

; Fill BIOS Parameter Block with zeros
; times 33 db 0
;times 0x3c-3 db 0 ; This is much more than needed


; What is no start label?   
start:
  jmp 0:step2 ; Set code segment

step2:
  cli ; Clear interrups
  mov ax, 0x0
  mov ds, ax ; Data segment
  mov es, ax ; Extra segment, not nessesary
  mov ss, ax
  mov sp, 0x7c00 ; Top of stack, just below our code?
  sti ; Enables interrups

.load_protected:
  cli
  lgdt [gdt_descriptor]
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax
  ; https://en.wikipedia.org/wiki/Protected_mode
  ; clear prefetch queue; (using far jump instruction jmp)
  jmp CODE_SEG:load32
  
  


; Global Descriptor Table
gdt_start:
gdt_null:
  dd 0x0
  dd 0x0
; offset 0x8
gdt_code:   ; CS Should point to this
  dw 0xffff ; Segment limit first 0-15 bits
  dw 0x0    ; Base first 0-15 bits
  db 0x0    ; Base 16-23 bits
  db 10011010b ; 0x9a   ; Access byte
  db 11001111b ; High 4 bit flags and low 4 bit limit 16-19
  db 0      ; Base 24-31 bits
; offset 0x10
gdt_data:   ; DS, SS, ES, FS, GS
  dw 0xffff ; Segment limit first 0-15 bits
  dw 0x0    ; Base first 0-15 bits
  db 0x0    ; Base 16-23 bits
  db 10010010b ; 0x92   ; Access byte
  db 11001111b ; High 4 bit flags and low 4 bit limit 16-19
  db 0      ; Base 24-31 bits

gdt_end:

gdt_descriptor:
  dw gdt_end - gdt_start - 1
  dd gdt_start

[BITS 32]
load32:
  mov eax, 1 ; Starting sector to load 
  mov ecx, 100 ; Count
  mov edi, 0x0100000 ; 1 megabyte
  call ata_lba_read
  jmp CODE_SEG:0x0100000


ata_lba_read:
  mov ebx, eax ; backup LBA from EAX
  ; Send the highest 8 bits of LBA to the drive
  shr eax, 24
  or eax, 0xE0 ; Select the master drive
  mov dx, 0x01F6 ; Port
  out dx, al
  
  ; Send the total sectors to read
  mov eax, ecx
  mov dx, 0x01F2 ; Port
  out dx, al

  ; Send more bits of LBA
  mov eax, ebx ; Restore LBA backup
  mov dx, 0x01F3 ; Port
  out dx, al

  ; Send more bits of LBA
  mov eax, ebx ; Restore LBA backup
  shr eax, 8
  mov dx, 0x01F4 ; Port
  out dx, al

  ; Send upper 16 bits of LBA
  mov dx, 0x1F5
  mov eax, ebx
  shr eax, 16
  out dx, al

  mov dx, 0x1F7 ; Command
  mov al, 0x20
  out dx, al

  .next_sector:
    push ecx
  
  ; Checking if we need to read
   .try_again:
      mov dx, 0x1F7 ; Command
      in al, dx
      test al, 8
    jz .try_again
  
    ; We need to read 256 words at time
    mov ecx, 256
    mov dx, 0x1F0 ; Data port
    ; insw is to read from port
    ; rep is to repeat ecx times
    rep insw
    pop ecx
    loop .next_sector ; Decrement ecx and go to next sector if not zero

  ret


; Set last two bytes to signature
times 510-($ - $$) db 0

dw 0xAA55

