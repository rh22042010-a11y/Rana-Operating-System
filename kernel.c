#define VIDEO_MEMORY (char*) 0xb8000

unsigned char current_color = 0x0f; 
int cursor = 0; 

void clear_screen() {
    char* vidmem = VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25; i++) {
        vidmem[i * 2] = ' ';           
        vidmem[i * 2 + 1] = current_color; 
    }
    cursor = 0; 
}

void print_char(char c) {
    if (cursor >= 80 * 25) clear_screen();
    char* vidmem = VIDEO_MEMORY;
    vidmem[cursor * 2] = c;
    vidmem[cursor * 2 + 1] = current_color; 
    cursor++;
}

void print_string(char* string) {
    int i = 0;
    while (string[i] != '\0') {
        print_char(string[i]); 
        i++;
    }
}

void print_newline() {
    cursor = cursor + 80 - (cursor % 80);
    if (cursor >= 80 * 25) clear_screen();
}

// ---------------------------------------------------------
// নতুন ম্যাজিক ফাংশন: লেখাকে মাঝখানে (Center) আনার জন্য
// ---------------------------------------------------------
void print_centered(char* string) {
    int len = 0;
    while(string[len] != '\0') len++; // স্ট্রিংয়ের সাইজ বের করা
    
    int padding = (80 - len) / 2;     // মাঝখানে আনার জন্য স্পেস ক্যালকুলেশন
    for(int i = 0; i < padding; i++) {
        print_char(' ');              // বামপাশে ফাঁকা জায়গা তৈরি
    }
    print_string(string);
    print_newline();
}

void print_backspace() {
    if (cursor > 0) {
        cursor--;
        char* vidmem = VIDEO_MEMORY;
        vidmem[cursor * 2] = ' ';
        vidmem[cursor * 2 + 1] = current_color; 
    }
}

