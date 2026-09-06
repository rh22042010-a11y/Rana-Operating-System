typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);

volatile uint32_t timer_ticks = 0;

// IRQ0 ইন্টারাপ্ট হ্যান্ডলার (প্রতি ট্রিগারে কল হবে)
void timer_handler(void) {
    timer_ticks++;
    // Master PIC-কে EOI সিগন্যাল পাঠানো
    outb(0x20, 0x20);
}

// PIT চিপ চালু করার ফাংশন (100 Hz = প্রতি 10ms এ 1 টিক)
void init_timer(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    // পোর্ট 0x43-এ Mode 3 সেট করা
    outb(0x43, 0x36);

    // পোর্ট 0x40-এ ডিভাইজার বাইট পাঠানো
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

// মিলি-সেকেন্ড ভিত্তিক sleep (100 টিক = ১ সেকেন্ড)
void sleep(uint32_t ticks) {
    uint32_t target_ticks = timer_ticks + ticks;
    while (timer_ticks < target_ticks) {
        __asm__ volatile("hlt");
    }
}

uint32_t get_uptime_seconds(void) {
    return timer_ticks / 100;
}
