/* 1. Define the Entry Point */
/* This tells the debugger where the code technically starts.
   We will write a function called Reset_Handler in assembly later. */
ENTRY(Reset_Handler)
/* 2. Define Memory Architecture */
MEMORY
{
    /* Name   Permissions    Start Address    Size */
    /* Read-Execute (rx) for code */
    FLASH (rx) : ORIGIN = 0x08000000 , LENGTH = 128K 

    /* Read-Write-Execute (rwx) for data/stack */
    RAM   (rwx) : ORIGIN = 0x20000000 , LENGTH = 8K
}
/* 3. Map Sections to Memory */
SECTIONS
{
    /* The ".text" section contains your actual code (C and Assembly instructions) */
    .text :
    {
        /* Keep the Vector Table at the very beginning of Flash (Essential!) */
        *(.isr_vector) 
        /* Then put all other code */
        *(.text)
        /* Read-only data (constants) */
        *(.rodata)
	} > FLASH  /* <-- Tell Linker to shove this into the FLASH memory block */
    /* The ".data" section contains initialized variables (e.g., int x = 5;) */
    .data : { *(.data) } > RAM    /* <-- Variables live in RAM... BUT wait. */
    /* NOTE: In a real system, .data is stored in FLASH and copied to RAM at boot. 
       For this minimal emulator test, we can cheat and just tell QEMU to put it 
       in RAM. But standard practice is > RAM AT > FLASH */
    /* The ".bss" section contains uninitialized variables (e.g., int y;) */
    .bss :{ *(.bss) } > RAM
    /* 4. Define the Stack Pointer */
    /* The stack grows downwards from the END of RAM. */
    /* So we need a symbol telling us where RAM ends. */
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
