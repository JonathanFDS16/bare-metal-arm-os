/* 1. Tell the assembler we are using the Thumb instruction set (Cortex-M standard) */
.syntax unified
.cpu cortex-m3
.thumb

/* 2. Expose the Reset_Handler to the Linker so it can see it */
.global Reset_Handler
.global _startheap

/* 3. The Vector Table */
/* This specific section MUST go at 0x08000000 */
.section .isr_vector, "a"
.type g_pfnVectors, %object

g_pfnVectors:
    .word _estack        /* 1. Initial Stack Pointer (Defined in linker.ld) */
    .word Reset_Handler  /* 2. Reset Handler Address (The function below) */
	.word 0 // NMI
	.word 0 // HardFault
	.word 0 // MemManage
	.word 0 // BusFault
	.word 0 // UsageFault
	.word 0 // Reserved
	.word 0 // Reserved
	.word 0 // Reserved
	.word 0 // Reserved
	.word SVC_Handler
	.word 0 // DebugMon
	.word 0 // Reserved
	.word PendSV_Handler
	.word SysTick_Handler

	.equ USART_IRQ_NUM, 37
	.rept USART_IRQ_NUM
	.word 0
	.endr

	.word Usart_IRQHandler // Interrupt Request Handler from USART
