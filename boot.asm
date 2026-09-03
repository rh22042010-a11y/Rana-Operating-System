[org 0x7c00]
KERNEL_OFFSET equ 0x1000   ; মেমরির যেখানে কার্নেল লোড হবে

    mov [BOOT_DRIVE], dl       
    mov bp, 0x9000             
    mov sp, bp

    call load_kernel           

    cli                        
    lgdt [gdt_descriptor]      
    mov eax, cr0
    or eax, 0x1                
    mov cr0, eax
    jmp CODE_SEG:init_pm       

[bits 16]
load_kernel:
    mov bx, KERNEL_OFFSET      
    mov al, 50                 ; <-- এখানে ৫ এর বদলে 50 করে দাও!
    mov ch, 0x00               
    mov dh, 0x00               
    mov cl, 0x02               
    mov ah, 0x02               
    int 0x13                   
    jc disk_error              
    ret


disk_error:
    jmp $                      

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

    call KERNEL_OFFSET         
    jmp $                      

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

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xaa55
