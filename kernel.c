#define VIDEO_MEMORY (char*) 0xb8000

unsigned char current_color = 0x0f;
int cursor = 0;
volatile unsigned int timer_ticks = 0;
volatile int command_ready = 0;

// ---------------------------------------------------------
// স্ক্রিন কন্ট্রোল ও স্ক্রোলিং ফাংশন
// ---------------------------------------------------------
void clear_screen() {
    char* vidmem = VIDEO_MEMORY;
    for (int i = 0; i < 80 * 25; i++) { vidmem[i*2] = ' '; vidmem[i*2+1] = current_color; }
    cursor = 0;
}
void scroll() {
    char* vidmem = VIDEO_MEMORY;
    for (int i = 0; i < 24*80*2; i++) vidmem[i] = vidmem[i + 80*2];
    for (int i = 24*80*2; i < 25*80*2; i += 2) { vidmem[i] = ' '; vidmem[i+1] = current_color; }
    cursor = 24 * 80;
}
void print_char(char c) {
    if (cursor >= 80*25) scroll();
    char* vidmem = VIDEO_MEMORY; vidmem[cursor*2] = c; vidmem[cursor*2+1] = current_color; cursor++;
}
void print_string(char* string) { int i = 0; while (string[i] != '\0') { print_char(string[i]); i++; } }
void print_newline() { cursor = cursor + 80 - (cursor % 80); if (cursor >= 80*25) scroll(); }
void print_int(unsigned int n) { if (n == 0) { print_char('0'); return; } char buffer[12]; int i = 0; while (n > 0) { buffer[i++] = (n % 10) + '0'; n /= 10; } for (int j = i-1; j >= 0; j--) print_char(buffer[j]); }
void print_hex(unsigned int n) {
    print_string("0x"); if (n == 0) { print_char('0'); return; }
    char hex_chars[] = "0123456789ABCDEF"; char buffer[10]; int idx = 0;
    while (n > 0) { buffer[idx++] = hex_chars[n % 16]; n /= 16; }
    for (int j = idx-1; j >= 0; j--) print_char(buffer[j]);
}
void print_hex_byte(unsigned char byte) {
    char hex_chars[] = "0123456789ABCDEF";
    print_char(hex_chars[(byte >> 4) & 0x0F]); print_char(hex_chars[byte & 0x0F]);
}
void print_centered(char* string) {
    int len = 0; while(string[len] != '\0') len++;
    int padding = (80 - len) / 2; for(int i = 0; i < padding; i++) print_char(' ');
    print_string(string); print_newline();
}
void print_backspace() {
    if (cursor > 0) { cursor--; char* vidmem = VIDEO_MEMORY; vidmem[cursor*2] = ' '; vidmem[cursor*2+1] = current_color; }
}

// ---------------------------------------------------------
// স্ট্রিং ও নেটওয়ার্ক ইউটিলিটি ফাংশন
// ---------------------------------------------------------
int strcmp(char s1[], char s2[]) { int i; for (i = 0; s1[i] == s2[i]; i++) if (s1[i] == '\0') return 0; return s1[i] - s2[i]; }
int strncmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) { if (s1[i] != s2[i]) return (unsigned char)s1[i] - (unsigned char)s2[i]; if (s1[i] == '\0') return 0; } return 0;
}
unsigned char char_to_hex(char c) { if (c >= '0' && c <= '9') return c - '0'; if (c >= 'a' && c <= 'f') return c - 'a' + 10; if (c >= 'A' && c <= 'F') return c - 'A' + 10; return 0; }
void set_terminal_color(unsigned char color) { current_color = color; char* vidmem = VIDEO_MEMORY; for (int i = 0; i < 80*25; i++) vidmem[i*2+1] = current_color; }
void memcpy(void* dest, const void* src, unsigned int len) { char* d = (char*)dest; const char* s = (const char*)src; for(unsigned int i = 0; i < len; i++) d[i] = s[i]; }
void memset(void* dest, int val, unsigned int len) { char* d = (char*)dest; for(unsigned int i = 0; i < len; i++) d[i] = val; }

unsigned short htons(unsigned short v) { return (v >> 8) | (v << 8); }
#define ntohs htons
unsigned int htonl(unsigned int v) { return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v >> 8) & 0xFF00) | ((v >> 24) & 0xFF); }
#define ntohl htonl

unsigned short calculate_checksum(unsigned short *ptr, int bytes) {
    unsigned int sum = 0;
    while (bytes > 1) { sum += *ptr++; bytes -= 2; }
    if (bytes > 0) sum += *(unsigned char*)ptr;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (unsigned short)(~sum);
}

// ---------------------------------------------------------
// পোর্ট এবং হার্ডওয়্যার (IDT & PIC)
// ---------------------------------------------------------
struct idt_entry { unsigned short base_lo; unsigned short sel; unsigned char always0; unsigned char flags; unsigned short base_hi; } __attribute__((packed));
struct idt_ptr { unsigned short limit; unsigned int base; } __attribute__((packed));
struct idt_entry idt[256]; struct idt_ptr idtp;
unsigned char port_in(unsigned short port) { unsigned char result; __asm__ volatile("inb %1, %0" : "=a"(result) : "d"(port)); return result; }
void port_out(unsigned short port, unsigned char data) { __asm__ volatile("outb %0, %1" : : "a"(data), "d"(port)); }
unsigned short port_in16(unsigned short port) { unsigned short result; __asm__ volatile("inw %1, %0" : "=a"(result) : "d"(port)); return result; }
void port_out16(unsigned short port, unsigned short data) { __asm__ volatile("outw %0, %1" : : "a"(data), "d"(port)); }
unsigned int port_in32(unsigned short port) { unsigned int res; __asm__ volatile("inl %1, %0" : "=a"(res) : "d"(port)); return res; }
void port_out32(unsigned short port, unsigned int data) { __asm__ volatile("outl %0, %1" : : "a"(data), "d"(port)); }
void io_wait(void) { __asm__ volatile("outb %%al, $0x80" : : "a"(0)); }

