# 🚀 ROS — Rana Operating System

### Next-Generation Bare-Metal x86 Monolithic Kernel & Hardware-Abstracted Execution Environment

<p align="center">
  <img src="https://shields.io" alt="Architecture">
  <img src="https://shields.io" alt="Kernel">
  <img src="https://shields.io" alt="Firmware">
  <img src="https://shields.io" alt="Toolchain">
  <img src="https://shields.io" alt="Host">
  <img src="https://shields.io" alt="License">
</p>

<p align="center">
  <b>A zero-dependency bare-metal x86 operating system engineered, cross-compiled, and emulated entirely within a resource-constrained Android mobile terminal via Termux.</b>
</p>

---

## 📖 Overview

**ROS (Rana Operating System)** is an independent, 32-bit monolithic operating system kernel and interactive command-line environment built from absolute scratch. 

Guided by a strict **Zero-Abstraction Doctrine**, the platform completely bypasses high-level firmware interfaces or hosted OS services. Instead, it directly implements the fundamental mechanisms required to transition an x86 processor from BIOS Real Mode into 32-bit Flat Protected Mode, operating raw hardware straight from a freestanding kernel space.

### 🌟 Key Highlights
* **Pure Systems Engineering:** Written in freestanding C and x86 Assembly with zero dependencies on `libc`, POSIX, or standard headers.
* **Mobile-Native Development:** The entire lifecycle—source editing, assembly, compilation, linking, and emulation—is executed on a smartphone via Termux.
* **Direct Hardware Control:** Native implementation of low-level drivers, custom interrupt routing, and physical memory mapping.

---

## 🏛️ System Architecture

ROS follows a layered monolithic architecture where the kernel directly interfaces with the underlying x86 hardware layout.

```text
+-------------------------------------------------------------------------+

|                    ROS Interactive Shell & Command Dispatcher           |
+-------------------------------------------------------------------------+

|  help  |  clear  |  whoami  |  sysinfo  |  rosfetch  |  echo  | color  |
|                              reboot                                    |
+-------------------------------------------------------------------------+

|                         Terminal Engine (Scrolling & Attributes)        |
+-------------------------------------------------------------------------+

|                     Freestanding C Runtime Primitives (string/memory)   |
+-------------------------------------+-----------------------------------+

|          Hardware Drivers           |      Interrupt Management Core     |
|   VGA Framebuffer | PS/2 Keyboard   |    256-Entry IDT | Dual PIC       |
+-------------------------------------+-----------------------------------+

|                    32-Bit x86 Flat Protected Mode (Ring 0 GDT)          |
+-------------------------------------------------------------------------+

|             16-Bit Real-Mode MBR Bootstrap & Disk Loader (0x7C00)       |
+-------------------------------------------------------------------------+
```

### 🔬 Core Subsystems

* **Stage-1 Bootstrap:** A legacy 16-bit MBR loader initialized by the BIOS at `0x7C00`. It uses BIOS disk services (`INT 0x13`, `AH = 0x02`) to load 50 sectors of kernel payload into memory segment `0x1000`.
* **Protected Mode Entry:** Constructs a custom Global Descriptor Table (GDT) for a flat 4 GB memory model. It updates the `PE` bit in the `CR0` register and executes a far jump to establish Ring 0 kernel privileges.
* **VGA Text Engine:** Directly maps memory to the video framebuffer at `0x000B8000` on an 80×25 grid. Each cell uses a 2-byte structure packing the ASCII character and color attributes. Features automatic scrolling and boundary tracking.
* **Interrupt Subsystem:** Remaps the legacy Dual 8259 PIC architecture (Master base `0x20`, Slave base `0x28`) to prevent exception collisions, routing hardware events through a 256-entry Interrupt Descriptor Table (IDT).
* **PS/2 Keyboard Driver:** Listens natively on data I/O ports `0x60` and `0x64` to intercept hardware IRQ 1 interrupts, translating raw scan codes into ASCII characters for shell execution.

---

## 💻 Interactive Kernel Shell

