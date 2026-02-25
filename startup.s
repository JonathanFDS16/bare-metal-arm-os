/* 1. Tell the assembler we are using the Thumb instruction set (Cortex-M standard) */
.syntax unified
.cpu cortex-m3
.thumb

/* 2. Expose the Reset_Handler to the Linker so it can see it */
.global Reset_Handler

/* 3. The Vector Table */
/* This specific section MUST go at 0x08000000 */
.section .isr_vector, "a"
.type g_pfnVectors, %object

g_pfnVectors:
    .word _estack        /* 1. Initial Stack Pointer (Defined in linker.ld) */
    .word Reset_Handler  /* 2. Reset Handler Address (The function below) */

/* 4. The Actual Code */
.section .text
.type Reset_Handler, %function

Reset_Handler:
    /* Usually we copy data from Flash to RAM here, but for now... */
    
    /* Jump to the C function 'main' */
    bl main
    
    /* If main returns (it shouldn't), loop forever */
    b .