unsigned int pci_config_read(unsigned short bus, unsigned short slot, unsigned short func, unsigned char offset) {
    unsigned int address = (unsigned int)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    port_out32(0xCF8, address); return port_in32(0xCFC);
}
void pci_config_write(unsigned short bus, unsigned short slot, unsigned short func, unsigned char offset, unsigned int val) {
    unsigned int address = (unsigned int)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    port_out32(0xCF8, address); port_out32(0xCFC, val);
}
void scan_pci_bus() {
    print_string("B  S  F   VENDOR     DEVICE     DETAILS\n"); print_string("-  -  -   ------     ------     -------\n");
    for (int bus = 0; bus < 1; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int func = 0; func < 8; func++) {
                unsigned int vendor_device = pci_config_read(bus, slot, func, 0);
                unsigned short vendor = vendor_device & 0xFFFF; unsigned short device = vendor_device >> 16;
                if (vendor != 0xFFFF) {
                    print_int(bus); print_string("  "); print_int(slot); print_string("  "); print_int(func); print_string("   ");
                    print_hex(vendor); print_string("     "); print_hex(device);
                    if (vendor == 0x1AF4) print_string("  [VirtIO]");
                    else if (vendor == 0x8086 && device == 0x100E) print_string("  [Intel e1000 NIC]");
                    else if (vendor == 0x8086) print_string("  [Intel Chipset]");
                    else if (vendor == 0x1234) print_string("  [QEMU VGA]");
                    else print_string("  [Device]");
                    print_newline();
                }
            }
        }
    }
}

// ---------------------------------------------------------
// প্রসেস ও টাস্ক শিডিউলার
// ---------------------------------------------------------
#define MAX_PROCESSES 5
struct process { int id; int is_active; unsigned int esp; unsigned int ebp; char name[32]; };
struct process proc_list[MAX_PROCESSES]; int current_proc_idx = 0; int total_procs = 0;
void init_scheduler() {
    for(int i = 0; i < MAX_PROCESSES; i++) proc_list[i].is_active = 0;
    proc_list[0].id = 0; proc_list[0].is_active = 1;
    char* kernel_name = "System Idle"; int j = 0;
    while (kernel_name[j] != '\0') { proc_list[0].name[j] = kernel_name[j]; j++; }
    proc_list[0].name[j] = '\0'; total_procs = 1;
}
void create_process(char* name) {
    if (total_procs >= MAX_PROCESSES) { print_string("Error: Task limit reached!\n"); return; }
    int idx = total_procs; proc_list[idx].id = idx; proc_list[idx].is_active = 1;
    int j = 0; while(name[j] != '\0' && j < 31) { proc_list[idx].name[j] = name[j]; j++; }
    proc_list[idx].name[j] = '\0'; total_procs++;
    print_string("Process '"); print_string(name); print_string("' created with PID: "); print_int(idx); print_newline();
}

