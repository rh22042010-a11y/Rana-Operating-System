[org 0x7c00]

    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov bp, 0x9000
    mov sp, bp

    ; BIOS LBA Read (Extended Read - int 0x13, AH=0x42)
    ; এটি CHS লিমিটেশন ছাড়াই যেকোনো সাইজের কার্নেল লোড করতে পারে
    mov ah, 0x42
    mov si, dap         ; Disk Address Packet (DAP) পয়েন্টার
    int 0x13
    jc disk_error

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:init_pm

disk_error:
    ; রিড ফেইল করলে স্ক্রিনে লাল রঙের 'E' দেখাবে
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $

; -------------------------------------------
; Disk Address Packet (DAP)
; -------------------------------------------
align 4
dap:
    db 0x10             ; Size of DAP (16 bytes)
    db 0                ; Unused
    dw 120              ; Sectors to read (120 sectors = 60 KB)
    dw 0x0000           ; Offset (0x0000)
    dw 0x1000           ; Segment (0x1000) -> 0x10000 মেমোরিতে কার্নেল বসবে
    dq 1                ; Start Sector (LBA 1, অর্থাৎ বুটলোডারের ঠিক পরের সেক্টর)

[bits 32]
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

    jmp 0x10000         ; সরাসরি কার্নেলে জাম্প!

gdt_start:
    dd 0x0, 0x0
gdt_code: 
    dw 0xffff, 0x0, 0x9a00, 0x00cf
gdt_data: 
    dw 0xffff, 0x0, 0x9200, 0x00cf
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xaa55