ROS features a responsive, built-in command interpreter executing inside the Ring 0 kernel environment.

| Command | Syntax | Description |
| :--- | :--- | :--- |
| **help** | `help` | Iterates and lists all currently active kernel shell commands. |
| **clear** | `clear` | Purges the VGA text buffer and resets the cursor to the top-left corner `(0,0)`. |
| **whoami** | `whoami` | Displays internal kernel developer ownership credentials (`Rana Halder`). |
| **sysinfo** | `sysinfo` | Displays compilation targets, execution mode flags, and core system details. |
| **rosfetch** | `rosfetch` | Emits an ASCII logo alongside quick system metrics using the alignment engine. |
| **echo** | `echo [text]` | Parses dynamic user arguments and pushes strings forward to the text matrix. |
| **color** | `color [bg][fg]` | Changes the terminal theme via a two-character hex code updating the VGA Attribute Byte. |
| **reboot** | `reboot` | Writes pulse command `0xFE` to port `0x64` (Intel 8042) to force a hard system reset. |

---

## 🧰 Cross-Compilation & Build Pipeline

The unique architecture allows ROS to be built entirely on an ARM-based Android device using Termux.

### ⚙️ Environment Setup
Initialize your Termux utility suite by installing the bare-metal toolchain components:
```bash
pkg update && pkg upgrade -y
pkg install clang lld nasm qemu-system-x86-64-headless llvm -y
```

### 🔨 Compilation Pipeline
Run the following commands sequentially to clean, assemble, compile, and run the operating system image:

```bash
pkill qemu-system-x86_64
cd ~/storage/shared/MyCustomOS

clang --target=i386-pc-none-elf -c kernel.c -o kernel.o -ffreestanding -fno-pie -mno-red-zone && \
ld.lld -m elf_i386 --omagic --image-base 0 -Ttext 0x1000 kernel_entry.o kernel.o -o kernel.elf && \
llvm-objcopy -O binary kernel.elf kernel.bin && \
cat boot.bin kernel.bin > os-image.bin && \
dd if=/dev/zero bs=512 count=50 >> os-image.bin && \
qemu-system-x86_64 -drive format=raw,file=os-image.bin -display curses


---

## 🗺️ Architectural Roadmap

* [x] **Phase 1 — Core Execution Subsystem:** Legacy MBR bootloader, Real-to-Protected mode entry, flat GDT/IDT mappings, VGA display rendering, and interactive CLI shell interpreter.
* [ ] **Phase 2 — Hardware Clocking:** Native integration of Intel 8253/8254 PIT (Programmable Interval Timer) to service automated scheduler ticks and system delays.
* [ ] **Phase 3 — Persistent Block Storage Layer:** Development of raw ATA/IDE PIO-mode storage drivers enabling disk-sector block operations.
* [ ] **Phase 4 — Virtual Memory Subsystem:** Implementing hardware paging structures, CR3 directory controls, and a deterministic `kmalloc` / `kfree` allocator heap.
* [ ] **Phase 5 — Multitasking Loop:** Engineering Process Control Blocks (PCBs) and a round-robin task scheduler engine for preemptive execution threads.

---

## 📊 Platform Profile Matrix

* **Architecture Target:** x86 32-bit Flat Protected Mode (Ring 0)
* **Kernel Paradigm:** Monolithic Freestanding Core (C / Assembly Cross-layer)
* **Build Host Environment:** AArch64 Android Device via Termux Userspace
* **Workspace Framework:** VGA Text Mode Display Matrix (80×25 Resolution)
* **Toolchain Ecosystem:** Clang/LLVM, LLD Linker, NASM Engine, QEMU Headless Emulator
* **Distribution Terms:** Open-source software licensed under the **GNU General Public License v2.0 (GPL-2.0)**.

---

## ⚖️ License & Author

* **License:** Copyright © 2026 Rana Halder. Distributed under the **GPL-2.0 License**.
* **Author:** **Rana Halder** — *Engineered with Absolute Precision, Resilience, and Zero Abstractions.*
