CONTEXT FOR AI:

I am a Senior CS Student interested in low-level systems programming, bare-metal development, and kernel architecture.
I have just completed a "from scratch" Board Bring-up project on an emulated ARM Cortex-M3 (STM32).

CURRENT PROJECT STATUS:

I have built a minimal Bare Metal OS/Kernel running on QEMU (stm32vldiscovery / stm32-p103)
using arm-none-eabi-gcc and GDB, with zero standard libraries (freestanding).

TECHNICAL ACHIEVEMENTS:


1. System Boot: Wrote the Linker Script (.ld) and Assembly Bootloader (startup.s) manually.
	- Solved: Fixed a Hard Fault caused by a memory map mismatch (Linker defined 20KB RAM, device had 8KB) by inspecting the QEMU info mtree.


2. Memory Mapped I/O: Implemented drivers by directly manipulating hardware registers via volatile pointers.
	- RCC: Configured clock gating for peripherals.

	- GPIO: Configured Pin Muxing (Alternate Function Push-Pull) for UART TX/RX.

	- UART: Wrote a driver from scratch (Calculated Baud Rate BRR for 8MHz clock).


3. Kernel Shell:
	- Implemented uart_putc, uart_getc, and string handling (no <string.h>).

	- Created a Command Line Interface (CLI) that parses commands (e.g., HELP).

	- Implemented a System Reset command (REBOOT) using the SCB_AIRCR register.


4. Interrupts & Concurrency:
	- Moved from Polling to Interrupt-Driven I/O by configuring the NVIC and modifying the Vector Table.

	- Implemented the SysTick timer to generate 1ms "Heartbeat" interrupts.


WHAT I KNOW:


- Cross-compilation & GDB remote debugging.

- ARM Thumb-2 instruction set basics.

- How to read Datasheets vs. Reference Manuals vs. Programming Manuals.

- Bitwise manipulation for register configuration.

GOALS / NEXT STEPS:

I am looking to advance into:

1. Context Switching: Implementing a scheduler (saving/restoring R0-R12/SP).

3. Advanced Systems: Memory Allocators (malloc from scratch) or a Userspace TCP/IP stack.
