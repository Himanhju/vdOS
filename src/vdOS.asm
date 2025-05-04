[org 0x7E00]

global int_init
global syscall

int_init:
    push ax
    cli                 ; Disable interrupts
    mov ax, cs          ; Get current code segment
    mov [0x0000 + 4*0x27 + 2], ax     ; Set segment part of int 0x27
    mov word [0x0000 + 4*0x27], syscall ; Set offset
    sti                 ; Enable interrupts
    pop ax

    jmp 0x8000

syscall:
    test dh, dh
    je .poweroff
    cmp dh, 0x01
    je .read_sect
    cmp dh, 0x02
    je .write_sect
    mov dx, 0xFFFF
    iret

.poweroff:
    pusha
    mov ax, 0x5307
    mov bx, 0x0001
    mov cx, 0x0003
    int 0x15
    popa
    iret

.read_sect:
    push es
    push dx
    push bx

    mov ah, 2              ; BIOS: read sectors
    mov al, 1
    ; cl = sector
    mov ch, 0x00           ; cylinder
    mov dh, 0x00           ; head
    mov dl, 0x80           ; first hard disk

    mov bx,0x0500
    mov es, bx
    xor bx, bx

    ;BIOS_BUF = 0x5000

    int 0x13               ; Do the read

    pop bx
    pop dx
    pop es
    iret

.write_sect:
    push es
    push dx
    push bx

    mov ah, 3              ; BIOS: write sectors
    mov al, 1
    ; cl = sector
    mov ch, 0x00           ; cylinder
    mov dh, 0x00           ; head
    mov dl, 0x80           ; first hard disk

    mov bx, 0x0500
    mov es, bx
    xor bx, bx

    ;BIOS_BUF = 0x5000

    int 0x13               ; Do the write

    pop bx
    pop dx
    pop es
    iret

times 512 - ($ - $$) db 0   ; Fill the rest with 0