// ---------------------------------------------------------
// Intel e1000 নেটওয়ার্কিং ও HTTP TCP স্ট্যাক
// BUG FIX: সব TX buffer এখন static/global — stack buffer DMA-safe noy!
// ---------------------------------------------------------
unsigned int e1000_mmio_base = 0;
unsigned char mac_addr[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
unsigned char my_ip[4] = {10, 0, 2, 15};
int e1000_initialized = 0;
unsigned int* page_directory_ptr = (void*)0;
unsigned char e1000_page_table_raw[8192];

struct ethernet_header { unsigned char dest_mac[6]; unsigned char src_mac[6]; unsigned short ethertype; } __attribute__((packed));
struct arp_header { unsigned short hw_type; unsigned short proto_type; unsigned char hw_len; unsigned char proto_len; unsigned short opcode; unsigned char sender_mac[6]; unsigned char sender_ip[4]; unsigned char target_mac[6]; unsigned char target_ip[4]; } __attribute__((packed));
struct ipv4_header { unsigned char ihl_version; unsigned char tos; unsigned short length; unsigned short id; unsigned short flags_frag; unsigned char ttl; unsigned char protocol; unsigned short checksum; unsigned char src_ip[4]; unsigned char dest_ip[4]; } __attribute__((packed));
struct icmp_header { unsigned char type; unsigned char code; unsigned short checksum; unsigned short id; unsigned short sequence; } __attribute__((packed));
struct tcp_header { unsigned short src_port; unsigned short dest_port; unsigned int seq_num; unsigned int ack_num; unsigned char data_offset_res; unsigned char flags; unsigned short window_size; unsigned short checksum; unsigned short urgent_ptr; } __attribute__((packed));
struct tcp_pseudo_header { unsigned char src_ip[4]; unsigned char dest_ip[4]; unsigned char reserved; unsigned char protocol; unsigned short tcp_length; } __attribute__((packed));

void mmio_write32(unsigned int offset, unsigned int val) { *(volatile unsigned int*)(e1000_mmio_base + offset) = val; }

#define NUM_TX_DESC 16
#define NUM_RX_DESC 16
struct e1000_tx_desc { unsigned int addr_lo; unsigned int addr_hi; unsigned short length; unsigned char cso; unsigned char cmd; unsigned char status; unsigned char css; unsigned short special; } __attribute__((packed));
struct e1000_rx_desc { unsigned int addr_lo; unsigned int addr_hi; unsigned short length; unsigned short checksum; unsigned char status; unsigned char errors; unsigned short special; } __attribute__((packed));
struct e1000_tx_desc tx_ring[NUM_TX_DESC] __attribute__((aligned(16)));
struct e1000_rx_desc rx_ring[NUM_RX_DESC] __attribute__((aligned(16)));
unsigned char rx_buffers[NUM_RX_DESC][2048];
int tx_tail = 0; int rx_cur = 0;
unsigned int next_seq = 1000;

// ===== CRITICAL FIX: Static global TX buffers =====
// Stack-allocated buffer DMA use korle corrupt hoy!
static unsigned char arp_tx_buf[64];
static unsigned char icmp_tx_buf[2048];
static unsigned char tcp_tx_buf[1500];
static unsigned char tcp_checksum_buf[1500];
// ==================================================

void init_e1000() {
    for (int slot = 0; slot < 32; slot++) {
        unsigned int vendor_device = pci_config_read(0, slot, 0, 0);
        if ((vendor_device & 0xFFFF) == 0x8086 && (vendor_device >> 16) == 0x100E) {
            unsigned int pci_command = pci_config_read(0, slot, 0, 0x04);
            pci_config_write(0, slot, 0, 0x04, pci_command | (1<<0) | (1<<1) | (1<<2));
            e1000_mmio_base = pci_config_read(0, slot, 0, 0x10) & ~0xF;

            if (e1000_mmio_base != 0 && page_directory_ptr != 0) {
                int pd_idx = e1000_mmio_base >> 22;
                unsigned int* e1000_pt = (unsigned int*)(((unsigned int)e1000_page_table_raw + 4095) & ~4095);
                unsigned int base_addr = e1000_mmio_base & 0xFFC00000;
                for(int i = 0; i < 1024; i++) e1000_pt[i] = (base_addr + (i * 4096)) | 3;
                page_directory_ptr[pd_idx] = ((unsigned int)e1000_pt) | 3;
                __asm__ volatile("mov %%cr3, %%eax\n\t mov %%eax, %%cr3" : : : "eax");
            }

            for(int i=0; i<NUM_TX_DESC; i++) { tx_ring[i].addr_lo = 0; tx_ring[i].addr_hi = 0; tx_ring[i].cmd = 0; tx_ring[i].status = 1; }
            mmio_write32(0x3800, (unsigned int)tx_ring); mmio_write32(0x3804, 0);
            mmio_write32(0x3808, sizeof(tx_ring)); mmio_write32(0x3810, 0); mmio_write32(0x3818, 0);
            mmio_write32(0x0400, (1<<1) | (1<<3) | (0x0F<<4));

            for(int i=0; i<NUM_RX_DESC; i++) { rx_ring[i].addr_lo = (unsigned int)rx_buffers[i]; rx_ring[i].addr_hi = 0; rx_ring[i].status = 0; }
            mmio_write32(0x2800, (unsigned int)rx_ring); mmio_write32(0x2804, 0);
            mmio_write32(0x2808, sizeof(rx_ring)); mmio_write32(0x2810, 0); mmio_write32(0x2818, NUM_RX_DESC - 1);
            mmio_write32(0x0100, (1<<1) | (1<<2) | (1<<15));
            e1000_initialized = 1; break;
        }
    }
}

void show_netinfo() {
    if (!e1000_initialized) { print_string("[ERROR] No Intel e1000 Network Card found.\n"); return; }
    print_string("--- Intel e1000 Gigabit NIC ---\nStatus     : TX & RX Rings Active via MMIO\nMMIO Base  : ");
    print_hex(e1000_mmio_base); print_newline();
    print_string("MAC Address: ");
    for (int i = 0; i < 6; i++) { print_hex_byte(mac_addr[i]); if (i < 5) print_char(':'); }
    print_newline();
    print_string("IP Address : 10.0.2.15\nWeb Server : ONLINE (Port 80)\n");
}

// TX send helper — tail bump kore MMIO write kore
void e1000_tx_send(unsigned char* buf, unsigned short len) {
    tx_ring[tx_tail].addr_lo = (unsigned int)buf;
    tx_ring[tx_tail].addr_hi = 0;
    tx_ring[tx_tail].length  = len;
    tx_ring[tx_tail].cmd     = (1<<3) | (1<<1) | (1<<0); // RS | IFCS | EOP
    tx_ring[tx_tail].status  = 0;
    tx_tail = (tx_tail + 1) % NUM_TX_DESC;
    mmio_write32(0x3818, tx_tail);
}

void e1000_poll() {
    while (rx_ring[rx_cur].status & 0x01) {
        unsigned char* buf = rx_buffers[rx_cur];
        struct ethernet_header* eth = (struct ethernet_header*)buf;

        // ---- ARP ----
        if (ntohs(eth->ethertype) == 0x0806) {
            struct arp_header* arp = (struct arp_header*)(buf + sizeof(struct ethernet_header));
            if (ntohs(arp->opcode) == 1 &&
                arp->target_ip[0] == 10 && arp->target_ip[1] == 0 &&
                arp->target_ip[2] == 2  && arp->target_ip[3] == 15) {

                memset(arp_tx_buf, 0, 64);
                struct ethernet_header* rep_eth = (struct ethernet_header*)arp_tx_buf;
                struct arp_header* rep_arp = (struct arp_header*)(arp_tx_buf + sizeof(struct ethernet_header));

                for(int i=0;i<6;i++) { rep_eth->dest_mac[i] = eth->src_mac[i]; rep_eth->src_mac[i] = mac_addr[i]; }
                rep_eth->ethertype = htons(0x0806);
                rep_arp->hw_type = htons(1); rep_arp->proto_type = htons(0x0800);
                rep_arp->hw_len = 6; rep_arp->proto_len = 4; rep_arp->opcode = htons(2);
                for(int i=0;i<6;i++) { rep_arp->sender_mac[i] = mac_addr[i]; rep_arp->target_mac[i] = arp->sender_mac[i]; }
                for(int i=0;i<4;i++) { rep_arp->sender_ip[i] = my_ip[i]; rep_arp->target_ip[i] = arp->sender_ip[i]; }

                e1000_tx_send(arp_tx_buf, 42);
            }
        }
        // ---- IPv4 ----
        else if (ntohs(eth->ethertype) == 0x0800) {
            struct ipv4_header* ip = (struct ipv4_header*)(buf + sizeof(struct ethernet_header));
            if (ip->dest_ip[0]==my_ip[0] && ip->dest_ip[1]==my_ip[1] &&
                ip->dest_ip[2]==my_ip[2] && ip->dest_ip[3]==my_ip[3]) {

                int ip_hlen = (ip->ihl_version & 0x0F) * 4;

                // ---- ICMP Ping ----
                if (ip->protocol == 1) {
                    struct icmp_header* icmp = (struct icmp_header*)((unsigned char*)ip + ip_hlen);
                    if (icmp->type == 8) {
                        int pkt_len = ntohs(ip->length) + sizeof(struct ethernet_header);
                        if (pkt_len > 2000) pkt_len = 2000;
                        memcpy(icmp_tx_buf, buf, pkt_len);

                        struct ethernet_header* rep_eth = (struct ethernet_header*)icmp_tx_buf;
                        struct ipv4_header* rep_ip = (struct ipv4_header*)(icmp_tx_buf + sizeof(struct ethernet_header));
                        struct icmp_header* rep_icmp = (struct icmp_header*)((unsigned char*)rep_ip + ip_hlen);

                        for(int i=0;i<6;i++) { rep_eth->dest_mac[i]=eth->src_mac[i]; rep_eth->src_mac[i]=mac_addr[i]; }
                        for(int i=0;i<4;i++) { rep_ip->dest_ip[i]=ip->src_ip[i]; rep_ip->src_ip[i]=my_ip[i]; }
                        rep_ip->checksum = 0; rep_ip->checksum = calculate_checksum((unsigned short*)rep_ip, ip_hlen);
                        rep_icmp->type = 0; rep_icmp->checksum = 0;
                        rep_icmp->checksum = calculate_checksum((unsigned short*)rep_icmp, ntohs(ip->length) - ip_hlen);

                        e1000_tx_send(icmp_tx_buf, pkt_len);
                    }
                }
                // ---- TCP / HTTP Port 80 ----
                else if (ip->protocol == 6) {
                    struct tcp_header* tcp = (struct tcp_header*)((unsigned char*)ip + ip_hlen);

                    if (ntohs(tcp->dest_port) == 80) {
                        int tcp_hlen = ((tcp->data_offset_res >> 4) & 0x0F) * 4;
                        int payload_len = ntohs(ip->length) - ip_hlen - tcp_hlen;

                        memset(tcp_tx_buf, 0, 1500);
                        struct ethernet_header* rep_eth = (struct ethernet_header*)tcp_tx_buf;
                        struct ipv4_header*  rep_ip  = (struct ipv4_header*)(tcp_tx_buf + sizeof(struct ethernet_header));
                        struct tcp_header*   rep_tcp = (struct tcp_header*)((unsigned char*)rep_ip + sizeof(struct ipv4_header));
                        char* rep_payload = (char*)rep_tcp + sizeof(struct tcp_header);

                        // Ethernet header
                        for(int i=0;i<6;i++) { rep_eth->dest_mac[i]=eth->src_mac[i]; rep_eth->src_mac[i]=mac_addr[i]; }
                        rep_eth->ethertype = htons(0x0800);

                        // IP header (base)
                        rep_ip->ihl_version = 0x45; rep_ip->tos = 0;
                        rep_ip->id = htons(0x4321); rep_ip->flags_frag = htons(0x4000);
                        rep_ip->ttl = 64; rep_ip->protocol = 6; rep_ip->checksum = 0;
                        for(int i=0;i<4;i++) { rep_ip->src_ip[i]=my_ip[i]; rep_ip->dest_ip[i]=ip->src_ip[i]; }

                        // TCP header (base)
                        rep_tcp->src_port  = tcp->dest_port;
                        rep_tcp->dest_port = tcp->src_port;
                        rep_tcp->window_size = htons(8192);
                        rep_tcp->urgent_ptr  = 0;
                        rep_tcp->checksum    = 0;

                        int rep_payload_len = 0;

                        // ---- SYN → SYN-ACK ----
                        if (tcp->flags & 0x02) {
                            rep_tcp->seq_num        = htonl(next_seq);
                            rep_tcp->ack_num        = htonl(ntohl(tcp->seq_num) + 1);
                            rep_tcp->data_offset_res = (5 << 4);
                            rep_tcp->flags          = 0x12; // SYN+ACK
                            next_seq++;
                            rep_payload_len = 0;

                            set_terminal_color(0x0e);
                            print_string("\n[WEB] SYN -> SYN-ACK sent.\n");
                            print_string("ROS> "); set_terminal_color(0x0f);
                        }
                        // ---- FIN → FIN-ACK ----
                        else if (tcp->flags & 0x01) {
                            rep_tcp->seq_num        = htonl(next_seq);
                            rep_tcp->ack_num        = htonl(ntohl(tcp->seq_num) + 1);
                            rep_tcp->data_offset_res = (5 << 4);
                            rep_tcp->flags          = 0x11; // FIN+ACK
                            next_seq++;
                            rep_payload_len = 0;
                        }
                        // ---- PSH/ACK with data → HTTP Response ----
                        else if (payload_len > 0) {
                            // HTTP Response HTML — beautiful RanaOS page
                            char* http_res =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                                "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                                "<title>RanaOS - Bare Metal Web Server</title>"
                                "<style>"
                                "body{margin:0;background:#0a0a0a;color:#00ff88;"
                                "font-family:'Courier New',monospace;text-align:center;"
                                "padding:60px 20px;}"
                                "h1{font-size:2.5em;letter-spacing:4px;color:#00ffcc;"
                                "text-shadow:0 0 20px #00ffcc;margin-bottom:10px;}"
                                "h2{color:#00ff88;font-size:1.2em;font-weight:normal;"
                                "margin-bottom:40px;}"
                                ".box{display:inline-block;border:1px solid #00ff88;"
                                "padding:20px 40px;margin:10px;min-width:200px;"
                                "box-shadow:0 0 10px #00ff8844;}"
                                ".box .label{color:#888;font-size:0.8em;margin-bottom:6px;}"
                                ".box .val{color:#00ffcc;font-size:1.3em;}"
                                ".footer{margin-top:50px;color:#444;font-size:0.85em;}"
                                "</style></head><body>"
                                "<h1>&#9635; RanaOS</h1>"
                                "<h2>Bare-Metal Web Server &mdash; Running on Custom x86 Kernel</h2>"
                                "<div class='box'><div class='label'>KERNEL</div>"
                                "<div class='val'>ROS v4.5</div></div>"
                                "<div class='box'><div class='label'>ARCH</div>"
                                "<div class='val'>x86 32-bit</div></div>"
                                "<div class='box'><div class='label'>NETWORK</div>"
                                "<div class='val'>Intel e1000</div></div>"
                                "<div class='box'><div class='label'>TCP STACK</div>"
                                "<div class='val'>Hand-Coded</div></div>"
                                "<div class='box'><div class='label'>IP ADDRESS</div>"
                                "<div class='val'>10.0.2.15</div></div>"
                                "<div class='footer'>Created by Rana Halder &mdash; "
                                "Zero dependencies. Pure C. Pure Metal.</div>"
                                "</body></html>";

                            int len = 0;
                            while(http_res[len]) { rep_payload[len] = http_res[len]; len++; }
                            rep_payload_len = len;

                            rep_tcp->seq_num        = htonl(next_seq);
                            rep_tcp->ack_num        = htonl(ntohl(tcp->seq_num) + (unsigned int)payload_len);
                            rep_tcp->data_offset_res = (5 << 4);
                            rep_tcp->flags          = 0x19; // FIN+PSH+ACK — response pathanor sathe connection close
                            next_seq += (unsigned int)rep_payload_len + 1;

                            set_terminal_color(0x0a);
                            print_string("\n[WEB] HTTP GET -> 200 OK sent! HTML delivered.\n");
                            print_string("ROS> "); set_terminal_color(0x0f);
                        }
                        // ---- Pure ACK (handshake completion) — skip ----
                        else {
                            goto rx_done;
                        }

                        // IP length & checksum
                        rep_ip->length = htons(sizeof(struct ipv4_header) + sizeof(struct tcp_header) + rep_payload_len);
                        rep_ip->checksum = 0;
                        rep_ip->checksum = calculate_checksum((unsigned short*)rep_ip, sizeof(struct ipv4_header));

                        // TCP checksum via pseudo header
                        memset(tcp_checksum_buf, 0, 1500);
                        struct tcp_pseudo_header* ph = (struct tcp_pseudo_header*)tcp_checksum_buf;
                        for(int i=0;i<4;i++) { ph->src_ip[i]=rep_ip->src_ip[i]; ph->dest_ip[i]=rep_ip->dest_ip[i]; }
                        ph->reserved = 0; ph->protocol = 6;
                        ph->tcp_length = htons(sizeof(struct tcp_header) + rep_payload_len);
                        memcpy(tcp_checksum_buf + sizeof(struct tcp_pseudo_header), rep_tcp,
                               sizeof(struct tcp_header) + rep_payload_len);
                        rep_tcp->checksum = 0;
                        rep_tcp->checksum = calculate_checksum((unsigned short*)tcp_checksum_buf,
                                            sizeof(struct tcp_pseudo_header) + sizeof(struct tcp_header) + rep_payload_len);

                        int total_len = sizeof(struct ethernet_header) + ntohs(rep_ip->length);
                        e1000_tx_send(tcp_tx_buf, total_len);
                    }
                }
            }
        }

        rx_done:
        rx_ring[rx_cur].status = 0;
        mmio_write32(0x2818, rx_cur);
        rx_cur = (rx_cur + 1) % NUM_RX_DESC;
    }
}

void e1000_send_ping() {
    static unsigned char ping_pkt[98];
    memset(ping_pkt, 0, 98);
    struct ethernet_header* eth = (struct ethernet_header*)ping_pkt;
    struct ipv4_header* ip = (struct ipv4_header*)(ping_pkt + sizeof(struct ethernet_header));
    struct icmp_header* icmp = (struct icmp_header*)((unsigned char*)ip + sizeof(struct ipv4_header));

    for(int i=0;i<6;i++) eth->dest_mac[i] = 0xFF; for(int i=0;i<6;i++) eth->src_mac[i] = mac_addr[i];
    eth->ethertype = htons(0x0800);
    ip->ihl_version = 0x45; ip->tos = 0;
    ip->length = htons(sizeof(struct ipv4_header) + sizeof(struct icmp_header) + 32);
    ip->id = htons(1); ip->flags_frag = 0; ip->ttl = 64; ip->protocol = 1; ip->checksum = 0;
    for(int i=0;i<4;i++) ip->src_ip[i] = my_ip[i];
    ip->dest_ip[0]=10; ip->dest_ip[1]=0; ip->dest_ip[2]=2; ip->dest_ip[3]=2;
    ip->checksum = calculate_checksum((unsigned short*)ip, sizeof(struct ipv4_header));
    icmp->type=8; icmp->code=0; icmp->id=htons(1); icmp->sequence=htons(1);
    char* payload = (char*)(ping_pkt + sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct icmp_header));
    char* msg = "RanaOS Ping Test Payload Data...";
    for(int i=0;msg[i]!='\0';i++) payload[i] = msg[i];
    icmp->checksum = 0; icmp->checksum = calculate_checksum((unsigned short*)icmp, sizeof(struct icmp_header) + 32);
    e1000_tx_send(ping_pkt, sizeof(struct ethernet_header) + ntohs(ip->length));
    print_string("PING 10.0.2.2 (Gateway Router) 32 bytes of data...\n");
}