int strcmp(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

int strncmp(char s1[], char s2[], int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return 1;
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

unsigned char char_to_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

void set_terminal_color(unsigned char color) {
    current_color = color;
    char* vidmem = VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25; i++) {
        vidmem[i * 2 + 1] = current_color; 
    }
}

struct idt_entry {
    unsigned short base_lo; unsigned short sel; unsigned char always0; unsigned char flags; unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit; unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256]; 
struct idt_ptr idtp;

unsigned char port_in(unsigned short port) {
    unsigned char result; __asm__ volatile("inb %1, %0" : "=a" (result) : "d" (port)); return result;
}

void port_out(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a" (data), "d" (port));
}

const char kbd_US[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0 
};

char key_buffer[256];
int buffer_index = 0;

void execute_command(char* input) {
    if (strcmp(input, "help") == 0) {
        print_string("Commands: help, clear, whoami, sysinfo, rosfetch, echo, color, reboot");
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
    } else if (strcmp(input, "whoami") == 0) {
        print_string("You are Rana Halder, Creator of ROS!");
    } else if (strcmp(input, "sysinfo") == 0) {
        print_string("--- System Information ---"); print_newline();
        print_string("OS Name    : ROS (Rana Operating System)"); print_newline();
        print_string("Version    : v3.0 (Centered UX Edition)"); print_newline();
        print_string("Kernel     : Custom Bare-Metal C Kernel"); print_newline();
        print_string("Processor  : x86 Architecture (32-bit Protected Mode)"); print_newline();
        print_string("Developer  : Rana Halder");
        
    } else if (strcmp(input, "rosfetch") == 0) { 
        print_newline();
        print_string("   ___  ___  ___           OS     : ROS v3.0"); print_newline();
        print_string("  | _ \\/ _ \\/ __|          Arch   : x86 32-bit"); print_newline();
        print_string("  |   / (_) \\__ \\          Kernel : Bare-Metal C"); print_newline();
        print_string("  |_|_\\\\___/|___/          Dev    : Rana Halder"); print_newline();
        print_newline();
        
    } else if (strncmp(input, "echo ", 5) == 0) {
        print_string(&input[5]);
        
    } else if (strncmp(input, "color ", 6) == 0) {
        char bg = input[6]; char fg = input[7];
        if (bg != '\0' && fg != '\0') {
            unsigned char new_color = (char_to_hex(bg) << 4) | char_to_hex(fg);
            set_terminal_color(new_color);
        } else {
            print_string("Usage: color [bg][fg] (e.g. 'color 0a')");
        }
        
    } else if (strcmp(input, "reboot") == 0) {
        print_string("Rebooting ROS...");
        port_out(0x64, 0xFE); 
        
    } else if (input[0] != '\0') {
        print_string("Unknown command: ");
        print_string(input);
    }
    print_newline();
    print_string("ROS> "); // নতুন প্রম্পট
}

void keyboard_handler_main() {
    unsigned char scancode = port_in(0x60); 
    if (scancode < 0x80) {
        char c = kbd_US[scancode];
        
        if (c == '\b') {
            if (buffer_index > 0) {
                buffer_index--; key_buffer[buffer_index] = '\0'; print_backspace();
            }
        } else if (c == '\n') { 
            print_newline(); key_buffer[buffer_index] = '\0';
            execute_command(key_buffer);
            buffer_index = 0; key_buffer[0] = '\0';
        } else if (c != 0) { 
            if (buffer_index < 255) {
                key_buffer[buffer_index] = c; buffer_index++; print_char(c);
            }
        }
    }
    port_out(0x20, 0x20);
}

__attribute__((naked)) void keyboard_handler() {
    __asm__ volatile("pushal \n\t" "cld \n\t" "call keyboard_handler_main \n\t" "popal \n\t" "iretl \n\t");
}

void set_idt_gate(int n, unsigned int handler) {
    idt[n].base_lo = handler & 0xFFFF; idt[n].sel = 0x08; idt[n].always0 = 0; idt[n].flags = 0x8E; idt[n].base_hi = (handler >> 16) & 0xFFFF;
}
void idt_init() {
    for (int i = 0; i < 256; i++) { idt[i].base_lo = 0; idt[i].sel = 0; idt[i].always0 = 0; idt[i].flags = 0; idt[i].base_hi = 0; }
    set_idt_gate(33, (unsigned int)keyboard_handler);
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1; idtp.base = (unsigned int) &idt;
    __asm__ volatile("lidt %0" : : "m" (idtp)); 
}
void pic_remap() {
    port_out(0x20, 0x11); port_out(0x21, 0x20); port_out(0x21, 0x04); port_out(0x21, 0x01); 
    port_out(0xA0, 0x11); port_out(0xA1, 0x28); port_out(0xA1, 0x02); port_out(0xA1, 0x01); 
    port_out(0x21, 0xFD); port_out(0xA1, 0xFF); 
}

void main() {
    clear_screen();
    print_newline();
    print_newline();
    
        // স্ক্রিনের ঠিক মাঝখানে বড় লোগো প্রিন্ট হবে (Professional Spaced ASCII)
    print_centered("  _____       ____       _____ ");
    print_centered(" |  __ \\     / __ \\     / ____|");
    print_centered(" | |__) |   | |  | |   | (___  ");
    print_centered(" |  _  /    | |  | |    \\___ \\ ");
    print_centered(" | | \\ \\    | |__| |    ____) |");
    print_centered(" |_|  \\_\\    \\____/    |_____/ ");

    
    print_newline();
    print_centered("Welcome to ROS (Rana Operating System)");
    print_centered("Version 3.0 | Bare-Metal C Kernel | x86 Arch");
    print_centered("Created by Rana Halder");
    print_newline();
    print_newline();
    
    print_string("Type 'help' to see available commands."); 
    print_newline(); 
    print_newline();
    print_string("ROS> ");
    
    idt_init(); 
    pic_remap(); 
    __asm__ volatile("sti"); 
    
    while(1) {}
}
