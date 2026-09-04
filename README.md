
# 🚀 ROS — Rana Operating System

### Next-Generation Bare-Metal x86 Monolithic Kernel & Hardware-Abstracted Execution Environment

<p align="center">

![Architecture](https://img.shields.io/badge/Architecture-x86__32%20Protected%20Mode-blue?style=for-the-badge&logo=intel)
![Kernel](https://img.shields.io/badge/Kernel-Freestanding%20Monolithic%20C-orange?style=for-the-badge&logo=c)
![Firmware](https://img.shields.io/badge/Firmware-Ground--Zero%20Legacy%20BIOS-red?style=for-the-badge)
![Toolchain](https://img.shields.io/badge/Toolchain-Clang%20%7C%20LLD%20%7C%20NASM-brightgreen?style=for-the-badge&logo=llvm)
![Host](https://img.shields.io/badge/Host-AArch64%20Android%20(Termux)-darkgreen?style=for-the-badge&logo=android)
![License](https://img.shields.io/badge/License-GPL%20v2.0-lightgrey?style=for-the-badge)

</p>

<p align="center">
  <b>A zero-dependency bare-metal x86 system platform engineered, cross-compiled, and emulated entirely within a resource-constrained mobile terminal environment.</b>
</p>

---

## 📖 Overview

**ROS (Rana Operating System)** is an independent 32-bit monolithic operating system kernel, hardware driver subsystem, interrupt-management layer, and interactive command-line environment developed from bare metal.

ROS is designed around a strict **Zero-Abstraction Doctrine**. Rather than depending on high-level firmware interfaces or hosted operating-system services, the platform directly implements the fundamental mechanisms required to transition an x86 processor from BIOS Real Mode into 32-bit Protected Mode and operate hardware from kernel space.

The system includes:

- 16-bit BIOS-compatible Master Boot Record (MBR) bootstrap
- Real Mode → Protected Mode transition
- Global Descriptor Table (GDT)
- Interrupt Descriptor Table (IDT)
- Dual 8259 Programmable Interrupt Controller (PIC) remapping
- PS/2 keyboard input through IRQ1
- Direct VGA text-mode framebuffer access
- Freestanding C kernel runtime
- Interactive kernel command shell
- Hardware-level reboot mechanism
- Cross-compilation entirely from an ARM-based Android environment
- x86 hardware emulation through QEMU

The complete development lifecycle—including source editing, assembly, compilation, linking, binary generation, and emulation—is performed within an Android-based Termux environment.

---

# 🏛️ System Architecture

ROS follows a layered architecture in which each subsystem is responsible for a clearly defined portion of the execution environment.

```text
+-------------------------------------------------------------------------+
|                    ROS Interactive Shell & Command Dispatcher           |
+-------------------------------------------------------------------------+
|  help  |  clear  |  whoami  |  sysinfo  |  rosfetch  |  echo  | color  |
|                              reboot                                    |
+-------------------------------------------------------------------------+
|                         Terminal Engine                                 |
| Screen Padding | Auto-Scroll | Attribute Pipeline | Cursor Management  |
+-------------------------------------------------------------------------+
|                     Freestanding Runtime Core                           |
|             memset | memcpy | strcmp | strncmp | Utility Primitives     |
+-------------------------------------+-----------------------------------+
|          Hardware Drivers           |      Interrupt Management Core     |
|                                     |                                   |
|  VGA 80x25 Memory (0xB8000)        |  Custom 256-Entry IDT            |
|  PS/2 Keyboard Engine (IRQ1)        |  Remapped 8259 Dual PIC          |
|  Intel 8042 Power Control (0x64)    |  Naked Interrupt Service Routine |
+-------------------------------------+-----------------------------------+
|                    32-Bit x86 Flat Protected Mode                      |
+-------------------------------------------------------------------------+
|             Real-Mode Bootstrap & Kernel Disk Loader                   |
|                         Entry: 0x7C00                                  |
+-------------------------------------------------------------------------+
|                            Raw Hardware                                 |
+-------------------------------------------------------------------------+
🔬 Core Subsystems
1. Stage-1 Bootstrap & Real-to-Protected Mode Engine
The ROS boot sequence begins with a traditional legacy BIOS execution environment.
1.1 BIOS Bootstrap Vector
The BIOS loads the 512-byte Master Boot Record into:
Physical Address: 0x0000:0x7C00
Execution Mode:   16-bit Real Mode
Execution begins at the bootloader entry point located at 0x7C00.
1.2 Kernel Loading
The bootloader uses BIOS disk services through interrupt 0x13 to load the kernel payload from the boot device.
The configured disk operation uses:
BIOS Interrupt : 0x13
Function       : AH = 0x02
Sectors        : AL = 50
Destination    : Segment 0x1000
This provides a contiguous kernel loading region of:
50 × 512 bytes = 25,600 bytes
The kernel is subsequently transferred into the protected execution environment.
1.3 Global Descriptor Table
Before entering Protected Mode, ROS constructs a flat 4 GB memory model through a custom Global Descriptor Table.
Selector
Descriptor
Base
Limit
Access
Flags
0x00
Null Descriptor
0x0
0x0
—
—
0x08
32-bit Code Segment
0x0
0xFFFFF
0x9A
0xCF
0x10
32-bit Data Segment
0x0
0xFFFFF
0x92
0xCF
The resulting memory model provides:
Base      = 0x00000000
Limit     = 0xFFFFFFFF
Mode      = 32-bit Flat Protected Mode
Privilege = Ring 0
1.4 Protected Mode Transition
The bootloader performs the following transition sequence:
Load the GDT using lgdt
Read CR0
Set the Protected Mode Enable (PE) bit
Write the modified value back to CR0
Execute a far jump to the 32-bit code segment
Reload DS, SS, ES, FS, and GS
Initialize the protected-mode stack
Transfer control to the kernel entry point
The kernel stack is initialized at:
0x90000
2. Freestanding Monolithic Kernel
The ROS kernel is written in freestanding C and does not depend on a hosted operating-system runtime.
There is no dependency on:
libc
POSIX
dynamic linking
operating-system system calls
desktop runtime services
The kernel provides its own fundamental runtime primitives, including:
memset
memcpy
strcmp
strncmp
The primary compilation model is:
Target:       i386-pc-none-elf
Environment:  Freestanding
Architecture: 32-bit x86
Code Model:   Non-PIE
Representative compiler configuration:
-ffreestanding
-fno-pie
-mno-red-zone
3. VGA Text-Mode Display Engine
ROS directly controls the VGA text framebuffer at:
0x000B8000
The standard VGA text-mode layout is:
Columns: 80
Rows:    25
Cells:   2000
Bytes:   4000
Each screen cell occupies two bytes:
+----------------------+----------------------+
| Character Byte       | Attribute Byte       |
+----------------------+----------------------+
        8 bits                 8 bits
Character Byte
Contains the ASCII character displayed on screen.
Attribute Byte
Contains the display attributes:
Bits 0–3 : Foreground Color
Bits 4–6 : Background Color
Bit  7   : Blink
The memory address of a cell is calculated as:
Address = 0xB8000 + 2 × (row × 80 + column)
Defensive Terminal Engine
The display subsystem implements:
Cursor boundary tracking
Automatic horizontal positioning
Dynamic string centering
Backspace handling
Screen clearing
Automatic scrolling
Attribute propagation
Viewport reset protection
A viewport reset is triggered when the active display position exceeds the configured screen boundary.
4. Interrupt Management
ROS implements its own interrupt-management subsystem consisting of:
A 256-entry Interrupt Descriptor Table
Custom interrupt gates
Dual 8259 PIC configuration
Hardware IRQ routing
Kernel-level interrupt service routines
4.1 Interrupt Descriptor Table
The IDT contains up to 256 interrupt descriptors and is registered with the CPU using:
lidt
Each hardware interrupt is routed through its corresponding interrupt gate.
4.2 Programmable Interrupt Controller
ROS remaps the legacy dual 8259 PIC architecture to prevent hardware IRQs from overlapping with the CPU exception range.
The configured vector layout is:
Master PIC → 0x20
Slave PIC  → 0x28
The initialization sequence uses the standard PIC initialization command words:
Command
Master
Slave
Purpose
ICW1
0x11
0x11
Initialization
ICW2
0x20
0x28
Interrupt vector offsets
ICW3
0x04
0x02
Cascade configuration
ICW4
0x01
0x01
8086/88 mode
This produces the following hardware IRQ mapping:
IRQ 0 → Interrupt 0x20
IRQ 1 → Interrupt 0x21
IRQ 2 → Interrupt 0x22
...
IRQ 7 → Interrupt 0x27

IRQ 8  → Interrupt 0x28
...
IRQ 15 → Interrupt 0x2F
5. PS/2 Keyboard Driver
Keyboard input is handled through the traditional PS/2 controller interface.
The keyboard generates:
IRQ 1
which is routed to:
Interrupt Vector 0x21
The driver reads raw scan codes from:
Data Port:    0x60
Controller:   0x64
The interrupt service routine:
Saves the processor register state
Reads the keyboard scan code
Determines make/break state
Converts supported scan codes into ASCII
Places characters into the kernel input buffer
Restores the register state
Sends an End-of-Interrupt command to the PIC
The interrupt handler uses a naked interrupt entry point with explicit register preservation.
6. Hardware-Level Reboot
ROS implements a direct hardware reboot mechanism through the legacy Intel 8042 keyboard controller interface.
The controller command interface is accessed through:
I/O Port: 0x64
Reset Command: 0xFE
The kernel sends:
0xFE → 0x64
to request a hardware reset, causing the machine or virtual machine to restart through the firmware boot path.
This mechanism is exposed to the interactive shell through the:
reboot
command.
💻 Interactive Shell
ROS includes a lightweight in-memory command interpreter running directly inside the kernel environment.
The shell receives keyboard input, stores it in an input buffer, parses the command, and dispatches it to the appropriate kernel command handler.
Available Commands
Command
Syntax
Description
help
help
Displays the available shell commands.
clear
clear
Clears the VGA text framebuffer and resets the cursor.
whoami
whoami
Displays the internal kernel author identification string.
sysinfo
sysinfo
Displays kernel and execution-environment information.
rosfetch
rosfetch
Displays the ROS system banner and active kernel metrics.
echo
echo [payload]
Outputs arbitrary user-provided text.
color
color [bg][fg]
Changes the global VGA foreground/background attributes.
reboot
reboot
Requests a hardware-level system reset.
Command Details
help
Scans the registered command table and prints the available commands to the active VGA terminal.
help
clear
Clears the complete VGA text-mode buffer and resets the terminal cursor to:
Row    = 0
Column = 0
The operation covers the complete 80×25 display region.
whoami
Displays the internal kernel author verification string:
Rana Halder
sysinfo
Displays internal kernel information including:
Kernel configuration
Compilation environment
CPU execution mode
32-bit Protected Mode status
Memory-related kernel telemetry
rosfetch
Displays an ASCII ROS system banner together with active kernel metrics.
The output is dynamically aligned using the terminal padding and centering engine.
echo
Syntax:
echo [payload]
The command parser identifies the payload following the command prefix and writes it directly to the terminal.
Example:
echo Hello from ROS
color
Syntax:
color [bg][fg]
The command accepts hexadecimal foreground/background values and packs them into a VGA attribute byte:
attribute = (background << 4) | foreground
Examples:
color 0a
color 0c
Where:
0a → Hacker Green
0c → Danger Red
The configured attribute is propagated across the active VGA framebuffer.
reboot
Syntax:
reboot
The command writes the hardware reset command to the 8042 controller interface:
0xFE → I/O Port 0x64
🧰 Development & Cross-Compilation Infrastructure
One of the defining characteristics of ROS is that the complete development environment runs on an ARM-based Android device.
+-----------------------+
|    Acode Editor       |
|      (Android)        |
+-----------+-----------+
            |
            v
+-----------------------+
|    Termux Environment |
+-----------+-----------+
            |
      +-----+-----+------------------+
      |           |                  |
      v           v                  v
   NASM        Clang/LLVM           LLD
      |           |                  |
      |           v                  |
      |      kernel.o                |
      |                              |
      +-------------+----------------+
                    |
                    v
              kernel.elf
                    |
                    v
             llvm-objcopy
                    |
                    v
               kernel.bin
                    |
                    v
          +-------------------+
          |  boot.bin         |
          |        +          |
          |  kernel.bin       |
          +---------+---------+
                    |
                    v
               os-image.bin
                    |
                    v
              +-----------+
              |   QEMU    |
              | x86_64 VM |
              +-----------+
Toolchain
Tool
Purpose
Acode
Source-code editing on Android
Termux
Linux userspace and development environment
NASM
x86 assembly and bootloader assembly
Clang/LLVM
Freestanding i386 cross-compilation
LLD
ELF linking
LLVM-Objcopy
ELF-to-flat-binary conversion
GNU Coreutils
Image construction and disk-image manipulation
QEMU
x86 hardware emulation
⚙️ Build Environment
ROS is designed to be built from a Termux environment.
Prerequisites
Install the required packages:
pkg update && pkg upgrade -y

pkg install clang lld nasm qemu-system-x86-64-headless llvm -y
🔨 Build Pipeline
The complete build process consists of the following stages.
1. Stop Previous QEMU Instances
pkill qemu-system-x86_64
2. Assemble the Bootloader
The 16-bit bootloader is assembled into a raw binary:
nasm -f bin boot.asm -o boot.bin
Output:
boot.bin
3. Compile the Freestanding Kernel
Compile the kernel for the i386 freestanding target:
clang --target=i386-pc-none-elf \
  -c kernel.c \
  -o kernel.o \
  -ffreestanding \
  -fno-pie \
  -mno-red-zone
Output:
kernel.o
4. Link the Kernel
The kernel objects are linked for the configured physical/text base:
ld.lld \
  -m elf_i386 \
  --omagic \
  --image-base 0 \
  -Ttext 0x1000 \
  kernel_entry.o \
  kernel.o \
  -o kernel.elf
Output:
kernel.elf
5. Generate the Flat Kernel Binary
The ELF headers are removed to generate a raw binary payload:
llvm-objcopy -O binary kernel.elf kernel.bin
Output:
kernel.bin
6. Construct the OS Image
The bootloader and kernel payload are concatenated:
cat boot.bin kernel.bin > os-image.bin
Output:
os-image.bin
7. Pad the Disk Image
The image is padded to the configured disk-sector boundary:
dd if=/dev/zero bs=512 count=50 >> os-image.bin
8. Boot the Operating System in QEMU
Launch the raw disk image:
qemu-system-x86_64 \
  -drive format=raw,file=os-image.bin \
  -display curses
ROS should then boot through the emulated legacy BIOS environment and transition into the kernel.
🗂️ Project Structure
A recommended source-tree organization is:
ROS/
├── boot.asm
├── kernel_entry.asm
├── kernel.c
├── boot.bin
├── kernel.o
├── kernel.elf
├── kernel.bin
├── os-image.bin
└── README.md
Source Files
File
Responsibility
boot.asm
BIOS bootstrap, disk loading, GDT setup, Protected Mode transition
kernel_entry.asm
Kernel entry and low-level execution setup
kernel.c
Freestanding kernel, drivers, terminal, shell and runtime logic
README.md
Project documentation
Generated Artifacts
boot.bin
kernel.o
kernel.elf
kernel.bin
os-image.bin
These files are generated during the build process and do not represent independent source components.
🗺️ Engineering Roadmap
ROS development is organized into multiple architectural phases.
✅ Phase 1 — Stabilized Core
Completed:
Real Mode → Protected Mode transition
Global Descriptor Table
Interrupt Descriptor Table
PIC cascade remapping
PS/2 keyboard driver
VGA text-mode driver
Dynamic VGA attribute engine
Terminal cursor management
Screen centering and padding
Interactive kernel shell
Hardware reset interface
⬜ Phase 2 — Hardware Clocking
Planned integration of the Intel 8253/8254 Programmable Interval Timer (PIT).
Objectives:
Hardware timer interrupts
Periodic kernel ticks
High-resolution timing primitives
Sleep/timer infrastructure
Foundation for preemptive scheduling
⬜ Phase 3 — Physical Disk Management
Implementation of an ATA/IDE PIO-mode storage driver.
Objectives:
Raw block-device access
Disk-sector reads
Disk-sector writes
Kernel-level storage abstraction
Persistent filesystem foundations
⬜ Phase 4 — Virtual Memory & Dynamic Heap
Implementation of x86 hardware paging.
Planned components:
Page Directory
Page Tables
CR3 management
Page mapping
Page allocation
Kernel heap
kmalloc
kfree
⬜ Phase 5 — Process Architecture & Multitasking
Introduction of the first process and task-management infrastructure.
Planned components:
Process Control Blocks (PCB)
CPU context switching
Task creation
Task termination
Round-robin scheduler
Multitasking infrastructure
🧭 Design Philosophy
ROS is built around several core engineering principles.
Zero Abstraction
Hardware interfaces should be understood and controlled at the lowest practical level.
Freestanding Execution
The kernel should not depend on a hosted runtime environment.
Explicit Hardware Control
CPU state, interrupt routing, memory mapping, display output, keyboard input, and reset operations are explicitly managed by the kernel.
Minimal Dependencies
The development and execution environment should require as few external layers as possible.
Mobile-Native Development
The entire operating-system development lifecycle is intentionally executable from an ARM-based Android device using Termux.
📊 Current Platform Profile
Component
Current Implementation
Architecture
x86 32-bit
CPU Mode
Protected Mode
Kernel Architecture
Monolithic
Kernel Language
Freestanding C
Low-Level Language
x86 Assembly
Boot Method
Legacy BIOS / MBR
Memory Model
Flat Protected Memory
Display
VGA Text Mode
Display Resolution
80×25
Keyboard
PS/2
Interrupt Controller
Dual 8259 PIC
Interrupt Table
256-entry IDT
Runtime
Freestanding
Host Development Platform
AArch64 Android
Development Environment
Termux
Emulator
QEMU
Assembler
NASM
Compiler
Clang/LLVM
Linker
LLD
Binary Utility
LLVM-Objcopy
License
GPL-2.0
⚖️ License
Copyright © 2026 Rana Halder.
ROS is free and open-source software licensed under the GNU General Public License v2.0 (GPL-2.0).
You are free to:
Audit the source code
Modify the implementation
Extend the kernel
Redistribute the software
Create derivative works
Redistributed versions must comply with the requirements of the GPL-2.0 license.
👤 Author
Rana Halder
ROS — Rana Operating System
�

🚀 Engineered with Absolute Precision, Resilience, and Zero Abstractions.
�
```
