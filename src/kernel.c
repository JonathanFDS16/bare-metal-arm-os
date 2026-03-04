#include "malloc.h"
#include "utils.h"

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


void interrupt_init() {
	// Enable NVIC
	NVIC_ISER1 |= (1 << 5);
}

void sys_tick_init() {
	SYSTICK_LOAD = 7999;
	SYSTICK_VAL = 0;

	SYSTICK_CTRL |= (1 << 0); //Enable Clock
	SYSTICK_CTRL |= (1 << 1); //Enable TICK Interrupt
	SYSTICK_CTRL |= (1 << 2); //Select Processor Clock
}


void Usart_IRQHandler() {
	if (USART1_SR & (1 << 5)) {
		char r = USART1_DR;
		shell_send(r);
	}
}

void SVC_Handler(void) {
	usart_print("Syscall Called");
}

int tick_counter = 0;
void SysTick_Handler() {
	tick_counter++;
}

void delay_ms(int ms) {
	int start = tick_counter;
	while (tick_counter - start < ms) {}
	usart_print("waited for 1000ms\n");
}



int _start(void *heap_start) {
	init_malloc(heap_start, 4 * 25);

	int allocnumber = 0;
	int *ptr = mmalloc(sizeof(int));
	int *init_ptr = ptr;
	int *last = ptr;

	while (allocnumber < 5) { 
		*ptr = allocnumber++;
		last = ptr;
		usart_print("Malloc returned allocated ptr: ");
		print_ptr(ptr);
		ptr = mmalloc(sizeof(int)); 
	}

	free(ptr);
	

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

	usart_print("> ");
	interrupt_init();
	sys_tick_init();

    while (1) {
		//delay_ms(1000);
		//__asm__ volatile ("svc #1");
    }

    return 0;
}