// ---------------------------------------------------------
// টাইমার (PIT) ও Sleep
// ---------------------------------------------------------
void timer_handler_main() {
    timer_ticks++;
    if (total_procs > 1 && timer_ticks % 100 == 0) { current_proc_idx++; if (current_proc_idx >= total_procs) current_proc_idx = 0; }
    port_out(0x20, 0x20);
}
__attribute__((naked)) void timer_handler() { __asm__ volatile("pushal\n\t cld\n\t call timer_handler_main\n\t popal\n\t iretl\n\t"); }
void init_timer(unsigned int frequency) { unsigned int divisor = 1193180 / frequency; port_out(0x43, 0x36); port_out(0x40, (unsigned char)(divisor & 0xFF)); port_out(0x40, (unsigned char)((divisor >> 8) & 0xFF)); }
void sleep(unsigned int ticks) { unsigned int target = timer_ticks + ticks; while (timer_ticks < target) { __asm__ volatile("hlt"); } }

// ---------------------------------------------------------
// ডাইনামিক হিপ মেমোরি অ্যালোকেটর
// ---------------------------------------------------------
#define HEAP_START 0x200000
#define HEAP_SIZE  0x100000
struct block_header { unsigned int size; unsigned char is_free; struct block_header* next; };
struct block_header* heap_head = (struct block_header*)HEAP_START; int heap_initialized = 0;
void init_heap() { heap_head->size = HEAP_SIZE - sizeof(struct block_header); heap_head->is_free = 1; heap_head->next = (void*)0; heap_initialized = 1; }
void* kmalloc(unsigned int size) {
    if (!heap_initialized) init_heap(); if (size % 4 != 0) size = (size + 4) - (size % 4);
    struct block_header* current = heap_head;
    while (current != (void*)0) {
        if (current->is_free && current->size >= size) {
            if (current->size >= size + sizeof(struct block_header) + 4) {
                struct block_header* new_block = (struct block_header*)((unsigned int)current + sizeof(struct block_header) + size);
                new_block->size = current->size - size - sizeof(struct block_header); new_block->is_free = 1; new_block->next = current->next; current->size = size; current->next = new_block;
            }
            current->is_free = 0; return (void*)((unsigned int)current + sizeof(struct block_header));
        } current = current->next;
    } return (void*)0;
}
void kfree(void* ptr) {
    if (ptr == (void*)0) return;
    struct block_header* header = (struct block_header*)((unsigned int)ptr - sizeof(struct block_header)); header->is_free = 1;
    struct block_header* current = heap_head;
    while (current != (void*)0) {
        if (current->is_free && current->next != (void*)0 && current->next->is_free) { current->size = current->size + sizeof(struct block_header) + current->next->size; current->next = current->next->next; } else { current = current->next; }
    }
}

