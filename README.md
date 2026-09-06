# 🚀 ROS (Rana Operating System) v4.5 — Bare-Metal Web Server & Advanced Storage Edition

An independent, bare-metal, 32-bit monolithic operating system kernel featuring a preemptive multitasking task scheduler, page-table based virtual memory virtualization, ELF executable loading engine, PATA storage controller, and a fully hand-coded TCP/IP network stack hosting a live bare-metal HTTP web server.

---

## ⚡ The Ultimate Mobile Engineering Landmark
What makes **ROS** fundamentally historic is its absolute zero-hardware-PC development infrastructure: **this entire internet-ready operating system was designed, engineered, hand-coded, compiled, and debugged natively on an Android smartphone.** 

By nesting an isolated mobile Linux subsystem (Termux) and mobile source editors (Acode/Spck), every single architectural protocol layer was engineered via a touch layout interface. This milestone stands as definitive validation that high-density systems engineering requires nothing but sheer algorithmic logic, unbreakable determination, and pure structural computing mastery.

---

## 🏗️ Deep-Dive Monolithic Architecture & Subsystem Mechanics

ROS operates strictly as a **Freestanding Executive Substrate**, completely bypassing corporate standard C hosted libraries and runtime APIs to directly govern raw x86 hardware maps, CPU state registers, and hardware IO ports.

### 1. Unified Storage Control & Virtual Filesystem (RAMFS)
* **PATA/ATA PIO Storage Interface:** Houses a custom hardware driver executing sector-polling IO pipelines directly against Parallel ATA master registers (`0x1F0` - `0x1F7`). Implements Logical Block Addressing (LBA) to safely perform raw sector-level sector reads and writes.
* **Virtual Filesystem Array (VFS):** Implements a highly optimized, dynamic file allocation layout (RAMFS) within the monolithic layer. Manages real-time data streaming, dynamic bounds verification, file mapping descriptors, and localized storage node allocations.

### 2. Virtual Memory Virtualization & The Freestanding Heap Core
* **Page-Table Paging Subsystem:** Configures the processor's hardware control register `cr3` to identity-map underlying memory tables into rigid **4KB Pages**, locking and separating kernel space components away from user application segment overlaps.
* **Dynamic Kernel Allocator (`kmalloc` / `kfree`):** Designed as an independent kernel memory tracker natively guarding block alignments and scanning linked segment memory slots to prevent runtime heap fragmentation panics.

### 3. Preemptive Task Scheduler & Binary ELF Loader
* **Context Switching Multi-Tasking Engine:** Driven directly by localized Programmable Interval Timer (PIT 8253/8254) interrupts clocking at 100Hz. Saves current CPU register contexts directly to active process thread stacks, seamlessly queueing task PID structures.
* **ELF 32-bit Executable Runtime Loader:** Features an internal binary parser node that directly reads compiled executable program flags, evaluates the target machine header magic bytes, and maps program bytes directly onto running registers.

### 4. Hand-Coded TCP/IP Internet Stack Matrix
* **Hardware Interfacing (Intel e1000 Gigabit NIC):** Configured over Memory-Mapped I/O (MMIO) networks across the PCI peripheral bus. Sets up low-latency static ring descriptor slots (`TX/RX rings`) for high-frequency live data packet polling.
* **Network Protocol Pipeline Layering:** Hand-coded from scratch featuring an Ethernet frame analyzer, active **ARP (Address Resolution Protocol)** mapping networks, and IPv4 checksum calculations.
* **Live HTTP Server Stack:** Tracks connection states through customized TCP syncing processes. Intercepts incoming `HTTP GET` packets on open ports to feed raw, structural HTML/CSS presentation buffers directly back to client web browsers.

---

## 💻 Full Subsystem Shell Reference (26 Native Commands Loaded)

The interactive shell acts as a powerful hardware abstraction utility deck, parsing dynamic variable input tokens and executing raw operations across the kernel layers:

### ⚙️ Basic & System Telemetry (6 Commands)
* `help` — Polls the shell context state array and lists all native commands available within the active ROS build.
* `clear` — Wipes the `0x000B8000` text video buffer memory, shifting internal pointers and returning the layout cursor to `(0,0)`.
* `whoami` — Streams a static data sequence confirming the sole sovereign author of the system (**Rana Halder**).
* `sysinfo` — Queries internal kernel tracking constants to display active driver mappings, kernel type, and OS configurations.
* `rosfetch` — Unix-inspired visual system summary tool. Renders a high-density ASCII layout of the ROS logo coupled with live server runtime data.
* `uptime` — Intersects underlying PIT timer tick maps to calculate and print total runtime lifespan in seconds.

