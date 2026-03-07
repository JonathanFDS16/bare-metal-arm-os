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
	asm("MRS R0, PSP"); // Save PSP in R0. What if PSP is not set yet
	asm("STMDB R0!, {R4-R11}"); // Push to PSP R4-R11
	// asm("MOV R0, R13"); This is wrong I am saving the MSP instead of the PSP
	asm("PUSH {LR}"); // Save LR in MSP
	asm("BL schedule_next"); //will load a new address in R0
	asm("POP {LR}"); // Get LR from MSP again
	asm("LDMIA R0!, {R4-R11}"); // Pop from PSP R4-R11
	//asm("MOV R14, R0"); this is wrong I am replace MSP with PSP I suppose
	asm("MSR PSP, R0"); // Write the new PSP in R0 to the current PSP
	asm("BX LR");// What does this do?
}

int count = 0;
void* schedule_next(void* old_sp) {
	count++;
	int index = count % 2;
	usart_print("Scheduling this SP: ");
	print_ptr(task[index].sp);
	return task[index].sp;
}

unsigned int stack_task_1[512];
unsigned int stack_task_2[512];
void schedule_task(int id, void* stack_addr, void (*task_func)()) {
	unsigned int *s_ptr = (unsigned int*)stack_addr + 512;
	s_ptr--;
	*(s_ptr--) = 0x0100000e; //xPSR
	*(s_ptr--) = (unsigned int)task_func; //PC
	*(s_ptr--) = 0xFFFFFFFD; //LR
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
	usart_print("Start Stack pointer for task: ");
	print_int(id);
	usart_print(" ");
	print_ptr(s_ptr);
	usart_print("Stack - 1 (size 4 bytes): ");
	print_int(id);
	usart_print(" ");
	print_ptr(s_ptr - 0x1);

	usart_print("TaskFunct address: ");
	print_ptr(task_func);
	task[id] = (Task){.sp=s_ptr};
}

void task1() {
	usart_print("task1 called\n");
	while (1) {
		delay_ms(1000);
		usart_print("task1\n");
	}
}

void task2() {
	while (1) {
		delay_ms(1000);
		usart_print("task2\n");
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
		//__asm__ volatile ("svc #1");
    }

    return 0;
}