// ---------------------------------------------------------
// হার্ডওয়্যার পেজিং ও ভার্চুয়াল মেমোরি
// ---------------------------------------------------------
unsigned char page_directory_raw[8192]; unsigned char first_page_table_raw[8192];
void init_paging() {
    page_directory_ptr = (unsigned int*)(((unsigned int)page_directory_raw + 4095) & ~4095);
    unsigned int* first_page_table = (unsigned int*)(((unsigned int)first_page_table_raw + 4095) & ~4095);
    for(int i = 0; i < 1024; i++) page_directory_ptr[i] = 0x00000002;
    for(int i = 0; i < 1024; i++) first_page_table[i] = (i * 4096) | 3;
    page_directory_ptr[0] = ((unsigned int)first_page_table) | 3;
    __asm__ volatile("mov %0, %%cr3\n\t mov %%cr0, %%eax\n\t or $0x80000000, %%eax\n\t mov %%eax, %%cr0\n\t" : : "r"(page_directory_ptr) : "eax");
}
void page_fault_handler_main() { set_terminal_color(0x4f); print_newline(); print_centered(" ======================================== "); print_centered(" ! KERNEL PANIC: PAGE FAULT ! "); print_centered(" ======================================== "); print_newline(); print_string(" A program tried to access invalid or unmapped memory.\n System Halted."); while(1) { __asm__ volatile("cli; hlt"); } }
__attribute__((naked)) void page_fault_handler() { __asm__ volatile("call page_fault_handler_main"); }
void hw_irq_main() { port_out(0xA0, 0x20); port_out(0x20, 0x20); }
__attribute__((naked)) void hw_irq() { __asm__ volatile("pushal\n\t cld\n\t call hw_irq_main\n\t popal\n\t iretl\n\t"); }
void default_isr_main() { set_terminal_color(0x4f); print_newline(); print_centered(" ======================================== "); print_centered(" ! KERNEL PANIC: UNHANDLED EXCEPTION ! "); print_centered(" ======================================== "); print_newline(); print_string(" System Halted."); while(1) { __asm__ volatile("cli; hlt"); } }
__attribute__((naked)) void default_isr() { __asm__ volatile("pushal\n\t call default_isr_main\n\t popal\n\t iretl\n\t"); }

