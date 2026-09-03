# 🚀 ROS (Rana Operating System) v3.0 — Final Power Edition

An independent, bare-metal, 32-bit monolithic operating system kernel and dynamic interactive shell built completely from scratch.

---

## ⚡ The Ultimate Engineering Twist
What makes **ROS** exceptionally unique is its constrained development infrastructure: **this entire operating system was designed, coded, compiled, linked, and emulated exclusively on an Android smartphone.** 

By leveraging a native mobile Linux subsystem, every line of low-level code was written on a virtual interface, proving that absolute computing engineering requires nothing but sheer architectural logic and determination.

---

## 🏗️ System Architecture & Low-Level Mechanics

ROS operates as a pure **Freestanding System**, meaning it completely bypasses standard C libraries (`<stdio.h>`, `<string.h>`) and external operating system APIs. It interfaces directly with raw x86 hardware through explicit memory maps and I/O registers.

### 1. The Bootloader Layer (x86 Assembly)
* **Real Mode Initialization:** The bootloader wakes up the processor in **16-bit Real Mode**.
* **Disk I/O Management:** It directly instructs the BIOS to read the raw kernel sectors (allocating up to 50 sectors) sequentially off the storage medium into physical memory.
* **Protected Mode Transition:** It builds the temporary Global Descriptor Table (GDT), enables the Protected Mode bit in the `cr0` control register, and fires a `Far Jump` to cleanly switch the CPU state into **32-bit Protected Mode**, passing code execution over to the C kernel.

### 2. The Bare-Metal Kernel (Freestanding C)
* **VGA Hardware Manipulation:** Text rendering, color changes, and window alignments (such as centered ASCII layouts) are achieved by writing directly to the **VGA Text Mode Memory Buffer (`0x000B8000`)** byte by byte.
* **Custom Core Library:** Features its own isolated utility layer to handle freestanding memory copy (`memcpy`), setting blocks (`memset`), and string manipulation routines without hosting overhead.

### 3. Hardware Interrupts & I/O Ports
* **IDT Implementation:** Houses a fully functional custom **Interrupt Descriptor Table (IDT)** to register low-level CPU trap handlers and ISRs.
* **PIC Remapping:** Remaps the dual Intel 8259 Programmable Interrupt Controllers (PIC) via `outb` instructions to separate hardware interrupts from CPU exceptions.
* **Keyboard Driver:** Catches real-time hardware interrupts (**IRQ1**) triggered by the PS/2 controller via I/O ports (`inb(0x60)`), decoding raw scan codes directly into characters for the interactive CLI shell.

---

## 💻 Command Line Interface (CLI) Reference

ROS hosts an optimized, interactive command-line environment parsing dynamic string arguments directly through the custom kernel:

| Command | Usage | Functional Description |
| :--- | :--- | :--- |
| **`help`** | `help` | Polls the shell context and lists all compiled, executable system commands natively available within the ROS build. |
| **`clear`** | `clear` | Clears the entire VGA video buffer memory, shifts layout pointer structures, and resets the visual cursor index to `(0,0)` top-left. |
| **`whoami`** | `whoami` | Emits a hardcoded data sequence identifying the core creator and system architect of this OS (**Rana Halder**). |
| **`sysinfo`** | `sysinfo` | Queries kernel constants to display the OS name, version tag, runtime architecture, and low-level development credits. |
| **`rosfetch`** | `rosfetch` | Inspired by Unix utilities like `neofetch`, this streams an advanced, centered ASCII representation of the ROS logo coupled with hardware runtime data. |
| **`echo`** | `echo [text]` | Evaluates dynamic input pointer arrays, parses multi-token user arguments, and streams the variable string buffer back onto the live terminal terminal display. |
| **`color`** | `color [bg][fg]` | Overwrites the VGA Attribute Byte across video memory. Accepts a 2-digit hex sequence altering terminal themes instantly (e.g., `color 0a` for Matrix Green, `color 0c` for Danger Red). |
| **`reboot`** | `reboot` | Generates a critical low-level system cycle. Sends the trigger byte `0xFE` directly to the keyboard controller port `0x64`, causing the physical motherboard to execute an instant hard reset. |

---

## 🛠️ The Mobile Infrastructure Toolchain
The entire cross-compilation matrix was configured on a standalone Android mobile unit using this custom open-source stack:
* **Termux Environment:** Hosted the localized Linux platform environment, compiling binary routines via `clang/gcc` and assembling `nasm` targets.
* **Acode/Spck Editors:** Used for low-level structural code tracking, formatting C pointers, and managing remote code commits.
* **QEMU Hardware Emulator:** Acting as the primary headless/display virtualization layer (`qemu-system-x86_64`) to test raw `os-image.bin` blocks natively on a mobile surface.

---

## 🗺️ Extended Roadmap & System Evolution
* **Phase 1:** Complete open-source structuring of the monolithic command-line kernel core (`ROS v3.0 Final Power Edition`). *(Current Landmark)*
* **Phase 2:** Engineering custom fsys blocks (FAT allocation logic) to enable native raw storage file structures.
* **Phase 3:** Porting the headless ROS binary footprint onto single-board computer layouts (e.g., Raspberry Pi Architecture) to operate as a secure 24/7 web hosting and server environment.
* **Phase 4:** Linking ROS as the fundamental bare-metal substrate driving the **Californium Ecosystem**—orchestrating P2P distributed computing grids and sovereign server arrays.

---
**Chief Architect & Developer:** Rana Halder (Systems Engineer & Student)  
**Source Code Distribution License:** GNU General Public License v2.0 (Free, Sovereign, & Open-Source Forever)
