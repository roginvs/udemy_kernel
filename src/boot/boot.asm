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
BPB_BytsPerSec          dw 0x200 ; =512, Mostly ignored
BPB_SecPerClus          db 0x4 ; Let it be 4 sectors per cluster. The legal values are 1, 2, 4, 8, 16, 32, 64, and 128.
BPB_ResvdSecCnt         dw 0x200 ; (BPB_RsvdSecCnt) We are storing kernel in this reserved sectors
BPB_NumFATs             db 0x02 ; Two FAT tables total
BPB_RootEntCnt          dw 0x0 ; zero because of FAT32
BPB_TotSec16            dw 0x00 ; BPB_TotSec16 = 0x0 so SectorsBig is used
BPB_Media               db 0xF8 ; not using this
BPB_FATSz16             dw 0x0 ; Zero because of FAT32, use BPB_FATSz32
BPB_SecPerTrk           dw 0x20  ; ? why this
BPB_NumHeads            dw 0x40 ; ? why this

; Count of hidden sectors preceding the partition that contains this
;FAT volume. This field is generally only relevant for media visible
;on interrupt 0x13. This field should always be zero on media that
;are not partitioned. Exactly what value is appropriate is operating
;system specific.
BPB_HiddSec             dd 0x00 ; 

 ; Because of FAT32:
 ;  RootDirSectors=0 because it is FAT32
 ;  DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
 ;  CountofClusters = DataSec / BPB_SecPerClus;                                    
 ;  CountofClusters must be >= 65525

 ; So in our case
 ; CountofClusters = 65525
 ; DataSec = 65525 * 0x4 = 262100 sectors
 ; 
 ; TotSec = DataSec + (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
 ; TotSec = 262100 + (0x200 + 0x02 * 0x200) + 0
BPB_TotSec32             dd 0x405d4  ; This is a total amount of sectors in the disk!

; Extended BPB (Dos 4.0)

; This field is only defined for FAT32 media and does not exist on
; FAT12 and FAT16 media. This field is the FAT32 32-bit count of
; sectors occupied by ONE FAT. BPB_FATSz16 must be 0
;
; We are using 65525 clusters, 4 byte each cluster record, so
; 65525*4 bytes = 512 sectors which is 512 bytes each = 0x200
BPB_FATSz32             dd 0x00000200 ; 

BPB_ExtFlags            dw 0x0000
BPB_FSVer               dw 0x0000
 ; This is set to the cluster number of the 
 ; first cluster of the root directory, usually 2 but not required to be 2. 
BPB_RootClus            dd 0x2 

 ; . Sector number of FSINFO structure in the 
 ; reserved area of the FAT32 volume. Usually 1. 
BPB_FSInfo              dw 0x1  ; TODO: Write fsinfo
BPB_BkBootSec           dw 0x0 ; No copy of boot sector
; 12 bytes of zeros
BPB_Reserved            db 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

BS_DrvNum               db 0x80
BS_Reserved1            db 0
BS_BootSig              db 0x29
BS_VolID                dd 0xD105
BS_VolLab               db 'PEACHOS BOO'
BS_FilSysType           db 'FAT32   '
                

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
  ; For some unknown reasons the next instruction works.
  ; We enabled protected mode so "cs = code selector" register is still zero.
  ; So the next instruction should trigger some error because CS points to zero entry in GDT
  ; Or maybe no, TODO: check CPU documentation
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
  ; Here we can use privileged instruction, example:
  ;  mov ecx, 0xB8000
  ;  INVLPG [ecx]

  mov eax, 10 ; Starting sector to load 
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

; First sector, FSInfo

FSI_LeadSig    dd 0x41615252
times 480 db 0 ; FSI_Reserved1
FSI_StrucSig   dd 0x61417272
FSI_Free_Count dd 0xFFFFFFFF
FSI_Nxt_Free   dd 0xFFFFFFFF
times 12 db 0 ; FSI_Reserved2
FSI_TrailSig   dd 0xAA550000

times 5120-($ - $$) db 0