struct registers { unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; };
void syscall_handler_main(struct registers regs) { if (regs.eax == 1) print_string((char*)regs.ebx); else if (regs.eax == 2) { print_string("Syscall Uptime: "); print_int(timer_ticks / 100); print_string("s\n"); } else print_string("Unknown Syscall!\n"); }
__attribute__((naked)) void syscall_handler() { __asm__ volatile("pushal\n\t cld\n\t call syscall_handler_main\n\t popal\n\t iretl\n\t"); }

// ---------------------------------------------------------
// ATA PIO ও ভার্চুয়াল ফাইল সিস্টেম (RAMFS)
// ---------------------------------------------------------
void ata_wait() { while(port_in(0x1F7) & 0x80); while(!(port_in(0x1F7) & 0x40)); }
void ata_read_sector(unsigned int lba, unsigned char* buffer) { ata_wait(); port_out(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); port_out(0x1F2, 1); port_out(0x1F3, (unsigned char)lba); port_out(0x1F4, (unsigned char)(lba >> 8)); port_out(0x1F5, (unsigned char)(lba >> 16)); port_out(0x1F7, 0x20); while(!(port_in(0x1F7) & 0x08)); unsigned short* ptr = (unsigned short*)buffer; for(int i=0;i<256;i++) ptr[i] = port_in16(0x1F0); }
void ata_write_sector(unsigned int lba, unsigned char* buffer) { ata_wait(); port_out(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); port_out(0x1F2, 1); port_out(0x1F3, (unsigned char)lba); port_out(0x1F4, (unsigned char)(lba >> 8)); port_out(0x1F5, (unsigned char)(lba >> 16)); port_out(0x1F7, 0x30); while(!(port_in(0x1F7) & 0x08)); unsigned short* ptr = (unsigned short*)buffer; for(int i=0;i<256;i++) port_out16(0x1F0, ptr[i]); port_out(0x1F7, 0xE7); ata_wait(); }

