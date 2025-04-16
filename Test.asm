[org 0x7C00]          ; Bootloader starts at 0x7C00

; --------- Bootloader: Load kernel into memory ---------

mov ah, 0x02          ; BIOS function: read sectors
mov al, 10            ; Read 10 sectors
mov ch, 0x00          ; Cylinder 0
mov cl, 0x02          ; Start reading from sector 2
mov dh, 0x00          ; Head 0
mov dl, 0x80          ; Drive (HDD or floppy)
mov bx, 0x7E0         ; Destination address for kernel (0x7E00)
mov es, bx            ; Set ES:BX = 0x7E00:0000
xor bx, bx            ; Clear BX, so ES:BX = 0x7E00:0000 = 0x7E00
int 0x13              ; BIOS interrupt to read sectors

; --------- Setup Stack ---------

cli                   ; Disable interrupts
mov ax, 0x0E00        ; Stack segment at 0xE000
mov ss, ax
mov sp, 0xFFFF        ; Stack pointer at top of segment (0xFFFF)
sti                   ; Enable interrupts

; --------- Setup Segments ---------

;mov ax, 0x7E0
;mov ds, ax

; --------- Jump to kernel ---------

jmp 0x7E00            ; Jump to kernel (address 0x7E00)

; --------- Bootloader padding and signature ---------
times 510 - ($ - $$) db 0   ; Fill the rest of the bootloader with zeroes
dw 0xAA55                   ; Bootloader signature (required by BIOS)