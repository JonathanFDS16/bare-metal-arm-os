#include "malloc.h"
#include "utils.h"
#include "scheduler.h"
#include "shell.h"

// 1. RCC (Power)
#define RCC_BASE_ADDR       0x40021000
#define RCC_APB2ENR         (*((volatile uint32_t *)(RCC_BASE_ADDR + 0x18)))

// 2. GPIO (Wiring)
#define GPIOA_BASE_ADDR     0x40010800
#define GPIOA_CRH           (*((volatile uint32_t *)(GPIOA_BASE_ADDR + 0x04)))

// NVIC
#define NVIC_BASE 			0xE000E100
#define NVIC_ISER1          (*((volatile uint32_t *)(NVIC_BASE + 0x004))) // Control

// SYSTEM TICK
#define SYSTICK_BASE 		0xE000E010
#define SYSTICK_CTRL        (*((volatile uint32_t *)(SYSTICK_BASE))) // Control
#define SYSTICK_LOAD        (*((volatile uint32_t *)(SYSTICK_BASE + 0x04))) // Control
#define SYSTICK_VAL         (*((volatile uint32_t *)(SYSTICK_BASE + 0x08))) // Control

#define SCB_BASE 			0xE000ED00
#define SCB_ICSR			(*((volatile uint32_t *)(SCB_BASE + 0x04)))
#define SCB_AIRCR			(*((volatile uint32_t *)(SCB_BASE + 0x0C)))

#define USART_BUFFER_SIZE 128
#define ALLOC_HEAP_SIZE_BYTES 4096 //4096 bytes

void delay_ms(int ms);
void context_switch();
void reset();

enum STACK_TYPE {
	MSP,
	PSP
};

void interrupt_init() {
	// Enable NVIC
	NVIC_ISER1 |= (1 << 5);
}

void sys_tick_init() {
	SYSTICK_LOAD = 799999;
	SYSTICK_VAL = 0;

	SYSTICK_CTRL |= (1 << 0); //Enable Clock
	SYSTICK_CTRL |= (1 << 1); //Enable TICK Interrupt
	SYSTICK_CTRL |= (1 << 2); //Select Processor Clock
}

char usart_buffer[USART_BUFFER_SIZE];
int usart_index = 0;
void Usart_IRQHandler() {
	if (USART1_SR & (1 << 5)) {
		char r = USART1_DR;
		usart_buffer[usart_index++] = r;
	}
}

char poll_usart() {
	if (usart_index) {
		char c = usart_buffer[--usart_index];
		return c;
	}
	return 0;
}

enum STACK_TYPE get_stack_type(int mask) {
	switch (mask) {
		case 0x1: return MSP;
		case 0x9: return MSP;
		case 0xd: return PSP;
	}
	return -1;
}

void SVC_Handler(void) {
	/*
	 * Incredible discovery :) The way to collect the service code is by literally
	 * reading the encoded machine code and mask out the immediate from there
	 *
	 * 1. Get what stack the PC will be (MSP or PSP)
	 * 2. Then get the read the address of SP+0x18 (PC)
	 * 3. Addr -2 (SVC is 2 butes long) = where the SVC Imm encoded is located
	 *
	 * Ex.: PSP PC is 80004ca, then -2 = 80004c8. If I collect only 2 bytes from this address
	 * I will have df01 which means SVC 01
	 *  80004c8:	df01      	svc	1
	 * 	80004ca:	bf00      	nop
	 * */
	size_t lr_register;
	__asm__ volatile (
			"MOV %[res], LR\n\t"
			: [res] "=r" (lr_register)
			);
	int mask = lr_register & 0x0000000F; //this keeps the last of EXC_RETURN
	enum STACK_TYPE type = get_stack_type(mask);

	void *sp;
	if (type == MSP) {
		__asm__ volatile (
				"MRS %[res], MSP\n\t"
				: [res] "=r" (sp)
				);
	}
	else if (type == PSP) {
		__asm__ volatile (
				"MRS %[res], PSP\n\t"
				: [res] "=r" (sp)
				);
	}
	
	size_t *pc_ptr = (sp + 0x18);
	size_t *enc_addr = (size_t*)(*pc_ptr - 0x2);
	int svc_imm = *enc_addr & 0x000000FF;

	switch(svc_imm) {
		case 1:
			usart_print("Called service 1\n");
			reset();
			break;
		default:
			usart_print("svc not implemented\n");
			print_int(svc_imm);
	}
}

int tick_counter = 0;
void SysTick_Handler() {
	tick_counter++;
	context_switch();
}

void context_switch() {
	SCB_ICSR |= (1 << 28);
}

void task1() {
	usart_print("task1 called\n");
	while (1) {
		//usart_print("task1\n");
	}
}

void task2() {
	usart_print("task2 called\n");
	while (1) {
		//usart_print("task2\n");
	}
}

void reset() {
	usart_print("reset requested\n");
	usart_print("value read of AIRCR: ");
	print_ptr((void*)SCB_AIRCR);

	size_t value = SCB_AIRCR;
    
    value &= 0x0000FFFF;         // Clear the read-back key
    value |= (0x5FA << 16);      // Write magic key 0x05FA into upper 16 bits
    value |= (1 << 2);           // Set SYSRESETREQ (Bit 2)
    
    // Ensure all memory operations finish before triggering reset
    __asm__ volatile("dsb" : : : "memory");

    SCB_AIRCR = value;

    // Flush the instruction pipeline
    __asm__ volatile("isb");

    // Trap the CPU while the hardware reset asserts
    while (1) {
    }
}

int _start(void *heap_start) {
    // 1. Enable Clocks (RCC)
    // We need Bit 14 (USART1) and Bit 2 (GPIOA)
    RCC_APB2ENR |= (1 << 14) | (1 << 2);

    // 2. Configure Pin PA9 (GPIO)
    // PA9 lives in CRH bits [7:4].
    // RESET STATE: The bits are usually 0100 (Input Floating).
    // TARGET: We want 1011 (Alternate Function Push-Pull 50MHz).
    // A. CLEAR the bits first (Safety Step)
    // We create a mask 1111 (0xF) shifted to position 4, then invert it (~).
    GPIOA_CRH &= ~(0xF << 4); 
    // B. SET the new bits
    // We want 1011 (0xB) shifted to position 4.
    GPIOA_CRH |= (0xB << 4);

    // 3. Configure Baud Rate (USART)
    // Target: 9600 Baud @ 8MHz Clock.
    // Calculation: 8000000 / 9600 = 833 (Decimal) = 0x341 (Hex)
    USART1_BRR = 0x341;

    // 4. Enable UART (USART)
    // Bit 13: UE (USART Enable)
    // Bit 5: RXNEIE (Rx not empty interrup enable)
    // Bit 3:  TE (Transmitter Enable)
    // Bit 2:  RE (Receiver Enable)
    USART1_CR1 |= (1 << 13) | (1 << 5) | (1 << 3) | (1 << 2);

	init_malloc(heap_start, ALLOC_HEAP_SIZE_BYTES / sizeof(size_t));
	create_thread(*run_shell);
	interrupt_init();
	sys_tick_init();
	context_switch();

    return 0;
}