#define MAX_FILES 10
struct vfs_file { char name[32]; char* content; int size; int is_used; };
struct vfs_file ramfs[MAX_FILES];
void init_vfs() { for(int i=0;i<MAX_FILES;i++) ramfs[i].is_used = 0; }
void vfs_create_file(char* filename) {
    if(filename[0]=='\0') { print_string("Error: Filename empty."); return; }
    for(int i=0;i<MAX_FILES;i++) { if(ramfs[i].is_used && strcmp(ramfs[i].name, filename)==0) { print_string("Error: Exists."); return; } }
    for(int i=0;i<MAX_FILES;i++) {
        if(!ramfs[i].is_used) {
            int j=0; while(filename[j]!='\0' && j<31) { ramfs[i].name[j]=filename[j]; j++; } ramfs[i].name[j]='\0';
            ramfs[i].content = (char*)kmalloc(1024); ramfs[i].content[0]='\0'; ramfs[i].size=0; ramfs[i].is_used=1;
            print_string("File '"); print_string(filename); print_string("' created."); return;
        }
    } print_string("Error: FS full!");
}
void vfs_list_files() { int count=0; print_string("--- RAMFS Files ---\n"); for(int i=0;i<MAX_FILES;i++) { if(ramfs[i].is_used) { print_string("- "); print_string(ramfs[i].name); print_newline(); count++; } } if(count==0) print_string("No files."); }
void vfs_write_file(char* filename) {
    for(int i=0;i<MAX_FILES;i++) {
        if(ramfs[i].is_used && strcmp(ramfs[i].name, filename)==0) {
            char* text = "Hello from ROS Virtual File System!"; int j=0;
            while(text[j]!='\0') { ramfs[i].content[j]=text[j]; j++; }
            ramfs[i].content[j]='\0'; ramfs[i].size=j;
            print_string("Data written to '"); print_string(filename); print_string("'."); return;
        }
    } print_string("Error: File not found.");
}
void vfs_read_file(char* filename) { for(int i=0;i<MAX_FILES;i++) { if(ramfs[i].is_used && strcmp(ramfs[i].name, filename)==0) { if(ramfs[i].size==0) print_string("[Empty]"); else print_string(ramfs[i].content); return; } } print_string("Error: Not found."); }

// ---------------------------------------------------------
// ELF 32-bit বাইনারি লোডার
// ---------------------------------------------------------
#define ELF_MAGIC 0x464C457F
typedef struct { unsigned char e_ident[16]; unsigned short e_type; unsigned short e_machine; unsigned int e_version; unsigned int e_entry; unsigned int e_phoff; unsigned int e_shoff; unsigned int e_flags; unsigned short e_ehsize; unsigned short e_phentsize; unsigned short e_phnum; unsigned short e_shentsize; unsigned short e_shnum; unsigned short e_shstrndx; } __attribute__((packed)) Elf32_Ehdr;
typedef struct { unsigned int p_type; unsigned int p_offset; unsigned int p_vaddr; unsigned int p_paddr; unsigned int p_filesz; unsigned int p_memsz; unsigned int p_flags; unsigned int p_align; } __attribute__((packed)) Elf32_Phdr;
int load_and_execute_elf(unsigned char *file_buffer) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*)file_buffer; unsigned int *magic = (unsigned int*)elf_header->e_ident;
    if (*magic != ELF_MAGIC) return 0; if (elf_header->e_type != 2) return 0;
    Elf32_Phdr *program_headers = (Elf32_Phdr*)(file_buffer + elf_header->e_phoff);
    for (int i=0; i<elf_header->e_phnum; i++) {
        Elf32_Phdr *phdr = &program_headers[i];
        if (phdr->p_type == 1) { memcpy((void*)phdr->p_vaddr, (void*)(file_buffer + phdr->p_offset), phdr->p_filesz); if (phdr->p_memsz > phdr->p_filesz) memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz); }
    } void (*entry_point)(void) = (void(*)(void))elf_header->e_entry; entry_point(); return 1;
}

// ---------------------------------------------------------
// কীবোর্ড ও কমান্ড এক্সিকিউশন
// ---------------------------------------------------------
const char kbd_US[128] = { 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/','0','*',0,' ',0 };
char key_buffer[256]; int buffer_index = 0;

void execute_command(char* input) {
    if (strcmp(input, "help") == 0) {
        print_string("Commands: help, clear, whoami, sysinfo, rosfetch, echo, color, sleep, uptime\n");
        print_string("Network : lspci, netinfo, ping\n");
        print_string("Advanced: memtest, pagetest, crash, disktest, sysctest, elftest\n");
        print_string("FS/Tasks: ls, touch <name>, write <name>, cat <name>, elfrun <name>, ps, run");
    }
    else if (strcmp(input, "clear") == 0) clear_screen();
    else if (strcmp(input, "whoami") == 0) print_string("You are Rana Halder, Creator of ROS!");
    else if (strcmp(input, "sysinfo") == 0) print_string("OS Name    : ROS (Rana Operating System)\nKernel     : Bare-Metal C Kernel\nDrivers    : Intel e1000 NIC, ATA, VFS, ELF, PCI");
    else if (strcmp(input, "rosfetch") == 0) print_string("   ___  ___  ___           OS     : ROS v4.5\n  | _ \\/ _ \\/ __|          Arch   : x86 32-bit\n  |   / (_) \\__ \\          Net    : Intel e1000 Ready\n  |_|_\\\\___/|___/          Dev    : Rana Halder\n");
    else if (strncmp(input, "echo ", 5) == 0) print_string(&input[5]);
    else if (strncmp(input, "color ", 6) == 0) set_terminal_color((char_to_hex(input[6]) << 4) | char_to_hex(input[7]));
    else if (strcmp(input, "sleep") == 0) { print_string("Sleeping for 3 seconds..."); sleep(300); print_newline(); print_string("Awake! Hardware Timer working perfectly!"); }
    else if (strcmp(input, "uptime") == 0) { print_int(timer_ticks / 100); print_string(" s."); }
    else if (strcmp(input, "memtest") == 0) { char* ptr = (char*)kmalloc(64); if(ptr) { print_string("kmalloc success. "); kfree(ptr); print_string("kfree success."); } }
    else if (strcmp(input, "pagetest") == 0) { unsigned int cr0; __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); if (cr0 & 0x80000000) print_string("Paging ACTIVE!"); else print_string("Paging INACTIVE!"); }
    else if (strcmp(input, "crash") == 0) { unsigned int data = *(volatile unsigned int*)0x00500000; print_int(data); }
    else if (strcmp(input, "disktest") == 0) {
        unsigned char write_buf[512]; for(int i=0;i<512;i++) write_buf[i]=0; write_buf[0]='H'; write_buf[1]='e'; write_buf[2]='l'; write_buf[3]='l'; write_buf[4]='o';
        ata_write_sector(100, write_buf); print_string("[SUCCESS] Written to Hard Disk Sector 100.\n");
        unsigned char read_buf[512]; ata_read_sector(100, read_buf); print_string("Data read back: "); print_string((char*)read_buf); print_newline();
    }
    else if (strcmp(input, "sysctest") == 0) { char* msg = "[USER APP VIA SYSCALL] Hello Kernel!\n"; __asm__ volatile("mov $1, %%eax\n\t mov %0, %%ebx\n\t int $0x80\n\t" : : "r"(msg) : "eax","ebx"); }
    else if (strcmp(input, "elftest") == 0) print_string("ELF Loader Engine READY.");
    else if (strcmp(input, "lspci") == 0) scan_pci_bus();
    else if (strcmp(input, "netinfo") == 0) show_netinfo();
    else if (strcmp(input, "ping") == 0) { if (!e1000_initialized) print_string("Error: Network card not initialized!"); else e1000_send_ping(); }
    else if (strcmp(input, "ls") == 0) vfs_list_files();
    else if (strncmp(input, "touch ", 6) == 0) vfs_create_file(&input[6]);
    else if (strncmp(input, "write ", 6) == 0) vfs_write_file(&input[6]);
    else if (strncmp(input, "cat ", 4) == 0) vfs_read_file(&input[4]);
    else if (strncmp(input, "elfrun ", 7) == 0) {
        char* filename = &input[7]; int found = 0;
        for(int i=0;i<MAX_FILES;i++) { if(ramfs[i].is_used && strcmp(ramfs[i].name, filename)==0) { if(load_and_execute_elf((unsigned char*)ramfs[i].content)) print_string("ELF executed successfully."); else print_string("Error: Not a valid ELF."); found=1; break; } }
        if(!found) print_string("Error: File not found.");
    }
    else if (strncmp(input, "run ", 4) == 0) create_process(&input[4]);
    else if (strcmp(input, "ps") == 0) { for(int i=0;i<total_procs;i++) { if(proc_list[i].is_active) { print_int(proc_list[i].id); print_string(" - "); print_string(proc_list[i].name); print_newline(); } } }
    else if (strcmp(input, "reboot") == 0) port_out(0x64, 0xFE);
    else if (input[0] != '\0') { print_string("Unknown command: "); print_string(input); }
    print_newline(); print_string("ROS> ");
}