### 🎨 Output Controls (2 Commands)
* `echo [string_buffer]` — Parses multi-token user arguments downstream to dump the variable text string buffer back onto the screen.
* `color [hex_mask]` — Modifies the VGA control attribute byte across video memory sections to alter background/foreground text skins on the fly.

### 🧪 System Core Test (5 Commands)
* `sleep` — Puts the execution thread on hard hold for exactly 3 seconds by listening to underlying timer tick interrupts.
* `memtest` — Triggers a test routine validation loop across the dynamic allocator bounds (`kmalloc`/`kfree`) to confirm stability.
* `pagetest` — Reads control register `cr0` and queries the virtual memory module to confirm that paging is active.
* `crash` — Intentionally triggers an unmapped or invalid memory offset access to prove the stable trapping of the Page Fault exception handler.
* `reboot` — Pipes the raw reset pulse byte `0xFE` directly into the keyboard m mcontroller port `0x0064` to enforce an immediate CPU hard hardware reboot.

### 🌐 Bare-Metal Network Infrastructure (3 Commands)
* `lspci` — Iterates across the PCI configuration bus space to trace and scan connected peripheral device IDs and vendors.
* `netinfo` — Pulls data straight from the Intel e1000 controller to showcase MAC allocations, active MMIO bases, and link state registers.
* `ping` — Assembles an ICMP Echo Request data packet, compiles network checksum structures, and pushes raw frames across the wire to the network gateway.

### 📁 Virtual File System - RAMFS (4 Commands)
* `ls` — Iterates through structural file mapping entries to stream active file names and descriptor positions.
* `touch [filename]` — Registers a new file node allocation entry within the VFS RAMFS tracking indexes.
* `write [filename]` — Pipelines coming string keystrokes directly into raw allocated VFS file contents memory cells.
* `cat [filename]` — Loads data buffers sequentially off local storage nodes and dumps raw unbuffered text streams onto display cells.

### 🛠️ Advanced Binaries & Application Space (4 Commands)
* `disktest` — Triggers a raw sector-polling pattern write/read sequence directly on LBA 100 disk segments over PATA registers.
* `sysctest` — Fires a software interrupt gate `int 0x80` to safely validate user-space application string data transfers into active kernel processes.
* `elftest` — Evaluates data block streams, parsing compiled ELF binary file headers, target execution entries, and tags.
* `elfrun [filename]` — Loads binary program segments straight into memory tables and shifts execution instructions pointer registers to the program entry slot.

### 🧵 Task Scheduler (2 Commands)
* `ps` — Scans active PID multitasking queues to display currently registered processes, thread flags, and titles.
* `run [process_name]` — Registers and enqueues a new independent execution thread block into the active context switching tables.

---

## 🛠️ The Mobile Infrastructure Toolchain
The entire cross-compilation matrix was configured on a standalone Android mobile unit using this custom open-source stack:
* **Termux Environment:** Hosted the localized Linux platform environment, compiling binary routines via `clang/gcc` and assembling `nasm` targets.
* **Acode/Spck Editors:** Used for low-level structural code tracking, formatting C pointers, and managing remote code commits.
* **QEMU Hardware Emulator:** Acting as the primary headless/display virtualization layer (`qemu-system-x86_64`) to test raw `os-image.bin` blocks natively on a mobile surface.

---

## 🗺️ Extended System Evolution & Design Roadmap
* **Phase 1:** Complete open-source structuring of the monolithic command-line storage & web-server kernel core (`ROS v4.5 Final Power Edition`). *(Current Landmark)*
* **Phase 2:** Engineering custom filesystem structures (FAT allocation layers) to port persistent data onto local storage blocks.
* **Phase 3:** Transitioning the hardware matrix into VESA BIOS Extensions (VBE) to instantiate low-level **Linear Framebuffer (LFB)** pixel layouts to launch a mouse-driven custom graphcial window manager.

---
**Chief Architect & Lead Developer:** Rana Halder (Systems Engineer & Student)  
**Source Code Distribution License:** GNU General Public License v2.0 (100% Free, Sovereign, and Open-Source for the Entire Civilization)
