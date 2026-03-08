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

#define SCB_BASE 			0xE000ED00
#define SCB_ICSR			(*((volatile uint32_t *)(SCB_BASE + 0x04)))


void delay_ms(int ms);
void context_switch();

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

typedef struct Task {
	unsigned int *sp;
} Task;

Task task[2] = {0};

__attribute__((naked)) void PendSV_Handler(void) {
	/* PendSV
	 *• R0-R3, R12
	 • Return address
	 • PSR
	 • LR.
	 */
	__asm volatile (
        "MRS R0, PSP \n"             // Read PSP
        "CMP R0, #0 \n"              // Is it 0? (First run?)
        "BEQ restore_context \n"     // If 0, SKIP saving!

        // --- SAVE CONTEXT (Only if PSP != 0) ---
        "STMDB R0!, {R4-R11} \n"     // Save R4-R11
		"B restore_context\n"

        "restore_context: \n"
        "PUSH {LR} \n"               // We still need to call schedule_next for the first task!
        "BL schedule_next \n"        // This R0 has PSP which will pass to Schedule Next
        "POP {LR} \n"

		"ORR LR, LR, #4\n" // Ensure LR has returns to Thread PSP

        // --- FINISH ---
        "finish_switch: \n"
        "LDMIA R0!, {R4-R11} \n"     // Restore R4-R11
        "MSR PSP, R0 \n"             // Update PSP to the new stack
        "BX LR \n"                   // Return
    );
}

int count = 0;
void* schedule_next(void* old_sp) {
	count++;
	int index = count % 2;
	if (index == 1 && old_sp) {
		task[0].sp = old_sp;
	}
	else if (old_sp) {
		task[1].sp = old_sp;
	}
	return task[index].sp;
}

void task_return_trap(void) {
    usart_print("CRITICAL: A Task returned! Freezing.");
    while(1); // Trap the CPU here
}

unsigned int stack_task_1[512];
unsigned int stack_task_2[512];
void schedule_task(int id, void* stack_addr, void (*task_func)()) {
	unsigned int *s_ptr = (unsigned int*)stack_addr + 512;
	s_ptr--;
	*(s_ptr--) = 0x01000000; //xPSR
	*(s_ptr--) = (unsigned int)task_func; //PC
	*(s_ptr--) = (unsigned int)task_return_trap; //LR stores the return address 
	*(s_ptr--) = 0; //r12
	*(s_ptr--) = 0; //r3
	*(s_ptr--) = 0; //r2
	*(s_ptr--) = 0; //r1
	*(s_ptr--) = 0; //r0
	*(s_ptr--) = 0; //R11
	*(s_ptr--) = 0; //R10
	*(s_ptr--) = 0; //R9
	*(s_ptr--) = 0; //R8
	*(s_ptr--) = 0; //R7
	*(s_ptr--) = 0; //R6
	*(s_ptr--) = 0; //R5
	*(s_ptr) = 0; //R4 // TOP 0x200009cc
	task[id] = (Task){.sp=s_ptr};
}

void task1() {
	usart_print("task1 called\n");
	while (1) {
		//delay_ms(1000);
		usart_print("task1\n");
		context_switch();
	}
}

void task2() {
	usart_print("task2 called\n");
	while (1) {
		//delay_ms(1000);
		usart_print("task2\n");
		context_switch();
	}
}

int tick_counter = 0;
void SysTick_Handler() {
	tick_counter++;
}

void context_switch() {
	SCB_ICSR |= (1 << 28);
}

void delay_ms(int ms) {
	int start = tick_counter;
	while (tick_counter - start < ms) {}
	usart_print("waited for 1000ms\n");
}


int _start(void *heap_start) {
	init_malloc(heap_start, 4 * 25);

	schedule_task(0, stack_task_1, *task1);
	schedule_task(1, stack_task_2, *task2);
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
		delay_ms(1000);
		usart_print("Switching Context\n");
		context_switch();
    }

    return 0;
}