void keyboard_handler_main() {
    unsigned char scancode = port_in(0x60);
    if (scancode < 0x80) {
        char c = kbd_US[scancode];
        if (c == '\b') { if (buffer_index > 0) { buffer_index--; key_buffer[buffer_index]='\0'; print_backspace(); } }
        else if (c == '\n') { print_newline(); key_buffer[buffer_index]='\0'; command_ready = 1; }
        else if (c != 0) { if (buffer_index < 255) { key_buffer[buffer_index]=c; buffer_index++; print_char(c); } }
    }
    port_out(0x20, 0x20);
}
__attribute__((naked)) void keyboard_handler() { __asm__ volatile("pushal\n\t cld\n\t call keyboard_handler_main\n\t popal\n\t iretl\n\t"); }

void set_idt_gate(int n, unsigned int handler) { idt[n].base_lo = handler & 0xFFFF; idt[n].sel = 0x08; idt[n].always0 = 0; idt[n].flags = (n == 0x80) ? 0xEE : 0x8E; idt[n].base_hi = (handler >> 16) & 0xFFFF; }
void idt_init() {
    for(int i=0;i<256;i++) set_idt_gate(i, (unsigned int)default_isr);
    for(int i=32;i<48;i++) set_idt_gate(i, (unsigned int)hw_irq);
    set_idt_gate(14, (unsigned int)page_fault_handler);
    set_idt_gate(32, (unsigned int)timer_handler);
    set_idt_gate(33, (unsigned int)keyboard_handler);
    set_idt_gate(0x80, (unsigned int)syscall_handler);
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1; idtp.base = (unsigned int)&idt;
    __asm__ volatile("lidt %0" : : "m"(idtp));
}
void pic_remap() {
    port_out(0x20, 0x11); io_wait(); port_out(0x21, 0x20); io_wait(); port_out(0x21, 0x04); io_wait(); port_out(0x21, 0x01); io_wait();
    port_out(0xA0, 0x11); io_wait(); port_out(0xA1, 0x28); io_wait(); port_out(0xA1, 0x02); io_wait(); port_out(0xA1, 0x01); io_wait();
    port_out(0x21, 0xFC); io_wait(); port_out(0xA1, 0xFF); io_wait();
}

void main() {
    init_paging(); idt_init();
    clear_screen(); print_newline(); print_newline();
    print_centered("  _____       ____       _____ ");
    print_centered(" |  __ \\     / __ \\     / ____|");
    print_centered(" | |__) |   | |  | |   | (___  ");
    print_centered(" |  _  /    | |  | |    \\___ \\ ");
    print_centered(" | | \\ \\    | |__| |    ____) |");
    print_centered(" |_|  \\_\\    \\____/    |_____/ ");
    print_newline();
    print_centered("Welcome to ROS (Rana Operating System)");
    print_centered("Version 4.5 | HTTP Web Server LIVE | x86 Arch");
    print_centered("Created by Rana Halder");
    print_newline(); print_newline();
    print_string("Server URL: http://localhost:8080\n\nROS> ");

    init_vfs(); init_scheduler(); init_e1000(); pic_remap(); init_timer(100);
    __asm__ volatile("sti");

    // CRITICAL FIX: hlt er age poll kori — nahole network packet miss hoy!
    while(1) {
        if (e1000_initialized) { e1000_poll(); }
        if (command_ready) {
            execute_command(key_buffer);
            buffer_index = 0; key_buffer[0] = '\0'; command_ready = 0;
        }
        // hlt use korchi na — busy-wait e poll thakbe
        // (hlt interrupt enable thakle fire ashbe, kintu poll miss hoy)
        __asm__ volatile("pause");
    }
}
