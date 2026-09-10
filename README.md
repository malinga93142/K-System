# Known-System (KS)

Known-System (KS) is a small, educational, and experimental hobby operating system kernel designed by Malinga RK. KS is built from scratch to explore low-level operating system concepts and understand how kernels work internally.

The goal of KS is not to be a production operating system, but a simple platform for experimenting with CPU architecture, memory management, multitasking, and user-space execution.

## Features

Current implemented features:

* x86 protected mode kernel
* Multiboot boot support
* Global Descriptor Table (GDT)
* Interrupt Descriptor Table (IDT)
* Exception handling
* IRQ handling
* Programmable Interrupt Controller (PIC) support
* Local APIC support
* Symmetric Multiprocessing (SMP) bring-up
* Application Processor (AP) startup trampoline
* Spinlocks for synchronization
* Physical memory manager (PMM)
* Paging and virtual memory management
* Kernel/user-space separation foundation
* User-mode transition experiments
* Basic syscall framework
* UART and VGA output
* Keyboard and timer drivers

## Project Structure

```
.
├── boot        # Boot and Multiboot initialization
├── cpu         # CPU features, interrupts, SMP, APIC, GDT/IDT
├── drivers     # Hardware drivers
├── interrupt   # IRQ and syscall handling
├── io          # Low-level I/O helpers
├── kernel      # Core kernel code
├── mm          # Memory management
└── user        # User-mode experiments and test programs
```

## Current Status

KS is currently in the early kernel development stage.

Implemented:

* Booting a custom kernel
* Basic hardware initialization
* Memory management foundations
* SMP initialization
* Transitioning toward user-space execution

Work in progress:

* Dynamic memory allocation
* Task/process management
* Scheduler
* Executable loading
* Improved system calls
* Proper device I/O abstraction

## Goals

Future goals include:

* A simple task scheduler
* User process management
* `exec`-like program loading
* Basic filesystem support
* More complete user-space environment
* Improved hardware abstraction

## Building

KS currently targets a minimal x86 environment and is intended to be built and tested using an emulator such as QEMU.

Build instructions will be added as the project evolves.

## License

This project is an educational and experimental hobby OS kernel.
